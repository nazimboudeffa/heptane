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

#include "ARMPipelineAnalysis.h"
#include "arch.h"

#define LATENCY_BARREL_SHIFTER 1

//-----------Public -----------------------------------------------------------------

ARMPipelineAnalysis::ARMPipelineAnalysis(Program * p, int nbcache):PipelineAnalysis(p, nbcache)
{
}

bool ARMPipelineAnalysis::PerformAnalysis()
{
  return PipelineAnalysis::PerformAnalysis();
}

int ARMPipelineAnalysis::getLatency( const string codeInstr, bool BarrelShifterUsed)
{
  int lat = Arch::getLatency(codeInstr);
  if ( BarrelShifterUsed ) lat = lat + LATENCY_BARREL_SHIFTER;  // to be verified !!!
  return lat;
}

/**
 * Schedule the first instruction of a basic bloc.
 * 
 * \param inst instruction to insert in the pipeline
 * \param IP 
 * \param context context of the instruction
 * \param first first occurence of the instruction
 */
void ARMPipelineAnalysis::scheduleFirstInst(Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first)
{
  // ----------------------------------------------------------------------------------------------------
  // il faut tenir compte du barrel_shifter pour les instruction avec en second opereande un shift/rotate.
  // ----------------------------------------------------------------------------------------------------

  string codeInstr = inst.GetCode();
  InstructionPipeline *instTmp = new InstructionPipeline(PIPELINEDEPTH);
  pipeStage *pipeStageTmp;

  TRACE_PIPELINEANALYSIS(cout << " -- begin scheduleFirstInst() instr = " << codeInstr << endl);
  unsigned int fetchAt = getFetchLatency(inst, context, first);
  //fetch stage
  instTmp->insertInstruction(fetchAt);
  //decode stage
  instTmp->propagateInstruction(1);

  //execution stage 
  vector < string > FU = Arch::getResourceFunctionalUnits(codeInstr);
  int lat = getLatency(codeInstr, FU.size() > 1);
  pipeStageTmp = instTmp->propagateInstruction(lat);
  pipeStageTmp->FU = FU;
  pipeStageTmp->in = Arch::getResourceInputs(codeInstr);

  // assert (pipeStageTmp->FU.size () == 1); Not for ARM; the "barrel shifter" may be required.

  //WB stage
  pipeStageTmp = instTmp->propagateInstruction(1);
  pipeStageTmp->out = Arch::getResourceOutputs(codeInstr);

  IP.push_back(instTmp);
  TRACE_PIPELINEANALYSIS(instTmp->Print());
  TRACE_PIPELINEANALYSIS(cout << " -- end scheduleFirstInst " << endl);

}

/**
 * Schedule the first instruction of a basic bloc.
 * 
 * \param inst instruction to insert in the pipeline
 * \param IP 
 * \param context context of the instruction
 * \param first first occurence of the instruction
 */
void ARMPipelineAnalysis::scheduleNextInst(Instruction & inst, vector < InstructionPipeline * >&IP, Context * context, bool first)
{
  // ----------------------------------------------------------------------------------------------------
  // il faut tenir compte du barrel_shifter pour les instruction avec en second opereande un shift/rotate.
  // ----------------------------------------------------------------------------------------------------

  string codeInstr = inst.GetCode();
  InstructionPipeline *instTmp = new InstructionPipeline(PIPELINEDEPTH);

  TRACE_PIPELINEANALYSIS(cout << " -- begin scheduleNextInst() instr = " << codeInstr << endl);
  unsigned int fetchAt = IP[IP.size() - 1]->getPipeStage(0)->tick + getFetchLatency(inst, context, first);

  //fetch stage
  instTmp->insertInstruction(fetchAt);
  //decode stage
  instTmp->propagateInstruction(1);

  //check for dependencies
  //goes backward through instructions and stops at the first dependency found
  vector < string > inputs = Arch::getResourceInputs(codeInstr);
  unsigned int i = IP.size() - 1;
  unsigned int depTick = IP[i]->getDependencies(inputs);	// return the clock tick were the data needed by "inputs" are available or 0 if no dependencies
  while (depTick == 0 && i > 0)
    {
      i--;
      depTick = IP[i]->getDependencies(inputs);
    }

  //check FU avaliability
  vector < string > FUs = Arch::getResourceFunctionalUnits(codeInstr);
  unsigned int FUTick = 0;
  for (i = 0; i < IP.size(); i++)
    for (unsigned int j = 0; j < FUs.size(); j++)
      {
	unsigned int t = IP[i]->checkAvaliability(FUs[j]);
	if (t > FUTick)
	  {
	    FUTick = t;
	  }
      }

  //get the max of depTick FUTick and depTick
  int execLat = 0;
  if (depTick > FUTick)
    execLat = depTick - (fetchAt + 3);
  else if (FUTick > depTick)
    execLat = FUTick - (fetchAt + 3);

  if (execLat < 0)
    execLat = 0;

  //execution stage
  pipeStage *pipeStageTmp = instTmp->propagateInstruction(execLat + 1);
  pipeStageTmp->FU = FUs;
  pipeStageTmp->in = Arch::getResourceInputs(codeInstr);

  int lat = getLatency(codeInstr, FUs.size() > 1);
  if (lat <= 1)			//allow bypass
    pipeStageTmp->out = Arch::getResourceOutputs(codeInstr);
  //  else pipeStageTmp->FU = FUs; // already set

  //WB stage, do WB only after preceding inst WB
  unsigned int WBAt = IP[IP.size() - 1]->getPipeStage(PIPELINEDEPTH - 1)->tick + 1;
  int WBLat = WBAt - pipeStageTmp->tick;
  if (WBLat < 0)
    WBLat = 1;
  pipeStageTmp = instTmp->propagateInstruction(WBLat);
  pipeStageTmp->out = Arch::getResourceOutputs(codeInstr);

  IP.push_back(instTmp);
  TRACE_PIPELINEANALYSIS(instTmp->Print());
  TRACE_PIPELINEANALYSIS(cout << " -- end scheduleNextInst()" << endl);

}
