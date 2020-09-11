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
The MIPSAddressAnalysis and ARMAddressAnalysis classes inhrit of this class.

This analysis is very basic in the sense:
- it does not consider pointers 
- it is perfomed on an intra-basic block basis

*****************************************************************/


#ifndef STACKANALYSIS_H
#define STACKANALYSIS_H


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

#include "Analysis.h"

using namespace std;
class StackAnalysis: public Analysis
{
 
 private:
  long spinit; ///< Initial value of the stack pointer
  bool VerifModifStack (Cfg * cfg);
  int getStackMaxOffset (Cfg * cfg, int stack_size);
  void stackAnalysis(Cfg * acfg);  ///< Stack analysis for a CFG
  bool stackAnalysisInit(CallGraph &callgraph); ///< @return true if there is no modification of the stack frame in the middle of a function, false toterwise.
      

// The folowings are architecture dependent.
  virtual int getStackMaxOffset(string s_instr, int StackMaxOffset)=0;
  virtual int getStackSize (Cfg * cfg)=0;///< return the size of the stack frame
 
protected:
  bool stackAnalysis (); ///< Stack analysis starting point
    
public:
  StackAnalysis (Program * p, int sp);
};

#endif
