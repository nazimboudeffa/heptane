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

#ifndef PIPELINEANALYSIS_H
#define PIPELINEANALYSIS_H

// #define TRACE_PIPELINEANALYSIS(S) S
#define TRACE_PIPELINEANALYSIS(S) 


#include "Analysis.h"
#include "InstructionPipeline.h"

/** 
    Implementations of the pipeline analysis for the different targets.
    Some methods are specialized in MIPSPipelineAnalysis and ARMPipelineAnalysis classes.
*/
class PipelineAnalysis:public Analysis
{
 private:

  // Number of cache levels
  int nbCacheLevel;

  // String name of the classification attributes for every cache level
  map < int, string > CodeCHMC;
  map < int, string > DataCHMC;

 protected:
  unsigned int PIPELINEDEPTH;

 public:

  PipelineAnalysis (Program * p, int nbcache);

  ~PipelineAnalysis ();

  // Checks if all required attributes are in the CFG
  // Returns true if successful, false otherwise
  bool CheckInputAttributes ();

  // Performs the analysis
  // Returns true if successful, false otherwise
  bool PerformAnalysis ();

  // Remove all private attributes
  void RemovePrivateAttributes ();

 private:

  /**
     Insert the first instruction in the pipeline according its
     context and its occurence. 
     An InstructionPipeline is created for the instruction inst. Each
     pipeStage is set according to progression of the instruction. 
     This InstructionPipeline is inserted in IP.
   
     This function is architecture dependant.
  */
  virtual void scheduleFirstInst (Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first)=0;

  /**
     Insert subsequent instructions in the pipelineaccording its
     context and its occurence. 
     An InstructionPipeline is created for the instruction inst. Each
     pipeStage is set according to progression of the instruction. 
     This InstructionPipeline is inserted in IP.
    
     This function is architecture dependant.
  */
 virtual void scheduleNextInst (Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first)=0;
 
  /** 
      Used to check the validity of cache attribute on an instruction.

      Checks that attribute of name attr_name is attached
      to the instruction and contains valid values only
      NB: the attribute name is a parameter because there is
      an attribute name per context (base#context)
  */
  bool CheckCacheAttributes (Instruction * i, string attr_name);


  /**
     Generic function used for delta computing (normal edge, call and return).
     Compute the delta between Node pred and Node dest. Each node can have
     different contexts (respectivly predContext and destContext), and different 
     occurence (respectivly predOccur and destOccur)

     This function is architecture independant.
  */
  int Delta (Node * pred, Node * dest, Context * predContext, Context * destContext, bool predOccur, bool destOccur);

  /**
     Return the execution time of the basic bloc BB for a context (context) 
     and an occurence (first).
  */
  unsigned int computeBB (Node & BB, Context * context, bool first);

  /**
     Return the delta of an edge between two basic bloc of the same CFG.
  */
  int computeDelta (Edge * edge, Cfg & cfg, Context * context, bool predOccur, bool destOccur);

  /**
     Return the delta of a call (two distincts CFG).
     \param pred is the call node.
     \param dest is the start node of the callee CFG.
     \param context is the caller context.
     \param Occur is the occurence of the caller.
  */
  int computeCallDelta (Node * pred, Node * dest, Context * context, bool Occur);

  /*
    Return the highest return delta (in case of multiple return of the called
    function).
    \param endNodes vector of all the return point of the called function.
    \param returnNode is the node where the called function returns.
    \param context is the caller context.
    \param Occur is the occurence of the caller.
  */
  int computeReturnDelta (vector < Node * >endNodes, Node * returnNode, Context * context, bool Occur);

  /**
     Return true if the set of names contains a register name
  */
  bool containsRegisterNameNotZero (vector < string > &vs);

 protected:

  /**
     Return the fetch latency of an instruction according to its context and 
     and its occurence.
   
     \param inst instruction to evaluate
     \param context context of the instruction
     \param first first occurence of the access
   
  */ 
  unsigned int getFetchLatency (Instruction & inst, Context * context, bool first);

};

#endif
