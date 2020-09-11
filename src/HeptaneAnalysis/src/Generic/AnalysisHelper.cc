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

#include <queue>
#include "Analysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Specific/HtmlPrint/HtmlPrint.h"
#include "Generic/AnalysisHelper.h"

#define PREFIX_CONTEXT "_c"

// Useful functions
// ----------------

// --------------------------------------------------
// Call function f for every node of cfg c (once)
// Returns false if one of the function returns false
// The function is called for ALL nodes
// (it does not stop once the function has returned false)
//
// NB: function f is called on f's nodes only, regardless
//     of the nodes types (call, BB)
// --------------------------------------------------
bool AnalysisHelper::applyToAllCfgNodes(Cfg * c, t_node_function f, void *param)
{
  Node *CurrentNode;

  bool res = true;
  vector < Node * >vn = c->GetAllNodes();
  for (unsigned int i = 0; i < vn.size(); i++)
    {
      CurrentNode = vn[i];
      if (!CurrentNode->isIsolatedNopNode())	// filtering nop ARM
	{
	  if ((*f) (c, CurrentNode, param) == false)
	    res = false;
	}
    }
  return res;
}

// --------------------------------------------------
// Call function f for every node of the entry point
//   of program p and every node of any callee
// Function f is called once per node
// Returns false if one of the function returns false
// This method follows the CFG edges to determine which
//    nodes are reachable. Therfore, function f
//    should not modify the control flow.
// --------------------------------------------------
bool AnalysisHelper::applyToAllNodesRecursive(Program * p, t_node_function f, void *param)
{
  /** Statistics on the number of applications of function calls
     on nodes (methods applyToAllCfgNodes and applyToAllNodesRecursive).
     ncalls is the total number of function calls, ntrue is the number of
     calls returning true */
  int ncalls, ntrue;
  bool res = true;

  NonSerialisableIntegerAttribute visited(1);
  ncalls = ntrue = 0;

  Cfg *c = config->getEntryPoint();
  assert(c != NULL);
  assert(!(c->IsEmpty()));
  Node *n = c->GetStartNode();
  vector < Node * >vn;
  vn.push_back(n);
  n->SetAttribute(VisitedInternalAttributeName, visited);

  while (vn.size() != 0)
    {
      Node *n = vn[0];
      c = n->GetCfg();
      assert(n->HasAttribute(VisitedInternalAttributeName));

      // Scan the successors of n in the CFG
      vector < Node * >sucs = c->GetSuccessors(n);
      for (unsigned int i = 0; i < sucs.size(); i++)
	{
	  if (sucs[i]->HasAttribute(VisitedInternalAttributeName) == false)
	    {
	      sucs[i]->SetAttribute(VisitedInternalAttributeName, visited);
	      vn.push_back(sucs[i]);
	    }
	}

      // Scan the callees if n is a call node
      if (n->IsCall())
	{
	  Cfg *callee = n->GetCallee();
	  assert(callee != NULL);
	  if (! callee->IsExternal())
	    {
	      Node *entry = callee->GetStartNode();
	      if (! entry->HasAttribute(VisitedInternalAttributeName))
		{
		  entry->SetAttribute(VisitedInternalAttributeName, visited);
		  vn.push_back(entry);
		}
	    }
	}

      // Call function f
      ncalls++;
      if ((*f) (c, n, param) == false) res = false; else ntrue++;

      // Erase just treated element
      vn.erase(vn.begin());
    }

  // Remove all "visited" attributes
  vector < Cfg * >lcfg = p->GetAllCfgs();
  for (unsigned int i = 0; i < lcfg.size(); i++)
    {
      vector < Node * >vn = lcfg[i]->GetAllNodes();
      for (unsigned int n = 0; n < vn.size(); n++)
	{
	  vn[n]->RemoveAttribute(VisitedInternalAttributeName);
	}
    }

  return res;
}

