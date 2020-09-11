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


#include "Logger.h"
#include "AddressAnalysis.h"
#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"
#include "arch.h"
#include "Utl.h"

// #define TRACE(s) s
#define LOCTRACE(s) TRACE(s)
#define TEMPOTRACE(s) 


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


#define getRegMemNode(NODE, IDATTR) ((AbstractRegMemAttribute&)NODE->GetAttribute(IDATTR))
#define getRegMemContextualNode(ELEM, IDATTR) getRegMemNode(ELEM.node,IDATTR)
#define MyLocalAttribute "cfgStackArea"

/**
   Assigns to a node (n), for each context of its cfg (c) 
   - an empty "Addr_in" attribute (AddressInName)
   - an empty "Addr_out" attribute (AddressOutName)
   The attributes are different.   
*/
bool static initAddressAnalysis(Cfg * c, Node * n, void *param)
{
  AddressAnalysis *ca = (AddressAnalysis *) param;

  assert (c->HasAttribute (StackInfoAttributeName));
  StackInfoAttribute attribute = (StackInfoAttribute &) c->GetAttribute (StackInfoAttributeName);
  //get the stack size without caller of current cfg
  //  int stack_sizeWithoutCaller = attribute.getFrameSizeWithoutCaller ();
  // cout << " ++++++++++++++++++++++++++++++++++++=  getFrameSizeWithoutCaller = " << stack_sizeWithoutCaller << endl;

  int stack_sizeWithCaller = attribute.getFrameSizeWithCaller ();
  LOCTRACE( cout << " ++++++++++++++++++++++++++++++++++++=  getFrameSizeWithCaller = " << stack_sizeWithCaller << endl; );

  AbstractRegMem regMem_empty = AbstractRegMem(ca->NewRegState((stack_sizeWithCaller/4) + 1 ));
  AbstractRegMemAttribute att(regMem_empty);

  string in = AddressInName;
  string out = AddressOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      n->SetAttribute(in + currentContext, att);
      n->SetAttribute(out + currentContext, att);
    }

  return true;
}


/** 
    @return true if n is a "endnode" of c.
*/
bool static isEndNode(Cfg *c, Node * n)
{
  vector<Node*> exitNodes= c->GetEndNodes();
  return (std::find(exitNodes.begin(), exitNodes.end(), n) !=  exitNodes.end());
}


/** 
    Reseting the sp register of a abstrat register (vRegMem) when a node (n) is the start or exit node of a cfg (c).
*/
void static resetSPRegister(Cfg *c, Node * n, AbstractRegMem &vRegMem)
{
  if ( n == c->GetStartNode() || isEndNode(c, n))
    vRegMem.reset();
}


/**
   Assigns to each load/store instruction and for all context the AddressAttributeName
*/
bool static finalAddressAnalysis(Cfg * c, Node * n, void *param)
{
  AddressAnalysis *ca = (AddressAnalysis *) param;

  string in = AddressInName;
  string out = AddressOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      assert(n->HasAttribute(in + currentContext));
      AbstractRegMemAttribute& ca_attr_in = getRegMemNode(n, in + currentContext);
      string NumBlock = n->getIdentifier();
      // int IBB = Utl::string2int(NumBlock); for Debugging.
      AbstractRegMem v_in = ca_attr_in.getAbstractRegMem();
      RegState *state= v_in.getRegState();
      LOCTRACE( 
		cout << "      ---> Current context = " << getStringContextRepresentation(*context) << endl;
		cout << "           Node num = " << NumBlock << endl;
		v_in.print(););
      
      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  Instruction *vinstr = vi[i];
	  // Setting the address attribute for load and Store instructions.
	  if (ca->setLoadStoreAddressAttribute(c, vinstr, state, *context))
	    { 
	      LOCTRACE(cout << "   setLoadStoreAddressAttribute APPLIED " << endl); 
	    }
	  LOCTRACE( cout << " finalAddressAnalysis, instr = " << vinstr ->GetCode () << endl;);
	  state->simulate (vinstr);
	  LOCTRACE(state->printStates(); state->printStack(););
	}

      resetSPRegister(c, n, v_in);

      n->RemoveAttribute(in + currentContext);
      n->RemoveAttribute(out + currentContext); 
    }
  
  return true;
}


