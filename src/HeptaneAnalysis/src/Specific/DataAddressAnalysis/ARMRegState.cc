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

#include "InstructionARM.h"

#include "Logger.h"
#include "Utl.h"
#include "ARM.h"

#include <limits.h>
#include "GlobalAttributes.h"

using namespace std;

/**********************************************************/
/**********************************************************/
/**********************************************************/

ARMRegState::ARMRegState (int stackSize):RegState(ARM_NB_REGISTERS)
{
  setPredefValues();
  initStackInfos(stackSize);
}


void ARMRegState::setPredefValues()
{
  reset_sp();
  state[ARM_PC_REGISTER] = "pc";
  precision[ARM_PC_REGISTER] = true;
}

void ARMRegState::reset_sp()
{
  state[ARM_SP_REGISTER] = "sp";
  precision[ARM_SP_REGISTER] = true;
}


int ARMRegState::getAuxRegister()
{ 
  return ARM_AUX_REGISTER;
}

// Simple wrap-up to method simulate on instructionARM, except
// the check that the instruction does not kill gp and sp
void ARMRegState::simulate (Instruction* vinstr)
{
  string oreg, reg, offset, shifter_op, codeinstr;
  offsetType TypeOperand;
  int register_number;
  AddressingMode vaddrmode;
  unsigned long val, addr;
  string vtype;

  string instr = vinstr->GetCode ();
  if (!Arch::isPcLoadInstruction(instr)) // Filtering the indirect ARM jump (ldrxx pc [f(pc)]).
    {   
      // if vinstr  is of the form " ldr R, [ pc , value]" and mem[pc + value] is a immediate value VAL, 
      // then, simulating of mov R, VAL
      if (Arch::getLoadStoreARMInfos(false, instr, codeinstr, oreg, &vaddrmode, &TypeOperand, reg, offset, shifter_op))
	{
	  register_number = Arch::getRegisterNumber (reg);
	  if (IS_PC_REGISTER(register_number))
	    if (GetWordPCrelative(vinstr, TypeOperand, offset, codeinstr, &vtype, &val, &addr))
	      {
		instr = "mov "+ oreg + ", " + Utl::int2cstring(val);
	      }
	}
      else
	{  // add R, pc, #val  replaced by mov R val[pc+value]
	  string operand1, operand2, operand3;
	  string op = "add";
	  if (Arch::getInstr2ARMInfos(instr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
	    if (Arch::isARMClassInstr(codeinstr, op))
	      if (TypeOperand == immediate_offset)	// OP Ri, Rj, #imm8r
		{
		  if (IS_PC_REGISTER( Arch::getRegisterNumber(operand1)))
		    if (InstructionARM::GetWordPCrelative (vinstr, operand2, &vtype, &val, &addr))
		      {
			if (vtype == "imm")
			  {
			    instr = "mov "+ oreg + ", " + Utl::int2cstring(addr);
			  }
		      }
		}
	}
      // assumed : instr is NOT RESTRICTED to a load/write instruction.
      DAAInstruction *instruct = Arch::getDAAInstruction (instr);
      instruct->simulate (state, precision, vStack, vStackPrecision, instr);

      assert (precision[ARM_SP_REGISTER]);	// sp
      TRACE(print(ARM_AUX_REGISTER));
    }
}


bool  ARMRegState::getValueSP( string oper, long * val)
{
  size_t found = oper.find ("+");
  if (found == EOS)
    {
      found = oper.find ("-");
      if (found == EOS)
	{
	  assert (oper.length () == 2); //only sp
	  return false;
	}
      oper.erase (oper.begin (), oper.begin () + found );
    }
  else
    oper.erase (oper.begin (), oper.begin () + found + 2);
  bool b = Utl::isDecNumber (oper);
  if (!b)
    {
      b = Utl::evalexpr(oper, val);
      if (!b) cout << " LBesnard: ARMRegState::getValueSP :: not evaluated expression = " << oper << endl;
      return b;
    }
  *val = atol(oper.c_str());
  return true;
}

/**
   This is the third case, where the address is relative to the stack
   pointer sp, in a transitive fashion, as it  appears.
*/
bool ARMRegState::isAccessAnalysisSP(int register_number, string offset, vector < string > &result)
{ 
  string value, prec;
  long loffset, val;
  
  value = state[register_number];
  size_t found = value.find ("sp");
  if (found == EOS) return false;
  loffset = atol (offset.c_str ());
  // assert (Utl::isDecNumber (offset));
  if (getValueSP(value, &val))
    {
      loffset = loffset + val;
      prec = (precision[register_number] ? "1" :"0");
    }
  else
    {
      assert (!precision[register_number]);	//not precise (array access)
      prec = "0";
    };
  result.push_back ("sp");
  result.push_back (Utl::int2cstring(loffset));
  result.push_back (prec);
  return true;
}

vector < string > ARMRegState::makeAnalysisWordInfos(string vtype, unsigned long val, unsigned long addr)
{
  vector < string > result;

  bool bimmediate = (vtype == "imm");
  result.push_back ( bimmediate ? "immWord" : "gp");
  result.push_back (Utl::int2cstring(val));
  result.push_back ( "1");

  string strAddr = Utl::int2cstring(addr);
  if (bimmediate) result.push_back (strAddr);

  TRACE( if ( vtype == "imm") cout << " loading the value " << strAddr << endl ;
	 else cout << " loading the variable at adress 0x" <<  std::hex << val << std::dec << " ( long = " << val << " )" << endl);
  return result;
}

/**
   When the register is not pc or sp.
   Example:
    ldr R2, [ pc, #val1]
    ldr R3, [ sp, #val2]
    ldr R4, [ R2, R3, lsl 3]  R4 = MEM[ address =word[pc + val1] + (2**val2) * R3 ]  (ie access to an element of an array)
*/
bool ARMRegState::isAccessAnalysisOtherRegister( Instruction * instr, string codeinstr, int register_number, string offset, offsetType TypeOperand, vector < string > &result)
{ 
  unsigned long val, addr;
  string vtype;

  // examples: ldr (or str) Ri, [ Rj,...]
  string value = state[register_number];
  size_t found = value.find ("*");
  if (found != EOS) return false;
  // Rj is known.
  if (GetWordAt(instr, value, offset, TypeOperand, codeinstr, &vtype, &val, &addr))
    {
      // unsigned long val0 = atol (value.c_str ()); val = val + val0;
      result = makeAnalysisWordInfos(vtype, val, addr);
    }
  else
    {
      // keeping the value but not precise...
      result.push_back ("lui");
      result.push_back (value.c_str ());
      result.push_back ( (precision[register_number] ? "1" :"0"));
    }
  return true;
}

bool ARMRegState::GetWordPCrelative (Instruction * instr, offsetType TypeOperand, string offset, string codeinstr, string *vtype, unsigned long *val, unsigned long *addr)
{
  if (TypeOperand != immediate_offset) return false;
  if (Arch::isConditionnedARMInstr(codeinstr, "ldr")) return false;
  // if (Arch::isConditionnedARMInstr(codeinstr, "str")) return false;
  return InstructionARM::GetWordPCrelative(instr, offset, vtype, val, addr);
}


bool ARMRegState::GetWordAt (Instruction * instr, string regvalue, string offset, offsetType TypeOperand, string codeinstr, string *vtype, unsigned long *val, unsigned long *addr)
{
  if ((TypeOperand != immediate_offset) && (TypeOperand != none_offset)) return false;
  if (Arch::isConditionnedARMInstr(codeinstr, "ldr")) return false;
  // if (Arch::isConditionnedARMInstr(codeinstr, "str")) return false;
  long addrword  = atol (regvalue.c_str ()) + atol (offset.c_str ());
  return InstructionARM::GetWordAtAddress(addrword, instr, vtype, val, addr);
}

			
/**   
   Loading a value ldr R, [ pc, #val] , R := mem[ pc + val]

   @return true if the register_number is the PC one, false otherwise.
   When it returns true, result is
   - <"lui", value, precision> for a immediate offset (offset)
   - <"gp", value, precision> for a  address relative to the global,
   - <"*", "*", "0"> otherwise
*/
bool ARMRegState::isAccessAnalysisPC( Instruction * instr, string codeinstr, int register_number, string offset, offsetType TypeOperand, vector < string > &result)
{
  unsigned long val, addr;
  string vtype;

  if ( ! IS_PC_REGISTER(register_number)) return false;
  if (GetWordPCrelative(instr, TypeOperand, offset, codeinstr, &vtype, &val, &addr))
    {
      result = makeAnalysisWordInfos(vtype, val, addr);
    }
  else
    {
      result = makeAnalysisDefault();
    }
  return true;
}


/*-----------------------------------------------------------------------------
 * This function returns a vector of string which represent the memory access of the instruction.
 * this information is analysed after to determine the address, size etc... of this access
 *
 * possible returns (follow the order of the code):
 *	- in case the address comes from a lui
 *	0   "immWord"
 *	1   value
 *	2   precision 1
 *      3   addr

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
 -----------------------------------------------------------------------------*/
void ARMRegState::accessAnalysis ( Instruction* vinstr, vector < string > &result)
{
  // Warning: the state of a register is modified by the state->simulate() method.
  //  ASSUMED : instr is a load/write instruction,
  // "push", "pop", ldm* --load multiple sdm* --store multiple have been reduced in the "kernel".

  string oreg, reg, offset, shifter_op, codeinstr;
  offsetType TypeOperand;
  int register_number;
  AddressingMode vaddrmode;
  string vtype;
  
  string instr = vinstr->GetCode ();
  if (Arch::getLoadStoreARMInfos(true, instr, codeinstr, oreg, &vaddrmode, &TypeOperand, reg, offset, shifter_op))
    {
      register_number = Arch::getRegisterNumber (reg);
      if (! isAccessAnalysisPC(vinstr, codeinstr, register_number, offset, TypeOperand, result))
	if (! isAccessAnalysisSP(register_number, offset, result))
	  if (! isAccessAnalysisOtherRegister(vinstr, codeinstr, register_number, offset, TypeOperand, result))
	    {
	      assert( AccessAnalysisDefault(register_number, result)); // Default is not allowed for all instructions!
	    }
    }
  else
    {
      TRACE( cout << " ARMRegState::accessAnalysis : bad instruction type (not a load/strore)" << endl );
      assert(false); // abort
    }
}

// utl : printing the extracted decomposition of instr.
void ARMRegState::printInstrInfos(string &instr, string &codeinstr, string &oregister, bool &pre_indexed_addr, offsetType &TypeOperand, bool &WriteBackNewAddress, string &operand1, string &operand2, string &operand3)
{
  cout << "instr = " << instr  << endl;
  cout << "---- codeinstr  = " << codeinstr  << endl;
  cout << "---- Output Register  = " << oregister  << endl;
  cout << "---- pre_indexed_addr = " << (pre_indexed_addr ? "true": "false") << endl;
  cout << "---- TypeOperand = " << TypeOperand  << endl;  // index 
  if (WriteBackNewAddress) cout << "---- Write Back New Address " << endl;
  cout << "---- Operand 1 = " << operand1 << endl;
  cout << "---- Operand 2 = " << operand2 << endl;
  cout << "---- Operand 3 = " << operand3 << endl;
}