// ----------------------------------------------------------------
// Returns the start address of a node
// Assumes and checks the node is a BasicBlock
// Assumes there is an "address" attribute for every
//   instruction of the basic block
// ----------------------------------------------------------------
t_address AnalysisHelper::getStartAddress(Node * n)
{
  vector < Instruction * >vi = n->GetAsm();
  assert(vi.size() != 0);
  assert(vi[0]->HasAttribute(AddressAttributeName));
  AddressAttribute attr = (AddressAttribute &) vi[0]->GetAttribute(AddressAttributeName);
  return (attr.getCodeAddress());
}

//-----------------------------------------------
// compute context for each cfg
//
// like main#1#foo for the foo function
// called in main by the first call node
//
// the caller node has the call_number
// and the next node in the same cfg has the call_next_number
//
//-----------------------------------------------
bool AnalysisHelper::computeContext(Program * p)
{
  resetContext(p);
  
  vector < Cfg * >cfgs = p->GetAllCfgs();

  // Create the context tree which hold data for all contexts.
  ContextTree contexts = ContextTree();
  contexts.initialise(config->getEntryPoint());

  // Attach an empty context list to each cfg.
  ContextList l;
  for (size_t c = 0; c < cfgs.size(); ++c)
    {
      cfgs[c]->SetAttribute(ContextListAttributeName, l);
    }

  // Add each context to the concerned cfg context list.
  for (size_t c = 0; c < contexts.getContextsCount(); ++c)
    {
      Context *context = contexts.getContext(c);
      assert(context);
      Cfg *cfg = context->getCurrentFunction();
      assert(cfg);
      assert(cfg->HasAttribute(ContextListAttributeName));

      ContextList & contexts = (ContextList &) cfg->GetAttribute(ContextListAttributeName);
      contexts.push_back(context);
    }

  // Preserve the context tree by attaching it to the program.
  // Hence, the tree and associated context will be freed/cloned 
  // upon destruction/cloning of the program.
  p->SetAttribute(ContextTreeAttributeName, contexts);
  return true;
}

static bool EmptyIntersect(Loop * l1, Loop * l2)
{
  vector < Node * >vn1 = l1->GetAllNodes();
  vector < Node * >vn2 = l2->GetAllNodes();
  for (unsigned int i1 = 0; i1 < vn1.size(); i1++)
    {
      for (unsigned int i2 = 0; i2 < vn2.size(); i2++)
	{
	  if (vn1[i1] == vn2[i2])
	    return false;
	}
    }
  return true;
}

/* Checks every instruction in the node contains an address attribute */
static bool CheckInstrHaveAddresses(Cfg * c, Node * n, void *param)
{
  vector < Instruction * >vi = n->GetAsm();
  for (size_t i = 0; i < vi.size(); i++)
    {
      if (vi[i]->HasAttribute(AddressAttributeName) == false)
	return false;
    }
  return true;
}

/* Check program for WCET analyzability (tests useful for all or almost all analyses, performed only once)
   Permormed checks:
      - no cycle in call graph
      - loops have loop bounds attached
      - loops are properly nested
      - instructions have addresses attached

      Exits the analysis at first error found */
void AnalysisHelper::ProgramCheck(Program * p)
{

  // Check call that the program call graph is not cyclic, exit if cyclic
  CallGraph cg(p);
  if (cg.isCyclic())
    {
      Logger::addFatal("Analysis::ProgramCheck: the call graph is cyclic");
    }

  // Check that instructions have addresses attached
  if (AnalysisHelper::applyToAllNodesRecursive(p, CheckInstrHaveAddresses, NULL) == false)
    {
      Logger::addFatal("Analysis::ProgramCheck: instructions should have addresses attached");
    }

  // Check that all loops are bounded
  vector < Cfg * >lc = p->GetAllCfgs();
  for (unsigned int c = 0; c < lc.size(); c++)
    {

      // Check loop bounds
      if (!cg.isDeadCode(lc[c]))
	{
	  vector < Node * >vn = lc[c]->GetAllNodes();
	  // Check all loops have a maxiter
	  vector < Loop * >vl = lc[c]->GetAllLoops();
	  for (unsigned int l = 0; l < vl.size(); l++)
	    {
	      if (vl[l]->HasAttribute(MaxiterAttributeName) == false)
		{
		  Logger::addFatal("Analysis::ProgramCheck: all loops must have a loop bound attached");
		}
	    }
	}
    }

  // Check loops are properly nested
  for (unsigned int i = 0; i < lc.size(); i++)
    {
      Cfg *c = lc[i];
      vector < Loop * >vl = c->GetAllLoops();
      for (unsigned int l1 = 0; l1 < vl.size(); l1++)
	{
	  for (unsigned int l2 = 0; l2 < vl.size(); l2++)
	    {
	      if (!vl[l1]->IsNestedIn(vl[l2]) && !vl[l2]->IsNestedIn(vl[l1]) && !EmptyIntersect(vl[l1], vl[l2]))
		{
		  Logger::addFatal("Analysis::ProgramCheck: loops should be properly nested");
		}
	    }
	}
    }
}

