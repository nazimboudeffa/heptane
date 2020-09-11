/* ---------------------------------------------------------------------

   Copyright IRISA, 2003-2017

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

#include "InstructionFormat.h"
#include <algorithm>
#include "Utl.h"

//--------------------------------------------
//      Empty (no operand)
//--------------------------------------------
bool Empty::isFormat(const vector < string > &operands)
{
  if (operands.size() == 0)
    {
      return true;
    }
  else
    {
      return false;
    }
}

vector < string > Empty::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

vector < string > Empty::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

/**********************************************
 *
 *               1 OPERAND
 *
 **********************************************/

//--------------------------------------------
// rd : register destination
//--------------------------------------------
rd::rd(const string & in)
{
  rin = in;
}

bool rd::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  if (rin != "")
    {
      size_t pos;
      string inputs = rin;

      //Removing all "," in the string
      do
	{
	  pos = inputs.find(",");
	  if (pos != EOS)
	    {
	      inputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(inputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

vector < string > rd::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// hex : integer value in hexa (0x...)
//--------------------------------------------
bool Hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }
  return Utl::isHexNumber(operands[0]);
}

vector < string > Hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

vector < string > Hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
// addr : integer value corresponding to an address
//--------------------------------------------
bool Addr::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }
  return Utl::isAddr(operands[0]);
}

vector < string > Addr::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

vector < string > Addr::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
// rs : register source
//--------------------------------------------
bool rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/

//--------------------------------------------
// rd_mem : register destination / memory pattern
//--------------------------------------------
bool rd_mem::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_mem::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

vector < string > rd_mem::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractOutputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rd_int : register destination / integer value in decimal
//--------------------------------------------
bool rd_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

vector < string > rd_int::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_mem : register source / memory pattern
//--------------------------------------------
bool rs_mem::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_mem::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_mem::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractOutputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

//--------------------------------------------
// rs_addr : register source / integer value corresponding to an address
//--------------------------------------------
bool rs_addr::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Utl::isAddr(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_addr::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_addr::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
// rd_rs : register destination / register source
//--------------------------------------------
bool rd_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);

  return result;
}

vector < string > rd_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_rd : register source / register destination
//--------------------------------------------
bool rs_rd::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_rd::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_rd::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);

  return result;
}

//--------------------------------------------
// rs_rs : register source / register source
//--------------------------------------------
rs_rs::rs_rs(const string & out)
{
  rout = out;
}

bool rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);

  return result;
}

vector < string > rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  if (rout != "")
    {
      size_t pos;
      string outputs = rout;

      //Removing all "," in the string
      do
	{
	  pos = outputs.find(",");
	  if (pos != EOS)
	    {
	      outputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(outputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

//--------------------------------------------
// rd_hex : register destination / integer value in hexa (0x...)
//--------------------------------------------
bool rd_hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  return Utl::isHexNumber(operands[1]);
}

vector < string > rd_hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

vector < string > rd_hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

/**********************************************
 *
 *               3 OPERANDS
 *
 **********************************************/

//--------------------------------------------
// zero_rs_rs
//--------------------------------------------
zero_rs_rs::zero_rs_rs(const string & out)
{
  rout = out;
}

bool zero_rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isZeroRegister(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > zero_rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);
  result.push_back(operands[2]);

  return result;
}

vector < string > zero_rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  if (rout != "")
    {
      size_t pos;
      string outputs = rout;

      //Removing all "," in the string
      do
	{
	  pos = outputs.find(",");
	  if (pos != EOS)
	    {
	      outputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(outputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

//--------------------------------------------
// rd_rs_rs
//--------------------------------------------
bool rd_rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);
  result.push_back(operands[2]);

  return result;
}

vector < string > rd_rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rd_rs_int
//--------------------------------------------
bool rd_rs_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_rs_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);

  return result;
}

vector < string > rd_rs_int::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rd_rs_hex
//--------------------------------------------
bool rd_rs_hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  return Utl::isHexNumber(operands[2]);
}

vector < string > rd_rs_hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);

  return result;
}

vector < string > rd_rs_hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_zero_hex
//--------------------------------------------
bool rs_zero_hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isZeroRegister(operands[1]) == false)
    {
      return false;
    }
  return Utl::isHexNumber(operands[2]);
}

vector < string > rs_zero_hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_zero_hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
// rs_rs_addr
//--------------------------------------------
bool rs_rs_addr::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Utl::isAddr(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_rs_addr::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);

  return result;
}

vector < string > rs_rs_addr::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

/***************************************************************

	    ADDITIONNALS FORMATS FOR ARM

**************************************************************/

/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//      rs_int
//--------------------------------------------
bool rs_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_int::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
//      rs_hex
//--------------------------------------------
bool rs_hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  return Utl::isHexNumber(operands[1]);
}

vector < string > rs_hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
//      rds_hex
//--------------------------------------------
bool rds_hex::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  return Utl::isHexNumber(operands[1]);
}

vector < string > rds_hex::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rds_hex::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
//      rds_int
//--------------------------------------------
bool rds_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[1]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rds_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

