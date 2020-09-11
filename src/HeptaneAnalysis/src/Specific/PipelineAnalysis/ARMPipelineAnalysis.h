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
                             ARMPipelineAnalysis
This is the entry point of the pipeline analysis for ARM architecture.

*****************************************************************/

#include "PipelineAnalysis.h"

// using namespace std;

#ifndef ARMPIPELINEANALYSIS_H
#define ARMPIPELINEANALYSIS_H

class ARMPipelineAnalysis:public PipelineAnalysis
{
 protected:

  /** Implementation of Pipeline::scheduleFirstInst() for ARM */
  void scheduleFirstInst (Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first);
  /** Implementation of  Pipeline::scheduleNextInst() for ARM  */
  void scheduleNextInst (Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first);

 private:
  /** return the latency of the operand instruction(codeinstr), requiring the "barrel shifter" when BarrelShifterUsed. */
  int getLatency( const string codeInstr , bool BarrelShifterUsed);

 public:

  /** Constructor */
  ARMPipelineAnalysis (Program * p, int nbcache);
  
  /** Performs the analysis
      @return true if successful, false otherwise.
  */
  bool PerformAnalysis ();

};
#endif
