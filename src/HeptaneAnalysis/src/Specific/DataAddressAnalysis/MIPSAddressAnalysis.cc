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

#include "MIPSAddressAnalysis.h"
#include "Generic/CallGraph.h"
#include "Utl.h"

// #define TRACE(S) S

/**
   MIPS: This is used to get the size of the stack frame of a function with its first asm instruction
   return the size of the stack frame. 
*/
int MIPSAddressAnalysis::getStackSize (Cfg * cfg)
{
  // Get the first instruction to get the stack size  (mips example : addiu sp, sp, -24 )
  Node *firstNode = cfg->GetStartNode ();
  const vector < Instruction * >&listInstr = firstNode->GetAsm ();
  assert (listInstr.size () != 0);
  Instruction *instr = listInstr[0];

  //Split the instruction
  vector < string > v_instr = Arch::splitInstruction (instr->GetCode ());

  if (v_instr.size () == 4 && v_instr[1] == "sp" && v_instr[2] == "sp")	//sp modification  (example : addiu sp,sp,-72)
    {
      int stackSize = Utl::string2int(v_instr[3]);
      if (stackSize < 0) stackSize = -stackSize;
      return stackSize;
    }
  // a leaf in the call graph (no dedicated stack frame in MIPS) is accepted.
  assert ( ! isCfgWithCallNode(cfg));
  return 0;
}

/* mem_pattern is assume to be "val(sp)"
void MIPSAddressAnalysis::extractRegVal( string mem_pattern, string &reg, string &val)
{
  Utl::replace( mem_pattern, '(', ' ');
  Utl::replace( mem_pattern, ')', ' ');
  istringstream parse (mem_pattern);
  parse >> val >> reg;
  } */

int MIPSAddressAnalysis::getStackMaxOffset(string s_instr, int StackMaxOffset)
{
  string val, reg;

  // LBesnard : on peut acceder à des paramètres empilés ? dépendant compilateur
  if (s_instr.find ("sp") != EOS)  // instruction references "sp"
    {
      vector < string > v_instr = Arch::splitInstruction (s_instr);
      assert (v_instr.size () == 3);	//mnemonic op1, val(reg) 
      Utl::extractRegVal( v_instr[2], reg, val);
      assert (reg == "sp");
      return Utl::imax(abs(Utl::string2int(val)),StackMaxOffset);
    }
  return StackMaxOffset;
}



//----------------------------------------------------------------------------

//Intra block data analysis related functions

// Set the AddressAttribute for a sp access
void MIPSAddressAnalysis::analyzeStack (Cfg * cfg, Instruction * Instr, long offset, string access, int sizeOfMemoryAccess, bool precision, Context *context)
{

  //get the StackInfoAttributeName of current cfg
  assert (cfg->HasAttribute (StackInfoAttributeName));
  StackInfoAttribute attribute = (StackInfoAttribute &) cfg->GetAttribute (StackInfoAttributeName);

  //get the stack information of cfg
  long stack_size = attribute.getFrameSizeWithoutCaller ();
  long stack_maxoffset = attribute.getFrameSizeWithCaller ();

  /* 
  if (stack_maxoffset < offset)
    {
      Logger::addError ("stack analysis -> maxOffset < offset");
      //in case of error: stack_maxoffset computation needs to be checked
      //accesses to take into account:
      //addiu $v0,$sp,8
      //lw $a1,7($v0)
      } */
  if (stack_maxoffset < offset)
    {
      long addr, size, stack_maxoffset_caller;

      Context *context_caller = context->getCallerContext ();
      assert (context_caller != NULL);
      Cfg *caller = context_caller->getCurrentFunction ();
      assert (caller->HasAttribute (StackInfoAttributeName));
      StackInfoAttribute caller_attribute = (StackInfoAttribute &) caller->GetAttribute (StackInfoAttributeName);
      long sp_caller = caller_attribute.getSP (context_caller->getStringId ());
      stack_maxoffset_caller = caller_attribute.getFrameSizeWithCaller ();
      addr = sp_caller;
      // precision= false;
      size = stack_maxoffset_caller;
      AddressInfo contextual_info = mkAddressInfo(Instr, access, precision, "stack","", addr, size);
      setContextualAddressAttribute(Instr, context, contextual_info);
      }
  else
    {
      long addr, size, stack_maxoffset_caller;
      long sp = attribute.getSP (context->getStringId ());
      
      //check that the stack size is equal to the caller stack size
      //if the function is without stack frame (leaf of the context tree)
      //and keep the stack_maxoffset of the caller in case !precision
      if (stack_size == 0)
	{
	  Context *context_caller = context->getCallerContext ();
	  assert (context_caller != NULL);
	  Cfg *caller = context_caller->getCurrentFunction ();
	  assert (caller->HasAttribute (StackInfoAttributeName));
	  StackInfoAttribute caller_attribute = (StackInfoAttribute &) caller->GetAttribute (StackInfoAttributeName);
	  long sp_caller = caller_attribute.getSP (context_caller->getStringId ());
	  stack_maxoffset_caller = caller_attribute.getFrameSizeWithCaller ();
	  assert (sp == sp_caller);
	}
      //--
      
      if (precision)
	{
	  addr = sp + offset;
	  size = sizeOfMemoryAccess;
	}
      else
	{
	  addr = sp;
	  // TODO: check the MIPS ABI if the parameters of the caller can be accessed if not stack_size_caller can be used instead
	  if (stack_size == 0) size = stack_maxoffset_caller; else size = stack_maxoffset;
	}
      AddressInfo contextual_info = mkAddressInfo(Instr, access, precision, "stack","", addr, size);
      setContextualAddressAttribute(Instr, context, contextual_info);
    }
}


