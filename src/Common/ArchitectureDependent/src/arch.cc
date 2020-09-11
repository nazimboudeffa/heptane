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

#include <climits>
#include <algorithm>
#include "arch.h"
#include "MIPS.h"
#include "ARM.h"
#include "Logger.h"
#include "Utl.h"

//---------------------------------------------------------------------
// Arch class
//---------------------------------------------------------------------

//singleton declaration
Arch_dep *Arch::instance = NULL;

//architecture name declaration
string Arch::architecture_name = "";

//public constructor
void Arch::init(const string & arch, const bool is_big_endian)
{
  if (instance != NULL)
    {
      delete instance;
    }

  architecture_name = arch;

  if (arch == "MIPS")
    {
      instance = new MIPS(is_big_endian);
    }
  else if (arch == "ARM")
    {
      instance = new ARM(is_big_endian);
    }
  else
    {
      Logger::addFatal("Error: architecture '" + arch + "' not supported");
    }
}

//public destructor
void Arch::kill()
{
  if (instance != NULL)
    delete instance;
}

//public accessor
Arch_dep *Arch::getInstance()
{
  assert(instance != NULL);
  return instance;
}

/* bypass getInstance to specific public functions of Arch_dep */
//const string &Arch::getObjdumpTextMarker(){return getInstance()->getObjdumpTextMarker();}

bool Arch::isBigEndian()
{
  return getInstance()->isBigEndian();
}

bool Arch::isCall(const ObjdumpInstruction & instr)
{
  return getInstance()->isCall(instr);
}

bool Arch::isReturn(const ObjdumpInstruction & instr)
{
  return getInstance()->isReturn(instr);
}

bool Arch::isUnconditionalJump(const ObjdumpInstruction & instr)
{
  return getInstance()->isUnconditionalJump(instr);
}

bool Arch::isConditionalJump(const ObjdumpInstruction & instr)
{
  return getInstance()->isConditionalJump(instr);
}

bool Arch::isFunction(const string & line)
{
  return getInstance()->isFunction(line);
}

bool Arch::isInstruction(const string & line)
{
  return getInstance()->isInstruction(line);
}

ObjdumpFunction Arch::parseFunction(const string & line)
{
  return getInstance()->parseFunction(line);
}

ObjdumpInstruction Arch::parseInstruction(const string & line)
{
  return getInstance()->parseInstruction(line);
}

t_address Arch::getJumpDestination(const ObjdumpInstruction & instr)
{
  return getInstance()->getJumpDestination(instr);
}

int Arch::getNBInstrInDelaySlot()
{
  return getInstance()->getNBInstrInDelaySlot();
}

int Arch::getInstructionSize()
{
  return getInstance()->getInstructionSize();
}

bool Arch::isMemPattern(const string & operand)
{
  return getInstance()->isMemPattern(operand);
}

InstructionType *Arch::getInstructionTypeFromMnemonic(const string & mnemonic)
{
  return getInstance()->getInstructionTypeFromMnemonic(mnemonic);
}

InstructionType *Arch::getInstructionTypeFromAsm(const string & instr)
{
  return getInstance()->getInstructionTypeFromAsm(instr);
}

int Arch::getRegisterNumber(const string & reg_name)
{
  return getInstance()->getRegisterNumber(reg_name);
}

bool Arch::isRegisterName(const string & reg_name)
{
  return getInstance()->isRegisterName(reg_name);
}

bool Arch::isZeroRegister(const string & str)
{
  return getInstance()->isZeroRegister(str);
}

bool Arch::isLoad(const string & instr)
{
  return getInstance()->isLoad(instr);
}

bool Arch::isStore(const string & instr)
{
  return getInstance()->isStore(instr);
}

int Arch::getSizeOfMemoryAccess(const string & instr)
{
  return getInstance()->getSizeOfMemoryAccess(instr);
}

vector < string > Arch::splitInstruction(const string & instr)
{
  return getInstance()->splitInstruction(instr);
}

DAAInstruction *Arch::getDAAInstruction(const string & instr)
{
  return getInstance()->getDAAInstruction(instr);
}

int Arch::getLatency(const string & instr)
{
  return getInstance()->getLatency(instr);
}

//---------------------------------------------------------------------
//
// Arch_dep class
//
//---------------------------------------------------------------------