vector < string > rds_int::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

/**********************************************
 *
 *               3 OPERANDS
 *
 **********************************************/

//--------------------------------------------
// rd_mem_int 
//--------------------------------------------
bool rd_mem_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_mem_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

vector < string > rd_mem_int::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : ldr r1, [r2], #6 ------> r1 = MEM[R2]; r2 = r2+6;
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_mem_int 
//--------------------------------------------
bool rs_mem_int::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Utl::isDecNumber(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_mem_int::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

vector < string > rs_mem_int::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : str r1, [r2], #6 ------> MEM[R2] = r1; r2 = r2+6;
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

//--------------------------------------------
// rd_mem_rs
//--------------------------------------------
bool rd_mem_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rd_mem_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[2]);
  result.push_back("mem");

  return result;
}

vector < string > rd_mem_rs::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : ldr r1, [r2], r3 ------> r1 = MEM[R2]; r2 = r2+r3;
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_mem_rs
//--------------------------------------------
bool rs_mem_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  return true;
}

vector < string > rs_mem_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);
  result.push_back(operands[2]);

  return result;
}

vector < string > rs_mem_rs::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : str r1, [r2], r4 ------> MEM[R2] = r1; r2 = r2+r4;
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

//--------------------------------------------
//      rs_rs_shifter
//--------------------------------------------
bool rs_rs_shifter::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isShifterOperand(operands[2]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rs_rs_shifter::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);

  string shifted_register = Arch::extractRegisterFromShifterOperand(operands[2]);

  if (shifted_register != "")
    {
      result.push_back(shifted_register);
    }

  return result;
}

vector < string > rs_rs_shifter::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;
  assert(result.empty());
  return result;
}

//--------------------------------------------
//      rd_rs_shifter
//--------------------------------------------
bool rd_rs_shifter::isFormat(const vector < string > &operands)
{
  if (operands.size() != 3)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isShifterOperand(operands[2]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rd_rs_shifter::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);

  string shifted_register = Arch::extractRegisterFromShifterOperand(operands[2]);

  if (shifted_register != "")
    {
      result.push_back(shifted_register);
    }

  return result;
}

vector < string > rd_rs_shifter::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

/**********************************************
 *
 *               4 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//      rd_rd_rs_rs
//--------------------------------------------
bool rd_rd_rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rd_rd_rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[2]);
  result.push_back(operands[3]);

  return result;
}

vector < string > rd_rd_rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);

  return result;
}

//--------------------------------------------
//      rd_rs_rs_rs
//--------------------------------------------
bool rd_rs_rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rd_rs_rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);
  result.push_back(operands[2]);
  result.push_back(operands[3]);

  return result;
}

vector < string > rd_rs_rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
//      rds_rds_rs_rs
//--------------------------------------------
bool rds_rds_rs_rs::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rds_rds_rs_rs::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);
  result.push_back(operands[2]);
  result.push_back(operands[3]);

  return result;
}

vector < string > rds_rds_rs_rs::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back(operands[1]);

  return result;
}

//--------------------------------------------
// rd_mem_rs_shifter
//--------------------------------------------
bool rd_mem_rs_shifter::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isShifterOperand(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rd_mem_rs_shifter::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[2]);
  result.push_back("mem");

  string shifted_register = Arch::extractRegisterFromShifterOperand(operands[3]);

  if (shifted_register != "")
    {
      result.push_back(shifted_register);
    }

  return result;
}

vector < string > rd_mem_rs_shifter::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : ldr r1, [r2], r3, rrx ------> r1 = MEM[R2]; r2 = r2+rrx(r3);
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);

  return result;
}

//--------------------------------------------
// rs_mem_rs_shifter
//--------------------------------------------
bool rs_mem_rs_shifter::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isMemPattern(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isShifterOperand(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rs_mem_rs_shifter::getResourceInputs(const vector < string > &operands)
{
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back(operands[0]);
  result.push_back(operands[2]);

  string shifted_register = Arch::extractRegisterFromShifterOperand(operands[3]);

  if (shifted_register != "")
    {
      result.push_back(shifted_register);
    }

  return result;
}

vector < string > rs_mem_rs_shifter::getResourceOutputs(const vector < string > &operands)
{
  // Because it is post-indexed adressing mode, the INPUT register is written
  // ARM example : str r1, [r2], r3, rrx ------> MEM[R2] = r1; r2 = r2+rrx(r3);
  vector < string > result = Arch::extractInputRegistersFromMem(operands[1]);

  result.push_back("mem");

  return result;
}

//--------------------------------------------
//      rd_rs_rs_shifter
//--------------------------------------------
bool rd_rs_rs_shifter::isFormat(const vector < string > &operands)
{
  if (operands.size() != 4)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[1]) == false)
    {
      return false;
    }
  if (Arch::isRegisterName(operands[2]) == false)
    {
      return false;
    }
  if (Arch::isShifterOperand(operands[3]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rd_rs_rs_shifter::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[1]);
  result.push_back(operands[2]);

  string shifted_register = Arch::extractRegisterFromShifterOperand(operands[3]);

  if (shifted_register != "")
    {
      result.push_back(shifted_register);
    }

  return result;
}

vector < string > rd_rs_rs_shifter::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);

  return result;
}

/**********************************************
 *
 *               ONE OPERAND
 *
 **********************************************/
