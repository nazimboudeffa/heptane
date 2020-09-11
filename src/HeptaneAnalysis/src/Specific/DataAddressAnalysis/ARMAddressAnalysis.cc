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

#include "ARMAddressAnalysis.h"
#include "Utl.h"
#include "ARM.h"
#include "arch.h"

int ARMAddressAnalysis::getStackSize( Instruction * vinstr)
{

  // if (vinstr->HasAttribute(MetaInstructionAttributeName)) return 0;  // Added Sept 2017 lbesnard.

  // Split the first instruction (instr1)
 vector < string > v_instr = Arch::splitInstruction (vinstr->GetCode ());
 string  codeop = v_instr[0];

  if (codeop == "sub")  // allocating locals sub sp, sp, VALUE
    {
      if (v_instr[1] != "sp") return 0;
      if (v_instr[2] != "sp") return 0;
      assert (Utl::isDecNumber (v_instr[3]));
      int vsize = atoi (v_instr[3].c_str ());
      return abs(vsize);
    }

  // No occurrence (they are replaced)
  assert(codeop.find("push") == EOS);
  assert(codeop.find("stm") == EOS);
  assert(codeop.find("push") == EOS);
  assert(codeop.find("pop") == EOS);
  return 0;
}

int ARMAddressAnalysis::getStackSize(Cfg * cfg)
{ 
  int i, stackSize = 0;
  Instruction * vinstr;
  bool todo =true;
  Node *firstNode = cfg->GetStartNode ();
  const vector < Instruction * >&listInstr = firstNode->GetAsm ();
  int n = listInstr.size ();
  long vaddr1, vaddr2;

  TRACE(cout << " DEBUG__LB ARMAddressAnalysis::getStackSize (" << cfg->getStringName() << ")" << endl);
  assert (n != 0);
  if (n >= 2)
    {
      if (InstructionARM::getCodeAddress(listInstr[0], &vaddr1) && InstructionARM::getCodeAddress(listInstr[1], &vaddr2))
	if (vaddr1 == vaddr2)
	  {
	    // The push has been replaced by a set of store (str) and sub sp, sp ,<value> instructions.
	    stackSize = getStackSize(listInstr[0]);
	    i = 1;
	    vinstr = listInstr[i];
	    while ( i < n && (vaddr1 == vaddr2))
	      { 
	       stackSize = stackSize + getStackSize(vinstr);
	       i++;
	       vinstr = listInstr[i];
	       if (!InstructionARM::getCodeAddress(vinstr, &vaddr2)) vaddr2=-1;
	    }
	    if ( i < n ) stackSize = stackSize + getStackSize(vinstr);  //  may be a sub sp, sp ,<value>
	    todo = false;
	  }
    }
  if (todo)
    {
      // first instr is a push. Second may be a sub sp, sp ,<value>
      stackSize = getStackSize(listInstr[0]);
      if (n >= 2) stackSize = stackSize + getStackSize(listInstr[1]);
    }
  TRACE(cout << " DEBUG__LB ARMAddressAnalysis::getStackSize() = " << stackSize << endl);

  // a leaf in the call graph (no dedicated stack frame in ARM) is accepted.
  if (stackSize == 0) { assert ( ! isCfgWithCallNode(cfg));}
  return stackSize;
}

//  Waiting for  mnemonic regout [regin, val]
bool ARMAddressAnalysis::extractRegVal(string instr, string &reg, string &val)
{
  vector < string > v_instr = Arch::splitInstruction (instr);
  assert (v_instr.size () == 3);	// mnemonic op1, [xxx]
  string mem_pattern = v_instr[2];
  if ( ! Utl::replace(mem_pattern, '[', ' ')) return false;
  if ( ! Utl::replace(mem_pattern, ']', ' ')) return false;
  Utl::replace(mem_pattern, ',', ' ');
  val ="";  // case "codeop [sp]"
  istringstream parse (mem_pattern);
  parse >> reg >> val;
  return true;
}


long ARMAddressAnalysis::GetOffsetValue(string asm_code)
{
  string reg, val;
  assert(extractRegVal( asm_code, reg, val));
  return  Utl::string2long(val);
}


int ARMAddressAnalysis::getStackMaxOffset(string s_instr, int StackMaxOffset)
{
  string val, reg;

  //  LBesnard : on peut acceder à des paramètres empilés ? dépendant compilateur
  //    On peut avoir mnemonic op1, [sp] OU mnemonic op1, [sp, val]
  //    Voir pour   mnemonic op1, [PC, val] ???
  if (s_instr.find ("[sp") != EOS)
    {
      if (extractRegVal( s_instr, reg, val))
	{
	  assert (reg == "sp");
	  return Utl::imax(abs(Utl::string2int(val)),StackMaxOffset);
	}
    }
  return StackMaxOffset;
}

// COPY of MIPS
void ARMAddressAnalysis::analyzeStack (Cfg * cfg, Instruction * Instr, long offset, string access, int sizeOfMemoryAccess, bool precision, Context *context)
{

  //get the StackInfoAttributeName of current cfg
  assert (cfg->HasAttribute (StackInfoAttributeName));
  StackInfoAttribute attribute = (StackInfoAttribute &) cfg->GetAttribute (StackInfoAttributeName);

  //get the stack information of cfg
  long stack_size = attribute.getFrameSizeWithoutCaller ();
  long stack_maxoffset = attribute.getFrameSizeWithCaller ();

  TRACE(cout << "analyzeStack, stack_size = " << stack_size << " , stack_maxoffset = " << stack_maxoffset << endl);
  if (stack_maxoffset < offset)
    {
      Logger::addError ("stack analysis -> maxOffset < offset");
    }
  else
    {
      long  addr, size, stack_maxoffset_caller;
      long sp = attribute.getSP (context->getStringId ());
      TRACE(cout << "analyzeStack, context --sp = " <<  std::hex << sp << std::dec << endl);

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
      AddressInfo contextual_info = mkAddressInfo( Instr, access, precision, "stack","", addr, size);
      setContextualAddressAttribute(Instr, context, contextual_info);
    }
}