/*
  Compute the backedges for a program p using the call_graph for testing the dead code.
*/
set < Edge * >AnalysisHelper::compute_backedges(Program * p, CallGraph * call_graph)
{
  set < Edge * >backedges;
  vector < Cfg * >cfgs = p->GetAllCfgs();
  for (size_t i = 0; i < cfgs.size(); i++)
    {
      if (!call_graph->isDeadCode(cfgs[i]))
	{
	  vector < Loop * >loops = cfgs[i]->GetAllLoops();
	  for (size_t j = 0; j < loops.size(); j++)
	    {
	      vector < Edge * >edges = loops[j]->GetBackedges();
	      backedges.insert(edges.begin(), edges.end());
	    }
	}
    }
  return backedges;
}

/* return "true" if at least one caller of context c is in a loop, "false" otherwise */
bool AnalysisHelper::CallerInLoop(Context * c)
{
  bool found = false;
  while (c->getCallerNode() != NULL && !found)
    {
      Node *caller = c->getCallerNode();
      Cfg *caller_cfg = caller->GetCfg();
      vector < Loop * >loop_caller_cfg = caller_cfg->GetAllLoops();
      for (size_t i = 0; i < loop_caller_cfg.size(); i++)
	{
	  if (loop_caller_cfg[i]->FindInLoop(caller))
	    {
	      found = true;
	      break;
	    }
	}
      c = c->getCallerContext();
    }
  return found;
}

void AnalysisHelper::AttributeAllInstructions(Node * n, string attrName, SerialisableStringAttribute A)
{
  vector < Instruction * >vi = n->GetAsm();
  for (size_t i = 0; i < vi.size(); i++)
    {
      vi[i]->SetAttribute(attrName, A);
    }
}

bool AnalysisHelper::checkInstructionsAttributed(Node * n, string attrName)
{
  vector < Instruction * >vi = n->GetAsm();
  for (size_t i = 0; i < vi.size(); i++)
    {
      if (!vi[i]->HasAttribute(attrName)) { return false; }
    }
  return true;
}

/* Resets the shared attributes of a program (p).*/
void AnalysisHelper::resetContext(Program * p)
{
  Cfg * vcfg;
  if ( p->HasAttribute(ContextTreeAttributeName))
    {
      removeContextualAttributes(p);
      p->RemoveAttribute(ContextTreeAttributeName);
      vector < Cfg * >cfgs = p->GetAllCfgs();
      for (size_t c = 0; c < cfgs.size(); ++c)
	{
	  vcfg = cfgs[c];
	  vcfg-> RemoveAttribute(StackInfoAttributeName);
	  vcfg-> RemoveAttribute(ContextListAttributeName);
	  vcfg->RemoveAttribute(WCETAttributeName);
	}
    }
}

