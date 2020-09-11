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

/*****************************************************************
    
                         DAAInstruction

This abstract class is a generic definition of an instruction when
used to compute the contents of registers for the determination of
addresses of load/stores.

All the concrete instructions (or rather classes of instructions) are
derived from this abstract class.
        
Instructions are grouped into different categories depending on
the way they modify registers (nop, load, store, add, move,
etc). There is a concrete instruction class corresponding to each
category.

The correspondance between instruction names (mnemonics in asm code)
and instruction categories for address analysis (DAAInstruction *) is
done in InstructionType stored in Arch_dep (see files in 
ArchitectureDependent directory).

Rq: The implementation is done for MIPS architecture only !!!
 
*****************************************************************/
#include <vector>
#include <string>
#include <iostream>
#include "arch.h"

using namespace std;

///-------------------------------
///     Abstract class
///-------------------------------
#ifndef DAAINSTRUCTION
#define DAAINSTRUCTION

typedef vector < bool > regPrecisionTable;
typedef vector < string > regTable;
typedef vector < string > stackType;
typedef vector < bool > stackPrecType;


/*!  
 * Abstract class to define the methods for all categories of mips
 * instructions. The methods are called to determine the contents of
 * registers after all instructions, in order to finally obtained the
 * addresses referenced by load/store instructions.
*/
class DAAInstruction
{
  
  public:
  int  num_register0, num_register1, num_register2, num_register3;

    /*!
     * An instruction must implement a method simulate to compute
     * the state of registers after its execution.
     *
     * regs contains the contents of the machine
     * registers before the simulation of the instruction. In case some
     * computation can be done on register contents, simulate keeps the
     * computation in textual format (ex: "sp+4"). String "*" denote
     * that nothing is known about the register (and then the precision
     * for the given register is set to false).
     *
     * asmInstr (input) contains the textual representation of the instruction
     *
     * precision contains information on registers whose
     * contents is known precisely. 
     *
     * There are situations where the contents of a register is not set
     * to "*" and its precision is set to false (see for instance,
     * instructions of category add, used to compute offsets in arrays)
     *
     * Rq: regs and precision are modified by the simulate function
     *
     */
  virtual void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack,  stackPrecType &vStackPrecision, const string & asmInstr) = 0;

  /*! Virtual destructor */
  virtual ~ DAAInstruction () = 0;

  /*! Returns all the operands of an asm instruction in a vector (i.e. simply parse the asm line) */
  static vector < string > getOperands (const string & instructionAsm);

  void killop1(regTable &regs, vector < bool > &precision);
  void killop2(regTable &regs, vector < bool > &precision);
  void add(regTable &regs, regPrecisionTable &precision, bool bAugmentPrecision );
  void minus(regTable &regs, regPrecisionTable &precision);
  void mult(regTable &regs, regPrecisionTable &precision);
  void move(regTable &regs, regPrecisionTable &precision);
  void loadConstant(regTable &regs, regPrecisionTable &precision, string vcste);


  void arithmetic_shift_right(regTable &regs, regPrecisionTable &precision);
  void logical_shift_left(regTable &regs, regPrecisionTable &precision);
  void logical_shift_right(regTable &regs, regPrecisionTable &precision);
  void rotate_right(regTable &regs, regPrecisionTable &precision);
  void rotate_right_extended(regTable &regs, regPrecisionTable &precision);

  void op_and(regTable &regs, regPrecisionTable &precision);
  void op_or (regTable &regs, regPrecisionTable &precision);
  void op_eor(regTable &regs, regPrecisionTable &precision);
  void op_bic(regTable &regs, regPrecisionTable &precision);
  bool getStackIndex(string &operand1, string &operand2, string &operand3, int *i);

 private:
  void localop(regTable &regs, regPrecisionTable &precision, string vop);
  bool eval(string &operand1, string &codop, string &operand2, string &res);
  bool evalPlus(string &operand1, string &operand2, string &res);
  bool evalMinus(string &operand1, string &operand2, string &res);
  bool logical_shift(regTable & regs, regPrecisionTable & precision, bool bleft);   

  string eval(string operand1, string codop, string operand2);
};

#endif
