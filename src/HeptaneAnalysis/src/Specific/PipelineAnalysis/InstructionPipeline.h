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

#ifndef INSTRUCTIONPIPELINE_H
#define INSTRUCTIONPIPELINE_H

#include <string>
#include <vector>
#include <iostream>
using namespace std;


/**
 * Represent the state (clock tick and ressource used) of an
 * instruction for a particular pipeline stage.
 */
class pipeStage
{
public:
  ///clock tick when the instruction is located at this pipeline stage
  unsigned int tick;

  ///Set of register read by the instruction at this pipeline stage
    vector < string > in;
  ///Set of register write by the instruction at this pipeline stage
    vector < string > out;

  /// Functionnal units used by the instruction at this pipeline stage
    vector < string > FU;
};



/** 
 * contain the timing and ressources of one instruction for each pipeline stage 
 */
class InstructionPipeline
{
private:
  vector < pipeStage > *Pipe;
  unsigned int currentStage;

private:
  unsigned int getDependencySource (vector < string > &inputs,  string & source );
public:
  /** create an instruction pipeline of "size" stages */
    InstructionPipeline (unsigned int size);
   ~InstructionPipeline (void);

   /** insert instruction in the first stage of the pipeline
       return the corresponding pipeStage structure*/
  pipeStage *insertInstruction (unsigned int tick);

  /** propagate the instruction to the next stage
      return the corresponding pipeStage structure*/
  pipeStage *propagateInstruction (unsigned int lat);

  /** return the pipeStage structure of the stage "stageNum" */
  pipeStage *getPipeStage (unsigned int stageNum);

  /** return the clock tick were the data needed by "inputs" are available
      return 0 if no dependencies
  */
  unsigned int getDependencies (vector < string > &inputs);


  string getDependencySource (vector < string > &inputs);

  /** return the clock tick were the "FU" is free
      return 0 if "FU" is not used*/
  unsigned int checkAvaliability (string FU);

  /** return the clock tick were the execution in the FU finish*/
  unsigned int execFinished (void);

  /** Print the content of the instruction pipeline */
  void Print (void);
};


#endif
