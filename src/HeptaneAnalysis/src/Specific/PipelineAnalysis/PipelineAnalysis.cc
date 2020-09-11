/* ---------------------------------------------------------------------

Copyright IRISA, 2003-2017

This file is part of Heptane, a tool for Worst-Case Execution Time (WCET)
estimation.
APP deposit IDDN.FR.001.510039.000.S.P.2003.000.10600

Heptane is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Heptane is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (COPYING.txt).

See CREDITS.txt for credits of authorship

------------------------------------------------------------------------ */

#include "PipelineAnalysis.h"
#include "Generic/CallGraph.h"
#include "arch.h"

/**
 * Return the fetch latency of an instruction according to its context and 
 * and its occurence.
 * 
 * \param inst instruction to evaluate
 * \param context context of the instruction
 * \param first first occurence of the access
 * 
 */
unsigned int PipelineAnalysis::getFetchLatency(Instruction & inst, Context * context, bool first)
{
  unsigned int latency = config->getICacheLatency(1);	// By default, hit in the L1 cache
  int l = config->getMemoryLoadLatency();

  //get instruction latency (cache access)
  for (int i = 1; i <= nbCacheLevel; i++)
    {
      string cacheAccessClassif = AnalysisHelper::mkContextAttrName(CodeCHMC[i], context);
      string classif = ((cfglib::SerialisableStringAttribute &) inst.GetAttribute(cacheAccessClassif)).GetValue();
      if (first)
	{
	  if (classif == string("FM") || classif == string("AM") || classif == string("NC"))	//miss
	    latency += (i == nbCacheLevel) ? l: config->getICacheLatency(i + 1);
	  else			//hit
	    break;
	}
      else
	{ //next
	  if ( // classif == string("FH") ||     FH does not exist now
	      classif == string("AM") || classif == string("NC"))	//miss
	    latency += (i == nbCacheLevel) ? l: config->getICacheLatency(i + 1);
	  else //hit
	    break;
	}
    }

  return latency;
}

unsigned int PipelineAnalysis::computeBB(Node & BB, Context * context, bool first)
{
  unsigned int Time = 0;
  vector < InstructionPipeline * >IP;

  vector < Instruction * >insts = BB.GetInstructions();
  vector < Instruction * >::iterator it = insts.begin();

  if (insts.size() == 0) return 0;
  while (!(*it)->IsCode()) it++;

  //schedule first instruction
  scheduleFirstInst(*(*it), IP, context, first);

  //schedule next instructions
  it++;
  while (it < insts.end())
    {
      if ((*it)->IsCode())
	scheduleNextInst(*(*it), IP, context, first);
      it++;
    }

  Time = IP[IP.size() - 1]->getPipeStage(PIPELINEDEPTH - 1)->tick;

  // delete allocated InstructionPipeline
  for (unsigned int i = 0; i < IP.size(); i++)
    {
      delete IP[i];
    }

  return Time;
}

int PipelineAnalysis::Delta(Node * pred, Node * dest, Context * predContext, Context * destContext, bool predOccur, bool destOccur)
{

  int delta;
  unsigned int Time = 0;
  vector < InstructionPipeline * >IP;

  vector < Instruction * >insts = pred->GetInstructions();
  vector < Instruction * >::iterator it = insts.begin();

  if (insts.size() == 0)
    return 0;

  while (!(*it)->IsCode())
    it++;
  //schedule first instruction
  scheduleFirstInst(*(*it), IP, predContext, predOccur);

  //schedule next instructions from the source BB
  it++;
  while (it < insts.end())
    {
      if ((*it)->IsCode())
	scheduleNextInst(*(*it), IP, predContext, predOccur);
      it++;
    }
  //schedule instruction from the destination BB
  insts = dest->GetInstructions();
  it = insts.begin();

  while (it < insts.end())
    {
      if ((*it)->IsCode())
	scheduleNextInst(*(*it), IP, destContext, destOccur);
      it++;
    }
  Time = IP[IP.size() - 1]->getPipeStage(PIPELINEDEPTH - 1)->tick;

  //delete allocated InstructionPipeline
  for (unsigned int i = 0; i < IP.size(); i++)
    delete IP[i];

  // compute the delta
  string predAttrName;
  if (predOccur)
    predAttrName = AnalysisHelper::mkContextAttrName( NodeExecTimeFirstAttributeName, predContext);
  else
    predAttrName = AnalysisHelper::mkContextAttrName(NodeExecTimeNextAttributeName, predContext);
				     
  string destAttrName;
  if (destOccur)
    destAttrName = AnalysisHelper::mkContextAttrName(NodeExecTimeFirstAttributeName, destContext);
  else
    destAttrName = AnalysisHelper::mkContextAttrName(NodeExecTimeNextAttributeName, destContext);
				     
  SerialisableIntegerAttribute & predTime = (SerialisableIntegerAttribute &) pred->GetAttribute(predAttrName);
  SerialisableIntegerAttribute & destTime = (SerialisableIntegerAttribute &) dest->GetAttribute(destAttrName);
  delta = Time - (predTime.GetValue() + destTime.GetValue());

  return delta;
}