// COPY OF MIPS !!!
// set the AddressAttribute for a gp or lui access; 
// local to setLoadStoreAddressAttribute
void ARMAddressAnalysis::analyzeReg (Instruction * Instr, long addr, string access, int sizeOfMemoryAccess, bool precision)
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


// Setting the address attribute for load and Store instructions (Memory instructions)
// @return true when it is applied, false otherwise.
// ***WARNING***: push, pop, ldm, stm are currently replaced in the HeptaneExtract step.
bool ARMAddressAnalysis::setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state, Context *context)
{
  string reg, codeinstr, access, asm_code;
  vector < string > regList;
  long loffset;

  asm_code = vinstr->GetCode ();

  bool isLoad = Arch::isLoad (asm_code);
  bool isStore = Arch::isStore (asm_code);
  
  if (!isLoad && ! isStore) return false;

  // get the size of the access according to the op code
  int sizeOfMemoryAccess = Arch::getSizeOfMemoryAccess (asm_code);
  
  if (isLoad) access = "read"; else access = "write"; // access type



  //access using $sp register
  if (asm_code.find ("sp") != EOS)
    {
      loffset = GetOffsetValue(asm_code);
      TRACE( cout << "ARMAddressAnalysis::setLoadStoreAddressAttribute (sp) instr = " <<  asm_code << ", loffset= "  
	     << loffset << " , sizeOfMemoryAccess= " << sizeOfMemoryAccess << endl);
      analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true, context);
    }
  else
    {
      vector < string > access_pattern;
      state->accessAnalysis (vinstr, access_pattern);
	
      TRACE( cout << "  === state->accessAnalysis returns, code = " << access_pattern[0] << endl);
      TRACE( cout << "  === state->accessAnalysis returns, value = " << access_pattern[1] << endl);
      TRACE( cout << "  === state->accessAnalysis returns, precision = " << access_pattern[2] << endl);
      TRACE( if (access_pattern.size() == 4) cout << "  === state->accessAnalysis returns,  4th operand = " << access_pattern[3] << endl);
      TRACE( if (access_pattern.size() == 5) cout << "  === state->accessAnalysis returns,  5th operand = " << access_pattern[4] << endl);
	    
      string p0 = access_pattern[0];
      bool prec =  access_pattern[2] == "1";
      // access using $reg different from $sp
      // Example ldr R, [pc, #val] et MEM[pc + val] is an immediate value ( load a immediate value)
      if (p0 == "immWord")  
	{
	  string s_addr = access_pattern[3];
	  long addr = Utl::string2long(s_addr);;
	  TRACE( cout << "  === state->accessAnalysis returns, address of the IMMEDIATE VALUE = " << s_addr << endl);
	  mkAddressInfoAttribute(vinstr, access, prec, ".data", "", addr, sizeOfMemoryAccess);
	}
      else
	if (p0 == "lui")
	  {
	    long addr = Utl::string2long(access_pattern[1]);
	    analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, prec);
	  }
	else
	  if (p0 == "gp")
	    {
	      long addr = Utl::string2long(access_pattern[1]);
	      analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, prec);
	    }
	  else if (p0 == "sp")
	    {
	      loffset = Utl::string2long(access_pattern[1]);
	      TRACE(cout << " Stack pointer ----*****------" << asm_code << endl;);
	      analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, prec, context);
	    }
	  else //- unknown- pointer: all addresses can be accessed (stub)
	    {
	      TRACE(cout << " POINTER ----*****------" << asm_code  << endl;);
	      setPointerAccessInfo(vinstr, access);
	    }
    }
  return true;
}


RegState* ARMAddressAnalysis::NewRegState(int stackSize)
{
  return new ARMRegState(stackSize);
}

bool ARMAddressAnalysis::importCallerArguments(AbstractRegMem &vAbstractRegMemCaller, AbstractRegMem &vAbstractRegMemCalled)
{
  // R0-R3 may be used for argument in ARM. (aliases a1-a4)
  RegState *regsCalled, *regsCaller;
  bool modif = false;

  regsCaller =  vAbstractRegMemCaller.getRegState();
  regsCalled =  vAbstractRegMemCalled.getRegState(); 
  for (int i=0; i<4; i++) 
    {
      if (regsCalled->importRegister(regsCaller,i))
	modif = true;
    }
  return modif;
}

//-----------Public -----------------------------------------------------------------

ARMAddressAnalysis::ARMAddressAnalysis (Program * p, int sp):AddressAnalysis(p,sp)
{ 
}

bool
ARMAddressAnalysis::PerformAnalysis ()
{
  return AddressAnalysis::PerformAnalysis ();
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
ARMAddressAnalysis::CheckInputAttributes ()
{
  return AddressAnalysis::CheckInputAttributes ();
}

//nothig to remove
void
ARMAddressAnalysis::RemovePrivateAttributes ()
{
  AddressAnalysis::RemovePrivateAttributes ();
}


