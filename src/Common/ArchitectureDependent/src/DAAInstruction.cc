
#include "DAAInstruction.h"
#include "Utl.h"
#include "math.h"

#define LOCTRACE(s)
 
///-------------------------------
/// Generic functions of 
//  abstract class DAAInstruction
///-------------------------------
// getOperands
// -----------
// Operands are in the order they appear in the asm text file (destination first)
// The instruction mnemonic is not part of the returned vector,
// that contains operands only
vector < string > DAAInstruction::getOperands(const string & instructionAsm)
{
  vector < string > instr = Arch::splitInstruction(instructionAsm);
  vector < string > result(instr.begin() + 1, instr.end());	//+1 to not put the mnemonic
  return result;
}

DAAInstruction::~DAAInstruction()
{
}

bool DAAInstruction::evalPlus(string &operand1, string &operand2, string &res)
{
  int i, n;
  long val;
  string codop=" + ";
  string elemOperand2;
  
  LOCTRACE( cout << " Try to simplify = (" << operand1 << ") " << codop << " (" << operand2  << ")" << endl);
  if (! Utl::isDecNumber(operand1)) return false;
  if (operand2.find("-") == EOS) 
    {
      vector<string> vectOperand2 = Utl::split(operand2, string("+"));
      if (! Utl::getIndexFirstValue( vectOperand2, &i)) return false;
      elemOperand2 = vectOperand2[i];
      n = vectOperand2.size();
      if (! Utl::eval(operand1, codop, elemOperand2, &val)) return false;
      vectOperand2[i] = Utl::int2cstring(val);
      for (i=0; i<n;i++)
	{
	  if (i !=0) res = res + codop ;
	  res = res + vectOperand2[i];
	}
      LOCTRACE(cout << " --> Returned value = " << res << endl);
      return true;
    }

  if (operand2.find("+") != EOS) return false;
  vector<string> vectOperand2 = Utl::split(operand2, string("-"));
  if (! Utl::getIndexFirstValue( vectOperand2, &i)) return false;
  elemOperand2 = vectOperand2[i];
  n = vectOperand2.size();
  val = - atol (elemOperand2.c_str ());
  val = val + atol (operand1.c_str ());
  string nval = Utl::int2cstring(val);
  int ireplace = i;
  vectOperand2[ireplace] = nval;
  for (i=0; i<n;i++)
    {
      if (i == ireplace)
	{
	  if (val != 0)
	    {
	      if (val > 0) res = res + " + " ;
	      res = res + vectOperand2[i];
	    }
	}
      else
	{
	  if (i !=0) res = res + " - " ;
	  res = res + vectOperand2[i];
	}
    }
  LOCTRACE(cout << " --> Returned value = " << res << endl);
  return true;
}



