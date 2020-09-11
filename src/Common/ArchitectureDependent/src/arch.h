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
 
    Definition of 2 classes basically used as struct
        - ObjdumpFunction
        - ObjdumpInstruction
 
    Definition of Arch which is a wrapper of Arch_dep
        allows a singleton for all the architecture
        definitions which implements Arch_dep interface
 
    Definition of Arch_dep which is an abstract class
        All architecture definitions need 
        to implement this interface

 *****************************************************************/

#ifndef ARCH_DEP
#define ARCH_DEP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h>

#include "ParsingStructure.h"
#include "InstructionType.h"
#include "InstructionFormat.h"
#include "DAAInstruction.h"

using namespace std;

//---------------------------------------------------------------------
// Definition of singleton Arch
//
// This class is a wrapper of Arch_dep implementation classes
//
// Rq 1: each function is a wrapper of a function defined in
//       Arch_dep classes to simplify the utilisation
//       (no need to use getInstance each time)
//
// Rq 2: This is not possible to have directly the singleton in Arch_dep
//       (Abstract class with pure virtual functions)
//---------------------------------------------------------------------
typedef enum{ none_offset, immediate_offset, register_offset, scaled_register_offset} offsetType;
typedef enum{ pre_indexing, auto_indexing, post_indexing} AddressingMode;


class InstructionType;
class Arch_dep;
class Arch
{
private:
    /*!unique instance of the arch_dep class (singleton) */
    static Arch_dep* instance;
    
    /*! string which contains the Architecture's name of the current instance */
    static string architecture_name;
    
public :
    /*!public constructor of the singleton*/
    static void init(const string& arch, const bool is_big_endian);

    /*!public destructor of the singleton*/
    static void kill();
    
    /*!public accessor to the singleton*/
    static Arch_dep* getInstance();
    
    /* bypass getInstance() to specific public functions of arch_dep*/
    //static const string &getObjdumpTextMarker();
    static bool isCall(const ObjdumpInstruction& instr);
    static bool isReturn(const ObjdumpInstruction& instr);
    static bool isUnconditionalJump(const ObjdumpInstruction& instr);
    static bool isConditionalJump(const ObjdumpInstruction& instr);
    static bool isFunction(const string &line);
    static bool isInstruction(const string &line);
    static ObjdumpFunction parseFunction(const string &line);
    static ObjdumpInstruction parseInstruction(const string &line);
    static t_address getJumpDestination(const ObjdumpInstruction& instr);
    static int getNBInstrInDelaySlot();
    static int getInstructionSize();
    static bool isMemPattern(const string& operand);
    static InstructionType* getInstructionTypeFromMnemonic(const string& mnemonic);
    static InstructionType* getInstructionTypeFromAsm(const string& instr);
    static int getRegisterNumber(const string& reg_name);
    static bool isRegisterName(const string& reg_name);
    static bool isZeroRegister(const string& str);
    static bool isDecNumber(const string& str);
    static bool isHexNumber(const string& str);
    static bool isAddr(const string& str);
    static bool isLoad(const string& instr);
    static bool isStore(const string& instr);
    static int getSizeOfMemoryAccess(const string& instr);

    static bool isBigEndian();

    static vector<string> splitInstruction(const string& instr);
    
    static DAAInstruction* getDAAInstruction(const string& instr);
    static int getLatency(const string& instr);
    /*********************************************************
		    MODIFICATIONS FOR ARM
    *********************************************************/

    //static string extractRegisterFromMem(const string& operand);
     static vector<string> extractInputRegistersFromMem(const string& operand);
     static vector<string> extractOutputRegistersFromMem(const string& operand);
    
    //static string getResourceFunctionalUnit(const string& instr);
     static vector<string> getResourceFunctionalUnits(const string& instr);


    //static string getResourceInputs(const string& instr);
    //static string getResourceOutputs(const string& instr);
    static vector<string> getResourceInputs(const string& instr);
    static vector<string> getResourceOutputs(const string& instr);



    static bool isLoadMultiple(const string& instr);

    static int getNumberOfLoads(const string& instr);

    static bool isStoreMultiple(const string& instr);

    static int getNumberOfStores(const string& instr);

    static string getCalleeName(const ObjdumpInstruction& instr);

    static void parseSymbolTableLine(const string& line, ObjdumpSymbolTable& table);

    //added
    static void parseReadElfLine(const string& line, ObjdumpSymbolTable& table);
    
    static bool isWord(const ObjdumpInstruction& instr);
    
    static ObjdumpWord readWordInstruction(const ObjdumpInstruction& instr, ObjdumpSymbolTable& table);
    