int PipelineAnalysis::computeDelta(Edge * edge, Cfg & cfg, Context * context, bool sourceOccur, bool destOccur)
{
  Node *source = cfg.GetSourceNode(edge);
  Node *dest = cfg.GetTargetNode(edge);

  return Delta(source, dest, context, context, sourceOccur, destOccur);
}

int PipelineAnalysis::computeCallDelta(Node * pred, Node * dest, Context * context, bool Occur)
{
  return Delta(pred, dest, context, context->getCalleeContext(pred), Occur, Occur);
}

int PipelineAnalysis::computeReturnDelta(vector < Node * >endNodes, Node * returnNode, Context * context, bool Occur)
{
  int delta, deltaTmp;
  Node *previous_call;

  // Modif LBesnard.
  bool b=AnalysisHelper::getCallerNode(returnNode, &previous_call);
  assert(b);

  vector < Node * >::iterator it = endNodes.begin();
  Context *context2 = context->getCalleeContext(previous_call);
  delta = Delta(*it, returnNode, context2, context, Occur, Occur);
  it++;

  while (it != endNodes.end())
    {
      deltaTmp = Delta(*it, returnNode, context2, context, Occur, Occur);
      if (delta < deltaTmp)
	delta = deltaTmp;
      it++;
    }

  return delta;
}

// Public -------------------------------------------------------

 PipelineAnalysis::PipelineAnalysis(Program * p, int nbcache):Analysis(p)
{
  PIPELINEDEPTH = 4;
  nbCacheLevel = nbcache;

  // Fill-in attribute names for the different cache levels
  for (int l = 1; l <= nbCacheLevel; l++)
    {
      CodeCHMC[l] = CHMCAttributeNameCode(l);
    }
}

PipelineAnalysis::~PipelineAnalysis()
{
}

