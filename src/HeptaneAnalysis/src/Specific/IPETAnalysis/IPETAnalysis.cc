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

/* -----------------------------------------------------

   Generate an ILP system for this program. (NB: not yet integrated with
   pipeline analysis)

   The IPET computation is contextual. Contextual in this
   context means that different contexts are identified for every BB
   depending on its call path. Contextual IPET generate ILP constraints
   for every call context. Naming conventions of contexts are explained
   in file SharedAttributes/SharedAttributes.h.

   Variables of the ILP system:

   - execution frequency for first and next executions of every basic
   block, and total. First means the very execution of the basic block
   (even if the BB is in deeply nested loops). There is a pair (ffirst,fnext) per call context.

   - execution frequency for edges between basic blocks. There is a value per context.

   Naming conventions:

   - BB execution frequencies: n_NID_cCNB (ex: n_0_c0 for the execution
   frequency of BB number zero in execution context 0). prefixes nf and
   nn stand for first and next execution counts of a BB (ex: n_0_c0 =
   nf_0_c0 + nn_0_c0). Context numbers are used instead of context
   names because of the length of context names and the presence of
   characters likely to not be supported as variable identifiers in ILP
   solvers.

   - edge execution frequencies. Similar naming conventions are used:
   e_NID_NID_cCNB (ex: e_0_1_c0 for an edge between node 0 and node 1
   in call context number 0).

   Internal attributes used:

   - Node identifiers (InternalAttributeId) attached to every BB: 
   nodes are identified using integers starting at 0

   - WCET of first and next executions of every BB for every call context
   of the node (name InternalAttributeWCETfirst+"#"+contextName and
   respectively InternalAttributeWCETnext+"#"+contextName, where
   contextName is the context name in case of contextual IPET)

   Attributes used as inputs:

   - Loop maxiter

   - Cache classification. A cache classification should be attached to
   every instruction for every cache level (see SharedAttributes.h). 
   The attribute name is: BaseAttrName+"#"+contextName.

   -------------------------------------------------------- */

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include "Analysis.h"
#include "Generic/Config.h"
#include "Specific/IPETAnalysis/IPETAnalysis.h"
#include "Specific/IPETAnalysis/Solver.h"
#include "SharedAttributes/SharedAttributes.h"

#include "arch.h"
#include "Utl.h"

/*  METHOD_NOPIPELINE_ICACHE_DCACHE: generates 2 variables per BB (freq_first + freq_next) by context
 *  METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE: does not consider cache analysis results (simply counts 1 cycle per instruction)
 *  METHOD_PIPELINE_ICACHE_DCACHE: generates 2 variables per BB (freq_first + freq_next) by context
 *     The WCET value per BB is given by the PipelineAnalysis method.
*/
#define NOT_YET_IMPLEMENTED -1
#define METHOD_PIPELINE_ICACHE_DCACHE 1
#define METHOD_PIPELINE_ICACHE_PERFECTDCACHE 2
#define METHOD_NOPIPELINE_ICACHE_DCACHE 3
#define METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE 4
#define METHOD_NOPIPELINE_PERFECTICACHE_DCACHE 5
#define METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE 6

// NOW we should have m = METHOD_[PIPELINE, NOPIPELINE]_[ICACHE, PERFECT_ICACHE]_[ DCACHE | PERFECT_DCAHE] ]

int IPETAnalysis::getIPETMethodToApply(bool pipeline, bool perfectIcache, bool perfectDcache)
{
  if (pipeline)
    {
      if (! perfectIcache)
	{
	  Logger::print ("*** IPET analysis, the DATA CACHE is ignored in the PIPELINE analysis.");
	  if (! perfectDcache) return METHOD_PIPELINE_ICACHE_DCACHE;
	  return METHOD_PIPELINE_ICACHE_PERFECTDCACHE;
	}
    else
      {
	Logger::addFatal ("*** IPET analysis: Pipeline without instruction cache is not implemented");
	return NOT_YET_IMPLEMENTED;
      }
    }
  else
    {
      if (! perfectIcache)
      {
	if (! perfectDcache) return METHOD_NOPIPELINE_ICACHE_DCACHE;
	return METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE;
      }
    else
      {
	if (! perfectDcache) return METHOD_NOPIPELINE_PERFECTICACHE_DCACHE;
	return METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE;
      }
    }
  return NOT_YET_IMPLEMENTED;
}



// ---------------------------------------
// Constructor
// -----------
// - p: program whose WCET is to be computed
// - m: WCET computation method (METHOD_NOPIPELINE_ICACHE_DCACHE, METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE, METHOD_PIPELINE_ICACHE_DCACHE, ....)
// - used_solver: used solver (LP_SOLVE or CPLEX)
// - generate_wcet_info: true if WCET information is attached to the CFG of entry point
// - generate_node_freq: true if frequency information is attached to the nodes (one value per execution context)
// - latencyPerfectIcache : useful only for PerfectIcache method
// - latencyPerfectDcache : useful only for PerfectDcache method
// ---------------------------------------
IPETAnalysis::IPETAnalysis(Program * p, int used_solver, bool pipeline, bool generate_wcet_info, bool generate_node_freq, int nb_icache_levels, int nb_dcache_levels, 
			   map < int, vector < CacheParam * > >&cache_params):Analysis(p)
{
  bool perfectDcache = false;
  bool perfectIcache = false;
  
  // Check solver parameter is correct and create associated object
  assert(used_solver == LP_SOLVE || used_solver == CPLEX);
  if (used_solver == LP_SOLVE)
    solver = new LpsolveSolver((IPETAnalysis *) this);
  else
    solver = new CPLEXSolver((IPETAnalysis *) this);

  // Fill-in member variables from parameters
  generate_wcet_information = generate_wcet_info;
  generate_node_frequencies = generate_node_freq;
  NbICacheLevels = nb_icache_levels;
  NbDCacheLevels = nb_dcache_levels;
  MemoryStoreLatency = config->getMemoryStoreLatency();
  MemoryLoadLatency = config->getMemoryLoadLatency();

  this->call_graph = new CallGraph(p);

  PerfectICacheLatency = 0;
  // Fill-in attribute names for the different instruction and cache levels
  for (int l = 1; l <= NbICacheLevels; l++)
    {
      CodeCHMC[l] = CHMCAttributeNameCode(l);
      t_cache_type tCache = cache_params[l][0] ->type;
      if (tCache == ICACHE || tCache == PERFECTICACHE)
	levelAccessCostInstr[l] = cache_params[l][0] ->latency;
      else
	{
	  tCache = cache_params[l][1] ->type;
	  levelAccessCostInstr[l] = cache_params[l][1] ->latency;
	}
      if (tCache == PERFECTICACHE)
	{
	  PerfectICacheLatency=levelAccessCostInstr[l];
	  perfectIcache = true;
	}
    }
  PerfectDCacheLatency = 0;
  for (int l = 1; l <= NbDCacheLevels; l++)
    {
      DataCHMC[l] = CHMCAttributeNameData(l);
      blockCountName[l] = BlockCountAttributeName(l);
      t_cache_type tCache = cache_params[l][0] ->type;
      if (tCache == DCACHE ||  tCache == PERFECTDCACHE)
	levelAccessCostData[l] = cache_params[l][0] ->latency;
      else
	{
	  tCache = cache_params[l][1] ->type;
	  levelAccessCostData[l] = cache_params[l][1] ->latency;
	}
      if (tCache == PERFECTDCACHE)
	{
	  PerfectDCacheLatency=levelAccessCostData[l];
	  perfectDcache = true;
	}
    }

  method = getIPETMethodToApply(pipeline, perfectIcache, perfectDcache);
  printIPETCommand();

  // Fill-in attribute name of the different delta
  deltaName.push_back(DeltaFFAttributeName);
  deltaName.push_back(DeltaFNAttributeName);
  deltaName.push_back(DeltaNFAttributeName);
  deltaName.push_back(DeltaNNAttributeName);
  TRACE(cout << " IPETAnalysis::IPETAnalysis () : END " << endl);
  TRACE(cout << " ############################################################################" << endl);

}

