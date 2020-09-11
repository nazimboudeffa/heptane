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

#include <vector>
#include <string>
#include <iostream>

#include <fstream>
#include <sstream>
#include <cassert>

#include "DAAInstruction_MIPS.h"
#include "MIPS.h"
#include "arch.h"
#include "Utl.h"

using namespace std;

#define LOCTRACE(s)

//--------------------------------------------
//      NOP
//
// Classes of instructions that neither produce nor destroy contents of 
// registers -> no impact on address analysis
//--------------------------------------------
void
Nop::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
}

//---------------------------------------------
//      LOAD
//
// Classes of instruction that load information from memory
//
//---------------------------------------------
void DLoad::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  bool b;
  int i;
  string reg;
  b = Arch::getRegisterAndIndexStack(instructionAsm, reg, &i);
  if (b) 
    {
      int indexStack = vstack.size();
      assert(indexStack>0);
      num_register0 = Arch::getRegisterNumber(reg);
      int vindex = indexStack - (i/4)-1;
      assert(vindex >= 0);
      LOCTRACE ( cout << "DLOAD, indexStack = " << (indexStack) << " , index =" << vindex << ", Loading " << reg << " = " << vstack[vindex] << " instr = " << instructionAsm << endl;);
      regs[num_register0] = vstack[vindex];
      precision[num_register0] = vStackPrecision[vindex];
    }
  else
    {
      vector < string > operands = getOperands(instructionAsm);
      num_register0 = Arch::getRegisterNumber(operands[0]);
      killop1(regs, precision);
    }
}


//---------------------------------------------
//      STORE
//
// Classes of instruction that load information from memory
//
//---------------------------------------------
void DStore::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  bool b;
  int i;
  string reg;

  /* example : sw ra,20(sp)  */
  b = Arch::getRegisterAndIndexStack(instructionAsm, reg, &i);
  if (b)  
    {
      int indexStack = vstack.size();
      assert(indexStack>0);
      num_register0 = Arch::getRegisterNumber(reg);
      int vindex = indexStack -(i/4)-1;
      assert(vindex >= 0);
      LOCTRACE(  cout << "DSTORE index =" << vindex << " , Store " <<  regs[num_register0] << " to stack [" <<  vindex << "]" << " instr = " << instructionAsm << endl;);
      vstack[vindex] =  regs[num_register0];
      vStackPrecision[vindex] = precision[num_register0];
    }
}

//---------------------------------------------
//      CALL
//
// Call instruction.
// Calls have to be in a specific category
// because in the MIPS they kill the registers
// used for the return values of functions
//---------------------------------------------
void DCall::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  num_register0 = Arch::getRegisterNumber("v0");
  num_register1 = Arch::getRegisterNumber("v1");

  // Contents of registers $v0 and $v1, used to return function results,
  // are possibly destroyed, which is reflected in regs and precision
  killop1(regs, precision);
  killop2(regs, precision);
}

//---------------------------------------------
//      MOVE
//
// Transfer from register to register
//---------------------------------------------
void Move::simulate(regTable & regs, regPrecisionTable & precision,  stackType &vstack, stackPrecType &vStackPrecision, const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  num_register1 = Arch::getRegisterNumber(operands[1]);
  move(regs, precision);
}

//---------------------------------------------
//      KILL_OP1
//
// Classes of instruction that modify a register
// (destination operand, first in the list of operands) 
// and for which all information
// on the register content is lost.
//
// In case the instruction kills the second register
// operand and not the first one, category KILL_OP2
// should be used instead.
//---------------------------------------------
void KillOp1::simulate(regTable & regs, regPrecisionTable & precision,  stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  killop1(regs, precision);
}

//---------------------------------------------
//      KILL_OP2
//
// Classes of instruction that modify a register
// that is the second in the list of operands,
// and for which all information
// on the register content is lost.
//
// In case the instruction kills the first register operand and not
// the second one, category KILL_OP1 should be used instead.
// ---------------------------------------------
void KillOp2::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register1 = Arch::getRegisterNumber(operands[1]);
  killop2(regs, precision);
}