/* Resets the contextual shared attribute (atrName) assigned to the nodes of a program (p).*/
void AnalysisHelper::removeContextualNodesAttribute (Program * p, string attrName)
{
  vector < Cfg * >Cfgs;
  vector < Node * >callNodes;
  Cfg *CurrentCfg;
  Node * CurrentNode;
  string contextual_attrName;

  Cfgs = p->GetAllCfgs();
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      // cleaning for each context
      const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
      unsigned int nbContexts = contexts.size();
      for (unsigned int i = 0; i < nbContexts; i++)
	{
	  contextual_attrName = mkContextAttrName(attrName, contexts[i]);
	  vector < Node * >Nodes = CurrentCfg->GetAllNodes();
	  for (unsigned int j = 0; j < Nodes.size(); j++)
	    {
	      CurrentNode = Nodes[j];
	      CurrentNode->RemoveAttribute(contextual_attrName);
	    }
	}
    }
}

/* Resets the contextual shared attribute (atrName) assigned to the edges of a program (p).*/
void AnalysisHelper::removeContextualEdgesAttribute (Program * p, string attrName)
{
  vector < Cfg * > Cfgs;
  Cfg *CurrentCfg;
  string contextual_attrName;

  Cfgs = p->GetAllCfgs();
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      // cleaning for each context
      const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
      unsigned int nbContexts = contexts.size();
      for (unsigned int i = 0; i < nbContexts; i++)
	{
	  contextual_attrName = mkContextAttrName(attrName, contexts[i]);
	  vector < Edge * >Edges = CurrentCfg->GetAllEdges();
	  for (unsigned int j = 0; j < Edges.size(); j++)
	    {
	      Edges[j]->RemoveAttribute(contextual_attrName);
	    }
	}
    }
}


/* Resets the contextual shared attribute (atrName) assigned to the instructions of a program (p).*/
void AnalysisHelper::removeContextualInstructionsAttribute (Program * p, string attrName)
{
  vector < Cfg * > Cfgs;
  Cfg *CurrentCfg;
  string contextual_attrName;
  Node *CurrentNode;

  Cfgs = p->GetAllCfgs();
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
      unsigned int nbContexts = contexts.size();
      for (unsigned int i = 0; i < nbContexts; i++)
	{
	  contextual_attrName = mkContextAttrName(attrName, contexts[i]);
	  vector < Node * >Nodes = CurrentCfg->GetAllNodes();
	  for (unsigned int j = 0; j < Nodes.size(); j++)
	    {
	      CurrentNode = Nodes[j];
	      vector < Instruction * >vi = CurrentNode->GetAsm();
	      for (size_t i = 0; i < vi.size(); i++)
		{
		  vi[i]->RemoveAttribute(contextual_attrName);
		}
	    }
	}
    }
}

/* Remove the contextual attributes assigned to a program(p). */
void AnalysisHelper::removeContextualAttributes(Program * p)
{
  // AddressAnalysis: from GlobalAttributes.h
  removeContextualInstructionsAttribute(p, string(AddressAttributeName) ); 

  // From SharedAttributes.h
  // Pipeline contextual attributes
  removeContextualNodesAttribute(p, string(NodeExecTimeFirstAttributeName) );
  removeContextualNodesAttribute(p, string(NodeExecTimeNextAttributeName) );

  removeContextualEdgesAttribute(p, string(DeltaFFAttributeName) );
  removeContextualEdgesAttribute(p, string(DeltaFNAttributeName) );
  removeContextualEdgesAttribute(p, string(DeltaNFAttributeName) );
  removeContextualEdgesAttribute(p, string(DeltaNNAttributeName) );

  removeContextualNodesAttribute(p, string(CallDeltaFirstAttributeName) );
  removeContextualNodesAttribute(p, string(CallDeltaNextAttributeName) );
  removeContextualNodesAttribute(p, string(ReturnDeltaFirstAttributeName) );
  removeContextualNodesAttribute(p, string(ReturnDeltaNextAttributeName) );

  // DCacheAnalysis, ICacheAnalysis
  removeContextualInstructionsAttribute(p, CACAttributeNameData(1));
  int levelmax =config->getMaxLevelCacheAnalysis();
  for (int level = 1; level <= levelmax; level++)
    {
      removeContextualInstructionsAttribute(p, CHMCAttributeNameData(level) );
      removeContextualInstructionsAttribute(p, BlockCountAttributeName(level));
      removeContextualInstructionsAttribute(p, CACAttributeNameData(level + 1));
      removeContextualInstructionsAttribute(p, CHMCAttributeNameCode(level) );
      removeContextualInstructionsAttribute(p, AGEMustAttributeNameCode(level));
      removeContextualInstructionsAttribute(p, AGEPSAttributeNameCode(level) );  
      removeContextualInstructionsAttribute(p, CACAttributeNameCode(level + 1));
    }

  // Not used attributes: HitRatioAttributeName

  // currently, a specific context management for FrequencyAttributeName
  removeFrequencyAttribute(p); 
}


