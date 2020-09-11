
#ifndef DAAINSTRUCTION_ARM
#define DAAINSTRUCTION_ARM

#include "DAAInstruction.h"
#include "ARM.h"


// Common class to many operators.
class ARM_COMMON :public DAAInstruction
{
 protected:
  string codeinstr; 
  string oreg, operand1, operand2, operand3;
  offsetType TypeOperand;

  void simulateShifter(regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   string &operand1, string &operand2);
  void setRegistersInfos2ops(regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision);
  void setRegistersInfos3ops(regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision);
  void setRegistersInfosMultLong(regTable & regs, regPrecisionTable & precision);
 public:
  void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);

};

///-------------------------------
///  ADD, condition exec, set flags
///-------------------------------
class ARM_ADD:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///  SUBSTRACT, condition exec, set flags
///-------------------------------
class ARM_SUBTRACT:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};


///-------------------------------
///  REVERSE SUBSTRACT, condition exec, set flags
///-------------------------------
class ARM_REVERSE_SUB:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///  MUL, condition exec, set flags
///-------------------------------
class ARM_MUL:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///  MOV, condition exec, set flags
///-------------------------------
class ARM_MOV:public  ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM LOAD (Load)
///-------------------------------
class ARM_LOAD:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

class ARM_LOAD_MULTIPLE:public ARM_COMMON
{
 public:
  void simulate (regTable &regs, regPrecisionTable &precision, stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

class ARM_POP:public DAAInstruction
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM NOP
///-------------------------------
class ARM_NOP:public DAAInstruction
{
 public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM STORE
///-------------------------------
class ARM_PUSH:public DAAInstruction
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};


class ARM_STORE:public  ARM_COMMON
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

class ARM_STORE_MULTIPLE:public DAAInstruction
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM BRANCH
///-------------------------------
class ARM_BRANCH:public DAAInstruction
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM SHIFT / ROTATE
///-------------------------------
class  ARM_SHIFT:public  ARM_COMMON
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM LOGICAL: and, eor, orr, bic (and not) 
///-------------------------------
class  ARM_LOGICAL:public ARM_COMMON
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

///-------------------------------
///     ARM COMPARE
///-------------------------------
class  ARM_COMPARE:public DAAInstruction
{
  public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};

// for test.
class ARM_TODO_LOIC:public DAAInstruction
{
 public:void simulate (regTable &regs, regPrecisionTable &precision,  stackType &vstack, stackPrecType &vStackPrecision,   const string &instructionAsm);
};


#endif