/** Set the adress attribute (info) to an instruction (vinstr). The attribute is NOT a contextual one. */
void AddressAnalysis::setAddressAttribute(Instruction* vinstr, AddressInfo & info)
{ 
  AddressAttribute attribute;

  if (vinstr->HasAttribute (AddressAttributeName))
    {
      attribute = (AddressAttribute &) vinstr->GetAttribute (AddressAttributeName);
    }
  attribute.addInfo (info);
  vinstr->SetAttribute (string (AddressAttributeName), attribute);
}

void AddressAnalysis::setContextualAddressAttribute(Instruction* Instr, Context *context, AddressInfo &contextual_info)
{
 // Attach the contextual address attribute
  string attribute_name = AnalysisHelper::mkContextAttrName(AddressAttributeName, context);
  // begin--Added for test lbesnard.
  if (Instr->HasAttribute (attribute_name))
    {
      cout << " Instr " << Instr->GetCode () << " has already the attribute +++ ***++++ " << attribute_name << endl;
    }
  // end--Added for test lbesnard.
  assert (! Instr->HasAttribute (attribute_name));
  AddressAttribute contextual_address;
  contextual_address.addInfo (contextual_info);
  Instr->SetAttribute (attribute_name, contextual_address);
}


/**
   Check if a cfg is a leaf in the call graph (no dedicated stack frame in mips).
   @return true if a cfg (cfg) calls an another cfg, "false" otherwise.
 */
bool AddressAnalysis::isCfgWithCallNode (Cfg * cfg)
{
  vector < Node * >nodes = cfg->GetAllNodes ();
  for (size_t i = 0; i < nodes.size (); i++)
    {
      if (nodes[i]->IsCall ()) return true;
    }
  return false;
}

bool AddressAnalysis::CheckExternalCfg(Program *p)
{

  vector < Cfg * >listCfg = p->GetAllCfgs ();
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      if (listCfg[cfg]->GetStartNode () == NULL) return true;
    }
  return false;
}


void AddressAnalysis::setPointerAccessInfo(Instruction* vinstr, string access)
{
  Logger::addWarning ("pointer analysis does not exist: stub all addresses can be accessed");
  LOCTRACE( cout << "        xxxxxx >>>>>> setPointerAccessInfo(" << vinstr->GetCode() << ")" << endl);
  long addr_begin = symbol_table.getCodeStartAddr ();
  long addr_end = spinit;
  mkAddressInfoAttribute(vinstr, access, false, "all", "pointer fallback", addr_begin, addr_end - addr_begin + 1);
}


void AddressAnalysis::mkAddressInfoAttribute(Instruction * Instr, string access, bool precision, string section_name, string var_name, long addr,  int size)
{
  AddressInfo info = mkAddressInfo( Instr, access, precision, section_name,  var_name,  addr,   size);
  setAddressAttribute(Instr, info);
}

AddressInfo AddressAnalysis::mkAddressInfo(Instruction * Instr, string access, bool precision, string section_name, string var_name, long addr,  int size)
{
  AddressInfo info;

  info.setType (access);
  info.setPrecision(precision);
  info.setSegment (section_name);
  info.setName (var_name);
  info.addAdrSize ( Utl::int2cstring (addr), Utl::int2cstring (size));
  LOCTRACE(info.print());
  return info;
}

//-----------Public -----------------------------------------------------------------

AddressAnalysis::AddressAnalysis (Program * prog, int sp):StackAnalysis (prog, sp)
{
  p = prog;
  spinit = sp;
  //initialization of sp value with 7FFF FFFF - virtual pages alignement
  //long spinit=2147483647 - ( 2147483647 % TAILLEPAGE);
}

