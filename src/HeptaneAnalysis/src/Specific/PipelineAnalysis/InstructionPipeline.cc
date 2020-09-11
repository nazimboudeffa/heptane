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

#include <assert.h>
#include "InstructionPipeline.h"

InstructionPipeline::InstructionPipeline(unsigned int size)
{
  Pipe = new vector < pipeStage > (size);

  currentStage = 0;
}

InstructionPipeline::~InstructionPipeline(void)
{
  delete Pipe;
}

pipeStage *InstructionPipeline::insertInstruction(unsigned int tick)
{
  Pipe->at(0).tick = tick;
  return &Pipe->at(0);
}

pipeStage *InstructionPipeline::propagateInstruction(unsigned int lat)
{
  Pipe->at(currentStage + 1).tick = Pipe->at(currentStage).tick + lat;
  currentStage++;
  return &Pipe->at(currentStage);
}

pipeStage *InstructionPipeline::getPipeStage(unsigned int stageNum)
{
  return &Pipe->at(stageNum);
}

unsigned int InstructionPipeline::getDependencySource(vector < string > &inputs, string & source)
{
  unsigned int v, tick = 0;
  source = string("");
  for (unsigned int i = Pipe->size() - 1; i > 0; i--)
    {
      vector < string > outputs = Pipe->at(i).out;

      // Look for a match between inputs and outputs
      for (unsigned int j = 0; j < outputs.size(); j++)
	{
	  for (unsigned int k = 0; k < inputs.size(); k++)
	    {
	      if (outputs[j].compare(inputs[k]) == 0 /* && outputs[j].compare("mem")!=0 */ )
		{
		  v = Pipe->at(i).tick;;
		  if (v > tick)
		    {
		      tick = v;
		      source = outputs[j];
		    }
		}
	    }
	}
    }

  return tick;
}

unsigned int InstructionPipeline::getDependencies(vector < string > &inputs)
{
  string source;
  return getDependencySource(inputs, source);
}

string InstructionPipeline::getDependencySource(vector < string > &inputs)
{
  string source;
  getDependencySource(inputs, source);
  return source;
}

unsigned int InstructionPipeline::checkAvaliability(string FU)
{
  unsigned int v, tick = 0;
  int vsize;

  // Generalized by LBesnard for ARM: an instruction may require more thant one FU, when the "barrel shifter" is used.
  // For MIPS: the assert ( FUs.size () == 1) has already be verified before this call.
  for (unsigned int i = Pipe->size() - 1; i > 0; i--)
    {
      vector < string > FUs = Pipe->at(i).FU;
      vsize = FUs.size();
      // assert (vsize <= 1); for MIPS but ARM, an instruction may require the barrel shifter. // if (vsize != 0) 
      for (int j = 0; j < vsize; j++)
	{
	  string fu = FUs[j];
	  if (fu.compare(FU) == 0)
	    {
	      v = Pipe->at(i).tick;
	      if (v > tick)
		{
		  tick = v;
		}
	    }
	}
    }
  return tick;
}

unsigned int InstructionPipeline::execFinished(void)
{
  unsigned int tick = 0;

  for (unsigned int i = Pipe->size() - 1; i > 0; i--)
    {
      if (!Pipe->at(i).FU.empty())
	{
	  tick = Pipe->at(i).tick + 1;
	}
    }

  return tick;
}

// Print the content of the instruction pipeline
void InstructionPipeline::Print(void)
{
  for (unsigned int i = 0; i < Pipe->size(); i++)
    {
      cout << "\tStage " << i << " tick " << Pipe->at(i).tick;
      cout << "\tFU: ";
      for (unsigned int j = 0; j < Pipe->at(i).FU.size(); j++)
	cout << Pipe->at(i).FU[j] << " ";
      cout << "\t In: ";
      for (unsigned int j = 0; j < Pipe->at(i).in.size(); j++)
	cout << Pipe->at(i).in[j] << " ";
      cout << "\t Out: ";
      for (unsigned int j = 0; j < Pipe->at(i).out.size(); j++)
	cout << Pipe->at(i).out[j] << " ";
      cout << endl;
    }
}