// -----------------------------------------------------
// Used to check the validity of cache
// attribute on an instruction
//
// Checks that attribute of name attr_name is attached
// to the instruction and contains valid values only
// NB: the attribute name is a parameter because there is
//     an attribute name per context (base#context)
// -----------------------------------------------------
bool PipelineAnalysis::CheckCacheAttributes(Instruction * i, string attr_name)
{
  // Check the attribute is attached
  if (! i->HasAttribute(attr_name))
    {
      stringstream errorstr;
      errorstr << "PipelineAnalysis: Instruction (" << i->GetCode() << ") should have a classification (AH/AM/NC/FM/AU)";
      Logger::addFatal(errorstr.str());
      return false;
    }
  // Validity check of attribute contents
  SerialisableStringAttribute att = (SerialisableStringAttribute &) i->GetAttribute(attr_name);
  string s = att.GetValue();
  if (s != "AH" && s != "AM" && s != "NC" && s != "FM" && s != "AU")
    {
      stringstream errorstr;
      errorstr << "PipelineAnalysis: Invalid classification for instr (" << i->GetCode() << "): " << s << " Valid ones are (AH/AM/NC/FM/AU)";
      Logger::addFatal(errorstr.str());
      return false;
    }
  return true;
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool PipelineAnalysis::CheckInputAttributes()
{
  Cfg *CurrentCfg;
  Node * CurrentNode;
  Instruction * CurrentInstr;

  CallGraph callgraph (p);
  //check cache classification
  vector < Cfg * >Cfgs = p->GetAllCfgs();
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      // Ignore dead cfgs
      if (callgraph.isDeadCode (CurrentCfg)) {continue;}

      const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
      vector < Node * >vn = CurrentCfg->GetAllNodes();
      for (unsigned int n = 0; n < vn.size(); n++)
	{
	  CurrentNode = vn[n];
	  if (CurrentNode->IsBB())
	    { 
	      if ( ! CurrentNode->isIsolatedNopNode() ) // filtering nop ARM
		{
		  vector < Instruction * >vi = CurrentNode->GetInstructions();
		  for (unsigned int inst = 0; inst < vi.size(); inst++)
		    {
		      CurrentInstr = vi[inst];
		      if (CurrentInstr->IsCode())
			{
			  for (int currentCacheLevel = 1; currentCacheLevel <= this->nbCacheLevel; currentCacheLevel++)
			    {
			      unsigned int nc = contexts.size();
			      for (unsigned int ic = 0; ic < nc; ic++)
				{
				  string attributeName = AnalysisHelper::mkContextAttrName(CodeCHMC[currentCacheLevel], contexts[ic]);
				  if (! this->CheckCacheAttributes(CurrentInstr, attributeName)) return false;
				}
			    }
			}
		    }
		}
	    }
	}
    }
  return true;
}

