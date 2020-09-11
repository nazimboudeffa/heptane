/* ---------------------------------------------------------------------

   Copyright IRISA, 2003-2014

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
#include "MIPS.h"
#include "Logger.h"
#include "DAAInstruction_MIPS.h"
#include "Utl.h"

#define DEFAULT_VALUE_LATENCY 1
#define DEFAULT_VALUE_LATENCY_LOAD DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_STORE DEFAULT_VALUE_LATENCY

MIPS::MIPS(const bool is_big_endian_p)
{
  is_big_endian = is_big_endian_p;
  zero_register_num = 0;

  //------------------------------------------
  // markers for ReadELF and Objdump files
  //---------------------------------------
  readelf_sections_markers.push_back("Section Headers:");
  readelf_sections_markers.push_back("En-têtes de section:");

  readelf_flags_markers.push_back("Key to Flags:");
  readelf_flags_markers.push_back("Clé des fanions:");

  objdump_text_markers.push_back("Disassembly of section .text:");
  objdump_text_markers.push_back("Déassemblage de la section .text:");

  objdump_symboltable_markers.push_back("SYMBOL TABLE:");

  //------------------------------------------
  // sections to be extracted in ReadELF file
  //------------------------------------------
  sectionsToExtract.push_back(".text");
  sectionsToExtract.push_back(".bss");
  sectionsToExtract.push_back(".sbss");
  sectionsToExtract.push_back(".data");
  sectionsToExtract.push_back(".sdata");
  sectionsToExtract.push_back(".rodata");

  //------------------------------------------
  // formats declaration
  //------------------------------------------
  formats["rd_rs_rs"].insert(new rd_rs_rs());
  formats["hex"].insert(new Hex());
  formats["addr"].insert(new Addr());
  formats["rd_rs_int"].insert(new rd_rs_int());
  formats["rd_rs_hex"].insert(new rd_rs_hex());
  formats["rd_hex"].insert(new rd_hex());
  formats["rd_hi"].insert(new rd("HI"));
  formats["rd_lo"].insert(new rd("LO"));
  formats["rd_rs"].insert(new rd_rs());
  formats["rs_rd"].insert(new rs_rd());
  formats["rs_rs_lo_hi"].insert(new rs_rs("LO, HI"));
  formats["rs_rs"].insert(new rs_rs(""));
  formats["rs_addr"].insert(new rs_addr());
  formats["rs_rs_addr"].insert(new rs_rs_addr());
  formats["empty"].insert(new Empty());
  formats["rs"].insert(new rs());
  formats["rs_mem"].insert(new rs_mem());
  formats["rd_mem"].insert(new rd_mem());
  formats["rs_zero_hex"].insert(new rs_zero_hex());
  formats["jalr"].insert(new rs());
  formats["jalr"].insert(new rd_rs());
  formats["div"].insert(new zero_rs_rs("LO, HI"));
  formats["div"].insert(new rs_rs("LO, HI"));
  formats["li"].insert(new rd_int());
  formats["li"].insert(new rd_hex());

  //------------------------------------------
  // mnemonics declaration
  //------------------------------------------

  //macro
  mnemonicToInstructionTypes["nop"] = new Basic("alu", formats["empty"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["bnez"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["beqz"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["li"] = new Basic("alu", formats["li"], new Li(), DEFAULT_VALUE_LATENCY);

  // Instructions added for WCET2013 (cache management + co-processor registers)
  //mnemonicToInstructionTypes["cache"]  =new Basic();
  //mnemonicToInstructionTypes["wait"]   =new Basic();
  //mnemonicToInstructionTypes["eret"]   =new Basic();
  mnemonicToInstructionTypes["teq"] = new Basic("alu", formats["rs_zero_hex"], new Nop(), DEFAULT_VALUE_LATENCY);
  //mnemonicToInstructionTypes["mfc0"]   =new Basic();
  //mnemonicToInstructionTypes["mtc0"]   =new Basic();

  mnemonicToInstructionTypes["mul"] = new Basic("alu", formats["rd_rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["multu"] = new Basic("alu", formats["rs_rs_lo_hi"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mult"] = new Basic("alu", formats["rs_rs_lo_hi"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["div"] = new Basic("alu", formats["div"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["divu"] = new Basic("alu", formats["div"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["break"] = new Basic("", formats["hex"], new Nop(), DEFAULT_VALUE_LATENCY);	//TBC: alu ???
  mnemonicToInstructionTypes["nor"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  // Arithmetic operation
  //mnemonicToInstructionTypes["add"]    =new Basic("alu",formats["rd_rs_rs"]);
  mnemonicToInstructionTypes["addiu"] = new Basic("alu", formats["rd_rs_int"], new Addiu(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["addu"] = new Basic("alu", formats["rd_rs_rs"], new Add(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["subu"] = new Basic("alu", formats["rd_rs_rs"], new Subu(), DEFAULT_VALUE_LATENCY);

  // Branch
  mnemonicToInstructionTypes["bne"] = new ConditionalJump("alu", formats["rs_rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["blez"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["bgez"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["bgtz"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["beq"] = new ConditionalJump("alu", formats["rs_rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["j"] = new UnconditionalJump("alu", formats["addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["jr"] = new Return("alu", formats["rs"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["jal"] = new Call("alu", formats["addr"], new DCall(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["b"] = new UnconditionalJump("alu", formats["addr"], new Nop(), DEFAULT_VALUE_LATENCY);

  // Isabelle, Damien: this instruction (jumpr through register) is voluntarily
  // not supported to detect indirect calls as soon as possible
  // mnemonicToInstructionTypes["jalr"]   =new Call("alu",formats["jalr"]);

  mnemonicToInstructionTypes["bltz"] = new ConditionalJump("alu", formats["rs_addr"], new Nop(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["bc1f"] = new ConditionalJump("alu", formats["addr"], new Nop(), DEFAULT_VALUE_LATENCY);	//Floating point instruction
  mnemonicToInstructionTypes["bc1t"] = new ConditionalJump("alu", formats["addr"], new Nop(), DEFAULT_VALUE_LATENCY);	//Floating point instruction

  // Store
  mnemonicToInstructionTypes["sw"] = new Store(4, "alu", formats["rs_mem"], new DStore(), DEFAULT_VALUE_LATENCY_STORE);
  //mnemonicToInstructionTypes["swl"]  =new Store(4,"alu",formats["rs_mem"]);
  //mnemonicToInstructionTypes["swr"]  =new Store(4,"alu",formats["rs_mem"]);
  mnemonicToInstructionTypes["swc1"] = new Store(4, "alu", formats["rs_mem"], new DStore(),DEFAULT_VALUE_LATENCY_STORE);
  mnemonicToInstructionTypes["sh"] = new Store(2, "alu", formats["rs_mem"], new DStore(), DEFAULT_VALUE_LATENCY_STORE);
  mnemonicToInstructionTypes["sb"] = new Store(1, "alu", formats["rs_mem"], new DStore(), DEFAULT_VALUE_LATENCY_STORE);
  mnemonicToInstructionTypes["sdc1"] = new Store(8, "alu", formats["rs_mem"], new DStore(), DEFAULT_VALUE_LATENCY_STORE);

  // Load
  mnemonicToInstructionTypes["lw"] = new Load(4, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lwl"] = new Load(4, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lwr"] = new Load(4, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lwc1"] = new Load(4, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lh"] = new Load(2, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lhu"] = new Load(2, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lb"] = new Load(1, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["lbu"] = new Load(1, "alu", formats["rd_mem"], new DLoad(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["ldc1"] = new Load(8, "alu", formats["rd_mem"], new KillOp1(), DEFAULT_VALUE_LATENCY_LOAD);

  // Load immediate
  mnemonicToInstructionTypes["lui"] = new Basic("alu", formats["rd_hex"], new Lui(), DEFAULT_VALUE_LATENCY);

  // Move and conditional move
  mnemonicToInstructionTypes["move"] = new Basic("alu", formats["rd_rs"], new Move(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["movn"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  //mnemonicToInstructionTypes["movf"]  =new Basic("alu",formats["rd_rs_rs"],NULL, DEFAULT_VALUE_LATENCY);

  // others (mostly logic)
  mnemonicToInstructionTypes["slt"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sltiu"] = new Basic("alu", formats["rd_rs_int"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sltu"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  mnemonicToInstructionTypes["sll"] = new Basic("alu", formats["rd_rs_hex"], new Shift(), DEFAULT_VALUE_LATENCY);

  mnemonicToInstructionTypes["or"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["and"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  mnemonicToInstructionTypes["srl"] = new Basic("alu", formats["rd_rs_hex"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["andi"] = new Basic("alu", formats["rd_rs_hex"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["ori"] = new Basic("alu", formats["rd_rs_hex"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["xori"] = new Basic("alu", formats["rd_rs_hex"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["xor"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["slti"] = new Basic("alu", formats["rd_rs_int"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sllv"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["srlv"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sra"] = new Basic("alu", formats["rd_rs_hex"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  mnemonicToInstructionTypes["mfhi"] = new Basic("alu", formats["rd_hi"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mflo"] = new Basic("alu", formats["rd_lo"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["srav"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  // Floating point instructions
  mnemonicToInstructionTypes["abs.d"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["abs.s"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["cvt.w.s"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["cvt.s.d"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mtc1"] = new Basic("alu", formats["rd_rs"], new KillOp2(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mov.s"] = new Basic("alu", formats["rd_rs"], new Move(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mov.d"] = new Basic("alu", formats["rd_rs"], new Move(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["add.s"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["add.d"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["div.s"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["div.d"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  //mnemonicToInstructionTypes["cvt.s"]  =new Basic();
  mnemonicToInstructionTypes["cvt.d.s"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["cvt.d.w"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  //mnemonicToInstructionTypes["cvt.d.l"]=new Basic();
  mnemonicToInstructionTypes["cvt.w.d"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["cvt.s.w"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mul.s"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["mul.d"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sub.d"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["sub.s"] = new Basic("alu", formats["rd_rs_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  mnemonicToInstructionTypes["cfc1"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["ctc1"] = new Basic("alu", formats["rs_rd"], new KillOp2(), DEFAULT_VALUE_LATENCY);	//special move rs rd
  mnemonicToInstructionTypes["mfc1"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["c.lt.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	// Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the condition flag true if the first is less than the second.
  mnemonicToInstructionTypes["c.le.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	// Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the floating point condition flag true if the first is less than or equal to the second.
  mnemonicToInstructionTypes["c.eq.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the floating point condition flag true if they are equal.
  mnemonicToInstructionTypes["c.lt.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the condition flag true if the first is less than the second.
  mnemonicToInstructionTypes["c.le.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	// Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the floating point condition flag true if the first is less than or equal to the second.
  mnemonicToInstructionTypes["c.eq.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	// Compare the floating point double in register FRsrc1 against the one in FRsrc2 and set the floating point condition flag true if they are equal.
  mnemonicToInstructionTypes["c.ult.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare
  mnemonicToInstructionTypes["c.ult.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare
  mnemonicToInstructionTypes["c.olt.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare;
  mnemonicToInstructionTypes["c.olt.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare;
  //mnemonicToInstructionTypes["c.ole.s"] =new Basic();
  mnemonicToInstructionTypes["c.ole.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare;
  mnemonicToInstructionTypes["c.ule.d"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare;
  mnemonicToInstructionTypes["c.ule.s"] = new Basic("alu", formats["rs_rs"], new Nop(), DEFAULT_VALUE_LATENCY);	//should be compare;
  //mnemonicToInstructionTypes["c.nge.s"] =new Basic();
  //mnemonicToInstructionTypes["c.nge.d"] =new Basic();
  //mnemonicToInstructionTypes["c.ngl.s"] =new Basic();
  //mnemonicToInstructionTypes["c.ngl.d"] =new Basic();
  //mnemonicToInstructionTypes["c.ngt.d"] =new Basic();
  //mnemonicToInstructionTypes["c.ngt.s"] =new Basic();
  mnemonicToInstructionTypes["trunc.w.d"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  //mnemonicToInstructionTypes["trunc.w.s"] =new Basic();

  mnemonicToInstructionTypes["neg.d"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["neg.s"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);
  mnemonicToInstructionTypes["negu"] = new Basic("alu", formats["rd_rs"], new KillOp1(), DEFAULT_VALUE_LATENCY);

  //------------------------------------------
  // regs declaration
  //------------------------------------------
  //GPR
  regs["zero"] = 0;
  regs["$0"] = 0;
  regs["at"] = 1;
  regs["$1"] = 1;
  regs["v0"] = 2;
  regs["$2"] = 2;
  regs["v1"] = 3;
  regs["$3"] = 3;
  regs["a0"] = 4;
  regs["$4"] = 4;
  regs["a1"] = 5;
  regs["$5"] = 5;
  regs["a2"] = 6;
  regs["$6"] = 6;
  regs["a3"] = 7;
  regs["$7"] = 7;
  regs["t0"] = 8;
  regs["$8"] = 8;
  regs["t1"] = 9;
  regs["$9"] = 9;
  regs["t2"] = 10;
  regs["$10"] = 10;
  regs["t3"] = 11;
  regs["$11"] = 11;
  regs["t4"] = 12;
  regs["$12"] = 12;
  regs["t5"] = 13;
  regs["$13"] = 13;
  regs["t6"] = 14;
  regs["$14"] = 14;
  regs["t7"] = 15;
  regs["$15"] = 15;
  regs["s0"] = 16;
  regs["$16"] = 16;
  regs["s1"] = 17;
  regs["$17"] = 17;
  regs["s2"] = 18;
  regs["$18"] = 18;
  regs["s3"] = 19;
  regs["$19"] = 19;
  regs["s4"] = 20;
  regs["$20"] = 20;
  regs["s5"] = 21;
  regs["$21"] = 21;
  regs["s6"] = 22;
  regs["$22"] = 22;
  regs["s7"] = 23;
  regs["$23"] = 23;
  regs["s8"] = 24;
  regs["t8"] = 24;
  regs["$24"] = 24;
  regs["s9"] = 25;
  regs["t9"] = 25;
  regs["$25"] = 25;
  regs["kt0"] = 26;
  regs["$26"] = 26;
  regs["kt1"] = 27;
  regs["$27"] = 27;
  regs["gp"] = 28;
  regs["$28"] = 28;
  regs["sp"] = 29;
  regs["$29"] = 29;
  regs["fp"] = 30;
  regs["s8"] = 30;		// IP: added this register name (alias to fp, used by llvm)
  regs["$30"] = 30;
  regs["ra"] = 31;
  regs["$31"] = 31;

  //FP
  regs["$f0"] = 32;
  regs["$f1"] = 33;
  regs["$f2"] = 34;
  regs["$f3"] = 35;
  regs["$f4"] = 36;
  regs["$f5"] = 37;
  regs["$f6"] = 38;
  regs["$f7"] = 39;
  regs["$f8"] = 40;
  regs["$f9"] = 41;
  regs["$f10"] = 42;
  regs["$f11"] = 43;
  regs["$f12"] = 44;
  regs["$f13"] = 45;
  regs["$f14"] = 46;
  regs["$f15"] = 47;
  regs["$f16"] = 48;
  regs["$f17"] = 49;
  regs["$f18"] = 50;
  regs["$f19"] = 51;
  regs["$f20"] = 52;
  regs["$f21"] = 53;
  regs["$f22"] = 54;
  regs["$f23"] = 55;
  regs["$f24"] = 56;
  regs["$f25"] = 57;
  regs["$f26"] = 58;
  regs["$f27"] = 59;
  regs["$f28"] = 60;
  regs["$f29"] = 61;
  regs["$f30"] = 62;
  regs["$f31"] = 63;

  //FP Code Condition
  regs["$fcc0"] = 64;		//TBC: I can't find the number corresponding to $fcc0
  regs["raux"] = 65;		// auxiliary register
}

//-----------------------------------------------------
//
//  MIPS class functions
//
//-----------------------------------------------------

/** aaaaaaaaaaaaaaaaaaaa   */
MIPS::~MIPS()
{
  //delete formats map
  for (map < string, set < InstructionFormat * > >::iterator it1 = formats.begin(); it1 != formats.end(); it1++)
    {
      set < InstructionFormat * >current = (*it1).second;
      for (set < InstructionFormat * >::iterator it2 = current.begin(); it2 != current.end(); it2++)
	{
	  delete(*it2);
	}
    }

  //delete mnemonicToInstructionTypes map
  for (map < string, InstructionType * >::iterator it = mnemonicToInstructionTypes.begin(); it != mnemonicToInstructionTypes.end(); it++)
    {
      delete((*it).second);
    }
}

