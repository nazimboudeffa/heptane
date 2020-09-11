
#include <iostream>
#include <string>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cassert>

#include "RegState.h"

#include "Logger.h"
#include "Utl.h"

using namespace std;

#define LOCTRACE(s)

RegState::RegState (int nbRegisters)
{
  // All contents are assumed unknown
  state.resize (nbRegisters, "*");
  precision.resize (nbRegisters, false);
}

vector < string > RegState::makeAnalysisDefault()
{
  vector < string > result;
  result.resize (3);
  Logger::addWarning ("register with value *");
  result[0] = "*";
  result[1] = "*";
  result[2] = "*";
  return result;
}

// If the address construction does not fit the previously-handled cases, then I raise an error 
//     pattern: most likely pointer shape
bool RegState::AccessAnalysisDefault(int register_number, vector < string > &result)
{
  string value = state[register_number];
  // if (value != "*") return false;
  if ((value != "*")  && precision[register_number] ) return false;
  result = makeAnalysisDefault();
  return true;
}


/**
   Debug: printing the state of the vmax-th states excluding "*" value.
 */
void RegState::print(int vmax)
{
  string vstr;

  int auxReg = getAuxRegister();

  cout << "---- Registers ----" << endl;
  for (int i = 0; i < vmax; i++)
    {
      if (i != auxReg) 
	{
	  vstr = state[i];
	  if (vstr != "*")
	    {
	      cout << std::dec << "\t state[" << i << "]= " 
		   << vstr << ", " << (precision[i] ? "précis" : " non précis")  
		   << endl; 
	    }
	  else
	    if (precision[i])
	      {
		cout <<  " ERREUR :" <<  std::dec << "\t state[" << i << "]= " 
		   << vstr << ", " << (precision[i] ? "précis" : " non précis")  
		   << endl; 
	      }
	}
    }
}


bool RegState::EqualsRegisters(RegState *r)
{
  int n = state.size();
  regTable state2= r->state;
  regPrecisionTable precision2= r->precision;

  int auxReg = getAuxRegister();
  for (int i = 0; i < n; i++)   
    {
      if ( i != auxReg)
	{
	  if (state[i] != state2[i]) return false;
	  if (precision[i] != precision2[i]) return false;
	}
    }
  return true;
}

bool RegState::EqualsStacks(RegState *r)
{
  stackType vStack2 = r-> vStack;
  stackPrecType vStackPrecision2 = r-> vStackPrecision;
  if ( vStack.size() != vStack2.size()) return false;
  for (int j = 0; j< (int) vStack.size(); j++)
    {
      if (vStack[j] != vStack2[j]) return false;
      if (vStackPrecision[j] != vStackPrecision2[j]) return false;
    }
    return true;
}

void RegState::JoinStacks(RegState *r)
{
  int i, n = state.size();
  bool b;
  string v1, v2;
  stackType new_vStack, vStack2 = r-> vStack;
  stackPrecType new_vStackPrecision, vStackPrecision2 = r-> vStackPrecision;
  
  n = vStack.size();
  int m = vStack2.size();
  assert (n == m);
  for (i = 0; i < n; i++)
    {
      b = false;
      v1 = vStack[i];
      v2 = vStack2[i];
      if (v1 == v2)
	{
	  if (vStackPrecision[i] == vStackPrecision2[i]) b = vStackPrecision[i];
	}
      else
	v1 = "*";
      new_vStack.push_back(v1);
      new_vStackPrecision.push_back(b);
    }
  vStack.swap(new_vStack);
  vStackPrecision.swap(new_vStackPrecision);
}

