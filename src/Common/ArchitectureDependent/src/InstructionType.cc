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

#include "InstructionType.h"

//------------------------------------
//      InstructionType
//------------------------------------

InstructionType::~InstructionType()
{
  if (addrAnalysisInstruction != NULL)	//NULL is used as a stub for architectures where the data addr analysis is not defined
    {
      delete addrAnalysisInstruction;
    }
}

bool InstructionType::isLoad()
{
  return false;
}

bool InstructionType::isStore()
{
  return false;
}

bool InstructionType::isCall()
{
  return false;
}

bool InstructionType::isReturn()
{
  return false;
}

bool InstructionType::isUnconditionalJump()
{
  return false;
}

bool InstructionType::isConditionalJump()
{
  return false;
}

bool InstructionType::isWord()
{
  return false;
}

bool InstructionType::isBasic()
{
  return false;
}

bool InstructionType::isPredicated()
{
  return false;
}

int InstructionType::getSizeOfMemoryAccess()
{
  assert(false);
  return 0;
}

string InstructionType::getResourceFunctionalUnit()
{
  return functionalUnit;
}

vector < string > InstructionType::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  bool found = false;

  for (set < InstructionFormat * >::const_iterator it = formats.begin(); it != formats.end(); it++)
    {
      if ((*it)->isFormat(operands))
	{
	  assert(!found);
	  result = (*it)->getResourceInputs(operands);
	  found = true;
	}
    }

  //debug
  //if (!found)
  //  {
  //cout << "Input not found operands " << endl;
  //for (size_t i=0;i<operands.size();i++)
  //  {
  //    cout << "operand: " << operands[i] << endl;
  //  }
  //  }
  //--

  assert(found);
  return result;
}

vector < string > InstructionType::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  bool found = false;

  for (set < InstructionFormat * >::const_iterator it = formats.begin(); it != formats.end(); it++)
    {
      if ((*it)->isFormat(operands))
	{
	  assert(!found);
	  result = (*it)->getResourceOutputs(operands);
	  found = true;
	}
    }

  //debug
  //if (!found)
  //  {
  //cout << "Outputs not found operands " << endl;
  //for (size_t i=0;i<operands.size();i++)
  //  {
  //    cout << "operand: " << operands[i] << endl;
  //  }
  //  }
  //--
  assert(found);
  return result;

}

bool InstructionType::checkFormat(const vector < string > &operands)
{
  bool found = false;
  for (set < InstructionFormat * >::const_iterator it = formats.begin(); it != formats.end(); it++)
    {
      if ((*it)->isFormat(operands))
	{
	  assert(!found);
	  found = true;
	}
    }
  return found;
}

DAAInstruction *InstructionType::getDAAInstruction()
{
  return addrAnalysisInstruction;
}

int InstructionType::getLatency()
{
  return latency;
}

//------------------------------------
// Basic
//------------------------------------

Basic::Basic()
{;
}

Basic::Basic(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool Basic::isBasic()
{
  return true;
}

//------------------------------------
// Store
//------------------------------------

Store::Store(int size, const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  size_memory_access = size;
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool Store::isStore()
{
  return true;
}

int Store::getSizeOfMemoryAccess()
{
  return size_memory_access;
}

//------------------------------------
// Load
//------------------------------------

Load::Load(int size, const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  size_memory_access = size;
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool Load::isLoad()
{
  return true;
}

int Load::getSizeOfMemoryAccess()
{
  return size_memory_access;
}

//------------------------------------
// Call
//------------------------------------

Call::Call(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool Call::isCall()
{
  return true;
}

//------------------------------------
// Return
//------------------------------------

Return::Return(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool Return::isReturn()
{
  return true;
}

//------------------------------------
// UnconditionalJump
//------------------------------------

UnconditionalJump::UnconditionalJump(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool UnconditionalJump::isUnconditionalJump()
{
  return true;
}

//------------------------------------
// ConditionalJump
//------------------------------------

ConditionalJump::ConditionalJump(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool ConditionalJump::isConditionalJump()
{
  return true;
}

/***************************************************************

	    ADDITIONNALS TYPES FOR ARM

**************************************************************/

//------------------------------------
// PredicatedBasic
//------------------------------------

PredicatedBasic::PredicatedBasic()
{;
}

PredicatedBasic::PredicatedBasic(const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool PredicatedBasic::isPredicated()
{
  return true;
}

bool PredicatedBasic::isBasic()
{
  return true;
}

//------------------------------------
// PredicatedStore
//------------------------------------

PredicatedStore::PredicatedStore(int size, const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  size_memory_access = size;

  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool PredicatedStore::isStore()
{
  return true;
}

bool PredicatedStore::isPredicated()
{
  return true;
}

int PredicatedStore::getSizeOfMemoryAccess()
{
  return size_memory_access;
}

//------------------------------------
// PredicatedLoad
//------------------------------------

PredicatedLoad::PredicatedLoad(int size, const string & fu, const set < InstructionFormat * >&f, DAAInstruction * daainstr, int lat)
{
  size_memory_access = size;
  functionalUnit = fu;
  formats = f;
  addrAnalysisInstruction = daainstr;
  latency = lat;
}

bool PredicatedLoad::isLoad()
{
  return true;
}

bool PredicatedLoad::isPredicated()
{
  return true;
}

int PredicatedLoad::getSizeOfMemoryAccess()
{
  return size_memory_access;
}

//------------------------------------
// Word
//------------------------------------

Word::Word(const set < InstructionFormat * >&f)
{
  assert(Arch::getArchitectureName() == "ARM");
  formats = f;
  addrAnalysisInstruction = NULL;	//stub since .word is not a real instruction
}

bool Word::isWord()
{
  return true;
}