bool
AddressAnalysis::PerformAnalysis ()
{
  if (CheckExternalCfg(p)) { Logger::addFatal ("AddressAnalysis: external Cfg detected in DataAddressAnalysis");}
  stackAnalysis ();
  intraBlockDataAnalysis ();

  LOCTRACE( cout << " TRACE  finalAddressAnalysis " << endl;);
  AnalysisHelper::applyToAllNodesRecursive(p, finalAddressAnalysis, (void *)this);
  Logger::print ();

  return !(Logger::getErrorState ());
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
AddressAnalysis::CheckInputAttributes ()
{
  return p->HasAttribute (SymbolTableAttributeName);
}

//nothig to remove
void
AddressAnalysis::RemovePrivateAttributes ()
{}


// -------------------------------

/*
   @return the AbstractRegMem of a ContextualNode (current). The initial value is the given by the of the current analysis provided by inAnalysisName for the context.
*/
AbstractRegMem AddressAnalysis::compute_out(ContextualNode & current, string & inAnalysisName)
{
  Node * currentNode=current.getNode();
  Cfg* vCfg=currentNode->GetCfg();
  string idCurrentContext = current.context->getStringId();

  AbstractRegMemAttribute  &ca_attr_in = getRegMemContextualNode( current, inAnalysisName + idCurrentContext);
  AbstractRegMem v_out = ca_attr_in.getAbstractRegMem();
  RegState *state = v_out.getRegState();

  string idAccessName = AnalysisHelper::mkContextAttrName(inAnalysisName, idCurrentContext);
  vector < Instruction * >vi = currentNode->GetAsm();
    
  for (size_t i = 0; i < vi.size(); i++)
    {
      LOCTRACE( cout << " compute_out = " << vi[i] ->GetCode () << endl;);
      state->simulate (vi[i]);
    }
  LOCTRACE(state->printStates();state->printStack());

  resetSPRegister(vCfg, currentNode, v_out);
  
  return v_out;
}

set < ContextualNode > AddressAnalysis::intraBlockDataAnalysis_out(set < ContextualNode > &work, set < ContextualNode > &visited)
{
  string in = AddressInName;
  string out = AddressOutName;

  LOCTRACE( cout << "intraBlockDataAnalysis_out , BEGIN " << endl;);

  set < ContextualNode > work_in;
  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      string NumBlock = current.getNode()->getIdentifier();
      AbstractRegMemAttribute &ca_attr_out = getRegMemContextualNode(current, out + current.context->getStringId());

      LOCTRACE( cout << "intraBlockDataAnalysis_out , Current context = " << getStringContextRepresentation(current.getContext()) << endl;
		cout << endl << " ++++  intraBlockDataAnalysis_out START BLOCK " << NumBlock << endl;
		ca_attr_out.getAbstractRegMem().print(););

      AbstractRegMem v_out = compute_out(current, in);

      bool b = ! ca_attr_out.getAbstractRegMem().EqualsRegisters(v_out);
      if (b) ca_attr_out.setAbstractRegMemRegisters(v_out);
      bool b1 = ! ca_attr_out.getAbstractRegMem().EqualsStacks(v_out);
      if (b1) ca_attr_out.setAbstractRegMemStack(v_out);
      b = b || b1;
      // To force the visit of all nodes
      if (visited.find(current) == visited.end())
	{
	  visited.insert(current);
	  b = true;
	}
      
      if (b) AnalysisHelper::insertContextualSuccessors(current, work_in);

      LOCTRACE(  cout << endl << " ++++  intraBlockDataAnalysis_out END BLOCK " << NumBlock << endl;
		 ca_attr_out.getAbstractRegMem().print(); );
    }

  LOCTRACE( cout << "intraBlockDataAnalysis_out , END " << endl; );
  return work_in;
}