// set the AddressAttribute for a gp or lui access; 
// local to setLoadStoreAddressAttribute
void MIPSAddressAnalysis::analyzeReg (Instruction * Instr, long addr, string access, int sizeOfMemoryAccess, bool precision)
{
  string var_name;
  unsigned long start_addr = 0;
  int size = 0;
  string section_name;

  symbol_table.getInfo (addr, &var_name, &start_addr, &size, &section_name);

  if (start_addr == 0 || size == 0)
    {
      Logger::addError ("reg analysis -> access to unknown area");
    }
  else
    {
      mkAddressInfoAttribute(Instr, access, precision, section_name, var_name,  ( precision ? addr: start_addr) , ( precision ? sizeOfMemoryAccess : size ) );
    }
}


// ------------------ Private------------------------------------



long  MIPSAddressAnalysis::GetOffsetValue(string asm_code)
{
  //get the offset: offset(gp)
  vector < string > v_instr = Arch::splitInstruction (asm_code);
  assert (v_instr.size () == 3); // mnemonic op1,offset(reg)
  string op2 = v_instr[2];
  string offset = op2.erase (op2.find ("("));
  return  Utl::string2long(offset);
  // assert (Utl::isDecNumber (offset));  return atol (offset.c_str ());
}

// Setting the address attribute for load and Store instructions. "li, lui" are not a load.
// @return true when it is applied, false otherwise.
bool MIPSAddressAnalysis::setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state, Context *context)
{
  string access, asm_code;

  asm_code = vinstr->GetCode ();
  bool isLoad = Arch::isLoad (asm_code);
  bool isStore = Arch::isStore (asm_code);
  if (!isLoad && !isStore) return false;

  // lui is not a load. --See MIPS.cc : mnemonicToInstructionTypes["lui"]    =new Basic("alu",formats["rd_hex"],new Lui())--; 
  // Load MIPS= { ["lw"], ["lwl"], ["lwr"], ["lwc1"], ["lh"], ["lhu"], ["lb"], ["lbu"], ["ldc1"] }
  // Store MIPS = { ["sw"], ["swc1"], ["sh"], ["sb"], ["sdc1"] }

  // get the size of the access according to the op code
  int sizeOfMemoryAccess = Arch::getSizeOfMemoryAccess (asm_code);

  //define the access type
  if (isLoad) { access = "read"; } else { access = "write"; }
  TRACE(  state->printStack();  state->printStates(););
  TRACE( cout << " instruction = " << asm_code << endl);
  //access using gp register
  if (asm_code.find ("gp") != EOS)
    {
      long loffset = GetOffsetValue(asm_code); //mnemonic op1,offset(gp)
      long addr = symbol_table.getGP ();
      addr = addr + loffset;
      analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, true);
      TRACE( cout << " analyzeReg (gp) " << endl);
    }
  //access using $sp register
  else if (asm_code.find ("sp") != EOS)
    {
      long loffset = GetOffsetValue(asm_code); // mnemonic op1,offset(sp)
      analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true, context);
      TRACE( cout << " analyzeStack (sp) " << endl);
    }
  else
    {
      // access using $reg different from $gp and $sp, but the used register may be have state that reference sp, gp.
      vector < string > access_pattern;
      state->accessAnalysis (vinstr, access_pattern);

      TRACE( cout << " === state->accessAnalysis returns, code = " << access_pattern[0] << endl);
      TRACE( cout << " === state->accessAnalysis returns, value = " << access_pattern[1] << endl);
      TRACE( cout << " === state->accessAnalysis returns, precision = " << access_pattern[2] << endl);

      if (access_pattern[0] == "lui")
	{
	  long addr = Utl::string2long(access_pattern[1]);
	  analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, access_pattern[2] == "1");
	}
      else if (access_pattern[0] == "gp")
	{
	  long addr = symbol_table.getGP () + Utl::string2long(access_pattern[1]);
	  analyzeReg (vinstr, addr , access, sizeOfMemoryAccess, access_pattern[2] == "1");
	}
      else if (access_pattern[0] == "sp")
	{
	  long loffset = Utl::string2long(access_pattern[1]);
	  TRACE(cout << " Stack pointer ----*****------" << asm_code << endl;);
	  analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, access_pattern[2] == "1", context);
	}
      else //pointer: all addresses can be accessed (stub)
	{
	  TRACE(cout << " POINTER ----*****------" << asm_code  << endl;);
	  setPointerAccessInfo(vinstr, access);
	}
    }
  return true;
}

   
RegState* MIPSAddressAnalysis::NewRegState(int stackSize)
{
  return new MIPSRegState(stackSize);
}

bool MIPSAddressAnalysis::importCallerArguments(AbstractRegMem &vAbstractRegMemCaller, AbstractRegMem &vAbstractRegMemCalled)
{
  RegState *regsCalled, *regsCaller;
  bool modif = false;

  // R4-R7 are used for arguments (aliases a0-a3)
  regsCaller =  vAbstractRegMemCaller.getRegState();
  regsCalled =  vAbstractRegMemCalled.getRegState(); 
  for (int i=4; i<8; i++)
    {
      if (regsCalled->importRegister(regsCaller,i)) 
	modif = true;
    }
  return modif;
}


//-----------Public -----------------------------------------------------------------

MIPSAddressAnalysis::MIPSAddressAnalysis (Program * p, int sp):AddressAnalysis (p,sp)
{ }

bool
MIPSAddressAnalysis::PerformAnalysis ()
{
  return AddressAnalysis::PerformAnalysis ();
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
MIPSAddressAnalysis::CheckInputAttributes ()
{
  return AddressAnalysis::CheckInputAttributes ();
}

//nothig to remove
void
MIPSAddressAnalysis::RemovePrivateAttributes ()
{
  AddressAnalysis::RemovePrivateAttributes ();
}




