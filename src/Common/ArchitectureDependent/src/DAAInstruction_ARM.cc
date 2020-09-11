
#include <vector>
#include <string>
#include <iostream>

#include <fstream>
#include <sstream>
#include <cassert>

#include "ARM.h"
#include "Logger.h"
#include "DAAInstruction_ARM.h"
#include "Utl.h"

using namespace std;

#define DAA_TRACE(instr_cl, instr) TRACE(cout << " ---> " << instr_cl << "::simulate (" << instr << " )" <<  endl)
#define ERROR_ACCESS(instr_cl, instr) cout << instr_cl <<"::simulate ERROR ACCESS , instr = " << instr <<  endl
#define NOT_YET_IMPLEMENTED "--- Not yet implemented ::simulate "


void ARM_COMMON::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  assert(false);
}

void ARM_COMMON::simulateShifter(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   string & operand1, string & operand2)
{
  string shiftOperator, shiftOperand;

  TRACE( cout << " SimulateShifter = " << operand2 << endl);
  size_t pos = operand2.find(' ');
  if (pos == EOS)
    {
      shiftOperator = operand2;
      shiftOperand = "";
    }
  else
    {
      shiftOperator = operand2.substr(0, pos);
      shiftOperand = operand2.substr(pos + 1, operand2.length() - 1);
    }
  ARM_SHIFT *shifter = new ARM_SHIFT();
  string instr = shiftOperator + " raux, " + operand1 + ", " + shiftOperand;
  shifter->simulate(regs, precision, vstack, vStackPrecision, instr);
  num_register1 = Arch::getRegisterNumber("raux");
  TRACE( cout << " Shift operation = " << instr << endl);
}

/**
   Setting the num of the operand registers for a < code_op Rr, R1, op2> instruction.
   The auxiliary register ARM_AUX_REGISTER is used for immediate values and for scaled register offset.
 */
void ARM_COMMON::setRegistersInfos3ops(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision)
{
  num_register0 = Arch::getRegisterNumber(oreg);	// result
  if (TypeOperand != none_offset)
    {
      num_register1 = Arch::getRegisterNumber(operand1);
      if (TypeOperand == immediate_offset)	// OP Ri, Rj, #imm8r
	{
	  num_register2 = ARM_AUX_REGISTER;
	  regs[num_register2] = operand2;
	  precision[num_register2] = true;
	}
      else if (TypeOperand == register_offset)	//  OP Ri, Rj, Rk
	{
	  num_register2 = Arch::getRegisterNumber(operand2);
	}
      else if (TypeOperand == scaled_register_offset)	// OP Ri, Rj, Rk, shiftOperator shiftOperand
	{
	  simulateShifter(regs, precision, vstack, vStackPrecision, operand2, operand3);
	}
      else
	assert(false);
    }
}

/**
   Setting the num of the operand registers for Multiply umull, smull, umlal, slmal instructions.
 */
void ARM_COMMON::setRegistersInfosMultLong(regTable & regs, regPrecisionTable & precision)
{
  num_register0 = Arch::getRegisterNumber(oreg);	// result
  num_register1 = Arch::getRegisterNumber(operand1);    // result also
  num_register2 = Arch::getRegisterNumber(operand2);
  num_register3 = Arch::getRegisterNumber(operand3);
}

/**
   Setting the num of the operand registers for a < code_op Rr, op1> instruction ( ex: mov mvn cmp ...)
   The auxiliary register ARM_AUX_REGISTER is used for immediate values and for scaled register offset.
 */
void ARM_COMMON::setRegistersInfos2ops(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision)
{
  num_register0 = Arch::getRegisterNumber(oreg);	// result

  if (TypeOperand != none_offset)
    {
      if (TypeOperand == immediate_offset)	// OP Ri, #imm8r
	{
	  num_register1 = ARM_AUX_REGISTER;
	  regs[num_register1] = operand1;
	  precision[num_register1] = true;
	}
      else if (TypeOperand == register_offset)	//  OP Ri, Rk
	{
	  num_register1 = Arch::getRegisterNumber(operand1);
	}
      else
	{
	  if (TypeOperand == scaled_register_offset)	//  OP Ri, Rk, shiftOperator shiftOperand
	    {
	      simulateShifter(regs, precision, vstack, vStackPrecision, operand1, operand2);
	    }
	  else
	    assert(false);
	}
    }
}