string MIPS::removeUselessCharacters(const string & line)
{
  string result = line;
  for (size_t i = 0; i < line.length(); i++)	//remove useless char
    {
      if (result[i] == ':')
	{
	  result[i] = ' ';
	}
      else if (result[i] == '<')
	{
	  result[i] = ' ';
	}
      else if (result[i] == '>')
	{
	  result[i] = ' ';
	}
    }
  return result;
}

vector < string > MIPS::splitOperands(const string & operands)
{
  vector < string > result;

  if (operands.length() == 0)
    {
      return result;
    }

  string tmp = operands;
  int nb_operand = 1;

  for (size_t i = 0; i < tmp.length(); i++)
    {
      if (tmp[i] == ',')
	{
	  tmp[i] = ' ';
	  nb_operand++;
	}
    }

  string val;
  istringstream parse(tmp);
  for (int i = 0; i < nb_operand; i++)
    {
      parse >> val;
      result.push_back(val);
    }

  return result;
}

bool MIPS::isFunction(const string & line)
{
  return (line[0] != ' ');
}

bool MIPS::isInstruction(const string & line)
{
  return (line[0] == ' ');
}

ObjdumpFunction MIPS::parseFunction(const string & line)
{
  ObjdumpFunction result;
  result.line = line;

  string clean_line = removeUselessCharacters(line);

  string s_addr;
  istringstream parse(clean_line);
  parse >> s_addr >> result.name;

  //Convert the string in hexa to an addr
  result.addr = strtoul(s_addr.c_str(), NULL, 16);
  assert(result.addr != 0 && result.addr != ULONG_MAX);

  return result;
}

