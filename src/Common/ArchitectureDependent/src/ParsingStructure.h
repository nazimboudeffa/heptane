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
 
 Definition of 6 classes basically used as struct
 - ObjdumpFunction
 - ObjdumpInstruction
 - ObjdumpSymbolTable
 - ObjdumpSection
 - ObjdumpVariable
 - ObjdumpWord (ONLY USED WITH ARM)
 
 *****************************************************************/

#ifndef PARSING_STRUCTURE
#define PARSING_STRUCTURE

#include <string>
#include <vector>
#include <map>
#include "HeptaneStdTypes.h"

using namespace std;

class ObjdumpFunction
{
public:
    string name;
    t_address addr;
    string line;
};

class ObjdumpInstruction
{
public:
    string mnemonic;
    vector<string> operands;
    t_address addr;
    string extra;
    string line;
    string asm_code;
};

class ObjdumpVariable
{
public:
    string name;
    t_address addr;
    int size; //in bytes
    string section_name;
};

class ObjdumpSection
{
public:
    string name;
    t_address addr;
    int size; //in bytes
};

typedef list<std::string> ListOfString;
typedef  map<t_address, ListOfString > typeTableFunctions;
class ObjdumpSymbolTable
{
public:
    typeTableFunctions functions;
    vector<ObjdumpVariable> variables;
    vector<ObjdumpSection> sections;
    t_address MIPS_gp; //ONLY used with MIPS architecture
};

/* THIS CLASS IS ONLY USED WITH ARM ARCHITECTURE */
class ObjdumpWord
{
public:
    t_address addr;
    string type; //can be ".data", ".bss" or "imm" (the ".text" is not defined yet: Logger::addFatal)
    unsigned long value; // if ".data" or ".bss" it's an address, if "imm" it's an immediate value
};


#endif