void ARM_ADD::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  // add*, adc*
  string op = "add";
  DAA_TRACE("ARM_ADD", instructionAsm);
  string vinstr = instructionAsm;
  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    if (Arch::isARMClassInstr(codeinstr, op))
      {
	setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	if (Arch::isConditionnedARMInstr(codeinstr, op))
	  killop1(regs, precision);
	else
	  {
	    // add word : add R, pc, #value à traiter differemment... xxxxxxx
	    add(regs, precision, precision[num_register1]);
	  }
      }
    else
      ERROR_ACCESS("ARM_ADD_BAD_OPERATOR", instructionAsm);
  else
    ERROR_ACCESS("ARM_ADD", instructionAsm);
}

void ARM_SUBTRACT::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string op;
  // sub*, sbc*
  DAA_TRACE("ARM_SUBTRACT", instructionAsm);
  string vinstr = instructionAsm;
  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    {
      op = "sub";
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    minus(regs, precision);
	}
      else
	{
	  string op = "sbc";	// subtract with carry
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	      killop1(regs, precision);
	    }
	  else
	    ERROR_ACCESS("ARM_SUBTRACT_BAD_OPERATOR", instructionAsm);
	}
    }
  else
    ERROR_ACCESS("ARM_SUBTRACT", instructionAsm);
}

void ARM_REVERSE_SUB::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string op;
  // rsb*, rsc*
  DAA_TRACE("ARM_REVERSE_SUB", instructionAsm);
  string vinstr = instructionAsm;
  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    {
      op = "rsb";		// reverse subtract
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    {
	      int aux = num_register1;
	      num_register1 = num_register2;
	      num_register2 = aux;	/* swithing the operands */
	      minus(regs, precision);
	    }
	}
      else
	{
	  string op = "rsc";	// reverse subtract with carry
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	      killop1(regs, precision);
	    }
	  else
	    ERROR_ACCESS("ARM_REVERSE_BAD_OPERATOR", instructionAsm);
	}
    }
  else
    ERROR_ACCESS("ARM_REVERSE_SUB", instructionAsm);
}

void ARM_MUL::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string op;
  string vinstr = instructionAsm;

  DAA_TRACE("ARM_MUL", instructionAsm);
  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    {
      op = "mul";
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    mult(regs, precision);
	}
      else
	{
	  op = "mla"; // mla Rd, Rm, Rs, Rn  is a mult with accumulate Rd := (Rm x Rs) + Rn
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      if (Arch::isConditionnedARMInstr(codeinstr, op))
		killop1(regs, precision);
	      else
		{
		  // mla Rd, Rm, Rs, Rn is simulated by R':= Rm x Rs, Rd := R' + Rn
		  ARM_MUL *obj = new ARM_MUL();
		  obj->simulate(regs, precision, vstack, vStackPrecision, "mul raux," + operand1 + " , " + operand2);
		  ARM_ADD *obj_add = new ARM_ADD(); 
		  obj_add->simulate(regs, precision, vstack, vStackPrecision, "add " + oreg + "raux, " + operand3);
		}
	    }
	  else
	    {
	      //smull, umull,umlal, smlal : oreg and operand1 are outputs registers, operand2 and operand3 are input registers.
	      op = "smull";  // signed mult long: smull RdLo,RdHi,Rm,Rs is defined by (RdLo,RdHi) = signed(Rm*Rs)
	      if (Arch::isARMClassInstr(codeinstr, op))
		{
		  setRegistersInfosMultLong(regs, precision);
		  killop1(regs, precision);
		  killop2(regs, precision);
		  TRACE(cout << "Information on the output registers is lost for " << instructionAsm << endl);
		}
	      else
		{
		  op = "umull";  // unsigned mult long: umull RdLo,RdHi,Rm,Rs is defined by (RdLo,RdHi) = unsigned(Rm*Rs)
		  if (Arch::isARMClassInstr(codeinstr, op))
		    {
		      setRegistersInfosMultLong(regs, precision);
		      killop1(regs, precision);
		      killop2(regs, precision);
		      TRACE(cout << "Information on the output registers is lost for " << instructionAsm << endl);
		    }
		  else
		    { 
		      op = "umlal";  // unsigned mult with accumulate: umlal RdLo,RdHi,Rm,Rs is defined by (RdLo,RdHi) = unsigned((RdLo,RdHi) + Rm*Rs)
		      if (Arch::isARMClassInstr(codeinstr, op))
			{
			  setRegistersInfosMultLong(regs, precision);
			  killop1(regs, precision);
			  killop2(regs, precision);
			  TRACE(cout << "Information on the output registers is lost for " << instructionAsm << endl);
			}
		      else
			{ 
			  op = "smlal";  // signed mult with accumulate: smlal RdLo,RdHi,Rm,Rs is defined by (RdLo,RdHi) = signed((RdLo,RdHi) + Rm*Rs)
			  if (Arch::isARMClassInstr(codeinstr, op))
			    {
			      setRegistersInfosMultLong(regs, precision);
			      killop1(regs, precision);
			      killop2(regs, precision);
			      TRACE(cout << "Information on the output registers is lost for " << instructionAsm << endl);
			    }
			  else
			    ERROR_ACCESS(" ARM_MUL_BAD_OPERATOR", instructionAsm);
			}
		    }
		}
	    }
	}
    }
  else
    ERROR_ACCESS("ARM_MUL", instructionAsm);
}