// ---------------------------------------------------
// Removal of private attributes
// (node identifiers, WCET for first and next iterations)
// ---------------------------------------------------
static bool CleanupNodeInternalAttributes(Cfg * c, Node * n, void *param)
{
  string contextName, attr;
  // Remove InternalAttributeId (used to generate names for constraints in the ILP system)
  if (n->HasAttribute(InternalAttributeId))
    n->RemoveAttribute(InternalAttributeId);

  // Remove WCETs for first and next iterations, for all contexts
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  unsigned int nc = contexts.size();
  for (unsigned int ic = 0; ic < nc; ic++)
    {
      contextName = contexts[ic]->getStringId();
      attr = AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, contextName);
      if (n->HasAttribute(attr))
	n->RemoveAttribute(attr);

      attr = AnalysisHelper::mkContextAttrName(InternalAttributeWCETnext, contextName);
      if (n->HasAttribute(attr))
	n->RemoveAttribute(attr);
    }

  return true;
}

void IPETAnalysis::RemovePrivateAttributes()
{
  AnalysisHelper::applyToAllNodesRecursive(p, CleanupNodeInternalAttributes, (void *)(this));
}

// -----------------------------------------------------
// Used to check the validity of cache attribute on an instruction
//
// Checks that attribute of name attr_name is attached
// to the instruction and contains valid values only
// NB: the attribute name is a parameter because there is
//     an attribute name per context (base#context)
// -----------------------------------------------------
bool IPETAnalysis::CheckCacheAttributes(Instruction * i, string attr_name)
{

  // Check the attribute is attached

  //TODO Find a better way to check attribute correctness and still be able not to
  //perform either the instruction or the data cache analysis.
  /* if (i->HasAttribute(attr_name)==false) {
     stringstream errorstr;
     errorstr << "IPETAnalysis: "Instruction (" << i->GetCode()
     << ") should have a classification (AH/AM/NC/FM/AU)";
     Logger::addFatal(errorstr.str());
     return false;
     } else {
     // Validity check of attribute contents
     SerialisableStringAttribute att = (SerialisableStringAttribute &) i->GetAttribute(attr_name);
     string s=att.GetValue();
     if (s != "AH" && s != "AM" && s != "NC" && s != "FM" && s != "AU") {
     stringstream errorstr;
     errorstr << "IPETAnalysis: Invalid classification for instr (" << i->GetCode()
     << "): " << s << " Valid ones are (AH/AM/NC/FM/AU)";
     Logger::addFatal(errorstr.str());
     return false;
     }
     }
   */

  if (! i->HasAttribute(attr_name)) return false;
  // Validity check of attribute contents
  SerialisableStringAttribute att = (SerialisableStringAttribute &) i->GetAttribute(attr_name);
  string s=att.GetValue();
  if (s != "AH" && s != "AM" && s != "NC" && s != "FM" && s != "AU") return false;
  return true;
}