    static string getArchitectureName();
    
    static bool isReadelfSectionsMarker(const string& line);
    
    static bool isReadelfFlagsMarker(const string& line);
    
    static bool isObjdumpSymbolTableMarker(const string& line);
    
    static bool isObjdumpTextMarker(const string& line);
    
    
    static bool isPcInInputResources(const ObjdumpInstruction& instr);
    static bool isPcInOutputResources(const ObjdumpInstruction& instr);
    
    static vector<ObjdumpWord> getWordsFromInstr(const ObjdumpInstruction& instr1, const ObjdumpInstruction& instr2, vector<ObjdumpWord> words, bool& is_instr2_consumed);
    

    static bool isInputWrittenRegister(const string& operand);
    
    static string extractRegisterFromInputWrittenOperand(const string& operand);
    
    static bool isRegisterList(const string& operand);
    
    static vector<string> extractRegistersFromRegisterList(const string& operand);
    static int getSizeRegisterList(const string& operand);
    static bool isShifterOperand(const string& operand);
    
    static string extractRegisterFromShifterOperand(const string& operand);
    static bool getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i);

    static bool getLoadStoreARMInfos(bool strongContext, string &instr, string &codeinstr, string &oregister, AddressingMode *vaddrmode, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3);
    static bool isPcLoadInstruction(string & instr);

    static bool getInstr1ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2);
    static bool getInstr2ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3);
    static bool isARMClassInstr(const string &codeop, const string &prefix);
    static bool isConditionnedARMInstr(const string &codeop, const string &prefix);
    static bool getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack);
    /**********************************************************
    ***********************************************************
    ***********************************************************/
};


//---------------------------------------------------------------------
// arch_dep abstract class definition
//
// all the virtual methods needs to be implemented when defining
// an architecture
//---------------------------------------------------------------------
class InstructionType;
class InstructionFormat;
class Arch_dep
{
public:
    
  //IP: to avoid warnings with recent compilers
  virtual ~Arch_dep();
  
  /***** Objdump parsing functions *****/
    
  /*!Returns code section indicator of an objdump file*/
  //const string &getObjdumpTextMarker();
 
  /*! Returns true if the architecture is a big endian architecture */
  virtual bool isBigEndian()=0;
  
  /*! Returns true if instr is a call instruction*/
  bool isCall(const ObjdumpInstruction& instr);
  
  /*! Returns true if instr is an unconditional jump instruction*/
  bool isUnconditionalJump(const ObjdumpInstruction& instr);
  
  /*! Returns true if instr is a conditional jump instruction*/
  bool isConditionalJump(const ObjdumpInstruction& instr);
  
  /*! Returns true if the line corresponds to a function declaration in the objdump file*/
  virtual bool isFunction(const string &line)=0;
  
  /*! Returns true if the line corresponds to an instruction in the objdump file*/
  virtual bool isInstruction(const string &line)=0;
  
  /*! Parser of a function declaration line*/
  virtual ObjdumpFunction parseFunction(const string &line)=0;
    
  /*! Parser of an instruction line*/
  virtual ObjdumpInstruction parseInstruction(const string &line)=0;
    
  /*! Returns the jump target of instr*/
  virtual t_address getJumpDestination(const ObjdumpInstruction& instr)=0;
    
    
  /***** Format functions *****/
    
  /*! Returns true if operand is a memory access pattern */
  //memory access pattern means the address part of a load/store instruction. i.e. load register, offset (base_register)
  virtual bool isMemPattern(const string& operand)=0;


  /*******************************************************************
		MODIFICATIONS FOR ARM
  ********************************************************************/
  // Returns the register name present in operand 
  // The base_registerof a load/store instruction. i.e. load register, offset (base_register)
  //virtual string extractRegisterFromMem(const string& operand)=0;
  
  /*! Returns the names of the input registers present in a memory access */
  //example in ARM : ldr r1, [r2, r3, lsl r4] ---> result will be r2,r3,r4
  //example in MIPS : lw v0, 16(s8) --> result will be s8
  virtual vector<string> extractInputRegistersFromMem(const string& operand)=0;

  /*! Returns the names of the ouput registers present in a memory access */
  //example in ARM : ldr r1, [r2, r3, lsl r4]! ---> result will be r2
  //in MIPS, the result is always empty
  virtual vector<string> extractOutputRegistersFromMem(const string& operand)=0;

  // Returns the functional unit used by an instruction
  //assume instr: mnemonic operands
  //string getResourceFunctionalUnit(const string& instr);

  /*! Returns the functional units used by an instruction */
  //assume instr: mnemonic operands
  virtual vector<string> getResourceFunctionalUnits(const string& instr);