t_address MIPS::getJumpDestination(const ObjdumpInstruction & instr)
{
  //Get the last operand
  string s_addr = instr.operands[instr.operands.size() - 1];

  //Convert the string in hexa to an addr
  t_address result = strtoul(s_addr.c_str(), NULL, 16);
  assert(result != 0 && result != ULONG_MAX);

  return result;
}

bool MIPS::isMemPattern(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  //Remove "("
  size_t pos = op_local.find("(");
  if (pos == EOS)
    {
      return false;
    }
  op_local[pos] = ' ';

  //Remove ")"
  pos = op_local.find(")");
  if (pos == EOS)
    {
      return false;
    }
  if (pos != op_local.size() - 1)
    {
      return false;
    }				//if not the last char
  op_local[pos] = ' ';

  //get the offset and the base register
  string offset, reg_name;
  istringstream parse(op_local);
  parse >> offset >> reg_name;

  //Check if reg_name is defined
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  //Check if offset is a positive or negative integer
  if (Utl::isDecNumber(offset) == false)
    {
      return false;
    }

  return true;
}

/***************************************************************

	    MODIFICATIONS FOR ARM

**************************************************************/

vector < string > MIPS::extractInputRegistersFromMem(const string & operand)
{
  string op_local = operand;
  vector < string > result;

  //Removing ")"
  size_t pos = op_local.find("(");
  op_local[pos] = ' ';

  //Removing "("
  pos = op_local.find(")");
  op_local[pos] = ' ';

  string offset, reg_name("");
  istringstream parse(op_local);

  //Extracting the offset and the base register
  parse >> offset >> reg_name;

  assert(reg_name != "");

  result.push_back(reg_name);

  return result;
}