bool DAAInstruction::evalMinus(string &operand1, string &operand2, string &res)
{
  int i, n;
  long val;
  string codop = " - ";
  string elemOperand2;

  LOCTRACE( cout << " Try to simplify = (" << operand1 << ") " << codop << " (" << operand2  << ")" << endl);
  if (Utl::isDecNumber(operand1))
    {
      if (operand2.find("+") != EOS) return false;
  
      vector<string> vectOperand2 = Utl::split(operand2, string("-"));
      if (! Utl::getIndexFirstValue( vectOperand2, &i)) return false;
      elemOperand2 = vectOperand2[i];
      n = vectOperand2.size();

      if (! Utl::eval(operand1, codop, elemOperand2, &val)) return false;
      vectOperand2[i] = Utl::int2cstring(val);
      for (i=0; i<n;i++)
	{
	  if (i !=0) res = res + codop ;
	  res = res + vectOperand2[i];
	}
      LOCTRACE(cout << " --> Returned value = " << res << endl);
      return true;
    }
  if (Utl::isDecNumber(operand2))  // arg1 = sp - V1, operand2 = V2
    {
      if (operand1.find("+") == EOS)
	{
	  vector<string> vectOperand2 = Utl::split(operand1, string("-"));
	  if (! Utl::getIndexFirstValue( vectOperand2, &i)) return false;
	  elemOperand2 = vectOperand2[i];
	  n = vectOperand2.size();
	  string plus("+");
	  if (! Utl::eval(operand2, plus, elemOperand2, &val)) return false;
	  vectOperand2[i] = Utl::int2cstring(val);
	  for (i=0; i<n;i++)
	    {
	      if (i !=0) res = res + codop ;
	      res = res + vectOperand2[i];
	    }
	  LOCTRACE(cout << " --> Returned value = " << res << endl);
	  return true;
	}
      // Try to simplify = (sp + V0)  -  (V1)
      if (operand1.find("-") != EOS) return false;
      vector<string> vectOperand2 = Utl::split(operand1, string("+"));
      if (! Utl::getIndexFirstValue( vectOperand2, &i)) return false;
      elemOperand2 = vectOperand2[i];
      n = vectOperand2.size();
      
      val = atol (elemOperand2.c_str ());
      val = val - atol (operand2.c_str ());
      string nval = Utl::int2cstring(val);
      int ireplace = i;
      vectOperand2[ireplace] = nval;
      for (i=0; i<n;i++)
	{
	  if (i == ireplace)
	    {
	      if (val != 0)
		{
		  if (val > 0) res = res + " + " ;
		  res = res + vectOperand2[i];
		}
	    }
	  else
	    {
	      if (i !=0) res = res + " + " ;
	      res = res + vectOperand2[i];
	    }
	}
      LOCTRACE(cout << " --> Returned value = " << res << endl);
      return true;
    }

  return false;
}

/** For codop=+, 
    - if operand1 is a decimal value
    - if operand2 is a sum (S1 + ...+Sn) and it exists a decimal value S element of operand2,
    then the sum of operand1 and S is computed and res = eval(operand1 + S) + elements of operands2 minus S.
    @return true is the evaluation of operand1 codop operand2 is rewritten in res, false otherwise.
*/
bool DAAInstruction::eval(string &operand1, string &codop, string &operand2, string &res)
{
  // Useful for the access to an element of an array (MIPS, example "92" + "0x4lui + 9000").
  if (codop.find("+") != EOS) return evalPlus(operand1, operand2,res);
  if (codop.find("-") != EOS) return evalMinus(operand1, operand2,res);
  return false;
}


string DAAInstruction::eval(string operand1, string codop, string operand2)
{
  long val;
  string res;
  if (Utl::eval(operand1, codop, operand2, &val)) return Utl::int2cstring(val);
  if (eval(operand1, codop, operand2, res)) return res;
  if ( codop.find("+") != EOS ) // commutativity for +
    if (eval(operand2, codop, operand1, res)) return res;
  return (operand1 + codop + operand2);
}

void DAAInstruction::localop(regTable & regs, regPrecisionTable & precision, string vop)
{
  string operand1 = regs[num_register1];
  string operand2 = regs[num_register2];

  if (operand1 == "*")
    {
      regs[num_register0] = "*";
      assert(!precision[num_register1]);
    }
  else
    if (operand2 == "*")
      {
	regs[num_register0] = "*";
	assert(!precision[num_register2]);
      }
    else
      {
	regs[num_register0] = eval(operand1, vop, operand2);
	// bool b = precision[num_register1] && precision[num_register2];
	// if (!b) cout << " assertion not satisfied " << endl;
	// assert(b);
      }
  
  precision[num_register0] = precision[num_register1] && precision[num_register2];
}

// Consider that all information on the first operand (num_register0) is lost
void DAAInstruction::killop1(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register0] = "*";
  precision[num_register0] = false;
}

// Consider that all information on the second operand is lost
void DAAInstruction::killop2(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register1] = "*";
  precision[num_register1] = false;
}

