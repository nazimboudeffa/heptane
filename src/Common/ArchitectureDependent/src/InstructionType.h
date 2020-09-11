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
 
    InstructionType abstract class definition
    and the concrete classes:
    - Basic
    - Store
    - Load
    - Call
    - Return
    - UnconditionalJump
    - ConditionalJump
    - Word (Specific for ARM)
 
    The concrete classes are used in Arch_dep implementation classes
    like MIPS.cc for the mnemonic2InstructionType map
 
*****************************************************************/

#ifndef INSTRUCTION_TYPE_H
#define INSTRUCTION_TYPE_H

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <set>
#include <stdlib.h>

#include "InstructionFormat.h"

#include "DAAInstruction.h"

using namespace std;

//------------------------------------
//	Abstract class: InstructionType
//------------------------------------
class InstructionFormat;
class InstructionType
{
    protected:
    string functionalUnit;//functional unit
    set<InstructionFormat*> formats;//set of valid InstructionFormat
    
    DAAInstruction* addrAnalysisInstruction;//address analysis instruction
    int latency;  // latency of the instruction.

    public :
    
    /*! Returns true if the instruction is a load instruction */
    virtual bool isLoad();
    
    /*! Returns true if the instruction is a store instruction */
    virtual bool isStore();
    
    /*! Returns true if the instruction is a call instruction */
    virtual bool isCall();
    
    /*! Returns true if the instruction is a return instruction */
    virtual bool isReturn();
    
    /*! Returns true if the instruction is a Unconditionnal Jump instruction */
    virtual bool isUnconditionalJump();
    
    /*! Returns true if the instruction is a Conditionnal Jump instruction */
    virtual bool isConditionalJump();

    /*! Returns the address analysis instruction */
    DAAInstruction* getDAAInstruction();

    /*! Returns the latency of the instruction */
    int getLatency();

    /*****************************************************************
    *			ADDITIONNAL FOR ARM
    *****************************************************************/
    /*! Returns true if the instruction is Predicated (with a condition) */
    virtual bool isPredicated();

    /*! Returns true if the instruction is a Word */
    virtual bool isWord();
    
    /*! Returns true if the instruction is a basic instruction */
    virtual bool isBasic();
    /****************************************************************/


    
    /*! Returns the size of the memory access in bytes, if the instruction is a load/store and 0 otherwise */
    virtual int getSizeOfMemoryAccess();
    
    /*! Returns true if one and only one InstructionFormat corresponds to the operands */
    bool checkFormat(const vector<string>& operands);
    
    /*! Returns the functionalUnits */
    string getResourceFunctionalUnit();
    
    /*! Returns the input resources */
    //wrapper of InstructionFormat
    vector<string> getResourceInputs(const vector<string>& operands);
    
    /*! Returns the output resources */
    //wrapper of InstructionFormat
    vector<string> getResourceOutputs(const vector<string>& operands);
    
    /*! Virtual destructor*/
    virtual ~InstructionType();
};

//--------------------------------------------
//	Basic
//
// all non specific instructions
//--------------------------------------------
class Basic : public InstructionType
{
    public:
    Basic();
    Basic(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isBasic();
};

//---------------------------------------------
//	Store
//
// Classes of instruction that store information
// in memory
//---------------------------------------------
class Store : public InstructionType
{
    private :
    int size_memory_access;
    
    public :
    Store(int size,const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isStore();
    int getSizeOfMemoryAccess();
};

//---------------------------------------------
//	Load
//
// Classes of instruction that load information
// from memory
//---------------------------------------------
class Load : public InstructionType
{
    private :
    int size_memory_access;
    
    public :
    Load(int size, const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isLoad();
    int getSizeOfMemoryAccess();
};

//---------------------------------------------
//	Call
//
// Call instructions
//---------------------------------------------
class Call : public InstructionType
{
    public :
    Call(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isCall();
};

//---------------------------------------------
//	Return
//
// return instructions
//---------------------------------------------
class Return : public InstructionType
{
    public :
    Return(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isReturn();
};

//---------------------------------------------
//	UnconditionnalJump
//
// UnconditionnalJump instructions
//---------------------------------------------
class UnconditionalJump : public InstructionType
{
    public :
    UnconditionalJump(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isUnconditionalJump();
};

//---------------------------------------------
//	ConditionnalJump
//
// ConditionnalJump instructions
//---------------------------------------------
class ConditionalJump : public InstructionType
{
    public :
    ConditionalJump(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isConditionalJump();
};




/***************************************************************


	    ADDITIONNALS TYPES FOR ARM


**************************************************************/


//--------------------------------------------
//	PredicatedBasic
//
// all non specific instructions with a condition
//--------------------------------------------
class PredicatedBasic : public InstructionType
{
    public:
    PredicatedBasic();
    PredicatedBasic(const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isPredicated();
    bool isBasic();
};

//---------------------------------------------
//	PredicatedStore
//
// Classes of instruction with a condition that store information
// in memory
//---------------------------------------------
class PredicatedStore : public InstructionType
{
    private :
    int size_memory_access;
    
    public :
    PredicatedStore(int size,const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isStore();
    bool isPredicated();
    int getSizeOfMemoryAccess();
};

//---------------------------------------------
//	PredicatedLoad
//
// Classes of instruction with a condition that load information
// from memory
//---------------------------------------------
class PredicatedLoad : public InstructionType
{
    private :
    int size_memory_access;
    
    public :
    PredicatedLoad(int size, const string& fu,const set<InstructionFormat*>& f, DAAInstruction* daainstr, int lat);
    bool isLoad();
    bool isPredicated();
    int getSizeOfMemoryAccess();
};

//--------------------------------------------
//	Word
//
// Word instructions (not really instructions)
// Only used with ARM !!
//--------------------------------------------
class Word : public InstructionType
{
public:
    Word(const set<InstructionFormat*>& f);
    bool isWord();
};


#endif