/* Remove the context management for FrequencyAttributeName (currently not managed as the other contextual attributes). */
void AnalysisHelper::removeFrequencyAttribute(Program * p)
{
  vector < Cfg * > Cfgs;
  Cfg *CurrentCfg;
  string contextual_attrName;

  Cfgs = p->GetAllCfgs();
  for (unsigned int c = 0; c < Cfgs.size(); c++)
    {
      CurrentCfg = Cfgs[c];
      vector < Node * >vn = CurrentCfg->GetAllNodes();
      const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute(ContextListAttributeName);
      for (unsigned int i = 0; i < vn.size(); i++)
	{
	  unsigned int nc = contexts.size();
	  for (unsigned int ic = 0; ic < nc; ic++)
	    {
	      vn[i]->RemoveAttribute (getContextAttrFrequencyName(contexts[ic]->getStringId()));
	    }
	}
    }
}

/** Contextual attribute Names.
    @return the string <attr> + '#' + <contextName>
 */
string AnalysisHelper::mkContextAttrName(string attr, string contextName)
{
  return attr + '#' + contextName;
}

/** Contextual attribute Names.
    @return the string <attr> + '#' + nameOf(context)
 */
string AnalysisHelper::mkContextAttrName(string attr, Context *context)
{
  string vstr = context->getStringId();
  return mkContextAttrName(attr, vstr);
}


/** @return the value of the attribute ( f(attr, contextName)) assigned to an instruction (instr).*/
int AnalysisHelper::getInstrValueAttr(Instruction *instr, string attr, string contextName)
{
  string vstr = mkContextAttrName(attr,contextName);
  SerialisableIntegerAttribute & vAttr = (SerialisableIntegerAttribute &) instr->GetAttribute(vstr);
  return vAttr.GetValue();
}


/** @return the value of the attribute ( f(attr, contextName)) assigned to an edge (e).*/
int AnalysisHelper::getEdgeValueAttr(Edge & e, string attr, string contextName)
{
  string vstr = mkContextAttrName(attr,contextName);
  SerialisableIntegerAttribute & vAttr = (SerialisableIntegerAttribute &) e.GetAttribute(vstr);
  return vAttr.GetValue();
}
  
/** @return the value of the attribute ( f(attr, contextName)) assigned to a node (n).*/
int AnalysisHelper::getNodeValueAttr(Node *n, string attr, string contextName)
{
  string vstr = mkContextAttrName(attr,contextName);
  SerialisableIntegerAttribute & vAttr = (SerialisableIntegerAttribute &) n->GetAttribute(vstr);
  return vAttr.GetValue();
}
  
/**
   @return the attribute name of the Frequency in a context (ic).
   (Specific management of the contexts for the frequency)
 */
string  AnalysisHelper::getContextAttrFrequencyName(string vcontext)
{
  ostringstream tmp;

  tmp << FrequencyAttributeName << PREFIX_CONTEXT << vcontext;    
  return tmp.str();
}


string  AnalysisHelper::mkVariableNameSolver(string prefix, long v, string vcontext )
{
  ostringstream tmp;

  tmp << prefix << v <<  PREFIX_CONTEXT << vcontext;
  return tmp.str();
}

string AnalysisHelper::mkEdgeVariableNameSolver(string prefix, long source, long target, string vcontext)
{
  ostringstream tmp;

  tmp << prefix << source << '_' << target <<  PREFIX_CONTEXT << vcontext;
  return tmp.str();
}