void ARM_MOV::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string op;
  string vinstr = instructionAsm;

  if (Arch::getInstr1ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2))
    {
      setRegistersInfos2ops(regs, precision, vstack, vStackPrecision);
      op = "mov";
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    {
	      if (TypeOperand == immediate_offset)
		loadConstant(regs, precision, regs[num_register1]);
	      else
		move(regs, precision);
	    }
	}
      else
	{
	  op = "mvn";
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      if (Arch::isConditionnedARMInstr(codeinstr, op))
		killop1(regs, precision);
	      else
		{
		  if (TypeOperand == immediate_offset)
		    {
		      loadConstant(regs, precision, Utl::complement(regs[num_register1]));
		    }
		  else
		    killop1(regs, precision);
		}
	    }
	  else
	    ERROR_ACCESS(" ARM_MOV_BAD_OPERATOR", instructionAsm);
	}
    }
  else
    ERROR_ACCESS("ARM_MOV", instructionAsm);
}

void ARM_LOAD::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision, const string & instructionAsm)
{
  // ldr* (32-bits) ; half-word (16-bits): ldrh*, ldrsh* ; byte(8-bits):  ldrb*, ldrsb* 
  // ldrt* ldrbt* used in non-user mode -- IGNORED

  offsetType TypeOperand;
  AddressingMode vIndexAddressing;

  string vinstr = instructionAsm;
  DAA_TRACE("ARM_LOAD", instructionAsm);
  if (Arch::getLoadStoreARMInfos(true, vinstr, codeinstr, oreg, &vIndexAddressing, &TypeOperand, operand1, operand2, operand3))
    {
      setRegistersInfos2ops(regs, precision, vstack, vStackPrecision);
      if (TypeOperand != none_offset && (oreg != operand1))
	killop1(regs, precision); // result is unknown
      else ; // ldr ri, [ri]

      int i;
      if (getStackIndex(operand1, operand2, operand3, &i))
	if (i>=0)  // i < 0 : a metainstruction ???
	  {
	    int indexStack = vstack.size();
	    assert(indexStack>0);
	    num_register0 = Arch::getRegisterNumber(oreg);
	    int vindex = indexStack -(i/4)-1;
	    assert(vindex >= 0);
	    regs[num_register0] = vstack[vindex];
	    precision[num_register0] = vStackPrecision[vindex];
	    TRACE ( cout << "DLOAD, indexStack = " << (indexStack) << " , index =" << vindex << ", Loading " << oreg << " = " << vstack[vindex] << " instr = " << instructionAsm << endl;);
	  }
      if (vIndexAddressing != pre_indexing)	// updating the second register.
	{
	  if (Arch::isConditionnedARMInstr(codeinstr, "ldr"))
	    killop2(regs, precision);
	  else
	    {
	      // "LDR R0, [R1, #4]!"  or "LDR R0, [R1] #4". R1 := R1 + 4 after the memory transfert
	      ARM_ADD *obj_add = new ARM_ADD();
	      obj_add->simulate(regs, precision, vstack, vStackPrecision, "add " + operand1 + ", " + operand2 + "," + operand3);
	    }
	}
    }
  else
    ERROR_ACCESS("ARM_LOAD", instructionAsm);
}



void ARM_PUSH::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  DAA_TRACE("ARM_PUSH", instructionAsm);
  int num_register_v0 = ARM_SP_REGISTER;
  int n = Arch::getNumberOfStores(instructionAsm) * 4;
  ostringstream ossAddr;
  ossAddr << n;

  regs[num_register_v0] = regs[num_register_v0] + " + " + ossAddr.str();
}