vector < string > MIPS::extractOutputRegistersFromMem(const string & operand)
{
  //There are no output registers in memory access with MIPS
  vector < string > result;
  assert(result.empty());
  return result;
}

bool MIPS::isLoadMultiple(const string & instr)
{
  //There are no multiple load instructions in MIPS
  assert(false);
  return false;
}

int MIPS::getNumberOfLoads(const string & instr)
{
  int result = 1;		//only 1 in MIPS
  assert(getInstructionTypeFromAsm(instr)->isLoad());
  return result;
}

bool MIPS::isStoreMultiple(const string & instr)
{
  //There are no multiple store instructions in MIPS
  assert(false);
  return false;
}

int MIPS::getNumberOfStores(const string & instr)
{
  int result = 1;		//only 1 in MIPS
  assert(getInstructionTypeFromAsm(instr)->isStore());
  return result;
}

bool MIPS::isReturn(const ObjdumpInstruction & instr)
{
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isReturn();
}

ObjdumpInstruction MIPS::parseInstruction(const string & line)
{
  ObjdumpInstruction result;
  result.line = line;

  string clean_line = removeUselessCharacters(line);

  string operands;
  string s_addr;
  string binary_code;

  istringstream parse(clean_line);
  parse >> s_addr >> binary_code >> result.mnemonic >> operands >> result.extra;

  //if no binary asm code present
  /*
     //removed: no need with correct objdump flags
     if(!isBinaryAsmCode(binary_code))
     {
     istringstream parse(clean_line);
     parse >> s_addr >> result.mnemonic >> operands >> result.extra ;
     }
   */

  result.addr = strtoul(s_addr.c_str(), NULL, 16);
  assert(result.addr != 0 && result.addr != ULONG_MAX);

  result.operands = splitOperands(operands);

  /* check that the mnemonic is defined and the operand format is correct */
  if (!getInstructionTypeFromMnemonic(result.mnemonic)->checkFormat(result.operands))
    {
      Logger::addFatal("Error: instruction asm \"" + result.mnemonic + "\" format not valid");
    }
  //--

  if (operands != "")
    {
      result.asm_code = result.mnemonic + " " + operands;
    }
  else
    {
      result.asm_code = result.mnemonic;
    }

  return result;
}