/**
   The result (ie regs[num_register0]) is set to:
   - "unknown" if the operands are both unknown.
   - "+" if the operands are both known.
   - "unknown" otherwise when bAugmentPrecision is false.
   - the value of the known operand when bAugmentPrecision is true.
   (Damien 's note: this particular situation is to augment the precision of the analysis when accessing arrays (to detect
   that the array is accessed, even if the precise address in the array is not known)
*/
void DAAInstruction::add(regTable & regs, regPrecisionTable & precision, bool bAugmentPrecision)
{

  if (bAugmentPrecision)
    {
      string operand1 = regs[num_register1];
      string operand2 = regs[num_register2];
      if (operand1 == "*" && operand2 == "*")
	{
	  regs[num_register0] = "*";
	  assert(!precision[num_register1]);
	  assert(!precision[num_register2]);
	}
      else if (operand1 == "*")
	{
	  regs[num_register0] = operand2;	// for induction variable
	  assert(!precision[num_register1]);
	}
      else if (operand2 == "*")
	{
	  regs[num_register0] = operand1;	// for induction variable
	  assert(!precision[num_register2]);
	}
      else
	{
	  regs[num_register0] = eval(operand1, " + ", operand2);
	  //  removed for test lbesnard --- Jul 2017---
	  // assert(precision[num_register1]);
	  // assert(precision[num_register2]);
	}

      precision[num_register0] = precision[num_register1] && precision[num_register2];
    }
  else
    localop(regs, precision, " + ");
}

void DAAInstruction::minus(regTable & regs, regPrecisionTable & precision)
{
  localop(regs, precision, "- ");
}

void DAAInstruction::mult(regTable & regs, regPrecisionTable & precision)
{
  localop(regs, precision, " x ");
}

// Keep all information we add on source register on destination register    
void DAAInstruction::move(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register0] = regs[num_register1];
  precision[num_register0] = precision[num_register1];
}

// Loading a constant in a register.
void DAAInstruction::loadConstant(regTable & regs, regPrecisionTable & precision, string vcste)
{
  regs[num_register0] = vcste;
  precision[num_register0] = true;
}

void DAAInstruction::arithmetic_shift_right(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}


bool DAAInstruction::logical_shift(regTable & regs, regPrecisionTable & precision, bool bleft)
{
  int n,m,res;

  if (precision[num_register1])
    {
      string operand1 = regs[num_register1];
      if (Utl::isDecNumber(operand1))
	{
	  m = atoi (operand1.c_str());
	  if (m == 0) 
	    {
	      regs[num_register0] = "0";
	      precision[num_register0] = true;
	      return true;
	    }
	  else
	    if (Utl::getintvalue(regs[num_register2], &n))
	      {
		if (bleft)
		  {
		    if (m < 0) res = -m << n; else res = m << n;
		    LOCTRACE(cout << "  * Logical shift left( "<< m << ", " << n << ") = " << res << endl;);
		  }
		else
		  {
		    res = m >> n;
		    LOCTRACE(cout << "  * Logical shift right( "<<  m << ", " << n << ") = " << res << endl;);
		  }
		regs[num_register0] = Utl::int2cstring(res);
		precision[num_register0] = true;
		return true;
	      }
	}
    }
  return false;
}

void DAAInstruction::logical_shift_left(regTable & regs, regPrecisionTable & precision)
{
  if (!logical_shift(regs, precision, true))
    killop1(regs, precision);
}

void DAAInstruction::logical_shift_right(regTable & regs, regPrecisionTable & precision)
{
 if (!logical_shift(regs, precision, false))
   killop1(regs, precision);
}

void DAAInstruction::rotate_right(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_and(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_or(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_eor(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_bic(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

/** It provides the value of the contents of a register shifted right one bit. 
    The old carry flag is shifted into bit[31]. 
    If the S suffix is present, the old bit[0] is placed in the carry flag.
*/
void DAAInstruction::rotate_right_extended(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}


bool DAAInstruction::getStackIndex(string &operand1, string &operand2, string &operand3, int *i)
{
  // for ARM currently
  if (operand1.find("sp") == EOS) return false;
  if (operand3 != "") return false;
  *i=0;
  if (operand2 == "")   return true;
  return Utl::getintvalue(operand2, i);
}