// Performs the analysis
// Returns true if successful, false otherwise
bool PipelineAnalysis::PerformAnalysis()
{
  vector < Cfg * >Cfgs = p->GetAllCfgs();
  vector < Node * >callNodes;
  CallGraph callgraph (p);
  Cfg *CurrentCfg;
  Node * CurrentNode;

  TRACE_PIPELINEANALYSIS(cout << " ############################################################################" << endl);
  TRACE_PIPELINEANALYSIS(cout << "  PipelineAnalysis::PerformAnalysis () BEGIN" << endl);

  //compute basic bloc execution time and deltas for each cfg
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      // Ignore dead cfgs
      if (! callgraph.isDeadCode (CurrentCfg))
	{ 
	  //compute for each context
	  const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
	  unsigned int nbContexts = contexts.size();
	  for (unsigned int i = 0; i < nbContexts; i++)
	    {
	      TRACE_PIPELINEANALYSIS(cout << " ++++++++++++++++++++++ PipelineAnalysis::PerformAnalysis (): NEW CONTEXT =============== " << endl);
	      string contextName = contexts[i]->getStringId();
	      TRACE_PIPELINEANALYSIS(cout << "contextname = " << contextName << endl);
	      //compute BB execution time
	      vector < Node * >Nodes = CurrentCfg->GetAllNodes();
	      for (unsigned int j = 0; j < Nodes.size(); j++)
		{
		  SerialisableIntegerAttribute time;
		  string attrName;
		  CurrentNode = Nodes[j];
		  if (! CurrentNode->isIsolatedNopNode())  // filtering nop ARM
		    {
		      //for the "first" occurence
		      time.SetValue(computeBB(*CurrentNode, contexts[i], true));
		      attrName = AnalysisHelper::mkContextAttrName(NodeExecTimeFirstAttributeName, contextName);
		      CurrentNode->SetAttribute(attrName, time);
		  
		      //for the "next" occurences
		      time.SetValue(computeBB(*CurrentNode, contexts[i], false));
		      attrName = AnalysisHelper::mkContextAttrName(NodeExecTimeNextAttributeName, contextName);
		      CurrentNode->SetAttribute(attrName, time);
		  
		      if (CurrentNode->IsCall()) { callNodes.push_back(CurrentNode); }
		    }
		}

	      // compute deltas: these deltas are associated to the edge.
	      vector < Edge * >Edges = CurrentCfg->GetAllEdges();
	      for (unsigned int j = 0; j < Edges.size(); j++)
		{
		  string attrName;
		  SerialisableIntegerAttribute time;
		  // Selecting the edges of the basic blocks of the same cfg.
		  if ((Edges[j]->GetSource())->GetCfg() == (Edges[j]->GetTarget())->GetCfg())
		    {
		      //compute for the first-first occurence
		      time.SetValue(computeDelta(Edges[j], *CurrentCfg, contexts[i], true, true));
		      attrName = AnalysisHelper::mkContextAttrName( DeltaFFAttributeName, contextName);
		      Edges[j]->SetAttribute(attrName, time);
		      TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

		      //compute for the first-next occurence
		      time.SetValue(computeDelta(Edges[j], *CurrentCfg, contexts[i], true, false));
		      attrName = AnalysisHelper::mkContextAttrName( DeltaFNAttributeName, contextName);
		      Edges[j]->SetAttribute(attrName, time);
		      TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

		      //compute for the next-first occurence
		      time.SetValue(computeDelta(Edges[j], *CurrentCfg, contexts[i], false, true));
		      attrName = AnalysisHelper::mkContextAttrName( DeltaNFAttributeName, contextName);
		      Edges[j]->SetAttribute(attrName, time);
		      TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

		      //compute for the next-next occurence
		      time.SetValue(computeDelta(Edges[j], *CurrentCfg, contexts[i], false, false));
		      attrName = AnalysisHelper::mkContextAttrName(DeltaNNAttributeName, contextName);
		      Edges[j]->SetAttribute(attrName, time);
		      TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);
		    }
		}
	    }
	}
    }

  // compute call and return deltas for branch
  // these deltas are associated to the call node
  for (vector < Node * >::iterator it = callNodes.begin(); it != callNodes.end(); it++)
    {
      Cfg *caller = (*it)->GetCfg();
      Cfg *callee = (*it)->GetCallee();

      //compute for each context
      const ContextList & contexts = (ContextList &) caller->GetAttribute(ContextListAttributeName);
      unsigned int nbContexts = contexts.size();
      for (unsigned int i = 0; i < nbContexts; i++)
	{
	  string contextName = contexts[i]->getStringId();
	  Node *CallerNode = (*it);
	  vector < Node * >CallerSuccessors = caller->GetSuccessors(CallerNode);
	  assert(CallerSuccessors.size() == 1);	// LBesnard : it is possible to have more than 1, and how ?
	  Node *returnNode = CallerSuccessors[0];
	  Node *CalledNode = callee->GetStartNode();

	  string attrName;
	  SerialisableIntegerAttribute time;

	  //compute call delta for the first occurence
	  time.SetValue(computeCallDelta(CallerNode, CalledNode, contexts[i], true));
	  attrName = AnalysisHelper::mkContextAttrName( CallDeltaFirstAttributeName, contextName);
	  CallerNode->SetAttribute(attrName, time);
	  TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

	  //compute call delta for the next occurence
	  time.SetValue(computeCallDelta(CallerNode, CalledNode, contexts[i], false));
	  attrName = AnalysisHelper::mkContextAttrName(CallDeltaNextAttributeName, contextName);
	  CallerNode->SetAttribute(attrName, time);
	  TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

	  vector < Node * >endNodes = callee->GetEndNodes();	// set of "return"
	  //compute the return delta for the first occurence
	  time.SetValue(computeReturnDelta(endNodes, returnNode, contexts[i], true));
	  attrName = AnalysisHelper::mkContextAttrName(ReturnDeltaFirstAttributeName, contextName);
	  CallerNode->SetAttribute(attrName, time);
	  TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);

	  //compute the return delta for the next occurence
	  time.SetValue(computeReturnDelta(endNodes, returnNode, contexts[i], false));
	  attrName = AnalysisHelper::mkContextAttrName(ReturnDeltaNextAttributeName, contextName);
	  CallerNode->SetAttribute(attrName, time);
	  TRACE_PIPELINEANALYSIS(cout << attrName << " = " << time.GetValue() << endl);
	}
    }
  TRACE_PIPELINEANALYSIS(cout << " PipelineAnalysis::PerformAnalysis () : END " << endl);
  TRACE_PIPELINEANALYSIS(cout << " ############################################################################" << endl);
  return true;
}

/* Remove all private attributes */
void PipelineAnalysis::RemovePrivateAttributes()
{
}

