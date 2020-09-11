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

#include <iostream>
#include <string>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cassert>

#include "RegState.h"
#include "DAAInstruction.h"

#include "Logger.h"
#include "Utl.h"
#include "MIPS.h"

#include <limits.h>

using namespace std;
 
#define LOCTRACE(s)

/**********************************************************/
/**********************************************************/
/**********************************************************/
// RegState implem

MIPSRegState::MIPSRegState (int stackSize):RegState(MIPS_NB_REGISTERS )
{
  setPredefValues();
  initStackInfos(stackSize);
}

void MIPSRegState::setPredefValues()
{
  reset_sp();
  // gp is  known precisely
  // symbolic value, to be evaluated later
  state[28] = "gp";
  precision[28] = true;

  state[0] = "0";
  precision[0] = true;
}

void MIPSRegState::reset_sp()
{
  // sp is known precisely
  state[29] = "sp";
  precision[29] = true;
}

int MIPSRegState::getAuxRegister()
{ return MIPS_AUX_REGISTER;}


// Simple wrap-up to method simulate on instructionMIPS, except
// the check that the instruction does not kill gp and sp
void
MIPSRegState::simulate (Instruction * vinstr)
{
  string instr = vinstr->GetCode (); 

  // cout << " simulate = " << instr << endl << " the stack before = " << endl;  printStack();

  DAAInstruction *instruct = Arch::getDAAInstruction (instr);
  instruct->simulate (state, precision, vStack, vStackPrecision, instr);

  assert (precision[0]);  // zero
  assert (precision[28]); // gp
  assert (precision[29]); // sp
  LOCTRACE(print(MIPS_AUX_REGISTER));
}

/** 
    @return true if value is a "lui" instruction, false otherwise.
    pattern: 68lui [+ decimal_value]
    value = state[register_number]
*/
bool MIPSRegState::isAccessAnalysisLui (int register_number, string offset, vector < string > &result)
{
  // Warning: the state of a register is modified by the state->simulate() method.

  // examples : MIPS (I): lui v0, 0x40 ==> state[0] = "lui64" in simulate() method. 0x40(hexa) = 64 (dec)
  // if (state[0] == f("lui")) may be true (if not modified from (I))

  unsigned long addr;
  string value = state[register_number];
  size_t found = value.find ("lui");
  if (found == EOS) return false;

  // -------------------------------------
  // Computing v * (2**16) + v' + offset, with  state[register_number] = vlui + v'

  // Getting the base address from the lui.
  // addr can be in two distinct part: Addrh bits and Addrl bits
  string Addrh = value;
  Addrh.erase (found); // remove 'lui'

  // check that Addrh is a number
  // DH: FIXME: faire plus clean et voir pourquoi ici c'est en hexa
  if (! Utl::isDecNumber(Addrh))
    {
      addr = strtoul (value.c_str (), NULL, 16);
      assert (addr != 0 && addr != ULONG_MAX);
    }
  else
    {
      addr = atol (Addrh.c_str ());
    }
  addr = addr * 65536; // the immediate value is shifted left 16 bits (see LUI instruction)

  found = value.find ("+");
  if (found != EOS)
    {
      string Addrl = value;
      Addrl.erase (Addrl.begin (), Addrl.begin () + found + 2);	//remove '... + '
      //check that Addrl is a number
      bool bb = Utl::isDecNumber (Addrl);
      assert (bb);	
      addr = addr + atol (Addrl.c_str ());
    }
  else //check that 'lui' is not followed by something else in the register value
    {
      if (value.find ("lui") + 3 != value.length ())
	{
	  Logger::addError ("accessAnalysis -> LUI access");
	  assert (value.find ("lui") + 3 == value.length ());
	}
    }

  // Add the offset to the base address
  long loffset = atol (offset.c_str ());
  assert (Utl::isDecNumber (offset));
  addr = addr + loffset;
  ostringstream ossAddr;
  ossAddr << addr;
  // ------------------------------- 

  result.push_back ("lui");
  result.push_back (ossAddr.str ());
  result.push_back ( (precision[register_number] ? "1" :"0")); 
  LOCTRACE( cout << " DEBUG_LB, MIPSRegState::isAccessAnalysisLui. Registre = " << register_number <<", ossADDR = " << ossAddr.str () << endl);
  return true;
}