string MIPS::getCalleeName(const ObjdumpInstruction & instr)
{
  assert(getInstructionTypeFromMnemonic(instr.mnemonic)->isCall());

  //We assume that the extra field is in the format "FunctionName"
  assert(instr.extra != "");
  string result = instr.extra;

  return result;
}

void MIPS::parseSymbolTableLine(const string & line, ObjdumpSymbolTable & table)
{
  /* We assume that the input line is in the format : Value  Location  Type  Section  Size  Name
     for some lines, the field Type is empty */

  //Extracting each field of the line
  string value, location, type, section, size, name;
  istringstream parse(line);
  parse >> value >> location >> type >> section >> size >> name;

  //Function
  if (type == "F" && section == ".text")
    {
      assert(name != "");

      //Converting the string to Unsigned Long
      t_address addr = strtoul(value.c_str(), NULL, 16);
      assert(addr != 0 && addr != ULONG_MAX);

      //Adding the function in the map
      table.functions[addr].push_front(name);
    }

  //Variable
  else if (type == "O")
    {
      assert(section != "");
      assert(size != "");
      assert(name != "");

      //Convert the string to Unsigned Long
      t_address address = strtoul(value.c_str(), NULL, 16);
      assert(address != 0 && address != ULONG_MAX);

      //Convert the string to Integer
      int size_var = (int)strtol(size.c_str(), NULL, 16);
      assert(size_var != 0 && size_var != INT_MAX);

      //Search for the section of the variable in the vector of ObjdumpSymbolTable
      //to check if the section of the variable is a section we extracted
      vector < ObjdumpSection >::const_iterator it;
      for (it = table.sections.begin(); it != table.sections.end(); it++)
	{
	  if ((*it).name == section)
	    {
	      break;
	    }
	}
      assert(it != table.sections.end());

      //Checking if the address of the variable is really in the section
      unsigned long end_of_section = (*it).addr + (unsigned long)(*it).size;
      assert(address >= (*it).addr && address < end_of_section);

      //create the ObjdumpVariable
      ObjdumpVariable var;
      var.name = name;
      var.addr = address;
      var.size = size_var;	//in bytes
      var.section_name = section;

      //Add the variable in the vector
      table.variables.push_back(var);
    }

  //global pointer
  //In this case, Size is the Name because the field Type is empty so there is a shift in the parsing
  //like this  : 004096a0 g        *ABS*        00000000 _gp
  //instead of : 004004b0 g     F .text 000000f0 main
  else if (size == "_gp")
    {
      //Convert the string to unsigned Long
      t_address address = strtoul(value.c_str(), NULL, 16);
      assert(address != 0 && address != ULONG_MAX);

      table.MIPS_gp = address;
    }
}

