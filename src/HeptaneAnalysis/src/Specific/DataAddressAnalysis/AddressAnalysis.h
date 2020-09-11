/* ------------------------------------------------------------------------------------

   Copyright IRISA, 2003-2017

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

   ------------------------------------------------------------------------------------ */

/*****************************************************************
                             AddressAnalysis

This is the entry point of the dataflow analysis of 
data and stack addresses for a architecture (independant of the architecture).
The MIPSAddressAnalysis and ARMAddressAnalysis classes inhirit of this class.

This analysis is very basic in the sense:
- it does not consider pointers 
- it is perfomed on an intra-basic block basis

*****************************************************************/


#ifndef ADDRESSANALYSIS_H
#define ADDRESSANALYSIS_H


#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <set>
#include <map>

#include <errno.h>
#include "RegState.h"

#include "StackAnalysis.h"
#include "AbstractRegMem.h"
#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"
using namespace std;

/*************************************************************************************************************************
 Names of internal attributes
 **************************************************************************************************************************/
// AddressInName _in and AddressOutName, of type AbstractRegMemAttribute, attached to nodes
#define AddressInName  "Addr_in"
#define AddressOutName "Addr_out"

using namespace std;



class AddressAnalysis: public StackAnalysis
{
 private:
  Program *p;
  CallGraph *call_graph;

 private:
  bool CheckExternalCfg(Program *p); ///< return true if the program reference an external CFG, false otherwise.

 protected:
  SymbolTableAttribute symbol_table; // Symbol table (set in intraBlockDataAnalysis())
  long spinit; ///< Initial value of the stack pointer.
  bool isCfgWithCallNode (Cfg * cfg); ///< @return true if a cfg (cfg) calls an another cfg, "false" otherwise  
  void setAddressAttribute(Instruction* vinstr, AddressInfo & info);
  void setContextualAddressAttribute(Instruction* Instr, Context *context, AddressInfo &contextual_info);
  void setPointerAccessInfo(Instruction* vinstr, string access);
  void intraBlockDataAnalysis (Cfg * vCfg, Node * aNode, RegState *state);
  void mkAddressInfoAttribute(Instruction *  Instr, string access, bool precision, string section_name, string var_name, long addr, int size);
  AddressInfo mkAddressInfo(Instruction * Instr, string access, bool precision, string section_name, string var_name, long addr, int size);

  /** @return the AbstractRegMem of a ContextualNode (current). The initial value is the given by the of the current 
      analysis provided by inAnalysisName for the context.*/
  AbstractRegMem compute_out(ContextualNode & current, string & inAnalysisName);  

  /** Address Analysis: Compute the "AddressAttribute"_in of a set of nodes (work_in). 
      @return a set of nodes for which the "AddressAttribute"_out must be computed (all the nodes have to be visited at least once). */
  set < ContextualNode > intraBlockDataAnalysis_out(set < ContextualNode > &work, set < ContextualNode > &visited);
  
  /** Address Analysis: Compute the "AddressAttribute"_in of a set of nodes (work_in). 
      @return a set of nodes for which the "AddressAttribute"_out must be computed (all the nodes have to be visited at least once). */
  set < ContextualNode > intraBlockDataAnalysis_in(set < ContextualNode > &work_in, set < ContextualNode > &visited);
  
  /** Address Analysis implemented as a data flow analysis. Set the address attribute for all nodes of the entry point.
      All nodes have to be visited at least once.
  */
  bool intraBlockDataAnalysis (); 
  
  set < ContextualNode > FixPointStepInit_out(set < ContextualNode > &work, set < Edge * >&backedges, set < ContextualNode > &visited);
  set < ContextualNode > FixPointStepInit_in(set < ContextualNode > &work_in, set < Edge * >&backedges, set < ContextualNode > &visited);
  bool FixPointInit();
  virtual bool importCallerArguments(AbstractRegMem &vAbstractRegMemCaller, AbstractRegMem &vAbstractRegMemCalled)=0;

 public:
  AddressAnalysis (Program * p, int sp);
  bool PerformAnalysis ();
  bool CheckInputAttributes ();
  void RemovePrivateAttributes ();

  virtual RegState* NewRegState(int stackSize)=0;  // Architecture dependant.
  // The folowings are architecture dependent.
  virtual bool setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state, Context *context) = 0;
  
};

#endif