  /*! Returns true if the instructions is a Load of multiple data */
  virtual bool isLoadMultiple(const string& instr)=0;

  /*! Returns the number of Loads in an instruction */
  virtual int getNumberOfLoads(const string& instr)=0;

  /*! Returns true if the instructions is a Store of multiple data */
  virtual bool isStoreMultiple(const string& instr)=0;

  /*! Returns the number of Stores in an instruction */
  virtual int getNumberOfStores(const string& instr)=0;

  // Returns true if instr is a return instruction
  //bool isReturn(const ObjdumpInstruction& instr);

  /*! Returns true if instr is a return instruction*/
  virtual bool isReturn(const ObjdumpInstruction& instr)=0;


  /*! Returns the name of the function called in a Call Instruction */
  virtual string getCalleeName(const ObjdumpInstruction& instr)=0;


  /*! Parse a line of the symbol table and update the ObjdumpSymbolTable object in parameter */
  virtual void parseSymbolTableLine(const string& line, ObjdumpSymbolTable& table)=0;

  //added
    /*! Parse a line of the ReadELF file and update the ObjdumpSymbolTable object in parameter */
    void parseReadElfLine(const string& line, ObjdumpSymbolTable& table);
    
    
    /*! Returns true if instr is a Word (not really an instruction) */
    bool isWord(const ObjdumpInstruction& instr);
    
    
    /*! Returns an ObjdumpWord object containing all the useful information from instr */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    virtual ObjdumpWord readWordInstruction(const ObjdumpInstruction& instr, ObjdumpSymbolTable& table)=0;
    
    
    /*! Returns true if the input line is a marker of Sections in a ReadELF file */
    bool isReadelfSectionsMarker(const string& line);
    
    /*! Returns true if the input line is a marker of Flags in a ReadELF file */
    bool isReadelfFlagsMarker(const string& line);
    
    /*! Returns true if the input line is a marker of Symbol Table in an Objdump file */
    bool isObjdumpSymbolTableMarker(const string& line);
    
    /*! Returns true if the input line is a marker of Text Section in an Objdump file */
    bool isObjdumpTextMarker(const string& line);
    
    
    /*! Returns true if the instruction has PC (program counter) in input registers */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    virtual bool isPcInInputResources(const ObjdumpInstruction& instr)=0;
    virtual bool isPcInOutputResources(const ObjdumpInstruction& instr)=0;
    
    
    /*! Returns the .word used by a an instruction */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //instr1 : current instruction
    //instr2 : next instruction
    //words : list of .word of the current function
    //is_instr2_consumed : to know after the call of this function if instr2 has been consumed
    virtual vector<ObjdumpWord> getWordsFromInstr(const ObjdumpInstruction& instr1, const ObjdumpInstruction& instr2, vector<ObjdumpWord> words, bool& is_instr2_consumed)=0;

    
    /*! Returns true if the operand is an Input register which is written */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : r3! --> true  |  r3 --> false
    virtual bool isInputWrittenRegister(const string& operand)=0;
    
    /*! Returns the name of the register in an Input Written Register pattern */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : r3! --> r3
    virtual string extractRegisterFromInputWrittenOperand(const string& operand)=0;
    
    
    /*! Returns true if the operand is a register list */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : {r1, r2, r3-r6} ---> true  |  [r3, r5] ---> false
    virtual bool isRegisterList(const string& operand)=0;
    
    
    /*! Returns the name of the registers in the list */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : {r1, r2, r3-r6} --> r1, r2, r3, r4, r5, r6
    virtual vector<string> extractRegistersFromRegisterList(const string& operand)=0;

    /*! Returns the number of registers in operand assumed to be a register list. */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : {r1, r2, r3-r6} --> 6 
    virtual int getSizeRegisterList(const string& operand)=0;