/* Check all executed instructions have a cache classification */
bool IPETAnalysis::CheckInputAttributesCacheClassification(Cfg * aCfg)
{
  Node *currentNode;
  bool requireDCacheAttr, requireICacheAttr;
  Instruction * vinstr;
  string attributeName;
  string vcontext;

  if (method != METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE)
    {
      
      if (!call_graph->isDeadCode(aCfg))
	{
	  requireDCacheAttr = (method == METHOD_NOPIPELINE_ICACHE_DCACHE) || (method == METHOD_NOPIPELINE_PERFECTICACHE_DCACHE) || (method == METHOD_PIPELINE_ICACHE_DCACHE);
	  requireICacheAttr = (method == METHOD_NOPIPELINE_ICACHE_DCACHE) || (method == METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE) || (method == METHOD_PIPELINE_ICACHE_DCACHE);

	  const ContextList & contexts = (ContextList &) aCfg->GetAttribute(ContextListAttributeName);
	  unsigned int nc = contexts.size();
	  vector < Node * >vn = aCfg->GetAllNodes();
	  for (unsigned int n = 0; n < vn.size(); n++)
	    {
	      currentNode = vn[n];
	      if (currentNode->IsBB())
		if (! currentNode->isIsolatedNopNode())
		  {
		    vector < Instruction * >vi = currentNode->GetInstructions();
		    for (unsigned int inst = 0; inst < vi.size(); inst++)
		      {
			vinstr = vi[inst];
			if (vinstr->IsCode())
			  {
			    for (int currentCacheLevel = 1; currentCacheLevel <= NbICacheLevels; currentCacheLevel++)
			      {
				for (unsigned int ic = 0; ic < nc; ic++)
				  {
				    vcontext = contexts[ic]->getStringId();
				    if (requireICacheAttr)
				      {
					attributeName = AnalysisHelper::mkContextAttrName(CodeCHMC[currentCacheLevel], vcontext);
					if (! this->CheckCacheAttributes(vinstr, attributeName))
					  return false;
				      }
				    if (requireDCacheAttr)
				      {
					attributeName = AnalysisHelper::mkContextAttrName(DataCHMC[currentCacheLevel], vcontext);
					if (! this->CheckCacheAttributes(vinstr, attributeName)) 
					  return false;
				      }
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


/* 
   Check the pipeline timing provided by the pipelineAnalysis(nodes and edges)
*/
bool IPETAnalysis::CheckInputAttributesPipelineTiming(Cfg * aCfg)
{
  if ((method == METHOD_PIPELINE_ICACHE_DCACHE ) ||  (method == METHOD_PIPELINE_ICACHE_PERFECTDCACHE))
    {
      vector < Node * >vn = aCfg->GetAllNodes();
      const ContextList & contexts = (ContextList &) aCfg->GetAttribute(ContextListAttributeName);
      unsigned int nc = contexts.size();
      for (std::vector < Node * >::iterator itNode = vn.begin(); itNode != vn.end(); itNode++)
	{
	  //for each context
	  for (unsigned int ic = 0; ic < nc; ic++)
	    {
	      if (! (*itNode)->isIsolatedNopNode())
		{
		  string contextName = contexts[ic]->getStringId();
		  string attributeNameFirst =  AnalysisHelper::mkContextAttrName(NodeExecTimeFirstAttributeName, contextName);
		  string attributeNameNext = AnalysisHelper::mkContextAttrName(NodeExecTimeNextAttributeName, contextName);
		  if ((*itNode)->HasAttribute(attributeNameFirst) == false || (*itNode)->HasAttribute(attributeNameNext) == false)
		    {
		      stringstream errorstr;
		      errorstr << "IPETAnalysis: Nodes should have " << NodeExecTimeFirstAttributeName << " attribute set." << "(this attribute is provided with the PipelineAnalysis)";
		      Logger::addFatal(errorstr.str());
		      return false;
		    }
		}
	    }
	}

      //edges
      vector < Edge * >ve = aCfg->GetAllEdges();
      for (std::vector < Edge * >::iterator itEdge = ve.begin(); itEdge != ve.end(); itEdge++)
	{
	  for (unsigned int ic = 0; ic < nc; ic++)
	    {
	      string contextName = contexts[ic]->getStringId();
	      string attributeNameFF = AnalysisHelper::mkContextAttrName(DeltaFFAttributeName, contextName);
	      string attributeNameFN = AnalysisHelper::mkContextAttrName(DeltaFNAttributeName, contextName);
	      string attributeNameNF = AnalysisHelper::mkContextAttrName(DeltaNFAttributeName, contextName);
	      string attributeNameNN = AnalysisHelper::mkContextAttrName(DeltaNNAttributeName, contextName);
	      if ((*itEdge)->HasAttribute(attributeNameFF) == false || (*itEdge)->HasAttribute(attributeNameFN) == false || 
		  (*itEdge)->HasAttribute(attributeNameNF) == false || (*itEdge)->HasAttribute(attributeNameNN) == false)
		{
		  stringstream errorstr;
		  errorstr << "IPETAnalysis: Nodes should have " << DeltaFFAttributeName << " attribute set." << "(this attribute is provided with the PipelineAnalysis)";
		  Logger::addFatal(errorstr.str());
		  return false;
		}
	    }
	}
    }
  return true;
}



// ----------------------------------------------------------------
// Attribute check function
//
// Checks all attributes required for IPET calculation are attached
// 
// - cache classification attributes (see naming conventions on
//   top of file)
//
// ----------------------------------------------------------------
bool IPETAnalysis::CheckInputAttributes()
{
  Cfg *currentCfg;

  vector < Cfg * >lc = p->GetAllCfgs();
  for (unsigned int c = 0; c < lc.size(); c++)
    {
      currentCfg = lc[c];
      if (! CheckInputAttributesCacheClassification(currentCfg)) return false;
      if (! CheckInputAttributesPipelineTiming(currentCfg)) return false;
    }
  return true;
}

// ------------------------------------------------------------------
// Get an attribute value by name
// Shortcut to withdraw an NonSerialisableIntegerAttribute attached
// to a Cfg node
// ------------------------------------------------------------------
static long getIntegerAttribute(Node * n, string name)
{
  int val;
  assert(n != NULL);
  if (!n->HasAttribute(name))
    exit(-1);
  assert(n->HasAttribute(name));
  NonSerialisableIntegerAttribute ai = (NonSerialisableIntegerAttribute &) n->GetAttribute(name);
  val = ai.GetValue();
  return val;
}

// -----------------------------------------------------------------------
// Compute a static bound on the node n frequency on the given context
//
// Works in a context-dependent manner
//
// Tracks the call nodes of the given context.
// Then, for all nodes of interest (including n), compute the product of all
// their loops iteration bounds.
//
// -----------------------------------------------------------------------

unsigned int ComputeNodeFrequencyBound(Node * n, Context * context)
{
  unsigned int freq = 1;
  vector < Node * >vn;

  Context *caller_context = context;
  while (caller_context->getCallerNode())
    {
      vn.push_back(caller_context->getCallerNode());
      caller_context = caller_context->getCallerContext();
    }

  vn.push_back(n);
  for (unsigned int node = 0; node < vn.size(); node++)
    { //Compute n frequency based on the frequencies of the different involved loops
      vector < Loop * >vl = vn[node]->GetCfg()->GetAllLoops();
      for (unsigned int loop = 0; loop < vl.size(); loop++)
	{
	  if (vl[loop]->FindInLoop(vn[node]))
	    {
	      SerialisableIntegerAttribute bound = (SerialisableIntegerAttribute &) vl[loop]->GetAttribute(MaxiterAttributeName);
	      freq *= bound.GetValue();
	    }
	}
    }

  return freq;
}

string IPETAnalysis::mkVariableNameSolver(string prefix, Node * n, string vcontext)
{
  return AnalysisHelper::mkVariableNameSolver(prefix, getIntegerAttribute(n, InternalAttributeId), vcontext);
}

string IPETAnalysis::mkEdgeVariableNameSolver(string prefix, Node * source, Node * target, string vcontext)
{
  return AnalysisHelper::mkEdgeVariableNameSolver(prefix, getIntegerAttribute(source, InternalAttributeId), getIntegerAttribute(target, InternalAttributeId), vcontext);
}

// ------------------------------------------------------------------
// Get the 4 deltas of an edge and put it in a integer vector,
// in the following order: FF, FN, NF, NN
// These deltas are generated by the PipelineAnalysis
// to be used with METHOD_CACHE_PIPELINE estimation method
// ------------------------------------------------------------------
vector < int >IPETAnalysis::getDeltas(Edge & e, string context)
{
  vector < int >deltas;
  // ostringstream tmp;

  for (vector < string >::iterator it = deltaName.begin(); it != deltaName.end(); it++)
    {
      deltas.push_back(AnalysisHelper::getEdgeValueAttr(e, *it, context));
    }

  return deltas;
}

void IPETAnalysis::AddILPVariable(string prefix, long source_id, long target_id, string vcontext, vector < string > &edgeName)
{
  string s = AnalysisHelper::mkEdgeVariableNameSolver(prefix, source_id, target_id, vcontext);
  edgeName.push_back(s);
}

// ------------------------------------------------------------------
// Generate the variable name used by the ILP for an edge
// in the following order: FF, FN, NF, NN
// to be used with METHOD_CACHE_PIPELINE estimation method
// ------------------------------------------------------------------
vector < string > IPETAnalysis::generateEdgeVariableName(Edge & e, string vcontext)
{
  vector < string > edgeName;
  ostringstream tmp;

  Cfg *cfg = e.GetCfg();
  Node *source = cfg->GetSourceNode(&e);
  Node *target = cfg->GetTargetNode(&e);

  long source_id = getIntegerAttribute(source, InternalAttributeId);
  long target_id = getIntegerAttribute(target, InternalAttributeId);

  AddILPVariable("eff_", source_id, target_id, vcontext, edgeName);
  AddILPVariable("efn_", source_id, target_id, vcontext, edgeName);
  AddILPVariable("enf_", source_id, target_id, vcontext, edgeName);
  AddILPVariable("enn_", source_id, target_id, vcontext, edgeName);
  return edgeName;
}

/*
  Generate call constraints for one CFG in Contextual IPET mode (parameter  ).
  For every call point of id X in context C1,
  if Y is the id of the first BB of callee 
       (context C2=C1+"#"callnb+"#"calleeName), then
   the following constraint is generated: 
    n_X_cC1 = n_Y_cC2
*/
void IPETAnalysis::generateCallConstraints(ostringstream & os, Program * p)
{
  Cfg *vCFGCallee;
  vector < Cfg * >lcfg = p->GetAllCfgs();

  for (unsigned int callee = 0; callee < lcfg.size(); callee++)
    {
      // Generate call constraints for CFG callee
      vCFGCallee = lcfg[callee];
      if (!call_graph->isDeadCode(vCFGCallee))
	{
	  if (!vCFGCallee->IsExternal())
	    {
	      // Scan all contexts of callee
	      // Get the list of execution contexts of the node
	      const ContextList & contexts = (ContextList &) vCFGCallee->GetAttribute(ContextListAttributeName);
	      unsigned int nc = contexts.size();
	      for (unsigned int ic = 0; ic < nc; ic++)
		{
		  Context *callee_context = contexts[ic];
		  Node *n = callee_context->getCallerNode();
		  if (!n)
		    {
		      continue;
		    }		//FIXME Deal with the main function.
		  Context *caller_context = callee_context->getCallerContext();
		  Node *start = vCFGCallee->GetStartNode();
		  vector < string > ncallers;
		  ncallers.push_back(mkVariableNameSolver("n_", n, caller_context->getStringId()));
		  ncallers.push_back(mkVariableNameSolver("n_", start, contexts[ic]->getStringId()));
		  solver->generate_flow_constraint(os, ncallers);
		}
	    }
	}
    }
}


void IPETAnalysis::ComputeNodeExecutionTime_InstructionCacheLevel(Instruction * vinstr, Context * context, int numCache, int *wcet_first, int *wcet_next, bool * countFirst, bool * countNext)
{
  int currentAccessCost = levelAccessCostInstr[numCache];
  string classif = AnalysisHelper::getInstrStringAttr(vinstr, CodeCHMC[numCache], context);
  if (*countNext) (*wcet_next) = (*wcet_next) + currentAccessCost;
  if (*countFirst) (*wcet_first) = (*wcet_first) + currentAccessCost;	//latence to not find the address in the current level

  if (classif == "AH")
    {
      *countFirst = false;
      *countNext = false;
    }
  else if ((classif == "AM") || (classif == "NC"))
    {
      if (numCache == NbICacheLevels)
	{
	  if (*countFirst) (*wcet_first) = (*wcet_first) + MemoryLoadLatency;
	  if (*countNext) (*wcet_next) = (*wcet_next) + MemoryLoadLatency;
	}
    }
  else
    if (classif == "FM")
      {
	if ((numCache == NbICacheLevels) && (*countFirst)) (*wcet_first) = (*wcet_first) + MemoryLoadLatency ;
	*countNext = false;
      }
}

// Count latencies (wcet_first, wcet_next when updateNext), according to the data access attributes ( never_accessed_data, always_accessed_data, occurrence_bound_data).
void IPETAnalysis::DataCacheLevel_latency(int vCost, bool updateNext, int *wcet_first, int *wcet_next, bool never_accessed_data, bool always_accessed_data, unsigned int occurrence_bound_data)
{
  if (always_accessed_data)
    {
      (*wcet_first) = (*wcet_first) + vCost;
      if (updateNext) (*wcet_next) = (*wcet_next) + vCost;
    }
  else if (! never_accessed_data)
    {
      (*wcet_first) = (*wcet_first) + ( vCost * occurrence_bound_data);
    }
}


// Update the different attributes for the next level
unsigned int IPETAnalysis::DataCacheLevel_NextLevel(string classif, unsigned int memBlock, unsigned int frequency, bool * never_accessed_data, bool *always_accessed_data, unsigned int occurrence_bound_data )
{
  // Next value of never_accessed_data
  if ( classif == "AH") (*never_accessed_data) = true;

  // Next value of always_accessed_data
  if (*always_accessed_data) (*always_accessed_data) = ((classif == "AM") || (classif == "NC"));
  assert( (classif != "AH") || (!(*always_accessed_data) && (*never_accessed_data)));
  
  // Next value of occurrence_bound_data
  if (*never_accessed_data) return 0;
  if (*always_accessed_data) return min(memBlock, frequency);
  if (classif == "FM") return  min(memBlock, occurrence_bound_data);
  return occurrence_bound_data;

}



void IPETAnalysis::ComputeNodeExecutionTime_DataCacheLevel(Instruction * vinstr, Context * context, int numCache, unsigned int frequency, int *wcet_first, int *wcet_next,
							   bool * never_accessed_data, bool * always_accessed_data, unsigned int *occurrence_bound_data)
{
  string classif;
  unsigned int memBlock;

  classif = AnalysisHelper::getInstrStringAttr(vinstr, DataCHMC[numCache], context);
  if (classif != "AU")  
    {
      assert(numCache != 1 || (*always_accessed_data));
      assert(!(*never_accessed_data));

      // Count latencies for a cache level (numCache)
      DataCacheLevel_latency( levelAccessCostData[numCache], true, wcet_first, wcet_next, *never_accessed_data, *always_accessed_data, *occurrence_bound_data);

      // Update the different attributes for the next level
      memBlock = AnalysisHelper::getInstrIntAttr(vinstr, blockCountName[numCache], context);
      (*occurrence_bound_data) = DataCacheLevel_NextLevel( classif, memBlock, frequency, never_accessed_data, always_accessed_data, *occurrence_bound_data);
      
      // Accesses to the main memory
      if (numCache == NbDCacheLevels)
	{
	  DataCacheLevel_latency( MemoryLoadLatency, (classif == "AM" || classif == "NC"), wcet_first, wcet_next, *never_accessed_data, *always_accessed_data, *occurrence_bound_data);
	}
    }
  else
    {
      assert( (numCache == 1) || (! (*always_accessed_data) && (*never_accessed_data)));
      (*always_accessed_data) = false;
      (*never_accessed_data) = true;
    }
}

bool IPETAnalysis::ComputeInstrExecutionTime_NOPIPELINE_STORE_DCACHE(Instruction *vinstr, int *wcet_first, int *wcet_next )
{
  // This is introduce to not count for instance the next of an always miss 
  // in the current cache level while in previous level it is a First miss
  if (Arch::isStore(vinstr->GetCode()))
    {
      *wcet_first = *wcet_first + MemoryStoreLatency;
      *wcet_next = *wcet_next + MemoryStoreLatency;
      return true;
    }
  return false;
}




void IPETAnalysis::ComputeInstrExecutionTime_NOPIPELINE_ICACHE_PERFECTDCACHE(Instruction *vinstr, Context * context, int *wcet_first, int *wcet_next )
{
  int currentCache;
  bool countFirst, countNext;
  // This is introduce to not count for instance the next of an always miss 
  // in the current cache level while in previous level it is a First miss
  *wcet_first = *wcet_first + PerfectDCacheLatency;
  *wcet_next = *wcet_next + PerfectDCacheLatency;
  
  // For each Cache.
  countFirst = true; // to know if we count the first access for the current cache level
  countNext = true; // to know if we count the next access for the current cache level
  for (currentCache = 1; currentCache <= NbICacheLevels; currentCache++)
    {
      ComputeNodeExecutionTime_InstructionCacheLevel(vinstr, context, currentCache, wcet_first, wcet_next, &countFirst, &countNext);
    }
}


void IPETAnalysis::ComputeInstrExecutionTime_NOPIPELINE_ICACHE_DCACHE(Instruction *vinstr, Context * context,  unsigned int frequency,  int *wcet_first, int *wcet_next )
{
  int currentCache;
  string vinstr_code;
  bool countFirst, countNext, always_accessed_data,  never_accessed_data, bisLoad;
  unsigned int occurrence_bound_data;

  ComputeInstrExecutionTime_NOPIPELINE_STORE_DCACHE(vinstr, wcet_first, wcet_next);
  bisLoad =  Arch::isLoad(vinstr->GetCode());

  countFirst = true;	// to know if we count the first access for the current cache level
  countNext = true;	// to know if we count the next access for the current cache level
  always_accessed_data = true;
  never_accessed_data = false;
  occurrence_bound_data = 1;
  for (currentCache = 1; currentCache <= NbICacheLevels; currentCache++)  // ok because NbICacheLevels = NbDCacheLevels
    {
      // Instruction cache
  	ComputeNodeExecutionTime_InstructionCacheLevel(vinstr, context, currentCache, wcet_first, wcet_next, &countFirst, &countNext);
      // Data cache
  	if (bisLoad)
	  ComputeNodeExecutionTime_DataCacheLevel(vinstr, context, currentCache, frequency, wcet_first, wcet_next, &never_accessed_data, &always_accessed_data, &occurrence_bound_data);
    }
}

void IPETAnalysis::ComputeInstrExecutionTime_NOPIPELINE_PERFECTICACHE_DCACHE(Instruction *vinstr, Context * context, unsigned int frequency, int *wcet_first, int *wcet_next )
{
  int currentCache;
  string vinstr_code;
  bool always_accessed_data,  never_accessed_data;
  unsigned int occurrence_bound_data;

  // This is introduce to not count for instance the next of an always miss 
  // in the current cache level while in previous level it is a First miss
  *wcet_first = *wcet_first + PerfectICacheLatency;
  *wcet_next = *wcet_next + PerfectICacheLatency;


  ComputeInstrExecutionTime_NOPIPELINE_STORE_DCACHE(vinstr, wcet_first, wcet_next);
  if ( Arch::isLoad(vinstr->GetCode()))
    {
      always_accessed_data = true;
      never_accessed_data = false;
      occurrence_bound_data = 1;
      for (currentCache = 1; currentCache <= NbDCacheLevels; currentCache++)
	{
	  ComputeNodeExecutionTime_DataCacheLevel(vinstr, context, currentCache, frequency, wcet_first, wcet_next, &never_accessed_data, &always_accessed_data, &occurrence_bound_data);
	}
    }
}



// -----------------------------------------------------------------------
// Compute the execution time of a block for its first and next iterations 
//
// Works in both a context independent and context-dependent manner
// So far, this method considers the cache only
// (integration with pipeline analysis still to be done)
//
// Multiple levels of caches are considered (see Damiens's paper
// on WCET analysis for cache hierarchies for the details)
//
// Context is the execution context for which the BB execution time
// is computed ("" if the analysis is non-contextual)
// -----------------------------------------------------------------------
void IPETAnalysis::ComputeNodeExecutionTime_NOPIPELINE_CACHE(Node * n, Context * context, int *pwcet_first, int *pwcet_next, bool perfectIcache, bool perfectDcache)
{
  Instruction *vinstr;
  int wcet_first, wcet_next;

  assert ( !perfectIcache || !perfectDcache);  // perfectIcache && perfectDcache is filtered before ComputeNodeExecutionTime_NOPIPELINE_NOCACHE
  wcet_first = 0;
  wcet_next = 0;

  vector < Instruction * >vi = n->GetInstructions();
  for (unsigned int inst = 0; inst < vi.size(); inst++)
    {
      vinstr = vi[inst];
      if (vinstr->IsCode())
	{
	  if (perfectIcache)
	    {
	      if (! perfectDcache)
		ComputeInstrExecutionTime_NOPIPELINE_PERFECTICACHE_DCACHE(vinstr, context, ComputeNodeFrequencyBound(n, context),  &wcet_first, &wcet_next);
	      else; // Does not occur.
	    }
	  else
	    if (perfectDcache)
	      ComputeInstrExecutionTime_NOPIPELINE_ICACHE_PERFECTDCACHE(vinstr, context, &wcet_first, &wcet_next );
	    else
	      ComputeInstrExecutionTime_NOPIPELINE_ICACHE_DCACHE(vinstr, context, ComputeNodeFrequencyBound(n, context), &wcet_first, &wcet_next );
	}
    }
  *pwcet_first = wcet_first;
  *pwcet_next = wcet_next;
}

// ------------------------------------------------
//
// Generate Node Ids for one CFG
// --------------------------------
//
// Generate a unique Id to each node of a
// and add the node Id as an internal attribute
// attached to the node
//
// NB: this step was separated from constraint
// generation to allow edges between nodes
// of different Cfgs
//
// ------------------------------------------------
bool IPETAnalysis::generateNodeIds(ostringstream & os, Cfg * c)
{
  // Used to generate BB numbers
  // NB: should be static, because BB numbers should be unique for all Cfgs
  static int bb_id = 0;

  vector < Node * >vn = c->GetAllNodes();

  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  unsigned int nc = contexts.size();

  // Assign a unique number to every BB (used in constraint generation)
  // ------------------------------------------------------------------
  for (unsigned int i = 0; i < vn.size(); i++)
    {
      // Attribute an id
      NonSerialisableIntegerAttribute attr_id(bb_id);
      vn[i]->SetAttribute(InternalAttributeId, attr_id);

      // Store correspondance between id of variable in ILP system and node pointer
      for (unsigned int ic = 0; ic < nc; ic++)
	{
	  string idVar = AnalysisHelper::mkVariableNameSolver("n_", bb_id, contexts[ic]->getStringId());
	  node_ids[idVar] = vn[i];
	}
      bb_id++;
    }
  return true;
}

/*
  Set the attributes InternalAttributeWCETfirst and InternalAttributeWCETnext to each node of vn in  "contextual" mode.
  For all execution contexts (once only if the analysis is not contextual).
 */
void IPETAnalysis::ComputeNodesExecutionTime_NOPIPELINE_CACHE(vector < Node * >vn, const ContextList & contexts, bool perfectIcache, bool perfectDcache)
{
  int wcet_first, wcet_next;
  string contextName;
  unsigned int i, ic, nc;

  nc = contexts.size();
  for ( i = 0; i < vn.size(); i++)
    {
      // For all execution contexts (once only if the analysis is not contextual).
      // Compute the node execution time
      Node *n = vn[i];
      // To generate non contextual information once only
      for ( ic = 0; ic < nc; ic++)
	{
	  wcet_first = 0;
	  wcet_next = 0;
	  ComputeNodeExecutionTime_NOPIPELINE_CACHE(n, contexts[ic], &wcet_first, &wcet_next, perfectIcache, perfectDcache);

	  // Attach the execution time as an attribute to the node
	  contextName = contexts[ic]->getStringId();
	  NonSerialisableIntegerAttribute attr_first(wcet_first);
	  n->SetAttribute(AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, contextName), attr_first);
	  NonSerialisableIntegerAttribute attr_next(wcet_next);
	  n->SetAttribute(AnalysisHelper::mkContextAttrName(InternalAttributeWCETnext, contextName), attr_next);
	}
    }
}

void IPETAnalysis::generateConstraints_NOPIPELINE_CACHE(vector < Node * >vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  string contextName;
  unsigned int i, ic, nc;

  // Here, there is a variable in the ILP system per basic block,
  // first and next executions and execution
  // context, plus one variable per edge and execution context.
  // Execution contexts are coded as integers (see naming conventions on top)
  // In the case non-contextual IPET is used, context number 0 is used
  nc = contexts.size();  
  for ( i = 0; i < vn.size(); i++)
    {
      Node *n = vn[i];
      // Get the list of execution contexts of the node
      for ( ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();
	  // Get value of wcets for first and next iters
	  vid.push_back(mkVariableNameSolver("nf_", n, contextName));
	  int wcet_first = getIntegerAttribute(n, AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, contextName));
	  vwcet.push_back(wcet_first);

	  vid.push_back(mkVariableNameSolver("nn_", n, contextName));
	  int wcet_next = getIntegerAttribute(n, AnalysisHelper::mkContextAttrName(InternalAttributeWCETnext, contextName));
	  vwcet.push_back(wcet_next);
	}
    }
}

/* 
   Generate WCETs for all nodes (here, simple sum of number of instructions)
 */
void IPETAnalysis::ComputeNodesExecutionTime_NOPIPELINE_NOCACHE(vector < Node * >vn, const ContextList & contexts)
{
  Node *n;
  int wcet;
  string contextName;
  unsigned int nc, ic, i, inst;

  nc = contexts.size();
  for (i = 0; i < vn.size(); i++)
    {
      wcet = 0;
      n = vn[i];
      vector < Instruction * >vi = n->GetInstructions();
      for (inst = 0; inst < vi.size(); inst++)
	{
	  if (vi[inst]->IsCode())
	    wcet = wcet + PerfectICacheLatency;
	}
      NonSerialisableIntegerAttribute attr_wcet(wcet);
      for (ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();
	  string attr = AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, contextName);
	  n->SetAttribute(attr, attr_wcet);
	}
    }
}

void IPETAnalysis::generateConstraints_NOPIPELINE_NOCACHE(vector < Node * >vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  Node *n;
  int wcet;
  string contextName;
  unsigned int nc, ic, i;

  nc = contexts.size();
  for (i = 0; i < vn.size(); i++)
    {
      n = vn[i];
      for (ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();
	  // Get value of wcets for first iters
	  wcet = getIntegerAttribute(n, AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, contextName));
	  vid.push_back(mkVariableNameSolver("n_", n, contextName));
	  vwcet.push_back(wcet);
	}
    }
}

void IPETAnalysis::generateConstraints_PIPELINE_CACHE(Cfg * c, vector < Node * >vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  vector < Edge * >ve = c->GetAllEdges();
  Node *n;
  string contextName;
  unsigned int ic, nc;

  // Here, are generated variables for the ILP system per basic block :
  // first and next executions frequencies per context (vid vector). These variables
  // are associated with wcet values (vwcet vector).
  // Execution contexts are coded as integers (see naming conventions on top)
   nc = contexts.size();
   for (std::vector < Node * >::iterator it = vn.begin(); it != vn.end(); it++)
    {
      // Get the list of execution contexts of the node
      for (ic = 0; ic < nc; ic++)
	{
	  n = (*it);
	  contextName = contexts[ic]->getStringId();
	  //get value of wcet for first iteration
	  string idVar = mkVariableNameSolver("nf_", n, contextName);
	  vid.push_back(idVar);

	  //call and return deltas associated with node frequency
	  if (n->IsCall())
	    {
	      vid.push_back(idVar);	//for the call
	      vid.push_back(idVar);	//fot the return
	    }

	  vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, NodeExecTimeFirstAttributeName, contextName));
	  if (n->IsCall())
	    {
	      vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, CallDeltaFirstAttributeName, contextName));
	      vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, ReturnDeltaFirstAttributeName, contextName));
	    }

	  //get value of wcet for next iteration
	  idVar = mkVariableNameSolver("nn_", n, contextName);
	  vid.push_back(idVar);

	  //call and return deltas associated with node frequency
	  if (n->IsCall())
	    {
	      vid.push_back(idVar);	//for the call
	      vid.push_back(idVar);	//for the return
	    }

	  vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, NodeExecTimeNextAttributeName, contextName));
	  if ((*it)->IsCall())
	    {
	      vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, CallDeltaNextAttributeName, contextName));
	      vwcet.push_back(AnalysisHelper::getNodeValueAttr(n, ReturnDeltaNextAttributeName, contextName));
	    }
	}
    }

  // Here, are generated variables for the ILP system per edges(for the objective function).
  for (std::vector < Edge * >::iterator it = ve.begin(); it != ve.end(); it++)
    {
      for (ic = 0; ic < nc; ic++)
	{
	  if (!(c->GetSourceNode(*it))->IsCall())
	    {
	      contextName = contexts[ic]->getStringId();
	      vector < int >deltas = getDeltas(*(*it), contextName);
	      vector < string > variableName = generateEdgeVariableName(*(*it), contextName);

	      for (unsigned int i = 0; i < deltas.size(); i++)
		{
		  vid.push_back(variableName[i]);
		  vwcet.push_back(deltas[i]);
		}
	    }
	}
    }
}


