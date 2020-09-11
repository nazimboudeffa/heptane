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
#include "ARM.h"
#include "Logger.h"
#include "DAAInstruction_ARM.h"
#include "Utl.h"

#define BTRACE 0

#define DEFAULT_VALUE_LATENCY 1
#define DEFAULT_VALUE_LATENCY_STORE DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_LOAD DEFAULT_VALUE_LATENCY

#define DEFAULT_VALUE_LATENCY_LOGICAL DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_COMPARE DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_SHIFT DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_BRANCH  DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_MOV DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_SUB DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_RSUB DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_ADD DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_MUL DEFAULT_VALUE_LATENCY

// ???
#define DEFAULT_VALUE_LATENCY_POP DEFAULT_VALUE_LATENCY
#define DEFAULT_VALUE_LATENCY_PUSH DEFAULT_VALUE_LATENCY_POP

ARM::ARM(const bool is_big_endian_p) 
{
  is_big_endian = is_big_endian_p;
  zero_register_num = -1;	// No zero register

  // ------------------------------------------
  // markers for ReadELF and Objdump files
  // ---------------------------------------
  readelf_sections_markers.push_back("Section Headers:");
  readelf_sections_markers.push_back("En-têtes de section:");

  readelf_flags_markers.push_back("Key to Flags:");
  readelf_flags_markers.push_back("Clé des fanions:");

  objdump_text_markers.push_back("Disassembly of section .text:");
  objdump_text_markers.push_back("Déassemblage de la section .text:");

  objdump_symboltable_markers.push_back("SYMBOL TABLE:");

  // ------------------------------------------
  // sections to be extracted in ReadELF file
  // ------------------------------------------
  sectionsToExtract.push_back(".text");
  sectionsToExtract.push_back(".bss");
  sectionsToExtract.push_back(".data");
  sectionsToExtract.push_back(".rodata"); // Added LBesnard Jul 2017 (benchmark st)

  // ------------------------------------------
  // specific characters
  // ------------------------------------------
  list_begin_character = "{";
  list_end_character = "}";
  register_writting_character = "!";
  register_range_character = "-";
  memory_access_begin_character = "[";
  memory_access_end_character = "]";

  // ------------------------------------------
  // shifter operands
  // ------------------------------------------
  shifter_with_operand.push_back("lsl");
  shifter_with_operand.push_back("lsr");
  shifter_with_operand.push_back("asr");
  shifter_with_operand.push_back("ror");

  shifter_without_operand.push_back("rrx");

  // ------------------------------------------
  // ARM library functions
  // ------------------------------------------
  ARM_lib_functions.push_back("__adddf3");
  ARM_lib_functions.push_back("__addsf3");
  ARM_lib_functions.push_back("__divdf3");
  ARM_lib_functions.push_back("__divsf3");
  ARM_lib_functions.push_back("__divsi3");
  ARM_lib_functions.push_back("__eqdf2");
  ARM_lib_functions.push_back("__eqsf2");
  ARM_lib_functions.push_back("__extendsfdf2");
  ARM_lib_functions.push_back("__fixdfsi");
  ARM_lib_functions.push_back("__floatsidf");
  ARM_lib_functions.push_back("__floatunsidf");
  ARM_lib_functions.push_back("__gedf2");
  ARM_lib_functions.push_back("__gtdf2");
  ARM_lib_functions.push_back("__gtsf2");
  ARM_lib_functions.push_back("__ledf2");
  ARM_lib_functions.push_back("__ltsf2");
  ARM_lib_functions.push_back("__muldf3");
  ARM_lib_functions.push_back("__mulsf3");
  ARM_lib_functions.push_back("__subdf3");
  ARM_lib_functions.push_back("__subsf3");
  ARM_lib_functions.push_back("__truncdfsf2");
  ARM_lib_functions.push_back("__umodsi3");

  // ------------------------------------------
  // formats declaration
  // ------------------------------------------
  formats["rs"].insert(new rs());
  formats["addr"].insert(new Addr());
  formats["hex"].insert(new Hex());
  formats["empty"].insert(new Empty());
  formats["rd_rd_rs_rs"].insert(new rd_rd_rs_rs());
  formats["rd_rs_rs"].insert(new rd_rs_rs());
  formats["rd_rs"].insert(new rd_rs());
  // formats["rd_rs_rs_rs"].insert(new rd_rs_rs());
  formats["rds_rds_rs_rs"].insert(new rds_rds_rs_rs());

  formats["arithm"].insert(new rd_rs_int());	// ex : add r0, r1, #2
  // formats["arithm"].insert(new rd_rs_hex()); // ex : add r0, r1, #0x0002
  formats["arithm"].insert(new rd_rs_rs());	// ex : add r0, r1, r2
  // formats["arithm"].insert(new rds_int()); // ex : add r0, #2 (= add r0, r0, #2)
  // formats["arithm"].insert(new rds_hex()); // ex : add r0, #0x0002 (= add r0, r0, #0x0002)
  formats["arithm"].insert(new rd_rs_rs_shifter());	// ex : add r0, r0, r2, lsl r4 // Rq: rrxa shifter without operand can also be used

  formats["move"].insert(new rd_int());	// ex : mov r0, #2
  // formats["move"].insert(new rd_hex()); // ex : mov r0, #0xFA05
  formats["move"].insert(new rd_rs());	// ex : mov r0, r1
  formats["move"].insert(new rd_rs_shifter());	// ex : mov r0, r1, lsl r4 , mvnsne      ip, r4, asr #21

  formats["load"].insert(new rd_mem());	// ex : ldr r0, [*******] OR ldr r0, [********]!
  formats["load"].insert(new rd_mem_int());	// ex : ldr r0, [r1], #4  (post-indexed adressing)
  // formats["load"].insert(new rd_mem_rs()); // ex : ldr r0, [r1], r2  (post-indexed adressing)
  // formats["load"].insert(new rd_mem_rs_shifter()); // ex : ldr r0, [r1], r2, rrx  (post-indexed adressing)

  formats["load-multiple"].insert(new rs_rdlist());	// ex : ldm r1, {r4, r5, r8}
  formats["load-multiple"].insert(new rds_rdlist()); // ex : ldm r1!, {r4, r5, r8}

  formats["pop"].insert(new rdlist("sp", "sp"));

  formats["store"].insert(new rs_mem());	// ex : str r0, [*******] OR str r0, [********]!
  // formats["store"].insert(new rs_mem_int()); // ex : str r0, [r1], #4  (post-indexed adressing)
  // formats["store"].insert(new rs_mem_rs()); // ex : str r0, [r1], r2  (post-indexed adressing)
  // formats["store"].insert(new rs_mem_rs_shifter()); // ex : str r0, [r1], r2, rrx  (post-indexed adressing)

  formats["store-multiple"].insert(new rs_rslist());	// ex : stm r1, {r4, r5, r8}
  formats["store-multiple"].insert(new rds_rslist()); // ex : stm r1!, {r4, r5, r8}

  formats["push"].insert(new rslist("sp", "sp"));

  formats["shift-rotate"].insert(new rd_rs_int());	// ex : lsl r0, r1, #2
  // formats["shift-rotate"].insert(new rd_rs_hex()); // ex : lsl r0, r1, #0x0002
  formats["shift-rotate"].insert(new rd_rs_rs());	// ex : lsl r0, r1, r3

  formats["compare"].insert(new rs_int());	// ex : cmp r1, #4
  // formats["compare"].insert(new rs_hex()); // ex : cmp r1, #0x0004
  formats["compare"].insert(new rs_rs(""));	// ex : cmp r1, r2 (no static input in parameter but CPSR register is modified)
  formats["compare"].insert(new rs_rs_shifter());	// ex : cmp r0, r1, lsl r4

  formats["logical"].insert(new rd_rs_int());	// ex : and r2, r2, #255
  formats["logical"].insert(new rd_rs_rs());	// ex : orr r2, r2, r3
  formats["logical"].insert(new rd_rs_rs_shifter());	// ex : and r2, r3, r4, lsl r4

  // ------------------------------------------
  // mnemonics declaration
  // ------------------------------------------

  /*************************************** CONDITION CODES ****************************************
*************************************************************************************************
    CODE		MEANING							FLAGS TESTED
    *************************************************************************************************
    eq			Equal.							Z==1
    ne			Not equal.						Z==0
    cs or hs		Unsigned higher or same (or carry set).			C==1
    cc or lo		Unsigned lower (or carry clear).			C==0
    mi			Negative. The mnemonic stands for "minus".		N==1
    pl			Positive or zero. The mnemonic stands for "plus".	N==0
    vs			Signed overflow. The mnemonic stands for "V set".	V==1
    vc			No signed overflow. The mnemonic stands for "V clear".	V==0
    hi			Unsigned higher.					(C==1) && (Z==0)
    ls			Unsigned lower or same.					(C==0) || (Z==1)
    ge			Signed greater than or equal.				N==V
    lt			Signed less than.					N!=V
    gt			Signed greater than.					(Z==0) && (N==V)
    le			Signed less than or equal.				(Z==1) || (N!=V)
    al (or omitted)	Always executed.					Not tested.
  **************************************************************************************************/

  // Macro
  mnemonicToInstructionTypes["nop"] = new Basic("alu", formats["empty"], new ARM_NOP(), DEFAULT_VALUE_LATENCY);

  // specific to ARM: .word indicates addr, value, or indirect jump
  mnemonicToInstructionTypes[".word"] = new Word(formats["hex"]);

  // Arithmetic operation: add with carry
  mnemonicToInstructionTypes["adc"] = new Basic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with carry
  mnemonicToInstructionTypes["adcs"] = new Basic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with carry
  // add{cond}{s}
  mnemonicToInstructionTypes["add"] = new Basic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// Add
  mnemonicToInstructionTypes["adds"] = new Basic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);
  mnemonicToInstructionTypes["addeq"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal equal
  mnemonicToInstructionTypes["addne"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal not equal
  mnemonicToInstructionTypes["addcs"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal carry set
  // mnemonicToInstructionTypes["addhs"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal higher or same
  mnemonicToInstructionTypes["addcc"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal carry clear
  // mnemonicToInstructionTypes["addlo"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal lower
  // mnemonicToInstructionTypes["addmi"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal negative
  // mnemonicToInstructionTypes["addpl"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal positive or zero
  // mnemonicToInstructionTypes["addvs"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal signed overflow
  // mnemonicToInstructionTypes["addvc"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal no signed overflow
  mnemonicToInstructionTypes["addhi"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal higher
  mnemonicToInstructionTypes["addls"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal lower or same
  mnemonicToInstructionTypes["addge"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal greater or equal
  // mnemonicToInstructionTypes["addlt"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal less
  mnemonicToInstructionTypes["addgt"] = new PredicatedBasic("alu", formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD);	// add with conditionnal greater
  // mnemonicToInstructionTypes["addle"] = new PredicatedBasic("alu",formats["arithm"], new ARM_ADD(), DEFAULT_VALUE_LATENCY_ADD); // add with conditionnal less or equal

  // sub{cond}{s} Subtract
  mnemonicToInstructionTypes["sub"] = new Basic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);
  mnemonicToInstructionTypes["subs"] = new PredicatedBasic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub + set flags
  mnemonicToInstructionTypes["subeq"] = new PredicatedBasic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal equal
  mnemonicToInstructionTypes["subne"] = new PredicatedBasic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal not equal
  mnemonicToInstructionTypes["subcs"] = new PredicatedBasic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal carry set
  // mnemonicToInstructionTypes["subhs"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal higher or same
  mnemonicToInstructionTypes["subcc"] = new PredicatedBasic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal carry clear
  // mnemonicToInstructionTypes["sublo"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal lower
  // mnemonicToInstructionTypes["submi"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal negative
  // mnemonicToInstructionTypes["subpl"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal positive or zero
  // mnemonicToInstructionTypes["subvs"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal signed overflow
  // mnemonicToInstructionTypes["subvc"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal no signed overflow
  // mnemonicToInstructionTypes["subhi"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal higher
  // mnemonicToInstructionTypes["subls"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal lower or same
  // mnemonicToInstructionTypes["subge"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal greater or equal
  // mnemonicToInstructionTypes["sublt"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal less
  // mnemonicToInstructionTypes["subgt"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal greater
  // mnemonicToInstructionTypes["suble"] = new PredicatedBasic("alu",formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB); // sub with conditionnal less or equal
  mnemonicToInstructionTypes["subspl"] = new PredicatedBasic("alu", formats["arithm"], new ARM_TODO_LOIC(), DEFAULT_VALUE_LATENCY_SUB );	// sub with conditionnal carry clear ???
  mnemonicToInstructionTypes["subseq"] = new PredicatedBasic("alu", formats["arithm"], new ARM_TODO_LOIC(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal carry clear ???
  mnemonicToInstructionTypes["subscs"] = new PredicatedBasic("alu", formats["arithm"], new ARM_TODO_LOIC(), DEFAULT_VALUE_LATENCY_SUB);	// sub with conditionnal carry clear ???

  mnemonicToInstructionTypes["sbc"] = new Basic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with carry
  mnemonicToInstructionTypes["sbcs"] = new Basic("alu", formats["arithm"], new ARM_SUBTRACT(), DEFAULT_VALUE_LATENCY_SUB);	// sub with carry

  // reverse substract 
  mnemonicToInstructionTypes["rsb"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// reverse substract
  mnemonicToInstructionTypes["rsbs"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// reverse substract
  mnemonicToInstructionTypes["rsbmi"] = new Basic("alu", formats["logical"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// rsbmi R0, R0,#0 if R0<0 R0=-R0
  //  mnemonicToInstructionTypes["rsbsgt"] = new Basic("alu",formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB); // reverse substract ???? rsbgts
  //  mnemonicToInstructionTypes["rsbscs"] = new Basic("alu",formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB); // reverse substract  ????? rsbcss
  mnemonicToInstructionTypes["rsblt"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// reverse substract
  // mnemonicToInstructionTypes["rsble"] = new Basic("alu",formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB); // reverse substract ???? 
  mnemonicToInstructionTypes["rsbne"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// reverse substract
  // reverse substract with carry.
  mnemonicToInstructionTypes["rsc"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// rsb with carry
  mnemonicToInstructionTypes["rscs"] = new Basic("alu", formats["arithm"], new ARM_REVERSE_SUB(), DEFAULT_VALUE_LATENCY_RSUB);	// rsb with carry + set flags

  // Multiply
  mnemonicToInstructionTypes["mul"] = new Basic("multiplier", formats["rd_rs_rs"], new ARM_MUL(), DEFAULT_VALUE_LATENCY_MUL );	// (not entirely sure about the functionnal unit)
  mnemonicToInstructionTypes["mla"] = new Basic("multiplier", formats["rd_rs_rs_rs"], new ARM_MUL(), DEFAULT_VALUE_LATENCY_MUL);	// multiply-accumulate (not referenced in the benchmark)
  mnemonicToInstructionTypes["smull"] = new Basic("multiplier", formats["rd_rd_rs_rs"], new ARM_MUL(), DEFAULT_VALUE_LATENCY_MUL);	// signed long multiply (not entirely sure about the functionnal unit)
  // mnemonicToInstructionTypes["smlal"] = new Basic("multiplier",formats["rds_rds_rs_rs"], new ARM_TODO_LOIC()); // signed long multiply-accumulate (not referenced in the benchmark)
  // mnemonicToInstructionTypes["umlal"] = new Basic("multiplier",formats["rds_rds_rs_rs"], new ARM_TODO_LOIC()); // unsigned long multiply-accumulate (not referenced in the benchmark) 
  // mnemonicToInstructionTypes["umull"] = new Basic("multiplier",formats["rd_rd_rs_rs"], new ARM_TODO_LOIC()); // unsigned long multiply (not referenced in the benchmark)

  // Move
  mnemonicToInstructionTypes["mov"] = new Basic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);
  mnemonicToInstructionTypes["movs"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	//  mov + update flags (s)
  mnemonicToInstructionTypes["moveq"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal equal
  mnemonicToInstructionTypes["movne"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal not equal
  mnemonicToInstructionTypes["movcs"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal carry set
  // mnemonicToInstructionTypes["movhs"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal higher or same
  mnemonicToInstructionTypes["movcc"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal carry clear
  // mnemonicToInstructionTypes["movlo"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal lower
  // mnemonicToInstructionTypes["movmi"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal negative
  // mnemonicToInstructionTypes["movpl"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal positive or zero
  // mnemonicToInstructionTypes["movvs"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal signed overflow
  // mnemonicToInstructionTypes["movvc"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mov with conditionnal no signed overflow
  mnemonicToInstructionTypes["movhi"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal higher
  mnemonicToInstructionTypes["movls"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal lower or same
  mnemonicToInstructionTypes["movge"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal greater or equal
  mnemonicToInstructionTypes["movlt"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal less
  mnemonicToInstructionTypes["movgt"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal greater
  mnemonicToInstructionTypes["movle"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mov with conditionnal less or equal

  mnemonicToInstructionTypes["movw"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// MOVW instruction provides the same function as MOV, but is restricted to using the imm16 operand.
  mnemonicToInstructionTypes["movt"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// Move Top. Writes a 16-bit immediate value to the top halfword of a register, without affecting the bottom halfword.

  // move NOT
  mnemonicToInstructionTypes["mvn"] = new Basic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);
  mnemonicToInstructionTypes["mvns"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	//  bitwise logical negate operation + update flags (s) .
  mnemonicToInstructionTypes["mvneq"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mvn with conditionnal equal
  // mnemonicToInstructionTypes["mvnne"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal not equal
  // mnemonicToInstructionTypes["mvncs"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal carry set
  // mnemonicToInstructionTypes["mvnhs"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal higher or same
  mnemonicToInstructionTypes["mvncc"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mvn with conditionnal carry clear
  // mnemonicToInstructionTypes["mvnlo"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal lower
  // mnemonicToInstructionTypes["mvnmi"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal negative
  // mnemonicToInstructionTypes["mvnpl"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal positive or zero
  // mnemonicToInstructionTypes["mvnvs"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal signed overflow
  // mnemonicToInstructionTypes["mvnvc"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal no signed overflow
  // mnemonicToInstructionTypes["mvnhi"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal higher
  // mnemonicToInstructionTypes["mvnls"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal lower or same
  // mnemonicToInstructionTypes["mvnge"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal greater or equal
  // mnemonicToInstructionTypes["mvnlt"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal less
  mnemonicToInstructionTypes["mvngt"] = new PredicatedBasic("alu", formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV);	// mvn with conditionnal greater
  // mnemonicToInstructionTypes["mvnle"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); // mvn with conditionnal less or equal
  // mnemonicToInstructionTypes["mvnsne"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV( logical negate operation + update flags (s) . mvnnes
  // mnemonicToInstructionTypes["mvnseq"] = new PredicatedBasic("alu",formats["move"], new ARM_MOV(), DEFAULT_VALUE_LATENCY_MOV); //  bitwise logical negate operation + update flags (s) . mvneqs

  // Load
  mnemonicToInstructionTypes["ldr"] = new Load(4, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);
  mnemonicToInstructionTypes["ldrls"] = new PredicatedLoad(4, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);	// ldr with conditionnal lower or same
  // mnemonicToInstructionTypes["ldrt"] = new Load(4,"alu",formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD); used in non_user mode.

  mnemonicToInstructionTypes["ldrh"] = new Load(2, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);	// half-word
  mnemonicToInstructionTypes["ldrsh"] = new Load(2, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);	// signed half-word

  mnemonicToInstructionTypes["ldrb"] = new Load(1, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);	// byte
  mnemonicToInstructionTypes["ldrsb"] = new Load(1, "alu", formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD);	// signed byte 
  // mnemonicToInstructionTypes["ldrbt"] = new Load(1,"alu",formats["load"], new ARM_LOAD(), DEFAULT_VALUE_LATENCY_LOAD); // byte used in non_user mode.

  // Load multiple
  mnemonicToInstructionTypes["ldm"] = new Load(4, "alu", formats["load-multiple"], new ARM_LOAD_MULTIPLE(), DEFAULT_VALUE_LATENCY_LOAD);	// load multiple
  // mnemonicToInstructionTypes["ldmdb"] = new Load(4,"alu",formats["load-multiple"], new ARM_LOAD_MULTIPLE(), DEFAULT_VALUE_LATENCY_LOAD); // load multiple (Decrement Before each access)
  // mnemonicToInstructionTypes["ldmib"] = new Load(4,"alu",formats["load-multiple"], new ARM_LOAD_MULTIPLE(), DEFAULT_VALUE_LATENCY_LOAD); // load multiple (Increment Before each access)

  mnemonicToInstructionTypes["pop"] = new Load(4, "alu", formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
  /*  mnemonicToInstructionTypes["pophi"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
     mnemonicToInstructionTypes["popge"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
     mnemonicToInstructionTypes["popgt"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
     mnemonicToInstructionTypes["popne"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
     mnemonicToInstructionTypes["popcc"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
     mnemonicToInstructionTypes["pople"] = new Load(4,"alu",formats["pop"], new ARM_POP(), DEFAULT_VALUE_LATENCY_POP);
   */

  // Store
  mnemonicToInstructionTypes["str"] = new Store(4, "alu", formats["store"], new ARM_STORE(), DEFAULT_VALUE_LATENCY_STORE);
  mnemonicToInstructionTypes["strh"] = new Store(2, "alu", formats["store"], new ARM_STORE(), DEFAULT_VALUE_LATENCY_STORE);	// half-word
  mnemonicToInstructionTypes["strb"] = new Store(1, "alu", formats["store"], new ARM_STORE(), DEFAULT_VALUE_LATENCY_STORE);	// byte
  // mnemonicToInstructionTypes["strsh"] = new Store(2,"alu",formats["store"], new ARM_STORE(), DEFAULT_VALUE_LATENCY_STORE); // signed half-word
  // mnemonicToInstructionTypes["strsb"] = new Store(1,"alu",formats["store"], new ARM_STORE(), DEFAULT_VALUE_LATENCY_STORE); // signed byte

  mnemonicToInstructionTypes["stm"] = new Store(4, "alu", formats["store-multiple"], new ARM_STORE_MULTIPLE(), DEFAULT_VALUE_LATENCY_STORE);	// store multiple
  mnemonicToInstructionTypes["stmdb"] = new Store(4, "alu", formats["store-multiple"], new ARM_STORE_MULTIPLE(), DEFAULT_VALUE_LATENCY_STORE);	// store multiple (Decrement Before each access)
  mnemonicToInstructionTypes["stmib"] = new Store(4, "alu", formats["store-multiple"], new ARM_STORE_MULTIPLE(), DEFAULT_VALUE_LATENCY_STORE);	// store multiple (Increment Before each access)

  mnemonicToInstructionTypes["push"] = new Store(4, "alu", formats["push"], new ARM_PUSH(), DEFAULT_VALUE_LATENCY_PUSH);

  // Branch
  mnemonicToInstructionTypes["bl"] = new Call("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["b"] = new UnconditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bgt"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Greater
  mnemonicToInstructionTypes["bge"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Greater or Equal
  mnemonicToInstructionTypes["blt"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Less
  mnemonicToInstructionTypes["ble"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Less or Equal
  mnemonicToInstructionTypes["beq"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Equal / Equal Zero
  mnemonicToInstructionTypes["bne"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Not equal
  mnemonicToInstructionTypes["bpl"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Plus / Positive or Zero
  // mnemonicToInstructionTypes["bmi"] = new ConditionalJump("alu",formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH); // Minus / Negative
  mnemonicToInstructionTypes["bls"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Lower or Same
  mnemonicToInstructionTypes["bhi"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Unsigned higher
  mnemonicToInstructionTypes["bcc"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Carry clear
  // mnemonicToInstructionTypes["blo"] = new ConditionalJump("alu",formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH); // Unsigned lower
  mnemonicToInstructionTypes["bcs"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Carry set
  // mnemonicToInstructionTypes["bhs"] = new ConditionalJump("alu",formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH); // Unsigned higher or same
  // mnemonicToInstructionTypes["bvs"] = new ConditionalJump("alu",formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH); // Signed overflow
  // /mnemonicToInstructionTypes["bvc"] = new ConditionalJump("alu",formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH); // No signed overflow
  mnemonicToInstructionTypes["bleq"] = new ConditionalJump("alu", formats["addr"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);	// Less or Equal

  mnemonicToInstructionTypes["bx"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxhi"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxne"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxeq"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxcc"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxge"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxgt"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxle"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);
  mnemonicToInstructionTypes["bxlt"] = new Return("alu", formats["rs"], new ARM_BRANCH(), DEFAULT_VALUE_LATENCY_BRANCH);

  // Shift and rotation
  mnemonicToInstructionTypes["lsl"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left
  mnemonicToInstructionTypes["lsleq"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)
  mnemonicToInstructionTypes["lslcc"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)
  mnemonicToInstructionTypes["lsls"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)
  mnemonicToInstructionTypes["lslle"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)
  mnemonicToInstructionTypes["lslsne"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)
  mnemonicToInstructionTypes["lslseq"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shitf Left cond  (similar to lsl)

  mnemonicToInstructionTypes["lsr"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right
  mnemonicToInstructionTypes["lsrsne"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right
  mnemonicToInstructionTypes["lsrseq"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right
  mnemonicToInstructionTypes["lsrs"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right
  mnemonicToInstructionTypes["lsrcs"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right
  mnemonicToInstructionTypes["lsrne"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Logical Shift Right

  mnemonicToInstructionTypes["asr"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Arithmetic Shift Right
  mnemonicToInstructionTypes["asreq"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Arithmetic Shift Right
  mnemonicToInstructionTypes["asrhi"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Arithmetic Shift Right
  mnemonicToInstructionTypes["asreq"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Arithmetic Shift Right
  mnemonicToInstructionTypes["asrcs"] = new Basic("barrel_shifter", formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Arithmetic Shift Right

  // mnemonicToInstructionTypes["ror"] = new Basic("barrel_shifter",formats["shift-rotate"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT); // Rotate Right
  mnemonicToInstructionTypes["rrx"] = new Basic("barrel_shifter", formats["rd_rs"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Rotate Right with Extend
  mnemonicToInstructionTypes["rrxs"] = new Basic("barrel_shifter", formats["rd_rs"], new ARM_SHIFT(), DEFAULT_VALUE_LATENCY_SHIFT);	// Rotate Right with Extend + set flags

  // Comparison : update the CPSR flags.
  mnemonicToInstructionTypes["cmp"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare
  mnemonicToInstructionTypes["cmpcc"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare (similar to cmp)
  mnemonicToInstructionTypes["cmpeq"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare (similar to cmp)
  mnemonicToInstructionTypes["cmphi"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare (similar to cmp)
  mnemonicToInstructionTypes["cmpll"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare (similar to cmp)
  mnemonicToInstructionTypes["cmppl"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare (similar to cmp)
  mnemonicToInstructionTypes["cmn"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare Negative
  mnemonicToInstructionTypes["cmnmi"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Compare Negative

  mnemonicToInstructionTypes["teq"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Test Equivalence
  mnemonicToInstructionTypes["teqeq"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Test Equivalence (similar to teq)
  mnemonicToInstructionTypes["teqne"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Test Equivalence (similar to teq)
  mnemonicToInstructionTypes["tst"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Test
  mnemonicToInstructionTypes["tsteq"] = new Basic("alu", formats["compare"], new ARM_COMPARE(), DEFAULT_VALUE_LATENCY_COMPARE);	// Test 

  // Logical
  mnemonicToInstructionTypes["and"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND
  mnemonicToInstructionTypes["ands"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND
  mnemonicToInstructionTypes["andsne"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND
  mnemonicToInstructionTypes["andle"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND
  mnemonicToInstructionTypes["andlt"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND

  mnemonicToInstructionTypes["bic"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND NOT
  mnemonicToInstructionTypes["biceq"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND NOT
  mnemonicToInstructionTypes["bicsne"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND NOT
  mnemonicToInstructionTypes["bics"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical AND NOT

  mnemonicToInstructionTypes["eor"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical Exclusive OR
  mnemonicToInstructionTypes["eoreq"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical Exclusive OR
  mnemonicToInstructionTypes["eorgt"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical Exclusive OR
  mnemonicToInstructionTypes["eorne"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical Exclusive OR

  mnemonicToInstructionTypes["orr"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR
  mnemonicToInstructionTypes["orrcc"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrcs"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orreq"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrge"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrgt"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrle"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrlt"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrne"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrs"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrseq"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)
  mnemonicToInstructionTypes["orrsne"] = new Basic("alu", formats["logical"], new ARM_LOGICAL(), DEFAULT_VALUE_LATENCY_LOGICAL);	// Logical OR cond.(similar to orr)

  // ------------------------------------------
  // regs declaration
  // ------------------------------------------
  // User/System mode registers only (32-bit)
  // for APCS, check -> http:// infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/ch09s02s02.html

  regs["r0"] = 0;
  regs["a1"] = 0;		// APCS name (argument)

  regs["r1"] = 1;
  regs["a2"] = 1;		// APCS name (argument)

  regs["r2"] = 2;
  regs["a3"] = 2;		// APCS name (argument)

  regs["r3"] = 3;
  regs["a4"] = 3;		// APCS name (argument)

  regs["r4"] = 4;
  regs["v1"] = 4;		// APCS name (variable)

  regs["r5"] = 5;
  regs["v2"] = 5;		// APCS name (variable)

  regs["r6"] = 6;
  regs["v3"] = 6;		// APCS name (variable)

  regs["r7"] = 7;
  regs["v4"] = 7;		// APCS name (variable)

  regs["r8"] = 8;
  regs["v5"] = 8;		// APCS name (variable)

  regs["r9"] = 9;
  regs["v6"] = 9;		// APCS name (variable)
  regs["sb"] = 9;		// APCS name (static base)

  regs["r10"] = 10;
  regs["v7"] = 10;		// APCS name (variable)
  regs["sl"] = 10;		// APCS name (stack limit)

  regs["r11"] = 11;
  regs["v8"] = 11;		// APCS name (variable)
  regs["fp"] = 11;		// APCS name (frame pointer)

  regs["r12"] = 12;
  regs["ip"] = 12;		// APCS name (new static base in inter-link-unit calls {dedicated role only during function call} )

  regs["r13"] = 13;
  regs["sp"] = 13;		// APCS name (stack pointer)

  regs["r14"] = 14;
  regs["lr"] = 14;		// APCS name (link register {holds the address to return to when a function call} )

  regs["r15"] = 15;
  regs["pc"] = 15;		// APCS name (program counter)

  regs["cpsr"] = 16;		// current program status register

  // Floating-point registers (usefull ?)
  regs["f0"] = 17;
  regs["f1"] = 18;
  regs["f2"] = 19;
  regs["f3"] = 20;
  regs["f4"] = 21;
  regs["f5"] = 22;
  regs["f6"] = 23;
  regs["f7"] = 24;

  regs["raux"] = 25;		// An auxillary register. Used for the decomposition of the instructions (shift rotate, ...)
}

// -----------------------------------------------------
// 
//  ARM class functions
// 
// -----------------------------------------------------

ARM::~ARM()
{
  // delete formats map
  for (map < string, set < InstructionFormat * > >::iterator it1 = formats.begin(); it1 != formats.end(); it1++)
    {
      set < InstructionFormat * >current = (*it1).second;
      for (set < InstructionFormat * >::iterator it2 = current.begin(); it2 != current.end(); it2++)
	{
	  delete(*it2);
	}
    }

  // delete mnemonicToInstructionTypes map
  for (map < string, InstructionType * >::iterator it = mnemonicToInstructionTypes.begin(); it != mnemonicToInstructionTypes.end(); it++)
    {
      delete((*it).second);
    }
}

bool ARM::isLoadStoreSimple(string & instr)
{
  if (instr.find("[") == EOS) return false;
  return true;
}

string ARM::removeUselessCharacters(const string & line)
{
  string result = line;
  for (size_t i = 0; i < line.length(); i++)
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
      else if (result[i] == '#')
	{
	  result[i] = ' ';
	}
      else if (result[i] == '=')
	{
	  result[i] = ' ';
	}
      else if (result[i] == ';')
	{
	  result[i] = ' ';
	}
    }
  return result;
}

string ARM::removeUselessCharactersMemPattern(const string & line)
{
  string result = line;

  // Removing "["
  size_t pos = result.find(memory_access_begin_character);
  if (pos != EOS)
    {
      result[pos] = ' ';
    }

  // Removing all ","
  do
    {
      pos = result.find(",");
      if (pos != EOS)
	{
	  result[pos] = ' ';
	}
    }
  while (pos != EOS);

  // Removing "]"
  pos = result.find(memory_access_end_character);
  if (pos != EOS)
    {
      result[pos] = ' ';
    }

  return result;
}

/*
  Split the operands
 
  Example : operands = r3!, [r1, r3, lsr 8]!, {r4, r5, r6-r9}, lsl r5, r8, [r2, #4]
  The result will be :
  result[0] : r3!
  result[1] : [r1, r3, lsr 8]!
  result[2] : {r4, r5, r6-r9}
  result[3] : lsl r5
  result[4] : r8
  result[5] : [r2, #4]
*/
vector < string > ARM::splitOperands(const string & operands)
{
  vector < string > result;

  if (operands.length() == 0)
    return result;
  string current_op(""), tmp = operands;
  bool same_operand = false, badd;
  char c;

  for (size_t i = 0; i < tmp.length(); i++)
    {
      c = tmp[i];
      badd = true;
      // building a "block" such that [...] or { ...} when same_operand is true.
      if ((c == memory_access_begin_character[0]) || (c == list_begin_character[0]))
	{
	  same_operand = true;
	}
      if (same_operand)
	{
	  same_operand = (c != memory_access_end_character[0]) && (c != list_end_character[0]);
	}
      else
	{
	  if (c == ',')
	    {
	      if (current_op != "")
		{
		  result.push_back(current_op);
		}
	      current_op = "";
	      badd = false;
	    }
	  else
	    badd = (c != ' ') || (current_op != "");	// skipping the spaces after a comma.
	}
      if (badd)
	current_op += c;
    }

  if (current_op != "")
    {
      result.push_back(current_op);
    }
  return result;
}

bool ARM::isFunction(const string & line)
{
  return (line[0] != ' ');
}

bool ARM::isInstruction(const string & line)
{
  return (line[0] == ' ');
}

ObjdumpFunction ARM::parseFunction(const string & line)
{
  ObjdumpFunction result;
  result.line = line;

  string clean_line = removeUselessCharacters(line);

  string s_addr;
  istringstream parse(clean_line);
  parse >> s_addr >> result.name;
  result.addr = strtoul(s_addr.c_str(), NULL, 16);
  assert(result.addr != 0 && result.addr != ULONG_MAX);

  return result;
}

ObjdumpInstruction ARM::parseInstruction(const string & line)
{
  ObjdumpInstruction result;
  result.line = line;

  string clean_line = removeUselessCharacters(line);

  string operands;
  string operand_tmp;
  string s_addr;
  string binary_code;
  string extra_tmp;
  bool missing_comma = false;

  istringstream parse(clean_line);
  parse >> s_addr >> binary_code >> result.mnemonic;

  if (result.mnemonic != "nop")
    {
      while (!missing_comma)
	{
	  parse >> operand_tmp;
	  // For detecting end of operands section (there is no comma)
	  if (operand_tmp[operand_tmp.length() - 1] != ',')
	    {
	      missing_comma = find(shifter_with_operand.begin(), shifter_with_operand.end(), operand_tmp) == shifter_with_operand.end();
	    }
	  if (operands != "")
	    {
	      operands += " ";
	    }
	  operands += operand_tmp;
	}
    }

  // Extracting the extra field
  while (parse >> extra_tmp)
    {
      result.extra = result.extra + " " + extra_tmp;
    }

  result.addr = strtoul(s_addr.c_str(), NULL, 16);
  assert(result.addr != 0 && result.addr != ULONG_MAX);

  // Splitting the operands into a vector
  result.operands = splitOperands(operands);

  /* check that the mnemonic is defined and the operand format is correct */
  if (!getInstructionTypeFromMnemonic(result.mnemonic)->checkFormat(result.operands))
    {
      Logger::addFatal("Error: instruction asm \"" + result.mnemonic + "\" format not valid");
    }
  // --
  result.asm_code = result.mnemonic;
  if (operands != "")
    {
      result.asm_code = result.asm_code + " " + operands;
    }

  return result;
}

t_address ARM::getJumpDestination(const ObjdumpInstruction & instr)
{

  string s_addr = instr.operands[instr.operands.size() - 1];

  t_address result = strtoul(s_addr.c_str(), NULL, 16);
  assert(result != 0 && result != ULONG_MAX);

  return result;
}

bool ARM::isMemPattern(const string & operand)
{

  if (isMem_Basic_Pattern(operand))
    {
      return true;
    }
  else if (isMem_Basic_PatternRW(operand))
    {
      return true;
    }
  else if (isMem_RegReg_Pattern(operand))
    {
      return true;
    }
  else if (isMem_RegReg_PatternRW(operand))
    {
      return true;
    }
  else if (isMem_RegRegShifterReg_Pattern(operand))
    {
      return true;
    }
  else if (isMem_RegRegShifterReg_PatternRW(operand))
    {
      return true;
    }
  else if (isMem_RegRegShifterInt_Pattern(operand))
    {
      return true;
    }
  else if (isMem_RegRegShifterInt_PatternRW(operand))
    {
      return true;
    }

  return false;
}

// Format : [reg, int_offset] or [reg]
bool ARM::isMem_Basic_Pattern(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  // parse op_local
  string reg_name, offset(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> offset >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (!isRegisterName(reg_name))
    {
      return false;
    }

  if (offset != "")
    {
      if (!Utl::isDecNumber(offset))
	{
	  return false;
	}
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [reg, int_offset]! 
bool ARM::isMem_Basic_PatternRW(const string & operand)
{

  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, offset(""), exclamation(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> offset >> exclamation >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (offset == "")
    {
      return false;
    }
  if (Utl::isDecNumber(offset) == false)
    {
      return false;
    }

  if (exclamation != register_writting_character)
    {
      return false;
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [base, reg_offset]
bool ARM::isMem_RegReg_Pattern(const string & operand)
{

  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [base, reg_offset]!
bool ARM::isMem_RegReg_PatternRW(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, exclamation(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> exclamation >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  if (exclamation != register_writting_character)
    {
      return false;
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [base, reg, shifter reg]
bool ARM::isMem_RegRegShifterReg_Pattern(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, shifter, reg_shifter, extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> shifter >> reg_shifter >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  // Checking if it is "lsl", "lsr", "asr" or "ror"
  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) == shifter_with_operand.end())
    {
      return false;
    }

  if (reg_shifter == "")
    {
      return false;
    }
  if (isRegisterName(reg_shifter) == false)
    {
      return false;
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [base, reg, shifter reg]!
bool ARM::isMem_RegRegShifterReg_PatternRW(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, shifter, reg_shifter, exclamation(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> shifter >> reg_shifter >> exclamation >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  // Checking if it is "lsl", "lsr", "asr" or "ror"
  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) == shifter_with_operand.end())
    {
      return false;
    }

  if (reg_shifter == "")
    {
      return false;
    }
  if (isRegisterName(reg_shifter) == false)
    {
      return false;
    }

  if (exclamation != register_writting_character)
    {
      return false;
    }

  // Checking if there are no extra operands
  if (extra_operand != "")
    {
      return false;
    }

  return true;
}

// Format : [base, reg, shifter int] or [base, reg, rrx]
bool ARM::isMem_RegRegShifterInt_Pattern(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, shifter, shift_value(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> shifter >> shift_value >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  // Checking if the shifter is "lsl", lsr", "asr" or "ror"
  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) == shifter_with_operand.end())
    {
      // If it's not the case check if the shifter is "rrx"
      if (find(shifter_without_operand.begin(), shifter_without_operand.end(), shifter) == shifter_without_operand.end())
	{
	  return false;
	}
      else
	{
	  // The shifter is "rrx" so there is no integer or register after
	  // Checking if there are no extra operands
	  if (shift_value != "")
	    {
	      return false;
	    }
	}
    }
  else
    {
      // The shifter is "lsl", lsr", "asr" or "ror"
      if (shift_value == "")
	{
	  return false;
	}
      if (Utl::isDecNumber(shift_value) == false)
	{
	  return false;
	}

      // Checking if there are no extra operands
      if (extra_operand != "")
	{
	  return false;
	}
    }

  return true;
}

// Format : [base, reg, shifter int]! or [base, reg, rrx]!
bool ARM::isMem_RegRegShifterInt_PatternRW(const string & operand)
{
  if (operand.size() == 0)
    {
      return false;
    }

  string op_local = operand;

  // Searching for "["
  size_t pos = op_local.find(memory_access_begin_character);
  if (pos == EOS)
    {
      return false;
    }

  // Searching for "]"
  pos = op_local.find(memory_access_end_character);
  if (pos == EOS)
    {
      return false;
    }

  // Removing useless characters like '[' ']' ','
  op_local = removeUselessCharactersMemPattern(op_local);

  string reg_name, reg_offset, shifter, shift_value(""), exclamation(""), extra_operand("");
  istringstream parse(op_local);
  parse >> reg_name >> reg_offset >> shifter >> shift_value >> exclamation >> extra_operand;

  if (reg_name == "")
    {
      return false;
    }
  if (isRegisterName(reg_name) == false)
    {
      return false;
    }

  if (reg_offset == "")
    {
      return false;
    }
  if (isRegisterName(reg_offset) == false)
    {
      return false;
    }

  // Checking if the shifter is "lsl", lsr", "asr" or "ror"
  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) == shifter_with_operand.end())
    {
      // If it's not the case
      // Checking if the shifter is "rrx"
      if (find(shifter_without_operand.begin(), shifter_without_operand.end(), shifter) == shifter_without_operand.end())
	{
	  return false;
	}
      else
	{
	  // The shifter is "rrx" so there is no integer or register after
	  if (shift_value != register_writting_character)
	    {
	      return false;
	    }

	  // Checking if there are no extra operands
	  if (exclamation != "")
	    {
	      return false;
	    }
	}
    }
  else
    {
      // The shifter is "lsl", lsr", "asr" or "ror"

      if (shift_value == "")
	{
	  return false;
	}
      if (Utl::isDecNumber(shift_value) == false)
	{
	  return false;
	}

      if (exclamation != register_writting_character)
	{
	  return false;
	}

      // Checking if there are no extra operands
      if (extra_operand != "")
	{
	  return false;
	}
    }

  return true;
}

vector < string > ARM::extractInputRegistersFromMem(const string & operand)
{
  vector < string > result;
  string op_local = removeUselessCharactersMemPattern(operand);

  // Format : [reg, int_offset] or [reg]
  if (isMem_Basic_Pattern(operand))
    {
      string reg_name;
      istringstream parse(op_local);
      parse >> reg_name;

      result.push_back(reg_name);
    }

  // Format : [reg, int_offset]!
  else if (isMem_Basic_PatternRW(operand))
    {
      string reg_name;
      istringstream parse(op_local);
      parse >> reg_name;

      result.push_back(reg_name);
    }

  // Format : [base, reg_offset]
  else if (isMem_RegReg_Pattern(operand))
    {
      string reg_name, reg_offset;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset;

      result.push_back(reg_name);
      result.push_back(reg_offset);
    }

  // Format : [base, reg_offset]!
  else if (isMem_RegReg_PatternRW(operand))
    {
      string reg_name, reg_offset;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset;

      result.push_back(reg_name);
      result.push_back(reg_offset);
    }

  // Format : [base, reg, shifter reg]
  else if (isMem_RegRegShifterReg_Pattern(operand))
    {
      string reg_name, reg_offset, shifter, reg_shifter;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset >> shifter >> reg_shifter;

      result.push_back(reg_name);
      result.push_back(reg_offset);
      result.push_back(reg_shifter);
    }

  // Format : [base, reg, shifter reg]!
  else if (isMem_RegRegShifterReg_PatternRW(operand))
    {
      string reg_name, reg_offset, shifter, reg_shifter;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset >> shifter >> reg_shifter;

      result.push_back(reg_name);
      result.push_back(reg_offset);
      result.push_back(reg_shifter);
    }

  // Format : [base, reg, shifter int] or [base, reg, rrx]
  else if (isMem_RegRegShifterInt_Pattern(operand))
    {
      string reg_name, reg_offset;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset;

      result.push_back(reg_name);
      result.push_back(reg_offset);
    }

  // Format : [base, reg, shifter int]! or [base, reg, rrx]!
  else if (isMem_RegRegShifterInt_PatternRW(operand))
    {
      string reg_name, reg_offset;
      istringstream parse(op_local);
      parse >> reg_name >> reg_offset;

      result.push_back(reg_name);
      result.push_back(reg_offset);
    }

  return result;
}

vector < string > ARM::extractOutputRegistersFromMem(const string & operand)
{
  vector < string > result;
  string reg_name;

  string op_local = removeUselessCharactersMemPattern(operand);

  istringstream parse(op_local);

  // Format : [base, int_offset]!
  if (isMem_Basic_PatternRW(operand))
    {
      parse >> reg_name;

      result.push_back(reg_name);
    }
  // Format : [base, reg_offset]!
  else if (isMem_RegReg_PatternRW(operand))
    {
      parse >> reg_name;

      result.push_back(reg_name);
    }
  // Format : [base, reg, shifter reg]!
  else if (isMem_RegRegShifterReg_PatternRW(operand))
    {
      parse >> reg_name;

      result.push_back(reg_name);
    }
  // Format : [base, reg, shifter int]! or [base, reg, rrx]!
  else if (isMem_RegRegShifterInt_PatternRW(operand))
    {
      parse >> reg_name;

      result.push_back(reg_name);
    }

  return result;
}

vector < string > ARM::getResourceFunctionalUnits(const string & instr)
{
  vector < string > result;
  result.push_back(getInstructionTypeFromAsm(instr)->getResourceFunctionalUnit());

  string mnemonic, operands(instr);
  istringstream parse(instr);

  // Extracting the mnemonic
  parse >> mnemonic;

  // Getting the operands
  operands.erase(0, mnemonic.length());

  // Splitting properly these operands into a vector
  vector < string > splitted_operands = splitOperands(operands);

  for (vector < string >::const_iterator it = splitted_operands.begin(); it != splitted_operands.end(); ++it)
    {
      // If it is a Memory Pattern which use a shifter operand
      if (isMem_RegRegShifterReg_Pattern(*it) || isMem_RegRegShifterReg_PatternRW(*it) || isMem_RegRegShifterInt_Pattern(*it)
	  || isMem_RegRegShifterInt_PatternRW(*it))
	{
	  result.push_back("barrel_shifter");
	  break;
	}

      // If the current operand is a shifter
      if (isShifterOperand(*it))
	{
	  result.push_back("barrel_shifter");
	  break;
	}
    }

  return result;
}

bool ARM::isLoadMultiple(const string & instr)
{
  return (getNumberOfLoads(instr) > 1);
}

int ARM::getNumberOfLoads(const string & instr)
{
  string mnemonic, operands(instr);
  istringstream parse(instr);

  // Extracting the mnemonic
  parse >> mnemonic;
  assert(getInstructionTypeFromMnemonic(mnemonic)->isLoad());

  // We define the number of loads, at least 1
  int nb_loads = 1;

  // Getting the operands
  operands.erase(0, mnemonic.length());

  // We split the operands into a vector
  vector < string > splitted_operands = splitOperands(operands);

  /*
     We check if there is a register list in the operands
     example : ldm r1, {r2, r3}
     pop {r2, r3}
   */
  for (vector < string >::const_iterator it = splitted_operands.begin(); it != splitted_operands.end(); it++)
    {
      // If there is a register list
      if (isRegisterList(*it))
	{
	  // Getting the registers in the list
	  vector < string > registers_of_list = extractRegistersFromRegisterList(*it);

	  // the number of loads = the size of the list
	  nb_loads = registers_of_list.size();
	}
    }

  return nb_loads;
}

bool ARM::isStoreMultiple(const string & instr)
{
  return (getNumberOfStores(instr) > 1);
}

int ARM::getNumberOfStores(const string & instr)
{
  string mnemonic, operands(instr);
  istringstream parse(instr);

  // Extracting the mnemonic
  parse >> mnemonic;

  // Checking if the instruction is a Store
  assert(getInstructionTypeFromMnemonic(mnemonic)->isStore());

  // We define the number of stores, at least 1
  int nb_stores = 1;

  // Getting the operands
  operands.erase(0, mnemonic.length());

  // We split the operands into a vector
  vector < string > splitted_operands = splitOperands(operands);

  /*
     We check if there is a register list in the operands

     example : stm r1, {r2, r3}
     push {r2, r3}
   */
  for (vector < string >::const_iterator it = splitted_operands.begin(); it != splitted_operands.end(); it++)
    {
      if (isRegisterList(*it))
	nb_stores = getSizeRegisterList(*it);
    }

  return nb_stores;
}

/**
   @return the number of registers of the operand assumed to be {   }
*/
int ARM::getSizeRegisterList(const string & operand)
{
  vector < string > registers_of_list = extractRegistersFromRegisterList(operand);
  return registers_of_list.size();
}

bool ARM::isPopWithPC(const ObjdumpInstruction & instr)
{
  if (instr.mnemonic.length() >= 3)
    {
      // For checking it it's "pop" or one of its variants (popge, popgt, pople, poplt, ...)
      if (instr.mnemonic[0] == 'p' && instr.mnemonic[1] == 'o' && instr.mnemonic[2] == 'p')
	{

	  // We extract all registers in the list ( asm code is like : pop {r3, r5, fp, pc} )
	  vector < string > registers = extractRegistersFromRegisterList(instr.operands[0]);

	  for (vector < string >::const_iterator it = registers.begin(); it != registers.end(); ++it)
	    {
	      if ((*it) == "pc" || (*it) == "PC")
		{
		  return true;
		}
	    }
	}
    }

  return false;
}

bool ARM::isReturn(const ObjdumpInstruction & instr)
{

  // We check if the instruction is a Pop with PC register, because Pop is a Load instruction but with PC register 
  // it becomes also a Return instruction.
  if (isPopWithPC(instr)) return true;
  return getInstructionTypeFromMnemonic(instr.mnemonic)->isReturn();
}

string ARM::getCalleeName(const ObjdumpInstruction & instr)
{
  assert(getInstructionTypeFromMnemonic(instr.mnemonic)->isCall());

  // We assume that the extra field is in the format " FunctionName"

  string result = instr.extra;

  // Removing ' '
  result.erase(0, 1);

  return result;
}

void ARM::parseSymbolTableLine(const string & line, ObjdumpSymbolTable & table)
{
  /* We assume that the input line is in the format : Value  Location  Type  Section  Size  Name
     for some lines, the field Type is empty */

  string value, location, type, section, size, name;
  istringstream parse(line);

  // Extracting each field of the line. Several formats:
  // 000082e8 l       .text     00000000 .divsi3_skip_div0_test
  // 000082e0 g     F .text     00000000 .hidden __aeabi_idiv
  // 00008028 g     F .text     0000003c foo

  // parse >> value >> location >> type >> section >> size >> name;
  parse >> value >> location >> type;
  if (type == ".text")
    {
      section = ".text";
      type = "UNDEF";
      parse >> size >> name;
    }
  else
    parse >> section >> size >> name;
  if (name == ".hidden")
    parse >> name;		// Added LBesnard.

  // Function
  // if  (type == "F" && section ==".text")
  if (section == ".text" && (type == "F" || (type == "UNDEF" && location == "l")))
    {
      assert(name != "");
      if (BTRACE)
	cout << " value=" << value << ", location=" << location << ", type= " << type << ", section= " << section << ", size=" << size << ", name=" <<
	    name << endl;

      // Converting the string to Unsigned Long
      t_address addr = strtoul(value.c_str(), NULL, 16);
      assert((addr != 0) && (addr != ULONG_MAX));
      // Adding the function in the map
      // BUT WE MAY HAVE SAME VALUE FOR TWO FUNCTIONS..
      if (BTRACE)
	{
	  ListOfString lnames = table.functions[addr];
	  if (!lnames.empty())
	    cout << ">>>>>> Collision for " << name << " and " << *(lnames.begin()) << endl;
	}
      // table.functions[addr] = name; replaced by (Lbesnard)
      table.functions[addr].push_front(name);
    }
  // Variable
  else if (type == "O")
    {
      assert(section != "");
      assert(size != "");
      assert(name != "");

      // Converting the string to Unsigned Long
      t_address address = strtoul(value.c_str(), NULL, 16);
      assert(address != 0 && address != ULONG_MAX);

      // Converting the string to Integer
      int size_var = (int)strtol(size.c_str(), NULL, 16);
      assert(size_var != 0 && size_var != INT_MAX);

      // Searching the section of the variable in the vector of ObjdumpSymbolTable
      vector < ObjdumpSection >::const_iterator it;

      for (it = table.sections.begin(); it != table.sections.end(); it++)
	{
	  if ((*it).name == section)
	    {
	      break;
	    }
	}

      // Checking if the section of the variable is a section we extracted
      assert(it != table.sections.end());

      // Checking if the address of the variable is really in the section
      unsigned long end_of_section = (*it).addr + (unsigned long)(*it).size;
      assert(address >= (*it).addr && address < end_of_section);

      ObjdumpVariable var;
      var.name = name;
      var.addr = address;
      var.size = size_var;	// in bytes
      var.section_name = section;

      // Adding the variable in the vector
      table.variables.push_back(var);
    }
}

ObjdumpWord ARM::readWordInstruction(const ObjdumpInstruction & instr, ObjdumpSymbolTable & table)
{
  /*
     We assume the instruction is in the format :

     addr       binary          mnemonic        operand
     80dc:      000102e8        .word           0x000102e8
   */

  ObjdumpWord result;

  // Setting the address (position in .text)
  result.addr = instr.addr;

  // Converting the string corresponding to the value
  unsigned long val = strtoul(instr.operands[0].c_str(), NULL, 0);
  assert(val != ULONG_MAX);

  // Setting the value
  result.value = val;

  // Setting the default type
  result.type = "imm";

  // Checking all the sections to find the type of the .word (if it exists)
  for (vector < ObjdumpSection >::const_iterator it = table.sections.begin(); it != table.sections.end(); it++)
    {
      unsigned long end_of_section = (*it).addr + (unsigned long)(*it).size;

      // the value corresponds to an adress in the current section
      if (val >= (*it).addr && val < end_of_section)
	{
	  /* oct 2017 lbesnard
	     if ((*it).name == ".text")
	    {
	      Logger::addFatal("Error: .word corresponding to a .text address found (" + instr.operands[0] + ")");
	    }
	    else */
	    {
	      result.type = (*it).name;
	      break;
	    }
	}
    }

  return result;
}

bool ARM::isPcInInputResources(const ObjdumpInstruction & instr)
{
  vector < string > inputs = getInstructionTypeFromMnemonic(instr.mnemonic)->getResourceInputs(instr.operands);
  return (find(inputs.begin(), inputs.end(), "pc") != inputs.end());
}

bool ARM::isPcInOutputResources(const ObjdumpInstruction & instr)
{
  vector < string > outputs = getInstructionTypeFromMnemonic(instr.mnemonic)->getResourceOutputs(instr.operands);
  if (find(outputs.begin(), outputs.end(), "pc") == outputs.end()) return false;
  // Logger::addFatal("Error: PC register found in Output Resources (instruction : " + instr.asm_code + ")");
  return true;
}

vector < ObjdumpWord > ARM::getWordsFromInstr(const ObjdumpInstruction & instr1, const ObjdumpInstruction & instr2, 
					      vector < ObjdumpWord > words, bool & is_instr2_consumed)
{
  assert(words.size() > 0);

  vector < ObjdumpWord > result;

  // First case : Load Instruction
  // example : ldr      r3, [pc, #116]  ; 81ec <RandomInteger+0x84>
  if (getInstructionTypeFromMnemonic(instr1.mnemonic)->isLoad() &&  ! getInstructionTypeFromMnemonic(instr1.mnemonic)->isPredicated())
    {
      // We assume that the extra field of instr1 is in the format :  @.word  FunctionName+0xYY
      // because of the function ARM::removeUselessCharacters(const string &line);

      // Extracting the address of the .word
      istringstream parse(instr1.extra);
      string address_word;

      parse >> address_word;

      assert(Utl::isAddr(address_word));

      // Converting the string in unsigned long
      t_address addr = strtoul(address_word.c_str(), NULL, 16);
      assert(addr != 0 && addr != ULONG_MAX);

      // Checking if there is a .word with this address in the vector
      bool found = false;
      for (vector < ObjdumpWord >::const_iterator it = words.begin(); it != words.end(); it++)
	{
	  // We find the corresponding .word
	  if (it->addr == addr)
	    {
	      found = true;

	      // We add it to the result
	      result.push_back(*it);
	      break;
	    }
	}
      assert(found);

      is_instr2_consumed = false;
    }

  // Second case : Basic Instruction (add)
  // example : add r3, pc, #220    ; 0xdc
  // ldm   r3, {r2, r3}
  else if (getInstructionTypeFromMnemonic(instr1.mnemonic)->isBasic() && getInstructionTypeFromMnemonic(instr1.mnemonic)->isPredicated() == false)
    {
      assert(instr1.mnemonic == "add");

      // We assume the extra field of instr1 is in the format :  0xYY
      // It corresponds to the value added to PC

      istringstream parse(instr1.extra);
      string extra;

      // We parse it because the instr1.extra field has a space character at the beginning, so isHexNumber() always returns false
      parse >> extra;

      assert(Utl::isHexNumber(extra));

      // Converting this string in Unsigned Long
      t_address added_value = strtoul(extra.c_str(), NULL, 0);
      assert(added_value != 0 && added_value != ULONG_MAX);

      // Calculating the address of the first .word
      t_address address_word = instr1.addr + 8 + added_value;

      assert(isLoadMultiple(instr2.asm_code));
      assert(instr2.mnemonic == "ldm");	// To ensure the load multiple is incremental and not decremental

      int nb_of_load = getNumberOfLoads(instr2.asm_code);
      vector < ObjdumpWord >::const_iterator it;

      for (int i = 0; i < nb_of_load; i++)
	{
	  // If it's the first .word we are searching for
	  if (i == 0)
	    {
	      // We search in the vector
	      for (it = words.begin(); it != words.end(); it++)
		{
		  // We found the .word corresponding
		  if (it->addr == address_word)
		    {
		      assert(it->type == "imm");
		      result.push_back(*it);
		      break;
		    }
		}

	      assert(it != words.end());
	    }

	  else
	    {
	      // Moving to the next .word in the vector
	      it++;
	      assert(it != words.end());

	      assert(it->addr == address_word);
	      assert(it->type == "imm");

	      result.push_back(*it);
	    }

	  // Incrementing the address for the next .word
	  address_word += 4;
	}

      // We indicate instr2 has been consumed
      is_instr2_consumed = true;
    }
  else
    {
      Logger::addFatal("Error: Type of Instruction not recognized in ARM::getWordsFromInstr");
    }

  assert(result.size() > 0);

  return result;
}

bool ARM::isInputWrittenRegister(const string & operand)
{
  // Checking if the last character of the operand is '!'
  if (operand[operand.size() - 1] != register_writting_character[0])
    {
      return false;
    }

  string tmp_operand(operand);

  // Removing '!' to have only the name of the register
  tmp_operand.resize(operand.size() - 1);

  // Checking if the register is valid
  if (isRegisterName(tmp_operand) == false)
    {
      return false;
    }

  return true;
}

string ARM::extractRegisterFromInputWrittenOperand(const string & operand)
{
  // Checking if the operand is really correct
  assert(isInputWrittenRegister(operand));

  string tmp_operand(operand);

  // Removing '!' to have only the name of the register
  tmp_operand.resize(operand.size() - 1);

  return tmp_operand;
}

vector < string > ARM::splitRegisterListIntoVector(const string & operand)
{
  vector < string > result;
  string current_register("");

  for (unsigned int i = 0; i < operand.size(); i++)
    {

      // If the current character is '{' (which means the beginning of a register list)
      if (operand[i] == list_begin_character[0])
	{
	  // We add the character '{'
	  result.push_back(list_begin_character);
	  continue;
	}

      // If the current character is '}' (which means the end of a register list)
      if (operand[i] == list_end_character[0])
	{
	  // We add the previous register we parsed
	  if (current_register != "")
	    {
	      result.push_back(current_register);
	    }

	  current_register = "";

	  // We add the character '}'
	  result.push_back(list_end_character);
	  continue;
	}

      // We skip 'space' characters
      if (operand[i] == ' ')
	{
	  continue;
	}

      // If the current character is a coma, this means we parsed ONE register
      if (operand[i] == ',')
	{
	  // We add the register we parsed in the vector
	  if (current_register != "")
	    {
	      result.push_back(current_register);
	    }

	  current_register = "";

	  continue;
	}

      // If the current character is '-' (which means a register range)
      if (operand[i] == register_range_character[0])
	{
	  // We add the previous register we parsed
	  if (current_register != "")
	    {
	      result.push_back(current_register);
	    }

	  // We add the register range character
	  result.push_back(register_range_character);

	  current_register = "";

	  continue;
	}

      // We add the current character to build the register we are parsing
      current_register += operand[i];
    }

  return result;
}

/* Returns true if 'operand' is in the format like --> "{r1, r2, r3}"  or  "{r1, r3-r6, r8}" */
bool ARM::isRegisterList(const string & operand)
{
  bool result = true;

  // Splitting the string into a vector
  vector < string > operands_splitted = splitRegisterListIntoVector(operand);

  if (operands_splitted.empty())
    {
      return false;
    }

  // Checking if the first element is '{'  (which means the beginning of a register list)
  if (operands_splitted.front() != list_begin_character)
    {
      return false;
    }

  // Checking if the last element is '}'   (which means the end of a register list)
  if (operands_splitted.back() != list_end_character)
    {
      return false;
    }

  // Parsing all the elements (excepted the first and the last)
  for (unsigned int i = 1; i < (operands_splitted.size() - 1); i++)
    {
      // Checking if the current element is a Register
      if (isRegisterName(operands_splitted[i]) == false)
	{
	  // If it's not, we check if it is the register range character "-"
	  if (operands_splitted[i] != register_range_character)
	    {
	      // If it's not, it is not a valid element, we return false
	      result = false;
	      break;
	    }
	}
    }

  return result;
}

vector < string > ARM::extractRegistersFromRegisterList(const string & operand)
{
  // Checking if the operand is really correct
  assert(isRegisterList(operand));

  vector < string > result;
  // We split the string into a vector
  vector < string > operands_splitted = splitRegisterListIntoVector(operand);

  // Parsing all the elements (excepted the first ("{") and the last ("}") )
  for (unsigned int i = 1; i < (operands_splitted.size() - 1); i++)
    {

      // The current element is the register range character "-"
      if (operands_splitted[i] == register_range_character)
	{
	  int number_first_register = getRegisterNumber(operands_splitted[i - 1]);
	  int number_last_register = getRegisterNumber(operands_splitted[i + 1]);

	  // Just a check to be sure this is not an empty list
	  assert(number_first_register != number_last_register);

	  string prefixe("");

	  /* Extracting the prefix of registers' name
	     example : r1 --> the result will be "r", blabla12 --> the result will be "blabla" */
	  int j = 0;
	  while (operands_splitted[i - 1][j] < '0' || operands_splitted[i - 1][j] > '9')
	    {
	      prefixe += operands_splitted[i - 1][j];
	      j++;
	    }

	  // If the first the register is lower than the last of the range (increasing range)
	  if (number_first_register <= number_last_register)
	    {
	      for (j = number_first_register + 1; j < number_last_register; j++)
		{
		  stringstream ss;
		  ss << j;
		  // We build the name of the register with the prefix + the current number j
		  string current_reg = prefixe + ss.str();
		  result.push_back(current_reg);
		}
	    }

	  // The first the register is greater than the last of the range (decreasing range)
	  else
	    {
	      for (j = number_first_register - 1; j > number_last_register; j--)
		{
		  stringstream ss;
		  ss << j;
		  // We build the name of the register with the prefix + the current number j
		  string current_reg = prefixe + ss.str();
		  result.push_back(current_reg);
		}
	    }

	}
      // The current element is a register
      else
	{
	  result.push_back(operands_splitted[i]);
	}
    }

  return result;
}

/* Checking if operand is a shifter is "lsl", "lsr", "asr", "ror" or "rrx" 
 */
bool ARM::isShifterOperand(const string & operand)
{
  string shifter(""), value(""), extra_operand("");

  istringstream parse(operand);
  parse >> shifter >> value >> extra_operand;

  assert(shifter != "");

  // Checking if the shifter is "lsl", "lsr", "asr" or "ror"
  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) != shifter_with_operand.end())
    {
      // there is a value attached which can be an immediate value or a register (example : lsl r5 | ror #2 )
      if (value == "")
	return false;		// possible ?
      if (Utl::isDecNumber(value) || isRegisterName(value))
	{
	  return (extra_operand == "");
	};			// true if no extra operand
      return false;
    }
  // Checking if the shifter is "rrx"
  if (find(shifter_without_operand.begin(), shifter_without_operand.end(), shifter) != shifter_without_operand.end())
    {
      return (value == "");
    }				// true if no extra operand
  return false;
}

// @return the register of a shifter "lsl", "lsr", "asr" or "ror", or the empty string.
string ARM::extractRegisterFromShifterOperand(const string & operand)
{
  // Checking if operand is in a valid format
  assert(isShifterOperand(operand));

  string result(""), shifter(""), value("");
  istringstream parse(operand);
  parse >> shifter >> value;

  if (find(shifter_with_operand.begin(), shifter_with_operand.end(), shifter) != shifter_with_operand.end())
    {
      if (isRegisterName(value)) { result = value;}
    }
  return result;
}


bool ARM::getMultipleLoadStoreARMInfos(string & instr, string & codeinstr, string & oregister, vector < string > &regList, bool *writeBack)
{
  vector < string > registers_of_list, split_instruction;

  split_instruction = Arch::splitInstruction(instr);
  codeinstr = split_instruction[0];
  if (! isARMClassInstr(codeinstr, "stm") &&  ! isARMClassInstr(codeinstr, "ldm")) return false;
  if (codeinstr.compare("stm") == 0)
    codeinstr="stmia";
  else if (codeinstr.compare("ldm") == 0)
    codeinstr="ldmia";

  oregister = split_instruction[1];
  (*writeBack) = (oregister.find("!") != EOS);
  if (*writeBack) Utl::replace(oregister,'!', ' ');
  oregister = oregister.substr (0, oregister.find(" "));
  
  // cout << " codeintr = " << codeinstr << endl;  
  // cout << " reg = " << oregister << endl;  
  // cout << " others= " << split_instruction[2] << endl;
  Utl::replace(split_instruction[2], '{', ' ');
  Utl::replace(split_instruction[2], '}', ' ');

  vector < string > splitted_operands = splitOperands(split_instruction[2]);
  for (vector < string >::const_iterator it = splitted_operands.begin(); it != splitted_operands.end(); it++)
    {
      if (isRegisterList(*it))
	{
	  // Getting the registers in the list
	  registers_of_list = extractRegistersFromRegisterList(*it);
	  for (vector < string >::const_iterator itaux = registers_of_list.begin(); itaux != registers_of_list.end(); itaux++)
	    {
	      // regList.push_back(*itaux);
	      regList.push_back((*itaux).substr (0, (*itaux).find(" ")));
	    }
	}
      else
	{
	  // regList.push_back(*it);
	  regList.push_back((*it).substr (0, (*it).find(" ")));
	}
    }
  return true;
}

bool ARM::isPcLoadInstruction(string & instr)
{
  if (!isLoadStoreSimple(instr)) return false;
  vector < string > split_instruction = Arch::splitInstruction(instr);
  return (split_instruction[1] == "pc");
}

/**
   Decomposition of an LOAD/STORE instruction(instr), but not pop, push. 
   @return false for "pop" and "push" instructions, true otherwise, in this case:
   @strongContex used for debugging. When it is true, the instruction must be a load/sotre excluding pop, push.
   @param vaddrmode : the addressing mode of the instruction (pre, auto or post addressing)
   @param TypeOperand: is the offset type value, element of { none_offset, immediate_offset, register_offset, scaled_register_offset}
   @param operand1, operand2, operand3 are set according to the offset type value. Operand1 is a register, operand2 is an immediate value or a register, operand3 is the shifter instruction.
*/
bool ARM::getLoadStoreARMInfos(bool strongContext, string & instr, string & codeinstr, string & oregister, AddressingMode * vaddrmode,
			       offsetType * TypeOperand, string & operand1, string & operand2, string & operand3)
{
  string aux;
  vector < string > split_instruction = Arch::splitInstruction(instr);

  codeinstr = split_instruction[0];
  if (codeinstr.find("push") != EOS)
    {
      if (strongContext)
	TRACE(cout << "Calling getLoadStoreARMInfos, PUSH found !!!" << endl);
      return false;
    }
  if (codeinstr.find("pop") != EOS)
    {
      if (strongContext)
	TRACE(cout << "Calling getLoadStoreARMInfos, POP found !!!" << endl);
      return false;
    }
  if (!isLoadStoreSimple(instr))
    {
      if (strongContext)
	TRACE(cout << "Calling getLoadStoreARMInfos, but [  ] NOT FOUND !!!" << endl);
      return false;
    }

  oregister = split_instruction[1];

  operand1 = "";
  operand2 = "";
  operand3 = "";

  int nbelem = split_instruction.size();
  // splited in 3, 4, 5 parts. 4, 5 parts for post_indexed operation.
  bool b = (nbelem == 3);
  if (b)
    *vaddrmode = (split_instruction[2].find("!") == EOS ? pre_indexing : auto_indexing);
  else
    *vaddrmode = post_indexing;

  if (b)
    {
      aux = split_instruction[2];
      int n = Utl::count(aux, ',');
      int posco = aux.find("[");
      int poscf = aux.find("]");
      if (n == 0)
	{
	  *TypeOperand = none_offset;
	  operand1 = aux.substr(posco + 1, poscf - 1);
	}
      else
	{
	  int posv = aux.find(",");
	  if (aux[posv + 1] == ' ')
	    posv++;
	  operand1 = aux.substr(posco + 1, posv - posco - 2);
	  if (n == 1)
	    {			// [Rn, #value] or [ Rn, Rm] but in the xml file # has been removed.
	      operand2 = aux.substr(posv + 1, poscf - posv - 1);
	      // size_t pos = operand2.find ("#");
	      *TypeOperand = (Utl::isDecNumber(operand2) ? immediate_offset : register_offset);
	    }
	  else			// [Rn, Rm, shifter_op]
	    {
	      int posv2 = aux.find_last_of(",");
	      if (aux[posv2 + 1] == ' ')
		posv2++;
	      *TypeOperand = scaled_register_offset;
	      operand2 = aux.substr(posv + 1, posv2 - posv - 2);
	      operand3 = aux.substr(posv2 + 1, poscf - posv2 - 1);
	    }
	}
    }
  else // post-indexed address.
    {
      // [Rn], #value | [Rn], Rm | [Rn], Rm, shifter_op
      aux = split_instruction[2];
      operand1 = aux.substr(aux.find("[") + 1, aux.find("]") - 1);	// Rn
      operand2 = split_instruction[3];
      if (nbelem == 4)
	{
	  size_t pos = operand2.find("#");
	  *TypeOperand = (pos != EOS ? immediate_offset : register_offset);
	  if (pos != EOS)
	    operand2.erase(pos, 1);
	}
      else // [Rn], Rm, shifter_op
	{
	  *TypeOperand = scaled_register_offset;
	  operand3 = split_instruction[4];
	}
    }
  return true;
}

/**
   Decomposition of an instruction (instr) assumed NOT TO BE A LOAD/STORE INSTRUCTION, with at most 3 operands.
   CODE_OP REG, shift_op
   shift_op ::= #immm8r | REGISTER | SHIFT_ROTATE
   set={mov*, mvn*, and*, orr*, eor*, bic*}
*/
bool ARM::getInstr1ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2)
{
  if (isLoadStoreSimple(instr))
    {
      TRACE(cout << "ARM::getInstr1ARMInfos: instr is a LOAD/STORE" << endl);
      return false;
    }
  vector < string > split_instruction = Arch::splitInstruction(instr);
  int nbelem = (int)split_instruction.size();
  if ( nbelem == 1) 
    { // nop
      *TypeOperand = none_offset;
      return true;
    }
  if (nbelem < 2 || nbelem > 4)
    {
      TRACE(cout << "ARM::getInstr1ARMInfos: bad number of operands !!!" << endl);
      return false;
    }
  codeinstr = split_instruction[0];
  oregister = split_instruction[1];
  if (nbelem == 2)		// rrx Rn
    {
      *TypeOperand = none_offset;
      operand1 = "";
      operand2 = "";
      return true;
    }

  operand1 = split_instruction[2];
  if (nbelem == 3)
    {
      *TypeOperand = (Utl::isDecNumber(operand1) ? immediate_offset : register_offset);	// mov Rn, Rm ; move R0, #3
      operand2 = "";
    }
  else
    {
      operand2 = split_instruction[3];
      *TypeOperand = scaled_register_offset;
    }
  return true;
}

/**
   Decomposition of an instruction (instr) assumed not to be a load/store instruction, with at most 4 operands.
   CODE_OP REG, REG, shift_op
   shift_op ::= #immm8r | REGISTER | SHIFT_ROTATE
   set={add*, adc*, sub*, sbc*, rsb*, rsc*, mul*}
*/
bool ARM::getInstr2ARMInfos(string & instr, string & codeinstr, string & oregister, offsetType * TypeOperand, string & operand1, string & operand2, string & operand3)
{
  operand1 = "";
  operand2 = "";
  operand3 = ""; 
  if (isLoadStoreSimple(instr))
    {
      TRACE(cout << "ARM:: getInstr2ARMInfos: instr is a LOAD/STORE" << endl);
      return false;
    }
  vector < string > split_instruction = Arch::splitInstruction(instr);
  int nbelem = (int)split_instruction.size();
  if ( nbelem == 1) 
    { // nop
      *TypeOperand = none_offset;
      return true;
    }
  
  if (nbelem < 2 || nbelem > 5)
    {
      TRACE(cout << "ARM::getInstr2ARMInfos: bad number of operands!!!" << endl);
      return false;
    }
  codeinstr = split_instruction[0];
  oregister = split_instruction[1];
  if (nbelem == 2)		// rrx Rn
    {
      *TypeOperand = none_offset;
      return true;
    }

  operand1 = split_instruction[2];	// a register, Rj in <code_op> Ri, Rj, ...
  if (nbelem == 3)
    {
      *TypeOperand = (Utl::isDecNumber(operand1) ? immediate_offset : register_offset);
      return true;
    }
  operand2 = split_instruction[3];
  if (nbelem == 4)
    {
      // add Rn, Rm, #1 ; add Rn, Rm, Rt
      *TypeOperand = (Utl::isDecNumber(operand2) ? immediate_offset : register_offset);
    }
  else
    {
      operand3 = split_instruction[4];	// add Rn, Rm, Rt, SHIT_ROTATE 
      *TypeOperand = scaled_register_offset;
    }
  return true;
}


bool  ARM::isARMClassInstr(const string &codeinstr, const string &prefix)
{
  std::string str = codeinstr.substr (0,prefix.length());
  return (prefix == str);
}



bool  ARM::isConditionnedARMInstr(const string &codeop, const string &prefix)
{
  if (codeop == prefix) return false;
  if (codeop == prefix + "s") return false;
  // USED ???
  if (codeop == prefix + "al") return false;
  return (codeop != prefix + "als");
}


bool  ARM::getRegisterAndIndexStack(const string &instructionAsm, string &reg, int *i)
{
  return false;
}