    /*! Returns true if the operand is a shifter */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : "lsl r5" ---> true   |  "asr #4"  ----> true  |  "rrx"  ---> true
    virtual bool isShifterOperand(const string& operand)=0;
    
    
    /*! Returns the name of the register which is used by the shifter (result can be empty) */
    /*! ONLY USED WITH ARM ARCHITECTURE */
    //ARM example : "lsl r5" ---> r5    |  "asr #4" ----> ""   |  "rrx" ----> ""
    virtual string extractRegisterFromShifterOperand(const string& operand)=0;
    virtual bool getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i)=0;

    /*! ONLY USED WITH ARM ARCHITECTURE */

    virtual bool getLoadStoreARMInfos(bool strongContext, string &instr, string &codeinstr, string &oregister, AddressingMode *vaddrmode, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3)=0;
    virtual bool isPcLoadInstruction(string & instr)=0;
    virtual bool getInstr1ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2)=0;
    virtual bool getInstr2ARMInfos(string &instr, string &codeinstr, string &oregister, offsetType *TypeOperand, string &operand1, string &operand2, string &operand3)=0;
    virtual bool isARMClassInstr(const string &codeop, const string &prefix)=0;
    virtual bool isConditionnedARMInstr(const string &codeop, const string &prefix)=0;
    virtual bool getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack)=0;
  /**********************************************************************
   **********************************************************************
   *********************************************************************/

  /***** Pipeline analysis functions *****/
  
  /*! Returns the input resources */
  //assume instr: mnemonic operands
  // /!\ Warning /!\ the result can contain duplicate ressources
  // example in ARM : mul r0, r1, r1
  vector<string> getResourceInputs(const string& instr);
    
  /*! Returns the output resources */
  //assume instr: mnemonic operands
  // /!\ Warning /!\ the result can contain duplicate ressources
  vector<string> getResourceOutputs(const string& instr);
      
  /***** Useful functions *****/
  
  /*! split each part of the instruction into a vector */
  //instr='mnemonic op1,op2' -> vector=[mnemonic;op1;op2]
  vector<string> splitInstruction(const string& instr);
    
  /*! Returns the InstructionType object which corresponds to the mnemonic */
  InstructionType* getInstructionTypeFromMnemonic(const string& mnemonic);
    
  /*! Returns the InstructionType object which corresponds to the mnemonic present in instr*/
  //assume instr: mnemonic ...
  InstructionType* getInstructionTypeFromAsm(const string& instr);
    
  /*! Returns the number of instructions in the delay slot*/
  virtual int getNBInstrInDelaySlot()=0;
    
  /*! Returns the size in bytes of an instruction*/
  virtual int getInstructionSize()=0;
    
  /*! Returns the register number of reg_name*/
  int getRegisterNumber(const string& reg_name);
    
  /*! Returns true if reg_name is a register name*/
  bool isRegisterName(const string& reg_name);
    
  /*! Returns true if reg_name is the register with a content set to 0*/
  bool isZeroRegister(const string& reg_name);
    
  /*! Returns true if str is a positive or negative integer in Dec*/
  bool isDecNumber(const string& str);
    
  /*! Returns true if str is a positive integer in Hex (0x...) */
  bool isHexNumber(const string& str);
    
  /*! Returns true if str is an addr (32 bits) */
  bool isAddr(const string& str);
    
  /*! Returns true if instr is a load*/
  //assume instr: mnemonic ...
  //wrapper of the InstructionType function
  bool isLoad(const string& instr);
    
  /*! Returns true if instr is a store*/
  //assume instr: mnemonic ...
  //wrapper of the InstructionType function
  bool isStore(const string& instr);
    
  /*! Returns the size of the memory access in bytes, if the instr is a load/store and 0 otherwise */
  int getSizeOfMemoryAccess(const string& instr);
  
  /*! Returns the address analysis instruction */
  DAAInstruction* getDAAInstruction(const string& instr);

  /*! Returns the latency of the instruction */
  int getLatency(const string& instr);
    
protected:
    
  /***** Pipeline analysis functions *****/
  
  /*! split the operands into a vector*/
  virtual vector<string> splitOperands(const string& operands)=0;
  
  
  /***** class fields *****/
  
  /*!code section indicator of an objdump file*/
  //string objdump_text_marker;
  
  /*! number of the register which is set to 0*/
  int zero_register_num;
    
  /*! map which associate a mnemonic with an InstructionType object*/
  map<string, InstructionType*> mnemonicToInstructionTypes;
    
  /*! map which associate a string register with its integer value*/
  //Even if there is no number associated to register in the architecture
  //The description needs a unique ID for each register name
  map<string,int> regs;
    
  /*! map which associate a string format with an InstructionFormat object*/
  map<string, set<InstructionFormat*> > formats;
    
    /*! vector which contains all the sections (name) to extract from the ReadELF file */
    vector<string> sectionsToExtract;
    
    /*! vector which contains section list indicators of a readelf file */
    vector<string> readelf_sections_markers;
    
    /*! vector which contains flag list indicators of a readelf file */
    vector<string> readelf_flags_markers;
    
    /*! vector which contains code section indicators of an objdump file */
    vector<string> objdump_text_markers;
    
    /*! vector which contains symbol table section indicators of an objdump file */
    vector<string> objdump_symboltable_markers;
    
};

#endif