void RegState::JoinRegisters(RegState *r)
{
  int i, n = state.size();
  regTable state2= r->state;
  regPrecisionTable precision2 = r->precision;
 // int m = state2.size(); assert(n==m);

 regTable new_State;
 regPrecisionTable new_Precision;
 LOCTRACE(int auxReg = getAuxRegister());

 LOCTRACE(cout << " JOIN--JOIN--JOIN--JOIN--JOIN--JOIN--JOIN " << endl;
	  cout << " First ****"  << endl;  print(n);
	  cout << " Second ****"  << endl; r->print(n)
	  );
 for (i = 0; i < n; i++)   
   if (state[i] == state2[i])
     {
       new_State.push_back(state[i]);
       if (precision[i] == precision2[i]) new_Precision.push_back(precision[i]); else new_Precision.push_back(false);
     }
   else
     {
       LOCTRACE (  if ( i != auxReg) { cout << " JOIN JOIN JOIN JOIN JOIN JOIN JOIN  set *, i =" << i << ", state[i]= " << state[i] << ", state2[i]= " << state2[i] << endl;});
       new_State.push_back("*");
       new_Precision.push_back(false);
     }
 state.swap(new_State);
 precision.swap(new_Precision);
 LOCTRACE (cout << " Result ****"  << endl; print(n));
}


string RegState::getRegisterValue(int register_number)
{
  return state[register_number];
}

bool RegState::getRegisterPrecision(int register_number)
{
  return precision[register_number];
}

void RegState::setRegisterValue(int register_number, string val)
{
  state[register_number] = val;
}

void RegState::setRegisterPrecision(int register_number, bool val)
{
  precision[register_number] = val;
}

  
void RegState::printStack()
{
  bool b=false;
  int i, n = vStack.size();
  if ( n>0 )
    {
      cout << "---- Stack ----" << endl;
      for ( i= 0; i < n; i++)
	{
	  if (vStack[i] != "*")
	    {
	      cout << "\t stack [" <<  i << "] = " << vStack[i] << ", precision = " << vStackPrecision[i] << endl;
	      b=true;
	    }
	}
      if (!b)
	cout << "\t stack [for all indexes] = * " <<  endl;
    }
}

void RegState::printStates()
{
  print(state.size());
}


void RegState::initStackInfos(int stackSize)
{
  for (int i=0; i < stackSize; i++) {
    vStack.push_back("*");	
    vStackPrecision.push_back(false);	
  }
}


void RegState::cloneStack(RegState *r)
{
  state = r ->state;
  precision = r ->precision;
  int n = r->vStack.size();
  vStack.clear();
  vStackPrecision.clear();
  if (n >0)
    {
      for (int i = 0; i < n; i++) {
	vStack.push_back(r->vStack[i]);
	vStackPrecision.push_back(r->vStackPrecision[i]);
      }
    }
}

void RegState::setRegisters(RegState *other)
{
  state = other ->state;
  precision = other ->precision;
}


void RegState::setStack(RegState *other)
{
  vStack = other ->vStack;
  vStackPrecision = other ->vStackPrecision;
}

bool RegState::importRegister(RegState *other, int numreg)
{
  string vregCaller, vregCalled;
  vregCaller = other->getRegisterValue(numreg);
  vregCalled = this->getRegisterValue(numreg);

  bool b = true;
  size_t found = vregCaller.find ("sp");
  if (found != EOS)
    {
      string oper = vregCaller;
      found = oper.find ("+");
      if (found!= EOS)
	{
	  oper.erase (oper.begin (), oper.begin () + found + 2);
	  if (!Utl::isDecNumber (oper))
	    assert (Utl::isDecNumber (oper));	//check it is a number
	  int n = vStack.size();
	  long loffset = n + atol (oper.c_str ());
	  ostringstream ossOffset;
	  ossOffset << loffset;
	  this->setRegisterValue(numreg, "sp + " + ossOffset.str ());
	  b = false;
	}
    }

  if (vregCaller == vregCalled)
    return false;

  if (b) this->setRegisterValue(numreg, vregCaller);
  this->setRegisterPrecision(numreg, other->getRegisterPrecision(numreg));
  return true;
}