ObjdumpWord MIPS::readWordInstruction(const ObjdumpInstruction & instr, ObjdumpSymbolTable & table)
{
  assert(false);
  ObjdumpWord result;
  return result;
}

bool MIPS::isPcInInputResources(const ObjdumpInstruction & instr)
{
  assert(false);
  return false;
}


bool MIPS::isPcInOutputResources(const ObjdumpInstruction & instr)
{
  assert(false);
  return false;
}


vector < ObjdumpWord > MIPS::getWordsFromInstr(const ObjdumpInstruction & instr1, const ObjdumpInstruction & instr2, vector < ObjdumpWord > words,
					       bool & is_instr2_consumed)
{
  assert(false);
  vector < ObjdumpWord > result;
  return result;
}

bool MIPS::isInputWrittenRegister(const string & operand)
{
  assert(false);
  return false;
}

string MIPS::extractRegisterFromInputWrittenOperand(const string & operand)
{
  assert(false);
  return "";
}

bool MIPS::isRegisterList(const string & operand)
{
  assert(false);
  return "";
}

vector < string > MIPS::extractRegistersFromRegisterList(const string & operand)
{
  assert(false);
  vector < string > result;
  return result;
}

int MIPS::getSizeRegisterList(const string & operand)
{
  assert(false);
  return -1;
}

