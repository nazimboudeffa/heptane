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

#ifndef __REG_STATE_H
#define __REG_STATE_H

#include <vector>
#include <string>

#include "Instruction.h"
#include "arch.h"

using namespace std;
using namespace cfglib;

class RegState {
 protected:

  /** Content of each register, stored as a string.
     "*" denotes an unknown contents, if the contents is known, the string contains
     the operation that allows to compute the register contents.
   */
  regTable state;

  /** Precision of each register.
     True means that the register contents is known precisely (then 'state' allows to compute its value).
     False means that the register contents is not known precisely. In some situations, 'state' is not "*"
     and may contain useful information.
   */
  regPrecisionTable precision;

 public:

  /** Default constructor.*/
  RegState() {};

  RegState(int nbRegisters);

  void cloneStack(RegState * r);

  stackType vStack;		// stack precision
  stackPrecType vStackPrecision;	// stack precision 

  /** This function is used to compute the state of each register after the execution of the instruction.
      'instr' is the full text of the instruction.
   */
  virtual void simulate(Instruction * instr) = 0;

  /** This function returns a vector of strings which represent the memory access of the instruction.
      This information will be analyzed after to determine the address, size etc... of this access.
   */
  virtual void accessAnalysis(Instruction * instr, vector < string > &result) = 0;

  /** @return true if the state of the registers of r and this object are equals, false otherwise.*/
  bool EqualsRegisters(RegState * r);

  bool EqualsStacks(RegState * r);

  /** Merging  two RegState objects:
      - if the state of a register is different, the register is reset to undefined("*") and its precison is false, 
      - otherwise the state is unchanged. The precision of the result is false if one of the precisions is false.
   */
  void JoinRegisters(RegState * r);
  void JoinStacks(RegState * r);
  void setRegisters(RegState * other);
  void setStack(RegState * other);

  /** 
      It resets the status of the stack pointer register (sp): 
      - the state is set to "sp" 
      - and its precision is true.
  */
  virtual void reset_sp() = 0;

  string getRegisterValue(int register_number);
  bool getRegisterPrecision(int register_number);
  void setRegisterValue(int register_number, string val);
  void setRegisterPrecision(int register_number, bool val);

  bool importRegister(RegState * other, int numreg);

  // Added for stack management.
 protected:
  void initStackInfos(int stackSize);

  virtual void setPredefValues() = 0;
  virtual int getAuxRegister() = 0;

  vector < string > makeAnalysisDefault();
  bool AccessAnalysisDefault(int register_number, vector < string > &result);
  void print(int vmax);

 public:
  void printStack();
  void printStates();
};

/*****************************************************************
                             MipsRegState

This class represent the state of all the registers before and after the execution of an instruction.

It allows to compute the state of the registers after the execution
of the instruction using the instruction categorization stored
in mnemonicToInstructionTypes (see files MIPS.h/cc)

The possible values of a register are:
 - "*"
 - "val"
 - "gp"
 - "gp + val"
 - "sp"
 - "sp + val"
 - "val lui"
 - "val lui + val"
 
 where val represents an integer value.
 
*****************************************************************/
class MIPSRegState:public RegState {

 public:

  /** Default constructor
   *  Set all registers to '*' except gp (set to 'gp') and sp (set to 'sp') registers
   */
  MIPSRegState(int stackSize);

  /**
   * This function is used to compute the state of each register
   * after the execution of the instruction.
   * 'instr' is the full text of the instruction.
   */
  void simulate(Instruction * instr);

  /**
   * This function returns a vector of strings which represent the memory access of the instruction.
   * This information will be analyzed after to determine the address, size etc... of this access.
   *
   * The contents of the vector are documented in the source code of the
   * function (e.g. <"sp",offset> in case of stack access, <"gp",offset>
   * for static data access, etc).
   */
  void accessAnalysis(Instruction * instr, vector < string > &result);

  void reset_sp();

 protected:
  void setPredefValues();
  int getAuxRegister();

 private:
  bool isAccessAnalysisLui(int register_number, string offset, vector < string > &result);
  bool isAccessAnalysisGP(int register_number, string offset, vector < string > &result);
  bool isAccessAnalysisSP(int register_number, string offset, vector < string > &result);
};

/*****************************************************************
                             ARMRegState

This class represent the state of all the registers before 
and after the execution of an instruction.

It allows to compute the state of the registers after the execution
of the instruction using the instruction categorization stored
in mnemonicToInstructionTypes (see files ARM.h/cc)

The possible values of a register are:
 - "*"
 - "val"
 - "sp"
 - "sp + val"
 - "pc + val"
 
 where val represents an integer value.
 
*****************************************************************/
class ARMRegState:public RegState {
 public:

  /** Default constructor
   *  Set all registers to '*' except gp (set to 'gp') and sp (set to 'sp') registers
   */
  ARMRegState(int stackSize);

  /**
   * This function is used to compute the state of each register
   * after the execution of the instruction.
   * 'instr' is the full text of the instruction.
   */
  void simulate(Instruction * instr);

  /**
   * This function returns a vector of strings which represent the memory access of the instruction.
   * This information will be analyzed after to determine the address, size etc... of this access.
   *
   * The contents of the vector are documented in the source code of the
   * function (e.g. <"sp",offset> in case of stack access, <"gp",offset>
   * for static data access, etc).
   */
  void accessAnalysis(Instruction * instr, vector < string > &result);
  void reset_sp();

 protected:
  void setPredefValues();
  int getAuxRegister();

 private:
  bool GetWordPCrelative(Instruction * instr, offsetType TypeOperand, string offset, string codeinstr, string * vtype, unsigned long *val, unsigned long *addr);
  bool GetWordAt(Instruction * instr, string regvalue, string offset, offsetType TypeOperand, string codeinstr, string * vtype, unsigned long *val, unsigned long *addr);
  bool isAccessAnalysisPC(Instruction * instr, string codeinstr, int register_number, string offset, offsetType TypeOperand, vector < string > &result);
  bool isAccessAnalysisSP(int register_number, string offset, vector < string > &result);
  bool isAccessAnalysisOtherRegister(Instruction * instr, string codeinstr, int register_number, string offset, offsetType TypeOperand, vector < string > &result);
  void printInstrInfos(string & instr, string & codeinstr, string & oregister, bool & pre_indexed_addr, offsetType & TypeOperand, bool & updateBaseRegisterAfterMemoryTransfer, string & operand1,
		       string & operand2, string & operand3);
  vector < string > makeAnalysisWordInfos(string vtype, unsigned long val, unsigned long addr);
  bool getValueSP(string oper, long *val);
};

#endif