// called for the methods METHOD_NOPIPELINE_ICACHE_DCACHE, METHOD_NOPIPELINE_PERFECTICACHE_DCACHE,METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE,
//  METHOD_PIPELINE_ICACHE_DCACHE and METHOD_PIPELINE_ICACHE_PERFECTDCACHE  (when the CACHE_ANALYSIS is done)
void IPETAnalysis::generateConstraints_inside_CACHE_BB(ostringstream & os, vector < Node * >vn, const ContextList & contexts)
{
  Node *n;
  string contextName;

  unsigned int nc = contexts.size();
  for (unsigned int i = 0; i < vn.size(); i++)
    {
      n = vn[i];
      // BB frequency = freq first + freq next for all contexts
      // Get the list of execution contexts of the node
      for (unsigned int ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();
	  vector < string > vs;
	  vs.push_back(mkVariableNameSolver("n_", n, contextName));
	  vs.push_back(mkVariableNameSolver("nf_", n, contextName));
	  vs.push_back(mkVariableNameSolver("nn_", n, contextName));
	  solver->generate_flow_constraint(os, vs);

	  // Freq first <=1 (bound)
	  vector < string > vsf;
	  vsf.push_back(mkVariableNameSolver("nf_", n, contextName));
	  solver->generate_inequality(os, vsf, 1);
	}
    }
}