void ARM_POP::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,  const string & instructionAsm)
{
  DAA_TRACE("ARM_POP", instructionAsm);

  int num_register_v0 = ARM_SP_REGISTER;
  int n = Arch::getNumberOfLoads(instructionAsm) * 4;
  ostringstream ossAddr;
  ossAddr << n;

  regs[num_register_v0] = regs[num_register_v0] + " - " + ossAddr.str();
}

// Kept, but now the "multiple loads" are rewritten in the Extract step.
void ARM_LOAD_MULTIPLE::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string icode, incr, icodeinit;
  string vinstr = instructionAsm;
  bool writeBack;
  vector < string > regList;
  vector < string >::const_iterator it;
  unsigned int i;
  ARM_LOAD *obj_load1, *obj_load2;
  ARM_ADD *obj_add;
  ARM_SUBTRACT *obj_sub;

  // ldm*
  DAA_TRACE("ARM_LOAD_MULTIPLE", instructionAsm);
  if (Arch::getMultipleLoadStoreARMInfos(vinstr, codeinstr, oreg, regList, &writeBack))
    {
      obj_load1 = new ARM_LOAD();
      if ( codeinstr == "ldm" || codeinstr == "ldmia") // increment after.
	{
	  // LDM[IA] Rk, { Ro_1, ... Ro_n} is translated in : (  raux := Rk;  for i=1,n { Ro_i = [ raux ]; raux = raux + 4; end} )
	  icodeinit = "ldr raux, [ " + oreg + "]";

	  obj_load1->simulate(regs, precision, vstack, vStackPrecision, icodeinit);
	  TRACE(cout << instructionAsm << " simulée par:" << endl; cout << "            " << icodeinit << endl);

	  obj_add = new ARM_ADD();
	  obj_load2 = new ARM_LOAD();
	  for (it = regList.begin(); it != regList.end(); it++)
	    {
	      icode = "ldr ";
	      for ( i = 0; i < (*it).size(); i++) if ((*it)[i] != ' ') icode = icode + (*it)[i];
	      icode = icode + ", [ raux ]";
	      obj_load2->simulate(regs, precision, vstack, vStackPrecision, icode);
	      incr = "add raux, raux, 4";
	      obj_add->simulate(regs, precision, vstack, vStackPrecision, incr );
	      TRACE(cout << "            " << icode << "   " << incr << endl);
	    }
	}
      else
	if ( codeinstr == "ldmib") // increment before.
	  {
	    // LDMIB Rk, { Ro_1, ... Ro_n} is translated in : (  raux := Rk;  for i=1,n { raux = raux + 4; Ro_i = [ raux ];end} )
	  icodeinit = "ldr raux, [ " + oreg + "]";

	  obj_load1->simulate(regs, precision, vstack, vStackPrecision, icodeinit);
	  TRACE(cout << " ldr init = " << icodeinit << endl);

	  obj_add = new ARM_ADD();
	  obj_load2 = new ARM_LOAD();
	  for (it = regList.begin(); it != regList.end(); it++)
	    {
	      incr = "add raux, raux, 4";
	      obj_add->simulate(regs, precision, vstack, vStackPrecision, incr );
	      icode = "ldr ";
	      for ( i = 0; i < (*it).size(); i++) if ((*it)[i] != ' ') icode = icode + (*it)[i];
	      icode = icode + ", [ raux ]";
	      obj_load2->simulate(regs, precision, vstack, vStackPrecision, icode);
	      TRACE( cout << "simulée par " << incr << "   " << icode << endl);
	    }
	    
	  }
      else
	if ( codeinstr == "ldmda") // Decrement After
	  {
	    // LDMDA Rk, { Ro_1, ... Ro_n} is translated in : (  raux := Rk - |regs|*4 + 4;  for i=1,n {  Ro_i = [ raux ]; raux = raux - 4;end} )
	    int n =  regList.size()*4 + 4;
	    icodeinit = "ldr raux, [ " + oreg + "]";
	    obj_load1->simulate(regs, precision, vstack, vStackPrecision, icodeinit);

	    incr = "sub raux, raux," + Utl::int2cstring(n);
	    obj_sub = new ARM_SUBTRACT();
	    obj_sub->simulate(regs, precision, vstack, vStackPrecision, incr );
	    TRACE( cout << " ldr init = " << icodeinit << " " << incr << endl);

	    obj_add = new ARM_ADD();
	    obj_load2 = new ARM_LOAD();
	    for (it = regList.begin(); it != regList.end(); it++)
	      {
		icode = "ldr ";
		for ( i = 0; i < (*it).size(); i++) if ((*it)[i] != ' ') icode = icode + (*it)[i];
		icode = icode + ", [ raux ]";
		obj_load2->simulate(regs, precision, vstack, vStackPrecision, icode);
		incr = "sub raux, raux, 4";
		obj_sub->simulate(regs, precision, vstack, vStackPrecision, incr );
		TRACE( cout << "simulée par " << incr << "   " << icode << endl);
	      }
	  }
      else
	if ( codeinstr == "ldmdb") // Decrement Before
	  {
	    // LDMDB Rk, { Ro_1, ... Ro_n} is translated in : (  raux := Rk - |regs|*4;  for i=1,n { raux = raux - 4; Ro_i = [ raux ];;end} )
	    int n =  regList.size()*4 ;
	    icodeinit = "ldr raux, [ " + oreg + "]";
	    obj_load1->simulate(regs, precision, vstack, vStackPrecision, icodeinit);

	    incr = "sub raux, raux," + Utl::int2cstring(n);
	    obj_sub = new ARM_SUBTRACT();
	    obj_sub->simulate(regs, precision, vstack, vStackPrecision, incr );
	    
	    TRACE( cout << " ldr init = " << icodeinit << " " << incr << endl);

	    obj_add = new ARM_ADD();
	    obj_load2 = new ARM_LOAD();
	    for (it = regList.begin(); it != regList.end(); it++)
	      {
		incr = "sub raux, raux, 4";
		obj_sub->simulate(regs, precision, vstack, vStackPrecision, incr );
		icode = "ldr ";
		for ( i = 0; i < (*it).size(); i++) if ((*it)[i] != ' ') icode = icode + (*it)[i];
		icode = icode + ", [ raux ]";
		obj_load2->simulate(regs, precision, vstack, vStackPrecision, icode);
		TRACE( cout << "simulée par " << incr << "   " << icode << endl);
	      }
	  }
	else
	  {
	    ERROR_ACCESS(" ARM_LOAD_MULTIPLE", instructionAsm);	  
	  }

      if (writeBack) 
	{
	  // Rk = raux;
	  icodeinit = "ldr " + oreg + ",[raux]";
	  obj_load1->simulate(regs, precision, vstack, vStackPrecision, icodeinit);
	}

    }
  else
    ERROR_ACCESS(" ARM_LOAD_MULTIPLE", instructionAsm);
}

