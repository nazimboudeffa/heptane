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
 
    ARM architecture definition

	- User/System modes only (no [FIQ, IRQ, Supervisor, Abort, Undefined] modes)
	- No AArch64 (32-bit architecture only)
	- No Thumb/Thumb-2/ThumbEE/Jazelle/VFP/NEON Instruction Sets
	- cpsr register (pour les flags) pas pris en compte en tant que registre destination lors des instructions

 *****************************************************************/

#ifndef ARCH_ARM
#define ARCH_ARM

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h>

#include "arch.h"

using namespace std;


#define ARM_SP_REGISTER  13
#define ARM_PC_REGISTER  15

#define ARM_AUX_REGISTER 25
#define ARM_NB_REGISTERS 26 /* Using an auxiliary register (ARM_AUX_REGISTER) */

#define IS_PC_REGISTER(regnum) (regnum == ARM_PC_REGISTER)


class ARM : public Arch_dep
{
public:
    
    /*!constructor*/
  //    ARM(const bool is_big_endian_p);
  ARM(const bool is_big_endian_p);
    /*!destructor*/
    ~ARM();
    
    // Architecture endianess
    bool isBigEndian() {return is_big_endian;}
    
    /***** Objdump parsing functions *****/
    
    /*! Returns true if the line corresponds to a function declaration in the objdump file*/
    bool isFunction(const string &line);
    
    /*! Returns true if the line corresponds to an instruction in the objdump file*/
    bool isInstruction(const string &line);
    
    /*! Parser of a function declaration line*/
    ObjdumpFunction parseFunction(const string &line);
    
    /*! Parser of an instruction line*/
    ObjdumpInstruction parseInstruction(const string &line);
    
    /*! Returns the jump target of instr*/
    t_address getJumpDestination(const ObjdumpInstruction& instr);

    /*! Returns the name of the function called in a Call Instruction */
    string getCalleeName(const ObjdumpInstruction& instr);
    
    /*! Parse a line of the symbol table and update the ObjdumpSymbolTable object in parameter */
    void parseSymbolTableLine(const string& line, ObjdumpSymbolTable& table);
    
    /*! Returns an ObjdumpWord object containing all the useful information from instr */
    ObjdumpWord readWordInstruction(const ObjdumpInstruction& instr, ObjdumpSymbolTable& table);
    
    /*! Returns true if the instruction has PC (program counter) in input registers */
    bool isPcInInputResources(const ObjdumpInstruction& instr);

    /*! Returns true if the instruction has PC (program counter) in output registers */
    bool isPcInOutputResources(const ObjdumpInstruction& instr);
    
    /*! Returns the .word used by a an instruction */
    //instr1 : current instruction
    //instr2 : next instruction
    //words : list of .word of the current function
    //is_instr2_consumed : to know after the call of this function if instr2 has been consumed
    vector<ObjdumpWord> getWordsFromInstr(const ObjdumpInstruction& instr1, const ObjdumpInstruction& instr2, vector<ObjdumpWord> words, bool& is_instr2_consumed);

    /***** Format functions *****/
    
    /*! Returns true if operand is a memory access pattern */
    bool isMemPattern(const string& operand);

    /*! Returns the names of the input registers present in operand */
    vector<string> extractInputRegistersFromMem(const string& operand);

    /*! Returns the names of the ouput registers present in operand */
    vector<string> extractOutputRegistersFromMem(const string& operand);

    /*! Returns true if the instructions is a Load of multiple data */
    bool isLoadMultiple(const string& instr);

    /*! Returns the number of Loads in an instruction */
    int getNumberOfLoads(const string& instr);

    /*! Returns true if the instructions is a Store of multiple data */
    bool isStoreMultiple(const string& instr);

    /*! Returns the number of Stores in an instruction */
    int getNumberOfStores(const string& instr);

    /*! Returns true if instr is a return instruction*/
    bool isReturn(const ObjdumpInstruction& instr);

    /*! Returns true if the operand is an Input register which is written */
    //ARM example : r3! --> true  |  r3 --> false
    bool isInputWrittenRegister(const string& operand);
    
    /*! Returns the name of the register in an Input Written Register pattern */
    //ARM example : r3! --> r3
    string extractRegisterFromInputWrittenOperand(const string& operand);
    
    /*! Returns true if the operand is a register list */
    //ARM example : {r1, r2, r3-r6} ---> true  |  [r3, r5] ---> false
    bool isRegisterList(const string& operand);
    
    /*! Returns the name of the registers in the list */
    //ARM example : {r1, r2, r3-r6} --> r1, r2, r3, r4, r5, r6
    vector<string> extractRegistersFromRegisterList(const string& operand);