// Generate flow constraints for every edges ( restricted to METHOD_CACHE_PIPELINE )
void IPETAnalysis::generateConstraints_PIPELINE_CACHE_edges(ostringstream & os, vector < Edge * >ve, const ContextList & contexts)
{
  string contextName;
  unsigned int nc = contexts.size();

  for (vector < Edge * >::iterator it = ve.begin(); it != ve.end(); it++)
    {
      for (unsigned int ic = 0; ic < nc; ic++)
	{
	  vector < string > vs;

	  Cfg *cfg = (*it)->GetCfg();
	  Node *source = cfg->GetSourceNode((*it));
	  Node *target = cfg->GetTargetNode((*it));
	  contextName = contexts[ic]->getStringId();
	  vs.push_back(mkEdgeVariableNameSolver("e_", source, target, contextName));

	  vector < string > edgeVariable = generateEdgeVariableName(*(*it), contextName);
	  vs.insert(vs.end(), edgeVariable.begin(), edgeVariable.end());

	  // Fedge = Fff + Ffn + Fnf + Fnn
	  solver->generate_flow_constraint(os, vs);

	  // Fff + Ffn <= 1
	  vector < string > vtmp;
	  vtmp.push_back(edgeVariable[0]);
	  vtmp.push_back(edgeVariable[1]);
	  solver->generate_inequality(os, vtmp, 1);

	  // Fff + Fnf <= 1
	  vtmp.clear();
	  vtmp.push_back(edgeVariable[0]);
	  vtmp.push_back(edgeVariable[2]);
	  solver->generate_inequality(os, vtmp, 1);
	}
    }
}