///------------------------------------------------------------
///     Arithmetic and Logical Instructions
///------------------------------------------------------------

//---------------------------------------------
//      ADD
//
// Signed addition on registers
//---------------------------------------------
void Add::simulate(regTable & regs, regPrecisionTable & precision,stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  num_register1 = Arch::getRegisterNumber(operands[1]);
  num_register2 = Arch::getRegisterNumber(operands[2]);
  add(regs, precision, true);
}

//---------------------------------------------
//      ADDIU
//
// Unsigned addition of immediate to register
// (rt <- rs + immediate)
//---------------------------------------------
void Addiu::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
 
  if ((operands[0] == operands[1]) && (operands[0] == "sp")) // addiu sp,sp,value
    {
    }
 else
    {
      num_register0 = Arch::getRegisterNumber(operands[0]);
      num_register1 = Arch::getRegisterNumber(operands[1]);
      num_register2 = MIPS_AUX_REGISTER;
      regs[num_register2] = operands[2];
      precision[num_register2] = true;
      add(regs, precision, false);
    }
}

//---------------------------------------------
//      SUBU
//
// Unsigned substraction between registers
//---------------------------------------------
//TODO:2nd case: regs[num_register0]= "-" + operand2;
/* to be added if needed (should be similar to the add simulate function)
void Subu::simulate(vector<string>& regs, vector<bool>& precision, stackType &vstack, stackPrecType &vStackPrecision,  const string& instructionAsm)
{
    vector<string> operands= getOperands(instructionAsm);
    
    int num_register0 = MapRegistersMIPS::getRegisterNumber(operands[0]);
    int num_register1 = MapRegistersMIPS::getRegisterNumber(operands[1]);
    int num_register2 = MapRegistersMIPS::getRegisterNumber(operands[2]);

    string operand1=regs[num_register1];
    string operand2=regs[num_register2];
        
    if(operand1=="*" && operand2=="*")
    {
        regs[num_register0]="*";
    }
    else if(operand1=="*")
    {
        regs[num_register0]= "-" + operand2; // for induction variable
    }
    else if(operand2=="*")
    {
        regs[num_register0]=operand1; // for induction variable
    }
    else
    {
        regs[num_register0] = operand1 + " - " + operand2;
    }
	
    precision[num_register0]= precision[num_register1] && precision[num_register2];
}
*/

void Subu::simulate(regTable & regs, regPrecisionTable & precision,stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  num_register1 = Arch::getRegisterNumber(operands[1]);
  num_register2 = Arch::getRegisterNumber(operands[2]);
  minus(regs, precision);
}

///------------------------------------------------------------
///     Specific Load Instructions
///------------------------------------------------------------

///-------------------------------
///     LUI
///-------------------------------
void Lui::simulate(regTable & regs, regPrecisionTable & precision,stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  // the lui flag is used to specify that the immediate value must be shifted left 16 bits.
  loadConstant(regs, precision, operands[1] + "lui");
}

///-------------------------------
///     LI
///-------------------------------
void Li::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision, const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  loadConstant(regs, precision, operands[1]);
}


/// SHIFT 
// ssl R1,R2,i with i in [0,31], ssl v0,v0,0  is a nop.
void Shift::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision, const string & instructionAsm)
{
  vector < string > operands = getOperands(instructionAsm);
  num_register0 = Arch::getRegisterNumber(operands[0]);
  num_register1 = Arch::getRegisterNumber(operands[1]);
  num_register2 = MIPS_AUX_REGISTER;
  regs[num_register2] = operands[2];
  precision[num_register2] = true;
  //   num_register2 = Arch::getRegisterNumber(operands[2]);
  logical_shift_left(regs, precision);
}