set < ContextualNode > AddressAnalysis::intraBlockDataAnalysis_in(set < ContextualNode > &work_in, set < ContextualNode > &visited)
{
  string in = AddressInName;
  string out = AddressOutName;
  set < ContextualNode > work;
  ContextualNode current, pred;
  string idAttr;

  LOCTRACE( cout << "intraBlockDataAnalysis_IN " << endl;);
  
  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      current = *it;
      string NumBlock = current.node->getIdentifier();
      // int IBB = Utl::string2int(NumBlock);  // for Debugging.

      AbstractRegMemAttribute &ca_attr_in = getRegMemContextualNode (current, in + current.context->getStringId());
      AbstractRegMem vAbstractRegMem_in = ca_attr_in.getAbstractRegMem();

      LOCTRACE( cout << "      ---> Current context = " << getStringContextRepresentation(current.getContext()) << endl;
		cout << endl << " ++++  intraBlockDataAnalysis_in START BLOCK " << NumBlock << endl;
		vAbstractRegMem_in.print(););

      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	//it should not be the program's entry node
      bool b1 = false;
      
      int vind = getIndexElemSameCfg(predecessors, current);
      AbstractRegMem new_in = getRegMemContextualNode (predecessors[vind], out + predecessors[vind].context->getStringId()).getAbstractRegMem();
      
      for (size_t i = 0; i < predecessors.size(); i++)
	{
	  if ( (int)i != vind)
	    {
	      pred = predecessors[i];
	      AbstractRegMem vout = getRegMemContextualNode(pred, out + pred.context->getStringId()).getAbstractRegMem();
	      new_in.JoinRegisters(vout);
	      if (AreElemOfSameCfg(current, pred)) // pred is not the caller.
		{
		  new_in.JoinStacks(vout);
		  b1 = true;
		}
	    }
	}
   
      bool b = ! vAbstractRegMem_in.EqualsRegisters(new_in);
      if (b) ca_attr_in.setAbstractRegMemRegisters(new_in); 
      if (b1) 
	{
	  b1 = ! vAbstractRegMem_in.EqualsStacks(new_in);
	  if (b1) ca_attr_in.setAbstractRegMemStack(new_in);
	}

      // To force the visit of all nodes
      b = b1 || b || (visited.find(current) == visited.end());
      if (b) work.insert(current);
      LOCTRACE( cout << endl << " ++++  intraBlockDataAnalysis_in END BLOCK " << NumBlock << endl;
		vAbstractRegMem_in.print(););
    }
  LOCTRACE( cout << "intraBlockDataAnalysis_IN , END " << endl;);
  return work;
}


/* 
   FixPointStepInit_out analysis: Compute the "Stack_out" a set of nodes (work), without considering backedges.
   @return a set of nodes for which the Stack_in must be computed.
*/
set < ContextualNode > AddressAnalysis::FixPointStepInit_out(set < ContextualNode > &work, set < Edge * >&backedges, set < ContextualNode > &visited)
{
  string in = AddressInName;
  string out = AddressOutName;

  set < ContextualNode > work_in;
  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      string NumBlock = current.node->getIdentifier();
      // int IBB = Utl::string2int(NumBlock); // For debugging

      LOCTRACE( cout << endl << "========================================================================================" << endl;
		cout << " FixPointStepInit_out Num Node = " <<  NumBlock << endl;);

      AbstractRegMem v_out = compute_out(current, in);
      AbstractRegMemAttribute &ca_attr_out = getRegMemContextualNode(current, out + current.context->getStringId());
      AbstractRegMem vAbstractRegMem_out = ca_attr_out.getAbstractRegMem();

     LOCTRACE(  cout << endl << " ++++ FixPointStepInit_out START BLOCK " << NumBlock << endl;
		vAbstractRegMem_out.print(); );

      bool b = ! vAbstractRegMem_out.EqualsRegisters(v_out);
      if (b) ca_attr_out.setAbstractRegMemRegisters(v_out);
      bool b1 = ! vAbstractRegMem_out.EqualsStacks(v_out);
      if (b1)  ca_attr_out.setAbstractRegMemStack(v_out);
      if (visited.find(current) == visited.end())
	{
	  visited.insert(current);
	  b = true;
	} 
      if (b1 || b) AnalysisHelper::insertContextualSuccessorsExcludingBackEdges(current, work_in, backedges);
      
      LOCTRACE( cout << endl << " ++++  FixPointStepInit_out END BLOCK " << NumBlock << endl;
		vAbstractRegMem_out.print(););
    }
  return work_in;
}

/* FixPointStepInit_out analysis: Compute the "Stack_in" a set of nodes (work), without considering backedges.
   @return a set of nodes for which the Stack_out must be computed. */