// Generate a constraint for every BB / edge for every execution context  ( no restriction on current method )
void IPETAnalysis::generateConstraints_BB_edge_eachContext(Cfg * c, ostringstream & os, vector < Node * >vn, vector < Edge * >ve, const ContextList & contexts)
{
  string contextName;
  unsigned int nc = contexts.size();

  for (unsigned int i = 0; i < vn.size(); i++)
    {
      Node *n = vn[i];
      for (unsigned int ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();
	  // Constraint generation (flow constraints)
	  // ----------------------------------------
	  // Incoming edges
	  {
	    vector < Edge * >in_edges = c->GetIncomingEdges(n);
	    vector < string > vs;
	    vs.push_back(mkVariableNameSolver("n_", n, contextName));

	    for (unsigned int ei = 0; ei < in_edges.size(); ei++)
	      {
		Edge *edge_i = in_edges[ei];
		Node *source_i = c->GetSourceNode(edge_i);
		string s = mkEdgeVariableNameSolver("e_", source_i, n, contextName);
		vs.push_back(s);
	      }
	    solver->generate_flow_constraint(os, vs);
	  }

	  // Outgoing edges
	  {
	    vector < Edge * >out_edges = c->GetOutgoingEdges(n);
	    vector < string > vs;
	    vs.push_back(mkVariableNameSolver("n_", n, contextName));

	    for (unsigned int ei = 0; ei < out_edges.size(); ei++)
	      {
		Edge *edge_i = out_edges[ei];
		Node *dest_i = c->GetTargetNode(edge_i);
		string s = mkEdgeVariableNameSolver("e_", n, dest_i, contextName);
		vs.push_back(s);
	      }
	    solver->generate_flow_constraint(os, vs);
	  }
	}
    }
}