//--------------------------------------------
//      rslist 
//--------------------------------------------
rslist::rslist(const string & in, const string & out)
{
  rin = in;
  rout = out;
}

bool rslist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[0]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rslist::getResourceInputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[0]);

  if (rin != "")
    {
      size_t pos;
      string inputs = rin;

      //Removing all "," in the string
      do
	{
	  pos = inputs.find(",");
	  if (pos != EOS)
	    {
	      inputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(inputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

//  Warning : because of ARM, this format returns "mem" in the Outputs
vector < string > rslist::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back("mem");

  if (rout != "")
    {
      size_t pos;
      string outputs = rout;

      //Removing all "," in the string
      do
	{
	  pos = outputs.find(",");
	  if (pos != EOS)
	    {
	      outputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(outputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

//--------------------------------------------
//      rdlist 
//--------------------------------------------
rdlist::rdlist(const string & in, const string & out)
{
  rin = in;
  rout = out;
}

bool rdlist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 1)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[0]) == false)
    {
      return false;
    }

  return true;
}

//  Warning : because of ARM, this format returns "mem" in the Inputs
vector < string > rdlist::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back("mem");

  if (rin != "")
    {
      size_t pos;
      string inputs = rin;

      //Removing all "," in the string
      do
	{
	  pos = inputs.find(",");
	  if (pos != EOS)
	    {
	      inputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(inputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

vector < string > rdlist::getResourceOutputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[0]);

  if (rout != "")
    {
      size_t pos;
      string outputs = rout;

      //Removing all "," in the string
      do
	{
	  pos = outputs.find(",");
	  if (pos != EOS)
	    {
	      outputs[pos] = ' ';
	    }
	}
      while (pos != EOS);

      istringstream parse(outputs);
      string ressource;

      //Parsing each ressources one by one
      while (parse >> ressource)
	{
	  result.push_back(ressource);
	}
    }

  return result;
}

/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//      rs_rslist 
//--------------------------------------------
bool rs_rslist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[1]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rs_rslist::getResourceInputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[1]);

  result.push_back(operands[0]);

  return result;
}

//  Warning : because of ARM, this format returns "mem" in the Outputs
vector < string > rs_rslist::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back("mem");

  return result;
}

//--------------------------------------------
//      rds_rslist 
//--------------------------------------------
bool rds_rslist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }

  if (Arch::isInputWrittenRegister(operands[0]) == false)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[1]) == false)
    {
      return false;
    }

  return true;
}

vector < string > rds_rslist::getResourceInputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[1]);

  //Here we extract the register because in ARM, operands[0] is like "r3!" and we just want "r3"
  result.push_back(Arch::extractRegisterFromInputWrittenOperand(operands[0]));

  return result;
}

//  Warning : because of ARM, this format returns "mem" in the Outputs
vector < string > rds_rslist::getResourceOutputs(const vector < string > &operands)
{
  vector < string > result;

  //Here we extract the register because in ARM, operands[0] is like "r3!" and we just want "r3"
  result.push_back(Arch::extractRegisterFromInputWrittenOperand(operands[0]));

  result.push_back("mem");

  return result;
}

//--------------------------------------------
//      rs_rdlist 
//--------------------------------------------
bool rs_rdlist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }

  if (Arch::isRegisterName(operands[0]) == false)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[1]) == false)
    {
      return false;
    }

  return true;
}

//  Warning : because of ARM, this format returns "mem" in the Inputs
vector < string > rs_rdlist::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  result.push_back(operands[0]);
  result.push_back("mem");

  return result;
}

vector < string > rs_rdlist::getResourceOutputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[1]);

  return result;
}

//--------------------------------------------
//      rds_rdlist 
//--------------------------------------------
bool rds_rdlist::isFormat(const vector < string > &operands)
{
  if (operands.size() != 2)
    {
      return false;
    }

  if (Arch::isInputWrittenRegister(operands[0]) == false)
    {
      return false;
    }

  if (Arch::isRegisterList(operands[1]) == false)
    {
      return false;
    }

  return true;
}

//  Warning : because of ARM, this format returns "mem" in the Inputs
vector < string > rds_rdlist::getResourceInputs(const vector < string > &operands)
{
  vector < string > result;

  //Here we extract the register because in ARM, operands[0] is like "r3!" and we just want "r3"
  result.push_back(Arch::extractRegisterFromInputWrittenOperand(operands[0]));

  result.push_back("mem");

  return result;
}

vector < string > rds_rdlist::getResourceOutputs(const vector < string > &operands)
{
  //We extract the registers from the list
  vector < string > result = Arch::extractRegistersFromRegisterList(operands[1]);

  //Here we extract the register because in ARM, operands[0] is like "r3!" and we just want "r3"
  result.push_back(Arch::extractRegisterFromInputWrittenOperand(operands[0]));

  return result;
}