void ARM_BRANCH::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  DAA_TRACE("ARM_BRANCH", instructionAsm);
};

void ARM_SHIFT::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string op;
  string vinstr = instructionAsm;


  // Format as ADD: {asr, lsl, lsr, ror}; Format as mov : {RRX}
  DAA_TRACE("ARM_SHIFT", instructionAsm);

  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    {
      setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
      op = "rrx";
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    rotate_right_extended(regs, precision);
	}
      else
	{
	  op = "asr";		// Arithmetic shift right
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      if (Arch::isConditionnedARMInstr(codeinstr, op))
		killop1(regs, precision);
	      else
		arithmetic_shift_right(regs, precision);
	    }
	  else
	    {
	      string op = "lsl";	// Logical shift left
	      if (Arch::isARMClassInstr(codeinstr, op))
		{
		  if (Arch::isConditionnedARMInstr(codeinstr, op))
		    killop1(regs, precision);
		  else
		    logical_shift_left(regs, precision);
		}
	      else
		{
		  string op = "lsr";	// Logical shift right
		  if (Arch::isARMClassInstr(codeinstr, op))
		    {
		      if (Arch::isConditionnedARMInstr(codeinstr, op))
			killop1(regs, precision);
		      else
			logical_shift_right(regs, precision);
		    }
		  else
		    {
		      string op = "ror";	// rotate_right
		      if (Arch::isARMClassInstr(codeinstr, op))
			{
			  if (Arch::isConditionnedARMInstr(codeinstr, op))
			    killop1(regs, precision);
			  else
			    rotate_right(regs, precision);
			}
		      else
			ERROR_ACCESS("ARM_SHIFT_BAD_OPERATOR", instructionAsm);
		    }
		}
	    }
	}
    }
  else
    ERROR_ACCESS("(ARM_SHIFT ", instructionAsm);
}