/** 
    Constraint for back edges of loops:
    Generates for every loop node contraints of the form: f_node <= maxiter * sum(entry_edges)

    Nodes belonging to subloops should not be considered, 
    as well as the loop head (except if it is the only node in the loop)
  */
void IPETAnalysis::generateConstraints_back_edges_loops(Cfg * c, ostringstream & os, vector < Node * >vn, const ContextList & contexts)
{
  string contextName;
  vector < Loop * >vl = c->GetAllLoops();
  unsigned int nc = contexts.size();
  
// For all loops
  for (unsigned int l = 0; l < vl.size(); l++)
    {
      // For all execution contexts
      for (unsigned int ic = 0; ic < nc; ic++)
	{
	  contextName = contexts[ic]->getStringId();

	  SerialisableIntegerAttribute bound = (SerialisableIntegerAttribute &) vl[l]->GetAttribute(MaxiterAttributeName);
	  long maxiter = bound.GetValue();
	  vector < string > vs;
	  vector < long >vcst;
	  // Scan the incoming edges of the loop head
	  Node *head = vl[l]->GetHead();
	  vector < Edge * >ie = c->GetIncomingEdges(head);
	  for (unsigned int e = 0; e < ie.size(); e++)
	    {
	      Node *origin = c->GetSourceNode(ie[e]);
	      if (vl[l]->FindInLoop(origin) == false)
		{
		  Node *destination = c->GetTargetNode(ie[e]);
		  string s = mkEdgeVariableNameSolver("e_", origin, destination, contextName);
		  vs.push_back(s);
		  long constant = maxiter * (-1);
		  vcst.push_back(constant);
		}
	    }
	  vs.push_back("");
	  vcst.push_back(0L);
	  vector < Node * >vn = vl[l]->GetAllNodesNotNested();
	  for (unsigned int n = 0; n < vn.size(); n++)
	    {
	      Node *node = vn[n];
	      if (node != head || (node == head && vn.size() == 1))
		{
		  string s = mkVariableNameSolver("n_", node, contextName);
		  vs[vs.size() - 1] = s;
		  vcst[vcst.size() - 1] = 1L;
		  solver->generate_linear_inequality(os, vs, vcst, 0);
		}
	    }
	}
    }
}

vector < Node * > IPETAnalysis::IsolatedNopNode( Cfg * c)
 {
   vector < Node * >vn, vn_ori;
   Node *CurrentNode;

   vn_ori = c->GetAllNodes();
   for (unsigned int j = 0; j < vn_ori.size(); j++)
     {
       CurrentNode = vn_ori[j];
       if (!CurrentNode->isIsolatedNopNode())	// filtering nop ARM
	 vn.push_back(CurrentNode);
     }
   return vn;
 }

void IPETAnalysis::generateConstraints_PIPELINE_ICACHE_DCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, vector < Edge * > &ve, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  // TODO : Ajouter un warning car DCACHE non pris en compte dans analyse pipeline!
  generateConstraints_PIPELINE_CACHE(c, vn, contexts, vid, vwcet);
  generateConstraints_inside_CACHE_BB(os, vn, contexts);
  generateConstraints_PIPELINE_CACHE_edges(os, ve, contexts);  // Generate flow constraints inside the BB
}

void IPETAnalysis::generateConstraints_PIPELINE_ICACHE_PERFECTDCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, vector < Edge * > &ve, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  // to be modified : Perfect DataCache
  generateConstraints_PIPELINE_ICACHE_DCACHE(os, c, vn, ve, contexts, vid, vwcet);  
}

void IPETAnalysis::generateConstraints_NOPIPELINE_ICACHE_DCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  ComputeNodesExecutionTime_NOPIPELINE_CACHE(vn, contexts, false, false);
  generateConstraints_NOPIPELINE_CACHE(vn, contexts, vid, vwcet);
  generateConstraints_inside_CACHE_BB(os, vn, contexts);
}

void IPETAnalysis::generateConstraints_NOPIPELINE_ICACHE_PERFECTDCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  ComputeNodesExecutionTime_NOPIPELINE_CACHE(vn, contexts, false, true);
  generateConstraints_NOPIPELINE_CACHE(vn, contexts, vid, vwcet);
  generateConstraints_inside_CACHE_BB(os, vn, contexts);
}

void IPETAnalysis::generateConstraints_NOPIPELINE_PERFECTICACHE_DCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  ComputeNodesExecutionTime_NOPIPELINE_CACHE(vn, contexts, true, false);
  generateConstraints_NOPIPELINE_CACHE(vn, contexts, vid, vwcet);
  generateConstraints_inside_CACHE_BB(os, vn, contexts);
}