//const string &Arch_dep::getObjdumpTextMarker(){return objdump_text_marker;}

InstructionType *Arch_dep::getInstructionTypeFromMnemonic(const string & mnemonic)
{
  if (mnemonicToInstructionTypes.find(mnemonic) != mnemonicToInstructionTypes.end())
    {
      return mnemonicToInstructionTypes[mnemonic];
    }
  // the instruction does not exist, simply exit
  Logger::addFatal("Error: instruction asm \"" + mnemonic + "\" not defined");
  return NULL;		//stub for pedantic flag: this statement is never reached
}

InstructionType *Arch_dep::getInstructionTypeFromAsm(const string & instr)
{
  // assumed instr format: mnemonic operands
  string mnemonic;
  istringstream parse(instr);
  parse >> mnemonic;
  return getInstructionTypeFromMnemonic(mnemonic);
}

int Arch_dep::getRegisterNumber(const string & reg_name)
{
  if (regs.find(reg_name) != regs.end()) return regs[reg_name];

  // Register name does not exist, simply exit
  Logger::addFatal("Error: register \"" + reg_name + "\" unknow");
  return 0; //stub for pedantic flag: this statement is never reached
}

bool Arch_dep::isRegisterName(const string & reg_name)
{
  return regs.find(reg_name) != regs.end();
}

bool Arch_dep::isZeroRegister(const string & reg_name)
{
  return getRegisterNumber(reg_name) == zero_register_num;
}

bool Arch_dep::isCall(const ObjdumpInstruction & instr)
{
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isCall();
}

bool Arch_dep::isUnconditionalJump(const ObjdumpInstruction & instr)
{
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isUnconditionalJump();
}

bool Arch_dep::isConditionalJump(const ObjdumpInstruction & instr)
{
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isConditionalJump();
}

bool Arch_dep::isLoad(const string & instr)
{
  string s = instr;
  return getInstructionTypeFromAsm(instr)->isLoad() && ! isPcLoadInstruction(s);
}

bool Arch_dep::isStore(const string & instr)
{
  return getInstructionTypeFromAsm(instr)->isStore();
}

int Arch_dep::getSizeOfMemoryAccess(const string & instr)
{
  return getInstructionTypeFromAsm(instr)->getSizeOfMemoryAccess();
}

//IP: to avoid warnings with recent compilers
Arch_dep::~Arch_dep()
{ }

vector < string > Arch_dep::splitInstruction(const string & instr)
{
  string mnemonic, operands(instr);
  istringstream parse(instr);

  //Extracting the mnemonic
  parse >> mnemonic;

  //Getting the operands
  operands.erase(0, mnemonic.length());

  //Splitting these operands into a vector
  vector < string > result = splitOperands(operands);

  //add the mnemonic at the beginning
  result.insert(result.begin(), mnemonic);

  return result;
}

DAAInstruction *Arch_dep::getDAAInstruction(const string & instr)
{
  return getInstructionTypeFromAsm(instr)->getDAAInstruction();
}

int Arch_dep::getLatency(const string & instr)
{
  return getInstructionTypeFromAsm(instr)->getLatency();
}



/***************************************************************

	    MODIFICATIONS FOR ARM

**************************************************************/

/********* Arch *************/

string Arch::getArchitectureName()
{
  return architecture_name;
}

vector < string > Arch::extractInputRegistersFromMem(const string & operand)
{
  return getInstance()->extractInputRegistersFromMem(operand);
}

vector < string > Arch::extractOutputRegistersFromMem(const string & operand)
{
  return getInstance()->extractOutputRegistersFromMem(operand);
}

vector < string > Arch::getResourceFunctionalUnits(const string & instr)
{
  return getInstance()->getResourceFunctionalUnits(instr);
}

bool Arch::isLoadMultiple(const string & instr)
{
  return getInstance()->isLoadMultiple(instr);
}

int Arch::getNumberOfLoads(const string & instr)
{
  return getInstance()->getNumberOfLoads(instr);
}

bool Arch::isStoreMultiple(const string & instr)
{
  return getInstance()->isStoreMultiple(instr);
}

int Arch::getNumberOfStores(const string & instr)
{
  return getInstance()->getNumberOfStores(instr);
}