//--------------------------------------------------
//     This is the second case,
//     where the address is relative to the global
//     pointer gp.
//     pattern: gp + val
//--------------------------------------------------
bool MIPSRegState::isAccessAnalysisGP (int register_number, string offset, vector < string > &result)
{
  // Warning: the state of a register is modified by the state->simulate() method.
  ostringstream ossOffset;

  string value = state[register_number];
  size_t found = value.find ("gp"); // Not only for register_number=28, but mov ... copies the value.
  if (found == EOS) return false;
  
  long loffset = atol (offset.c_str ());
  assert (Utl::isDecNumber (offset));
  string oper = value;
  found = oper.find ("-");
  if (found == EOS)
    {
      Logger::addError ("reg analysis -> '-' not found for gp + -value");
    }
  else
    {
      oper.erase (oper.begin (), oper.begin () + found + 1);
      assert (Utl::isDecNumber (oper));	//check it is a number
      assert (oper[0] != '-');	//check it is a positive number
      assert (Utl::isDecNumber (oper));
      loffset = loffset - atol (oper.c_str ());
      ossOffset << loffset;

      result.push_back ("gp");
      result.push_back (ossOffset.str ());
      result.push_back ( (precision[register_number] ? "1" :"0"));
    }
  LOCTRACE(cout << " DEBUG_LB, MIPSRegState::isAccessAnalysisGP. Registre = " << register_number <<", ossADDR = " << ossOffset.str () << endl);
  return true;
}

/* 
   This is the third case, where the address is relative to the stack
   pointer sp, in a transitive fashion, as it  appears.
*/
bool MIPSRegState::isAccessAnalysisSP(int register_number,string offset, vector < string > &result)
{ 
  // Warning: the state of a register is modified by the state->simulate() method.

  ostringstream ossOffset;
  string prec;

  string value = state[register_number];
  size_t found = value.find ("sp");
  if (found == EOS) return false;

  long loffset = atol (offset.c_str ());
  assert (Utl::isDecNumber (offset));

  string oper = value;
  found = oper.find ("+");
  if (found == EOS)
    {
      //that case can be found in case of an array in the stack
      //benchmark: ud
      //4006b0:   8fa20004        lw      v0,4(sp)
      //4006b4:   00000000        nop
      //4006b8:   00021080        sll     v0,v0,0x2
      //4006bc:   03a21021        addu    v0,sp,v0
      //4006c0:   8c420010        lw      v0,16(v0)

      assert (oper.length () == 2);	//only sp
      assert (!precision[register_number]);	//not precise (array access)
      prec = "0";
    }
  else
    {
      oper.erase (oper.begin (), oper.begin () + found + 2);
      assert (Utl::isDecNumber (oper));	//check it is a number
      loffset = loffset + atol (oper.c_str ());
      prec = (precision[register_number] ? "1" :"0");
    }
  ossOffset << loffset;
  result.push_back ("sp");
  result.push_back (ossOffset.str ());
  result.push_back (prec);
  LOCTRACE( cout << " DEBUG_LB, MIPSRegState::isAccessAnalysisSP. Registre = " << register_number <<", ossADDR = " << ossOffset.str () << endl);
 return true;
}

/*-----------------------------------------------------------------------------
 * This function returns a vector of string which represents the memory access of the instruction.
 * this information is analysed after to determine the address, size etc... of this access
 *
 * possible returns (follow the order of the code):
 *	- in case the address comes from a lui
 *	0   "lui"
 *	1	addr
 *	2	precision (=="1" if precise analysis and "0" otherwise)
 *	- in case the address is based on gp
 *	0   "gp"
 *	1	offset
 *	2	precision (=="1" if precise analysis and "0" otherwise)
 *	- in case the address is based on sp
 *	0   "sp"
 *	1	offset
 *	2	precision (=="1" if precise analysis and "0" otherwise)
 *	- unknown
 *	0   "*"
 *	1	"*"
 *	2	"*"
 *
 -----------------------------------------------------------------------------*/
void MIPSRegState::accessAnalysis (Instruction* vinstr, vector < string > &result)
{
  vector < string > split_instruction;
  string op2, reg, offset, instr;
  int register_number;
  
  instr = vinstr->GetCode ();
  // Warning: the state of a register is modified by the state->simulate() method.
  LOCTRACE( cout << " DEBUG_LB, MIPSRegState::accessAnalysis: " << instr << endl);
  split_instruction = Arch::splitInstruction (instr);  
  assert (split_instruction.size () == 3);

  op2 = split_instruction[2];

  reg = Arch::extractInputRegistersFromMem (op2)[0];
  register_number = Arch::getRegisterNumber (reg);
  
  offset = op2.erase (op2.find ("("));
  // Analyzing the expression associated with the target register of the instuction
  if (! isAccessAnalysisLui( register_number, offset, result))
    if ( ! isAccessAnalysisGP( register_number, offset, result)) 
      if ( ! isAccessAnalysisSP( register_number, offset, result))
	{
	  bool b = AccessAnalysisDefault( register_number, result );
	  if (!b) cout << " AccessAnalysisDefault DEBUG_LB, MIPSRegState::accessAnalysis: " << instr << endl;
	  assert(b);  // Default is not allowed for all instructions!
	}
}