    /*! Returns the number of registers in operand assumed to be a register list. */
    int getSizeRegisterList(const string& operand);

    /*! Returns true if the operand is a shifter */
    //ARM example : "lsl r5" ---> true   |  "asr #4"  ----> true  |  "rrx"  ---> true
    bool isShifterOperand(const string& operand);
    
    /*! Returns the name of the register which is used by the shifter (result can be empty) */
    //ARM example : "lsl r5" ---> r5    |  "asr #4" ----> ""   |  "rrx" ----> ""
    string extractRegisterFromShifterOperand(const string& operand);
    
    
    /***** Useful functions *****/
    
    /*! Returns the number of instructions in the delay slot*/
    int getNBInstrInDelaySlot(){return 0;}
    
    /*! Returns the size in bytes of an instruction*/
    int getInstructionSize(){return 4;}


    
    /*! Returns the functional units used by an instruction */
    vector<string> getResourceFunctionalUnits(const string& instr);

    bool getLoadStoreARMInfos(bool strongContext, string &instr, string &codeinstr, string &oregister, AddressingMode *vaddrmode, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3);
    bool isPcLoadInstruction(string & instr);
    bool getInstr1ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2);
    bool getInstr2ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3);
    bool getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack);

    bool isARMClassInstr(const string &codeinstr, const string &prefix);
    bool isConditionnedARMInstr(const string &codeop, const string &prefix);
    bool getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i);

    int getMemoryStoreLatency();
    int getMemoryLoadLatency();
    void setMemoryStoreLatency(int val);
    void setMemoryLoadLatency(int val);

private:
    // Architecture endianness
    bool is_big_endian;
    
    /* Specific characters for ARM instructions */
    string list_begin_character;
    string list_end_character;
    string register_writting_character;
    string register_range_character;
    string memory_access_begin_character;
    string memory_access_end_character;


    /***** Pipeline analysis functions *****/
    
    /*! split the operands into a vector*/
    vector<string> splitOperands(const string& operands);

    /***** Specific functions for ARM *****/

    /** @return true the instruction (instr) is an STR or LDR instruction, false otherwise(LDM, STM) */
    bool isLoadStoreSimple(string &instr);

    /*! Removes useless characters of an objdump line*/
    string removeUselessCharacters(const string &line);

    /*! Removes useless characters of a MemPattern*/
    string removeUselessCharactersMemPattern(const string &line);
    
    /*! Returns true if instr is the binary representation of an asm instruction */
    //removed: no need with correct objdump flags
    //bool isBinaryAsmCode(const string& str);


    /*! For the detection of Shifting and Rotating operands in Instcutions parsing */
    vector<string> shifter_with_operand;
    vector<string> shifter_without_operand;


    /*! Returns true if operand is a memory access pattern */
    // operand: [base, int_offset] or [base]
    bool isMem_Basic_Pattern(const string& operand);

    /*! Returns true if operand is a memory access pattern with Register Writting*/
    // operand: [base, int_offset]!
    bool isMem_Basic_PatternRW(const string& operand);

    /*! Returns true if operand is a memory access pattern */
    // operand: [base, reg_offset]
    bool isMem_RegReg_Pattern(const string& operand);

    /*! Returns true if operand is a memory access pattern with Register Writting*/
    // operand: [base, reg_offset]!
    bool isMem_RegReg_PatternRW(const string& operand);
    
    /*! Returns true if operand is a memory access pattern */
    // operand: [base, reg, shifter reg]
    bool isMem_RegRegShifterReg_Pattern(const string& operand);

    /*! Returns true if operand is a memory access pattern with Register Writting*/
    // operand: [base, reg, shifter reg]!
    bool isMem_RegRegShifterReg_PatternRW(const string& operand);

    /*! Returns true if operand is a memory access pattern */
    // operand: [base, reg, shifter int] or [base, reg, rrx]
    bool isMem_RegRegShifterInt_Pattern(const string& operand);

    /*! Returns true if operand is a memory access pattern with Register Writting*/
    // operand: [base, reg, shifter int]! or [base, reg, rrx]!
    bool isMem_RegRegShifterInt_PatternRW(const string& operand);

    /*! Returns true if the instruction is a Pop with PC register, it is another way to do a Return Instruction */
    bool isPopWithPC(const ObjdumpInstruction& instr);
    
    /*! Split a register list into a vector */
    /* example
     operand = "{r1, r2-r4}"
     result = "{", "r1", "r2", "-", "r4", "}"
     */
    vector<string> splitRegisterListIntoVector(const string& operand);
    
    /*! Vector which contains all the functions used in ARM library */
    vector<string> ARM_lib_functions;
};

#endif