vector < string > Arch::getResourceInputs(const string & instr)
{
  return getInstance()->getResourceInputs(instr);
}

vector < string > Arch::getResourceOutputs(const string & instr)
{
  return getInstance()->getResourceOutputs(instr);
}

string Arch::getCalleeName(const ObjdumpInstruction & instr)
{
  return getInstance()->getCalleeName(instr);
}

void Arch::parseSymbolTableLine(const string & line, ObjdumpSymbolTable & table)
{
  getInstance()->parseSymbolTableLine(line, table);
}

void Arch::parseReadElfLine(const string & line, ObjdumpSymbolTable & table)
{
  getInstance()->parseReadElfLine(line, table);
}

bool Arch::isWord(const ObjdumpInstruction & instr)
{
  assert(architecture_name == "ARM");
  return getInstance()->isWord(instr);
}

ObjdumpWord Arch::readWordInstruction(const ObjdumpInstruction & instr, ObjdumpSymbolTable & table)
{
  return getInstance()->readWordInstruction(instr, table);
}

bool Arch::isReadelfSectionsMarker(const string & line)
{
  return getInstance()->isReadelfSectionsMarker(line);
}

bool Arch::isReadelfFlagsMarker(const string & line)
{
  return getInstance()->isReadelfFlagsMarker(line);
}

bool Arch::isObjdumpSymbolTableMarker(const string & line)
{
  return getInstance()->isObjdumpSymbolTableMarker(line);
}

bool Arch::isObjdumpTextMarker(const string & line)
{
  return getInstance()->isObjdumpTextMarker(line);
}

bool Arch::isPcInInputResources(const ObjdumpInstruction & instr)
{
  return getInstance()->isPcInInputResources(instr);
}

bool Arch::isPcInOutputResources(const ObjdumpInstruction & instr)
{
  return getInstance()->isPcInOutputResources(instr);
}

vector < ObjdumpWord > Arch::getWordsFromInstr(const ObjdumpInstruction & instr1, const ObjdumpInstruction & instr2, vector < ObjdumpWord > words,
					       bool & is_instr2_consumed)
{
  return getInstance()->getWordsFromInstr(instr1, instr2, words, is_instr2_consumed);
}

bool Arch::isInputWrittenRegister(const string & operand)
{
  return getInstance()->isInputWrittenRegister(operand);
}

string Arch::extractRegisterFromInputWrittenOperand(const string & operand)
{
  return getInstance()->extractRegisterFromInputWrittenOperand(operand);
}

bool Arch::isRegisterList(const string & operand)
{
  return getInstance()->isRegisterList(operand);
}

vector < string > Arch::extractRegistersFromRegisterList(const string & operand)
{
  return getInstance()->extractRegistersFromRegisterList(operand);
}

int Arch::getSizeRegisterList(const string & operand)
{
  return getInstance()->getSizeRegisterList(operand);
}

bool Arch::getLoadStoreARMInfos(bool strongContext, string & instr, string & codeinstr, string & oregister, AddressingMode * vaddrmode,
				offsetType * TypeOperand, string & operand1, string & operand2, string & operand3)
{
  return getInstance()->getLoadStoreARMInfos(strongContext, instr, codeinstr, oregister, vaddrmode, TypeOperand, operand1, operand2, operand3);
}

bool Arch::isPcLoadInstruction(string & instr)
{
  return getInstance()->isPcLoadInstruction(instr);
}

bool Arch::getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack)
{
  return getInstance()->getMultipleLoadStoreARMInfos(instr, codeinstr, oregister, regList, writeBack);
}


bool Arch::getInstr1ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2)
{
  return getInstance()->getInstr1ARMInfos(instr, codeinstr, oregister, TypeOperand, operand1, operand2);
}

bool Arch::getInstr2ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2,
			     string & operand3)
{
  return getInstance()->getInstr2ARMInfos(instr, codeinstr, oregister, TypeOperand, operand1, operand2, operand3);
}


bool Arch::isARMClassInstr(const string &codeop, const string &prefix)
{
  return getInstance()->isARMClassInstr(codeop, prefix);
}

/**
   @return true if codeop is a conditionned version of prefix, false otherwise.
*/
bool Arch::isConditionnedARMInstr(const string &codeop, const string &prefix)
{
  return getInstance()->isConditionnedARMInstr(codeop, prefix);
}