void ARM_LOGICAL::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  // and*, orr*,eor*, bic* (and not)
  string op;
  string vinstr = instructionAsm;

  DAA_TRACE("ARM_LOGICAL", instructionAsm);
  if (Arch::getInstr2ARMInfos(vinstr, codeinstr, oreg, &TypeOperand, operand1, operand2, operand3))
    {
      setRegistersInfos3ops(regs, precision, vstack, vStackPrecision);
      op = "and";
      if (Arch::isARMClassInstr(codeinstr, op))
	{
	  if (Arch::isConditionnedARMInstr(codeinstr, op))
	    killop1(regs, precision);
	  else
	    op_and(regs, precision);
	}
      else
	{
	  string op = "orr";
	  if (Arch::isARMClassInstr(codeinstr, op))
	    {
	      if (Arch::isConditionnedARMInstr(codeinstr, op))
		killop1(regs, precision);
	      else
		op_or(regs, precision);
	    }
	  else
	    {
	      string op = "eor";
	      if (Arch::isARMClassInstr(codeinstr, op))
		{
		  if (Arch::isConditionnedARMInstr(codeinstr, op))
		    killop1(regs, precision);
		  else
		    op_eor(regs, precision);
		}
	      else
		{
		  string op = "bic";	// op1 and not(op2)
		  if (Arch::isARMClassInstr(codeinstr, op))
		    {
		      if (Arch::isConditionnedARMInstr(codeinstr, op))
			killop1(regs, precision);
		      else
			op_bic(regs, precision);
		    }
		  else
		    ERROR_ACCESS("ARM_SHIFT_BAD_OPERATOR", instructionAsm);
		}
	    }
	}
    }
}



/**
   No effects on output register, but on the operand register in auto_indexing or post_indexing.
   Example : "STR R0, [R1, #4]!"  or "STR R0, [R1] #4". R1 := R1 + 4 after the memory transfert
*/
void ARM_STORE::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  string vinstr = instructionAsm;
  offsetType TypeOperand;
  AddressingMode vIndexAddressing;

  DAA_TRACE("ARM_STORE", instructionAsm);
  if (Arch::getLoadStoreARMInfos(true, vinstr, codeinstr, oreg, &vIndexAddressing, &TypeOperand, operand1, operand2, operand3))
    {
      if (vIndexAddressing != pre_indexing)	// updating the second register.
	{
	  if (Arch::isConditionnedARMInstr(codeinstr, "str"))
	    killop2(regs, precision);
	  else
	    {
	      ARM_ADD *obj_add = new ARM_ADD();
	      obj_add->simulate(regs, precision, vstack, vStackPrecision, "add " + operand1 + ", " + operand2 + "," + operand3);
	    }
	}

      int i;
      if (getStackIndex(operand1, operand2, operand3,&i))
	  if (i>=0) // i < 0 : a metainstruction ???
	    {
	      int indexStack = vstack.size();
	      assert(indexStack>0);
	      num_register0 = Arch::getRegisterNumber(oreg);
	      int vindex = indexStack -(i/4)-1;
	      assert(vindex >= 0);
	      TRACE( cout << "DSTORE index =" << vindex << " , Store " <<  regs[num_register0] << " to stack [" <<  vindex << "]" << " instr = " << instructionAsm << endl;);
	      vstack[vindex] =  regs[num_register0];
	      vStackPrecision[vindex] = precision[num_register0];
	    }
    }
}

void ARM_COMPARE::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  // DAA_TRACE("ARM_COMPARE", instructionAsm);
  // cpm*, cmn*, tst*; teq* : update the CPSR flags 
  // no effects on registers ( the flags are not managed , useful Damien ?)
};

void ARM_NOP::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  //  No effects on registers 
  //  DAA_TRACE("ARM_NOP", instructionAsm);
};

void ARM_STORE_MULTIPLE::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  // DAA_TRACE("ARM_STORE_MULTIPLE", instructionAsm);
  // No effects on registers
};

void ARM_TODO_LOIC::simulate(regTable & regs, regPrecisionTable & precision, stackType &vstack, stackPrecType &vStackPrecision,   const string & instructionAsm)
{
  DAA_TRACE("ARM_TODO_LOIC", instructionAsm);
};
