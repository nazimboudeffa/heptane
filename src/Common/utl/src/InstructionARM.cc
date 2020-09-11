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


#include <sstream>
#include <math.h>
#include "InstructionARM.h"
#include "GlobalAttributes.h"

using namespace std;


//singleton declaration
InstructionARM * InstructionARM;

InstructionARM::InstructionARM ()
{}

InstructionARM::~InstructionARM ()
{}


bool InstructionARM::GetWordAtAddress (long wordaddr, Instruction * instr, string *vtype, unsigned long *val, unsigned long *addr)
{
  string attr_addr=ARMWordsAttributeName ;

  if (! instr->HasAttribute (attr_addr)) return false;
  ARMWordsAttribute attr = (ARMWordsAttribute &) instr->GetAttribute (attr_addr);

  ARMWordsAttributeTable  words = attr.getTable();  
  ARMWordsAttributeTable::const_iterator it = words.begin ();
  bool found = false;
  while (!found)
    {
      if (it == words.end()) return false;
      if ((it->first) == (unsigned) wordaddr) found = true; else it++;
    }
  
  ARMWordsAttributeElemTableValue current = it->second;
  *vtype = current.first;
  *val = current.second;
  *addr = it->first;
  return true;
}


bool InstructionARM::GetWordPCrelative (Instruction * instr, string offset, string *vtype, unsigned long *val, unsigned long *addr)
{
  long wordaddr;

  if (!getCodeAddress(instr, &wordaddr)) return false;
  int voffset = Utl::string2int(offset);
  wordaddr = wordaddr + voffset + 2 * Arch::getInstructionSize();
  return GetWordAtAddress(wordaddr, instr, vtype, val, addr);
}


bool InstructionARM::getCodeAddress( Instruction * vinstr, long *add)
{
  if (!vinstr->HasAttribute (AddressAttributeName)) return false;
  AddressAttribute attr = (AddressAttribute &) vinstr->GetAttribute (AddressAttributeName);
  *add = attr.getCodeAddress ();
  return true;
}