bool Arch::isShifterOperand(const string & operand)
{
  return getInstance()->isShifterOperand(operand);
}

string Arch::extractRegisterFromShifterOperand(const string & operand)
{
  return getInstance()->extractRegisterFromShifterOperand(operand);
}


bool Arch::getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i)
{
  return getInstance()->getRegisterAndIndexStack(instructionAsm, reg, i );
}

/********* Arch_dep *************/

vector < string > Arch_dep::getResourceFunctionalUnits(const string & instr)
{
  vector < string > result;
  result.push_back(getInstructionTypeFromAsm(instr)->getResourceFunctionalUnit());
  return result;
}

vector < string > Arch_dep::getResourceInputs(const string & instr)
{
  string mnemonic, operands(instr);
  istringstream parse(instr);

  //Extracting the mnemonic
  parse >> mnemonic;

  //Getting the operands
  operands.erase(0, mnemonic.length());

  //Splitting these operands into a vector
  vector < string > v_operands = splitOperands(operands);

  return getInstructionTypeFromMnemonic(mnemonic)->getResourceInputs(v_operands);
}

vector < string > Arch_dep::getResourceOutputs(const string & instr)
{
  string mnemonic, operands(instr);
  istringstream parse(instr);

  //Extracting the mnemonic
  parse >> mnemonic;

  //Getting the operands
  operands.erase(0, mnemonic.length());

  //Splitting these operands into a vector
  vector < string > v_operands = splitOperands(operands);

  return getInstructionTypeFromMnemonic(mnemonic)->getResourceOutputs(v_operands);
}

void Arch_dep::parseReadElfLine(const string & line, ObjdumpSymbolTable & table)
{
  //We assume that the input line is in the format : [Nr]  Name  Type  Addr  Off  Size  ES  Flg  Lk  Inf  A
  string local_line(line);

  //Removing character '[' from the line
  if (!Utl::replace(local_line, '[', ' '))
    {
      Logger::addFatal("Error: character '[' not found in the ReadELF line : " + line);
    }

  //Removing character ']' from the line
  if (!Utl::replace(local_line, ']', ' '))
    {
      Logger::addFatal("Error: character ']' not found in the ReadELF line : " + line);
    }
  //Extracting each field
  istringstream parse(local_line);
  string number, name, type, addr, off, size;
  parse >> number >> name >> type >> addr >> off >> size;

  if (name != "NULL")
    {
      assert(number != "");
      assert(name != "");

      //Checking if the current line corresponds to a section to be extracted
      if (find(sectionsToExtract.begin(), sectionsToExtract.end(), name) != sectionsToExtract.end())
	{
	  assert(type != "");
	  assert(addr != "");
	  assert(off != "");
	  assert(size != "");

	  //Converting the string to Unsigned Long
	  t_address address = strtoul(addr.c_str(), NULL, 16);
	  assert(address != 0 && address != ULONG_MAX);

	  //Converting the string to Integer
	  int size_sect = (int)strtol(size.c_str(), NULL, 16);
	  assert(size_sect != 0 && size_sect != INT_MAX);

	  //create the section object
	  ObjdumpSection sect;
	  sect.name = name;
	  sect.addr = address;
	  sect.size = size_sect;

	  //and store it
	  table.sections.push_back(sect);
	}
    }
}

bool Arch_dep::isWord(const ObjdumpInstruction & instr)
{
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isWord();
}

bool Arch_dep::isReadelfSectionsMarker(const string & line)
{
  return (find(readelf_sections_markers.begin(), readelf_sections_markers.end(), line) != readelf_sections_markers.end());
}

bool Arch_dep::isReadelfFlagsMarker(const string & line)
{
  return (find(readelf_flags_markers.begin(), readelf_flags_markers.end(), line) != readelf_flags_markers.end());
}

bool Arch_dep::isObjdumpSymbolTableMarker(const string & line)
{
  return (find(objdump_symboltable_markers.begin(), objdump_symboltable_markers.end(), line) != objdump_symboltable_markers.end());
}

bool Arch_dep::isObjdumpTextMarker(const string & line)
{
  return (find(objdump_text_markers.begin(), objdump_text_markers.end(), line) != objdump_text_markers.end());
}


