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
                             ARMAddressAnalysis

This is the entry point of the dataflow analysis of 
data and stack addresses for ARM architecture.

This analysis is very basic in the sense:
- it does not consider pointers 
- it is perfomed on an intra-basic block basis

*****************************************************************/

#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <set>
#include <map>

#include <errno.h>

#include "AddressAnalysis.h"
#include "InstructionARM.h"
#include "RegState.h"

using namespace std;

#ifndef ARMADDRESSANALYSIS_H
#define ARMADDRESSANALYSIS_H

class ARMAddressAnalysis:public AddressAnalysis
{
 private:
  bool extractRegVal( string mem_pattern, string &reg, string & val);
  long GetOffsetValue(string asm_code);
  void analyzeStack (Cfg * cfg, Instruction * Instr, long offset, string access, int sizeOfMemoryAccess, bool precision, Context *context);
  void analyzeReg (Instruction * Instr, long addr, string access, int sizeOfMemoryAccess, bool precision);
  int getStackSize(Instruction * vinstr);
 protected:
  int getStackSize (Cfg * cfg);
  int getStackMaxOffset(string s_instr, int StackMaxOffset);
  RegState* NewRegState(int stackSize);
  bool importCallerArguments(AbstractRegMem &vAbstractRegMemCaller, AbstractRegMem &vAbstractRegMemCalled);
 public:

  /** Constructor */
  ARMAddressAnalysis (Program * p, int sp);
  
  /** Checks if all required attributes are in the CFG
      @return true if successful, false otherwise
  */
  bool CheckInputAttributes ();
  
  /** Performs the analysis
      @return true if successful, false otherwise.
  */
  bool PerformAnalysis ();
  
  /** remove all StackInfoAttributes (never used elsewhere) */
  void RemovePrivateAttributes ();

  bool setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state, Context *context);
  
};
#endif