unsigned int AnalysisHelper::getInstrIntAttr(Instruction* vinstr, string attrName, Context * context )
{
  SerialisableIntegerAttribute vattr = (SerialisableIntegerAttribute &) vinstr->GetAttribute(mkContextAttrName(attrName, context));
  return vattr.GetValue();
}


string AnalysisHelper::getInstrStringAttr(Instruction* vinstr,  string attrName, Context *context)
{
  SerialisableStringAttribute vattr=(SerialisableStringAttribute &) vinstr->GetAttribute(mkContextAttrName(attrName, context));
  return vattr.GetValue();
}

vector < string > AnalysisHelper::unicity(vector < string > &vid)
{
  vector < string > nvid;
 std::vector<string>::iterator it;
 string v;

 unsigned int nvars = vid.size ();
 for (unsigned int i = 0; i < nvars; i++)
   {
     v = vid[i];
     it = find (nvid.begin(), nvid.end(), v);
     if ( it == nvid.end()) nvid.push_back(v);
    }
  return nvid;
}

void  AnalysisHelper::insertContextualSuccessorsExcludingBackEdges(ContextualNode  &current, set < ContextualNode > &vSet, set < Edge * >& backedges)
{
  ContextualNode CurrentSucc;
  bool bInsert;
  vector < ContextualNode > succ = GetContextualSuccessors(current);
  Cfg * vCfg = current.node->GetCfg();

  for (size_t i = 0; i < succ.size(); i++)
    {
      CurrentSucc = succ[i];
      if (vCfg == CurrentSucc.node->GetCfg())
	{
	  Edge *edge = vCfg->FindEdge(current.node, CurrentSucc.node);
	  bInsert = (backedges.find(edge) == backedges.end()); // If backedge, ignore it
	}
      else
	bInsert = true;
      if (bInsert) vSet.insert(CurrentSucc);
    }
}



/** Insert in vSet the contextual successors of vContNode. */
void  AnalysisHelper::insertContextualSuccessors(ContextualNode & vContNode, set < ContextualNode > &vSet)
{
  vector < ContextualNode > succ = GetContextualSuccessors(vContNode);
  vSet.insert(succ.begin(), succ.end());
}

/*
  @return the initial contextual node of the program.
  (ie first context of the entry point of the program, the start node of the entry point)
*/
set < ContextualNode >  AnalysisHelper::initWork( )
{
  set < ContextualNode > work;
  Cfg *entry_point = config->getEntryPoint();
  Node *start_node = entry_point->GetStartNode();
  Context *root_context = ((const ContextList &)entry_point->GetAttribute(ContextListAttributeName))[0];
  ContextualNode entry_node(root_context, start_node);
  work.insert(entry_node);
  return work;
}



/* Debugging: it prints the code of the instructions of a set of contextual nodes (work). */
void  AnalysisHelper::printSet(set < ContextualNode > &work, string vcaller)
{
  string In=vcaller + "::printSet";
  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      string idCurrentContext = current.context->getStringId();
      cout << In << ", idCurrentContext = " << idCurrentContext << endl;
      vector < Instruction * >vi = current.node->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  Instruction *vinstr = vi[i];
	  cout << In << ", instruction [ " << i << "]= " << vinstr->GetCode() << endl;
	}
    }
}

bool  AnalysisHelper::FilterBackedge(Node * current, Node *pred, set < Edge * >&backedges)
{
  bool b = true;
  if (pred->GetCfg() == current->GetCfg())
    {
      Edge *edge = current->GetCfg()->FindEdge(pred, current);
      b = (backedges.find(edge) == backedges.end());	//If backedge ignore it
    }
  return b;
}

bool AnalysisHelper::getCallerNode(Node *returnNode, Node * *previous_call)
{
  int vindex = -1;
  const vector < Node * >&predecessors = returnNode->GetCfg()->GetPredecessors(returnNode);
  for (size_t i = 0; i < predecessors.size (); ++i)
    {
      if (predecessors[i]->IsCall())
	{
	  if (vindex != -1) return false; // one and only one
	  vindex = i;
	}
    }
  if (vindex == -1) return false;
  (*previous_call) = predecessors[vindex];
  return true;
}