void IPETAnalysis::generateConstraints_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE(ostringstream & os, Cfg * c, vector < Node * >&vn, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  ComputeNodesExecutionTime_NOPIPELINE_NOCACHE(vn, contexts);
  generateConstraints_NOPIPELINE_NOCACHE(vn, contexts, vid, vwcet);
}


/* Fill-in vid and WCET to be able to generate the objective function
   This part is dependent on the type of IPET method selected,
   which fixes the naming convention of variables
*/
void IPETAnalysis::generateConstraints_IPET_selected_method(ostringstream & os, Cfg * c, vector < Node * >&vn, vector < Edge * > &ve, const ContextList & contexts, vector < string > &vid, vector < long >&vwcet)
{
  switch (method)
    {
    case METHOD_PIPELINE_ICACHE_DCACHE:
      generateConstraints_PIPELINE_ICACHE_DCACHE(os, c, vn, ve, contexts, vid, vwcet );
      break;

    case METHOD_PIPELINE_ICACHE_PERFECTDCACHE:
      generateConstraints_PIPELINE_ICACHE_PERFECTDCACHE(os, c, vn, ve, contexts, vid, vwcet );
      break;

    case METHOD_NOPIPELINE_ICACHE_DCACHE:
      generateConstraints_NOPIPELINE_ICACHE_DCACHE(os, c, vn, contexts, vid, vwcet );
      break;

    case METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE:
      generateConstraints_NOPIPELINE_ICACHE_PERFECTDCACHE(os, c, vn, contexts, vid, vwcet );
      break;
      
    case METHOD_NOPIPELINE_PERFECTICACHE_DCACHE:
      generateConstraints_NOPIPELINE_PERFECTICACHE_DCACHE(os, c, vn, contexts, vid, vwcet );
      break;

    case METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE:
      generateConstraints_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE(os, c, vn, contexts, vid, vwcet );
      break;

    default:
      ;
    }
}


void IPETAnalysis::printIPETCommand()
{
  string str_method;
  switch (method)
    {
    case METHOD_PIPELINE_ICACHE_DCACHE:
      str_method = "PIPELINE_ICACHE_DCACHE";
      break;

    case METHOD_PIPELINE_ICACHE_PERFECTDCACHE:
      str_method = "PIPELINE_ICACHE_PERFECTDCACHE";
      break;

    case METHOD_NOPIPELINE_ICACHE_DCACHE:
      str_method = "NOPIPELINE_ICACHE_DCACHE";
      break;

    case METHOD_NOPIPELINE_ICACHE_PERFECTDCACHE:
      str_method = "NOPIPELINE_ICACHE_PERFECTDCACHE";
      break;
      
    case METHOD_NOPIPELINE_PERFECTICACHE_DCACHE:
      str_method = "NOPIPELINE_PERFECTICACHE_DCACHE";
      break;

    case METHOD_NOPIPELINE_PERFECTICACHE_PERFECTDCACHE:
      str_method = "NOPIPELINE_PERFECTICACHE_PERFECTDCACHE";
      break;

    default:
      str_method = "UNKNOWN";
    }
  Logger::print("IPET method: " + str_method );
}

// ------------------------------------------------
//
// Generate constraints for one CFG
// --------------------------------
//
// Generates all structural constraints for one Cfg
//
// - os is the stream where constraints should be generated
// - c is the Cfg for which constraints have to be generated
// - vid is the vector of generated variables in the ILP system
// - vwcet is the vector of constants in the ILP system
// (these two last parameters are subsequently used to
// generate the objective function)
//
// Constraint generation is not directly done in this method.
// A generic interface for constraint generation is
// provided in Solver.h and implemented for two solvers
// (lp_solve and CPLEX)
//
// Assumes each node has a node Id (done by function
// generateNodeIds)
//
// ------------------------------------------------

bool IPETAnalysis::generateConstraints(ostringstream & os, Cfg * c, vector < string > &vid, vector < long >&vwcet)
{
  vector < Node * >vn;
  vector < Edge * >ve = c->GetAllEdges();

  vn = IsolatedNopNode(c);
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  generateConstraints_IPET_selected_method(os,c, vn, ve, contexts, vid, vwcet );
  // Generate a constraint for every BB / edge for every execution context
  // ---------------------------------------------------------------------
  generateConstraints_BB_edge_eachContext(c, os, vn, ve, contexts);
  generateConstraints_back_edges_loops(c, os, vn, contexts);

  return true;
}

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>

// -------------------------------------------
// Core of the analysis
// generate an ILP problem to compute
// the program WCET
// -------------------------------------------
bool IPETAnalysis::PerformAnalysis()
{
  if (method == NOT_YET_IMPLEMENTED) return false;

  ostringstream strc;		// objective function
  ostringstream strf;		// flow constraints
  ostringstream stde;		// declarations

  char buffer[25] = "/tmp/IPETAnalysis_XXXXXX";
  mkstemp(buffer);
  ofstream os(buffer);
  string fout = buffer;

  // Get the Cfg of the program entry point
  // --------------------------------------
  vector < string > vid;
  vector < long >vwcet;
  vector < Cfg * >lcfg = p->GetAllCfgs();
  for (unsigned int c = 0; c < lcfg.size(); c++)
    {
      generateNodeIds(strc, lcfg[c]);
    }

  for (unsigned int c = 0; c < lcfg.size(); c++)
    {
      if (!call_graph->isDeadCode(lcfg[c]))
	generateConstraints(strc, lcfg[c], vid, vwcet);
    }

  generateCallConstraints(strc, p);
  solver->generate_objective_function(strf, vid, vwcet);
  solver->generate_declarations(stde, AnalysisHelper::unicity(vid));

  // Constraint for entry point
  {
    Cfg *c = config->getEntryPoint();
    assert(c != NULL);
    assert(!(c->IsEmpty()));
    Node *start_node = c->GetStartNode();
    long start_id = getIntegerAttribute(start_node, InternalAttributeId);
    ostringstream nid;
    nid << start_id;
    vector < string > vs;
    vs.push_back("n_" + nid.str() + "_c0");
    solver->generate_equality(strc, vs, 1);
  }

  // Write everything (objective first, constraints, then declarations last) in the output file Objective function
  os << strf.str();
  // All the constraints (except statistics)
  os << strc.str();
  // Declarations
  os << stde.str();
  os.close();

  // Launch the solver
  // char fileNameTemplate[19] = "/tmp/solver_XXXXXX";
  // string tmpFileName = mktemp(fileNameTemplate);  replaced by mkstemp (lbesnard) because ...
  // Compiler: the use of `mktemp' is dangerous, better use `mkstemp'

  string tmpFileName;
  if (!Utl::mktmpfile("/tmp/solver_", tmpFileName) ) return false;
  if (!solver->solve(fout, tmpFileName))
    return false;

  // Parse the solver output
  string wcet;
  if (solver->parse_output(tmpFileName, wcet)) ;

  Cfg *c = config->getEntryPoint();

  // Attach result to entry point
  if (this->generate_wcet_information)
    {
      SerialisableStringAttribute ba(wcet);
      assert(c->HasAttribute(WCETAttributeName) == false);
      c->SetAttribute(WCETAttributeName, ba);
    }
  return true;
}