set < ContextualNode > AddressAnalysis::FixPointStepInit_in(set < ContextualNode > &work_in, set < Edge * >&backedges, set < ContextualNode > &visited)
{
  string in = AddressInName;
  string out = AddressOutName;
  set < ContextualNode > work;
  ContextualNode current, pred;
  string idAttr;
  bool b, first;
  AbstractRegMem vout, new_in;

  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      current = *it;
      string NumBlock = current.node->getIdentifier();
      // int IBB = Utl::string2int(NumBlock);
      AbstractRegMemAttribute &ca_attr_in = getRegMemContextualNode (current, in + current.context->getStringId());      
      AbstractRegMem vAbstractRegMem_in = ca_attr_in.getAbstractRegMem();

      LOCTRACE( cout << endl << "========================================================================================" << endl;
		cout <<  endl << " ++++ FixPointStepInit_in START BLOCK " << NumBlock << endl;
		vAbstractRegMem_in.print(););
      
      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	// not the program's entry node
      first = true;  
      for (size_t i = 0; i < predecessors.size(); i++)
	{
	  pred = predecessors[i];
	  if (AreElemOfSameCfg(current, pred))
	    {
	      b =  AnalysisHelper::FilterBackedge(current.node, pred.node, backedges);
	      if (b)
		{
		  idAttr = out + pred.context->getStringId();
		  vout = getRegMemContextualNode(pred, idAttr).getAbstractRegMem();
		  if (first)
		    {
		      new_in = vout;
		      first = false;  
		    }
		  else
		    new_in.JoinStacks(vout);
		}
	    }
	}

      bool bb = true;     
      if (first)
	{
	  pred = predecessors[0];
	  new_in = getRegMemContextualNode (pred, out + pred.context->getStringId()).getAbstractRegMem();
	  Node * callernode = getContextualNodeCallerNode (current);
	  // Registers used for argmuments of a function call must be copied.
	  if (pred.node == callernode)
	    importCallerArguments(new_in, vAbstractRegMem_in);
	  // Otherwise it is a return of a function call (callernode =NULL).
	  work.insert(current);
	  bb = false;
	}

      if (bb)
	{
	  bool b = ! vAbstractRegMem_in.EqualsRegisters(new_in);
	  if (b) ca_attr_in.setAbstractRegMemRegisters(new_in); 
	  bool b1 = ! vAbstractRegMem_in.EqualsStacks(new_in);
	  if (b1) ca_attr_in.setAbstractRegMemStack(new_in);
	  b = b1 || b || (visited.find(current) == visited.end());
	  if (b) work.insert(current);
	}

    LOCTRACE( cout << endl << " ++++  FixPointStepInit_in END BLOCK " << NumBlock << endl;
	      vAbstractRegMem_in.print(););
    }
  return work;
}



/* blabla blabla blabla 
*/
bool AddressAnalysis::FixPointInit()
{
  set < ContextualNode > visited, work_in, work;
  set < Edge * >backedges = AnalysisHelper::compute_backedges(p, call_graph);
  work = AnalysisHelper::initWork();
  while (!work.empty())
    {
      work_in = FixPointStepInit_out(work, backedges, visited);
      work.clear();
      work = FixPointStepInit_in(work_in, backedges, visited);
      work_in.clear();
    }

  return true;
}

/*
  Address Analysis implemented as a data flow analysis 
  All nodes have to be visited at least once.
*/
bool AddressAnalysis::intraBlockDataAnalysis()
{
  set < ContextualNode > visited, work_in, work;

  this->call_graph = new CallGraph(p);

  // Initialising.
  symbol_table = (SymbolTableAttribute &) p->GetAttribute (SymbolTableAttributeName);
  AnalysisHelper::applyToAllNodesRecursive(p, initAddressAnalysis, (void *)this);

  FixPointInit();
  // fix point
  work = AnalysisHelper::initWork();
  while (!work.empty())
    {
      work_in = intraBlockDataAnalysis_out(work, visited);
      work.clear();
      work = intraBlockDataAnalysis_in(work_in, visited);
      work_in.clear();
    }
  return true;
}