bool MIPS::isShifterOperand(const string & operand)
{
  assert(false);
  return false;
}

string MIPS::extractRegisterFromShifterOperand(const string & operand)
{
  assert(false);
  return "";
}

bool MIPS::getLoadStoreARMInfos(bool strongContext, string & instr, string & codeinstr, string & oregister, AddressingMode * vaddrmode,
				offsetType * TypeOperand, string & operand1, string & operand2, string & operand3)
{
  assert(false);
  return false;
}

bool MIPS::isPcLoadInstruction(string & instr)
{
  // assert(false);
  return false;
}

bool MIPS::getInstr1ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2)
{
  assert(false);
  return false;
}

bool MIPS::getInstr2ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2, string & operand3)
{
  assert(false);
  return false;
}

bool MIPS::getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack)
{
  assert(false);
  return false;
}

bool MIPS::isARMClassInstr(const string &codeop, const string &prefix)
{
  assert(false);
  return false;
}

bool MIPS::isConditionnedARMInstr(const string &codeop, const string &prefix)
{
  assert(false);
  return false;
}


bool MIPS::getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i)
{
  string val, sp;
  if (instructionAsm.find ("sp") == EOS) return false;
  // instruction references "sp"
  vector < string > v_instr = Arch::splitInstruction (instructionAsm);
  reg= v_instr[1]; 
  Utl::extractRegVal( v_instr[2], sp, val);
  *i = Utl::string2int(val);
 return true;
}
