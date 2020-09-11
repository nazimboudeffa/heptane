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

/** *****************************************************

    Main file of the extractor
 
    Only MIPS and ARM architectures are currently supported
 
   ********************************************************/

/*
  TODO:
  - les nodes call si besoin (2 instructions) -> pas pour le moment voir si utile, inlining ?
  --> A faire pour check !!!
  - macro ?
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <stdlib.h>

#include "ConfigExtract.h"
#include "dominatorAnalysis.h"
#include "loopAnalysis.h"
#include "switchAnalysis.h"
#include "Annotations.h"
#include "GlobalAttributes.h"
#include "Utl.h"

using namespace std;
static bool isARMArchi= false, isMIPSArchi = false;
static bool opt_verbose = false;

/**
   Exporting a program (cfg form) in xml form.
*/
static void 
exportCfg(cfglib::Program & cfglib_program)
{
  std::ofstream output_file_stream;
  string exportXMLfilename = cfglib_program.GetName () + ".xml";
  output_file_stream.open (exportXMLfilename.c_str ());
  std::ostringstream serialised_form;
  cfglib_program.serialise_program (serialised_form);
  output_file_stream << serialised_form.str ();
  output_file_stream.close ();
}

static void cleanCfg(Cfg * vcfg)
{
  // Added LBesnard : cleaning the isolated nop nodes for ARM: it may induce 2 entry points in a loop...
  /* 
     86e8:	ea00008f 	b	892c <fft1+0x474>
     86ec:	e1a00000 	nop			; (mov r0, r0)
     86f0:	a0b5ed8d 	.word	0xa0b5ed8d
     86f4:	3eb0c6f7 	.word	0x3eb0c6f7
     ...
     8710:	00018d00 	.word	0x00018d00
     8714:	e59d2088 	ldr	r2, [sp, #136]	; 0x88
     8718:	e59d308c 	ldr	r3, [sp, #140]	; 0x8c
  */
  if (isARMArchi)
    {
      vector<Node*> Nodes = vcfg->GetAllNodes();
      size_t  lg = Nodes.size();
      for (size_t i = 1; i < lg; i++)
	{
	  Node * n =  Nodes[i];
	  if ( n->isIsolatedNopNodeLB())
	    {
	      std::vector<Edge*> edges = vcfg->GetOutgoingEdges(n);
	      for (std::vector<Edge*>::iterator it = edges.begin() ; it != edges.end() ; it++)
		vcfg-> RemoveEdge(*it);
	    }     
	}
    }
}

/** 
    Create a new cfg from the information obtained by the parser
    
    - cfglib_program: the program itself (created by the parser)
    - bb_start_addr: the basic block start address (except the first basic block, which is surprizingly not provided by the parser
    - instructions: the list of instructions by increasing address
    - function: information on the function whose cfg is under construction
    - succs: successors. in case of a jump/call, this is known at the instructions level, not at the BB level, 
             explaning why the cfg is constructed by scnanning all the cfg instructions
    - instrWithWords: (ARM SPECIFIC) mapping between instruction (t_address)
    that use .word and the corresponding words
    
*/
static void
build_heptane_cfg (cfglib::Program & cfglib_program, const set < t_address > &bb_start_addr, const vector < ObjdumpInstruction > &instructions,
		   const ObjdumpFunction & function, const map < t_address, set < t_address > >&succs, const map < t_address,
		   vector < ObjdumpWord > >&instrWithWords, map < int, ObjdumpInstruction > &MetaInstructionsTable )
{

  // Association of Node* to addresses to properly create the list of successors
  map < t_address, cfglib::Node * >address_to_node;
  map<int, ObjdumpInstruction>::iterator iter;

  // Create the cfg and add it to the program
  cfglib::Cfg * cfgcur = cfglib_program.GetCfgByName (function.name);
  if (cfgcur == (cfglib::Cfg *) 0)
    Logger::addFatal ("CFG extractor: cfg does not exist in symbol table " + function.name);

  // Populate the cfg
  // ----------------

  // First create the basic blocks
  // While creating the basic blocks, fill a map (cfglibnode,address)
  cfglib::Node * current_node = NULL;
  t_address current_addr, prev_addr = -1;  // added for kernel ARM (lbesnard)
  ObjdumpInstruction current_instruction;
  for (size_t i = 0; i < instructions.size (); i++)
    {
      current_instruction = instructions[i];
      current_addr = current_instruction.addr;
      if (prev_addr != current_addr)
	{
	  bool mkNewBB = (bb_start_addr.find (current_addr) != bb_start_addr.end ());
	  if ( mkNewBB || i == 0)
	    {
	      // Create the basic block itself
	      current_node = cfgcur->CreateNewNode (cfglib::BB);
	      address_to_node[current_instruction.addr] = current_node;
	      prev_addr = current_addr;
	    }
	}
      prev_addr = current_addr;
      // Insert the instruction in the basic block
      // The instruction text is re-constructed from the (mnemonic and operands) asm_code.
      string text_instruction = current_instruction.asm_code;
      cfglib::Instruction * inst = current_node->CreateNewInstruction (text_instruction, cfglib::Code, Arch::isReturn (current_instruction));
      // Add an address attribute to the instruction
      // FIXME: remove these hard-coded strings
      AddressAttribute attribute;
      AddressInfo info;
      info.setType ("read");
      info.setSegment ("code");
      info.setPrecision(true);
      ostringstream ossCodeAdr, ossSize;
      ossCodeAdr << current_instruction.addr;
      ossSize << Arch::getInstructionSize ();
      info.addAdrSize (ossCodeAdr.str (), ossSize.str ());
      ossCodeAdr.str ("");
      ossSize.str ("");
      attribute.addInfo (info);
      inst->SetAttribute (AddressAttributeNameExtract, attribute);


      iter = MetaInstructionsTable.find((int)i);
      if (iter != MetaInstructionsTable.end())
	{
	  MetaInstructionAttribute vattr;
	  vattr.setObjdumpInstr((iter->second).asm_code);
	  inst->SetAttribute (MetaInstructionAttributeName, vattr);
	}
      
      if (Arch::isCall (current_instruction))
	{
	  // Set the node type to call, with the called function as parameter
	  if (Arch::getCalleeName (current_instruction) == "")
	    Logger::addFatal ("CFG extractor: name of called empty returned by getCalleeName \n\t(probably an indirect call or a switch), instruction: "
			      + current_instruction.asm_code);
	  current_node->SetCall (Arch::getCalleeName (current_instruction), true);
	}

      if (isARMArchi) // ARM SPECIFIC: attach .word information
	{
	  map < t_address, vector < ObjdumpWord > >::const_iterator it = instrWithWords.find (current_instruction.addr);
	  if (it != instrWithWords.end ())	// the current instruction uses .word information
	    {
	      ARMWordsAttribute word_attr;

	      vector < ObjdumpWord > words = it->second;
	      for (size_t w = 0; w < words.size (); w++)
		word_attr.addWord (words[w].addr, words[w].type, words[w].value);
	      inst->SetAttribute (ARMWordsAttributeName, word_attr);
	    }
	}
    }

  // Create the successors, for all basic blocks create a node between the BB and its successor
  // All nodes already have been created by the loop before, just have to create the edges
  // As succs contain as a source of branch instruction addresses and not BB addresses, instructions have to be
  // scanned
  t_address curbb_start_addr = 0;	// Start address of current basic block
  bool previous_block_has_jump = false;
  prev_addr = -1; // added for kernel ARM (lbesnard)
  for (size_t i = 0; i < instructions.size (); i++)
    {
      current_instruction = instructions[i];
      current_addr = current_instruction.addr;
      if ( current_addr != prev_addr)
	{ 
	  bool mkNewBB = (bb_start_addr.find (current_addr) != bb_start_addr.end ());
	  if ( mkNewBB || i == 0)	
	    {
	      if (!previous_block_has_jump && curbb_start_addr != 0)
		{
		  cfglib::Node * ncur = address_to_node[curbb_start_addr];
		  cfglib::Node * nsuc = address_to_node[current_addr];
		  cfgcur->CreateNewEdge (ncur, nsuc);
		}
	      
	      curbb_start_addr = current_addr;
	      previous_block_has_jump = false;
	    }
	  if (succs.find (current_addr) != succs.end ())
	    {
	      set < t_address > current_successors = succs.find (current_addr)->second;
	      for (set < t_address >::const_iterator it = current_successors.begin (); it != current_successors.end (); it++)
		{
		  cfglib::Node * ncur = address_to_node[curbb_start_addr];
		  cfglib::Node * nsuc = address_to_node[*it];
		  cfgcur->CreateNewEdge (ncur, nsuc);
		}
	    }
	  previous_block_has_jump = previous_block_has_jump || Arch::isCall (current_instruction) || Arch::isReturn (current_instruction)
	    || Arch::isUnconditionalJump (current_instruction) || Arch::isConditionalJump (current_instruction);
	}
      prev_addr = current_addr;
    }

  cleanCfg(cfgcur); 
}

/**
   - Make sure functions are correctly linked for call nodes,
   - Set the program entry entry point,
   - Create loops,
   - Manage annotations.
 */
static void
finalize_program_construction (const ConfigExtract & config, cfglib::Program & cfglib_program)
{
  // Make sure functions are correctly linked for call nodes
  vector < cfglib::Cfg * >lcfg = cfglib_program.GetAllCfgs ();

  // Set the program entry entry point
  cfglib::Cfg * entry_cfg = cfglib_program.GetCfgByName (config.entry_point_name);
  if (entry_cfg == NULL) Logger::addFatal ("CFG extractor: can't set entry point to undefined Cfg " + config.entry_point_name);
  cfglib_program.SetEntryPoint (entry_cfg);

  vector < t_address> lswitches = getSwitchInfos( config.tmp_dir + "/" + config.program_name + ".switch" );
  bool bswitch = lswitches.size() > 0;

  // Create loops
  for (unsigned int c = 0; c < lcfg.size (); c++)
    {
      Cfg * acfg =lcfg[c];
      vector < cfglib::Node * >vn = acfg->GetAllNodes ();
      if (vn.size () != 0)
	{
	  if (bswitch) SwitchComputer::computeSwitch( acfg, lswitches, isMIPSArchi);
	  cfglib::helper::DominatorComputer::computeDominator (acfg);
	  cfglib::LoopComputer::computeLoop (acfg);
	}
    }

  // Manage annotations
  // If there is an annotation XML file, use it first
  if (config.binary_only) AttachAnnotationsFromXML (cfglib_program, config.annotation_file, opt_verbose);
  // Get the other annotations from the binary file
  AttachAnnotationFromBinary (cfglib_program, config.tmp_dir + "/" + config.program_name + ".annot", opt_verbose);
  // Output the final annotation file if required
  if (config.output_annot) GenerateAnnotationXMLFile (cfglib_program, config.result_dir + "/" + config.annotation_file, opt_verbose);
}


/** 
    It rewrites the multiple store/load (pop, push, ldm, stm) using simple load/store instructions.
    @return the number of instructions added in the vector "instructions".
*/
static int kernel(vector < ObjdumpInstruction > &instructions, ObjdumpInstruction &instr,  map< int, ObjdumpInstruction > &MetaInstructionsTable)
{
  string bsens, vcodeInstr, asm_code, codeinstr, oregister, asm_code_ori;
  vector < string > regList;
  bool isAfter, bIncr, writeBack;
  ObjdumpInstruction instr_copy; 
  int v, nbadded=0;
  size_t vindex;

  if (! isARMArchi)
    {
      instructions.push_back (instr);
      return 1;
    }
  asm_code_ori = asm_code;
  // pop reglist  <=> ldmia sp!, reglist
  // push reglist  <=> stmdb sp!, reglist
  vindex = instr.asm_code.find("push");
  if (vindex != EOS)
    {
      asm_code = "stmdb sp!," + instr.asm_code.substr (vindex+5);
    }
  else
    {
      vindex = instr.asm_code.find("pop");
      if (vindex != EOS)
      {
	asm_code = "ldmia sp!," + instr.asm_code.substr (vindex+4);
      }
    else
      {
	asm_code = instr.asm_code;
      }
    }
  if (Arch::getMultipleLoadStoreARMInfos(asm_code, codeinstr, oregister, regList, &writeBack))
    {
      // <OP>IA; <OP>IB; <OP>DA; <OP>DB; OP in {STM, LDM}.
      //      with IA: Increment After, IB : Increment Before, DA: Decrement After, DB : Decrement Before
      // cout << " Multiple LoadStore  detected, codeinstr = " << codeinstr << endl;
      vcodeInstr = (Arch::isARMClassInstr(codeinstr, "ldm" ) ?  "ldr " : "str ");
      bIncr = (codeinstr.find("i")!= EOS);
      bsens = (bIncr ? "" : "-");
      isAfter = (codeinstr.find("a")!= EOS);
      v=0;  
      for (size_t i = 0; i < regList.size(); i++)
	{
	  instr_copy=instr;
	  if (!isAfter) v=v+4;
	  instr_copy.asm_code= vcodeInstr + regList[i] + ", [" + oregister + ( (v==0) ? "" : ", " + bsens + int2string(v)) + "]";
	  if (isAfter) v=v+4;
	  instr_copy.mnemonic = vcodeInstr.substr(0, vcodeInstr.find(" "));
	  instructions.push_back (instr_copy);
	  MetaInstructionsTable[instructions.size()-1] = instr;
	  nbadded++;
	}
      if (writeBack && (v!=0))
	{
	  instr_copy=instr;
	  vcodeInstr = (bIncr ? "add" : "sub");
	  instr_copy.asm_code= vcodeInstr + " " + oregister + ", " + oregister + ", " + int2string(v);
	  instructions.push_back (instr_copy);
	  MetaInstructionsTable[instructions.size()-1] = instr;
	  nbadded++;
	}
    }
  else
    {
      instructions.push_back (instr);
      nbadded++;
    }
  return nbadded;
}

/** Parser: parse the objdump file and generate the CFG and the readelf file
    - First step: parsing of the readelf file  
    - Second step: parsing of the objdump file: parsing of the symbol table and if arch == ARM then search for .word in the .text and store them in wordsPerFunction
    - Third step: parsing of the objdump file (ARM specific: Detection of instructions using .word and store them in instrWithWords)
    
    - Program & Cfgs creation
         - Last step: parsing of the objdump file again: parsing the disassembly of section .text,  ...
	 - Finalize program construction ( see finalize_program_construction()),
    - Export the program (xml file) (see exportCfg()).
*/
static void
BuildCfg (const ConfigExtract & config)
{
  string objdumpname = config.program_name + ".objdump";
  ifstream input (objdumpname.c_str (), ios::in);
  if (!input.is_open ()) Logger::addFatal ("Error: file " + objdumpname + " not present");
  string readelfname = config.program_name + ".readelf";
  ifstream input_readelf (readelfname.c_str (), ios::in);
  if (!input_readelf.is_open ()) Logger::addFatal ("Error: file " + readelfname + " not present");

  // Variables for function parsing
  vector < ObjdumpInstruction > instructions;
  map < int, ObjdumpInstruction > MetaInstructionsTable;
  ObjdumpFunction function;
  set < t_address > bb_start_addr;
  map < t_address, set < t_address > >succs;
  ObjdumpSymbolTable symbol_table;

  isARMArchi = Arch::getArchitectureName () == "ARM";
  isMIPSArchi = Arch::getArchitectureName () == "MIPS";

  map < string, vector < ObjdumpWord > >wordsPerFunction;	// ARM SPECIFIC: This map associates each ObjdumpWord to a function, the string index is for functions' name
  map < t_address, vector < ObjdumpWord > >instrWithWords;	// ARM SPECIFIC: address of the instruction which needs the .word

  string line = "";

  /**********************************************************/
  /******     First step: parsing of the readelf file  ******/
  /**********************************************************/

  // go to the Section Headers:
  while (!input_readelf.eof () && ! Arch::isReadelfSectionsMarker (line))
    getline (input_readelf, line);

  // Skipping the first line of the List (This line :   [Nr]  Name  Type  Addr  Off  Size  ES  Flg  Lk  Inf  Al)
  getline (input_readelf, line);

  while (!input_readelf.eof ())
    {
      getline (input_readelf, line);
      if (Arch::isReadelfFlagsMarker (line)) break; // If the line is "Key to Flags:"
      Arch::parseReadElfLine (line, symbol_table);
    }
  input_readelf.close ();
  assert (symbol_table.sections.empty () == false);
  line = "";

  /**********************************************************/
  /******     Second step: parsing of the objdump file  *****/
  /**********************************************************/
  //  - parsing of the symbol table
  //  - if arch == ARM    then search for .word in the .text and store them in wordsPerFunction

  // go to the SYMBOL TABLE:
  while (!input.eof () && ! Arch::isObjdumpSymbolTableMarker (line))
    getline (input, line);

  // parsing the symbol table until the Disassembly of section .text:
  while (!input.eof () && ! Arch::isObjdumpTextMarker (line) )
    {
      getline (input, line);
      if (line.length () == 0 || Arch::isObjdumpTextMarker (line)) continue;
      Arch::parseSymbolTableLine (line, symbol_table);
    }

  // ARM specific
  if (isARMArchi)
    {
      // parsing the code (searching for .word)
      while (!input.eof ())
	{
	  getline (input, line);
	  if (line.length () == 0) continue; 	// skip empty lines

	  if (Arch::isFunction (line))
	    function = Arch::parseFunction (line); // function 
	  else if (Arch::isInstruction (line))	// instruction
	    {
	      ObjdumpInstruction instr = Arch::parseInstruction (line);
	      if (Arch::isWord (instr))
		{
		  ObjdumpWord data = Arch::readWordInstruction (instr, symbol_table);
		  wordsPerFunction[function.name].push_back (data);
		}
	    }
	}
    }
  input.close ();
  line = "";

  /**********************************************************/
  /******     Third step: parsing of the objdump file  ******/
  /**********************************************************/
  // ARM specific: Detection of instructions using .word and store them in instrWithWords
  if (isARMArchi && ! wordsPerFunction.empty ())
    {
      input.open (objdumpname.c_str (), ios::in);
      string line2;
      bool line2_has_been_consumed = true;

      // go to the Disassembly of section .text:
      while (!input.eof () &&  ! Arch::isObjdumpTextMarker (line)) 
	getline (input, line);

      // Parse it
      while (!input.eof ())
	{
	  if (line2_has_been_consumed)
	    {
	      getline (input, line);
	      if (line.length () == 0) continue; // skip empty lines
	    }
	  else
	    {
	      line = line2;
	      line2_has_been_consumed = true;
	    }
	  if (Arch::isFunction (line))	// function
	    function = Arch::parseFunction (line);
	  else if (Arch::isInstruction (line))	// instruction
	    {
	      ObjdumpInstruction instr = Arch::parseInstruction (line);

	      if (Arch::isPcInInputResources (instr))
		{ 
		  if (Arch::isPcInOutputResources(instr))
		    {
		      // the .word table are not assigned to the instruction.
		      // Indirect jump (switch) via .word table.
		      // 80f0:	e3530077 	cmp	r3, #119	; 0x77
		      // 80f4:	979ff103 	ldrls	pc, [pc, r3, lsl #2]
		      // 80f8:	ea000257 	b	8a5c <swi120+0x984>
		      // 80fc:	000082dc 	.word	0x000082dc
		      // 8100:	000082ec 	.word	0x000082ec
		      //  ...		      
		    }
		  else
		    {
		      // Reading the next line
		      getline (input, line2);
		    
		      // Some checks
		      assert (line2.length () != 0);
		      assert (Arch::isInstruction (line2));
		    
		      ObjdumpInstruction instr2 = Arch::parseInstruction (line2);
		      line2_has_been_consumed = true;
		    
		      // Getting the .word associated to the instruction
		      vector < ObjdumpWord > words_result = Arch::getWordsFromInstr (instr, instr2, wordsPerFunction[function.name], line2_has_been_consumed);
		    
		      // associate the .words to the instruction
		      if (!line2_has_been_consumed)	// example : ldr r3, [pc, #116]  ; 81ec <RandomInteger+0x84>
			{ // directly used by the instruction
			  instrWithWords[instr.addr] = words_result;
			}
		      else
			{
			  // example : add r3, pc, #220    ; 0xdc
			  //           ldm   r3, {r2, r3}
			
			  // the first .word is used by the first instruction
			  // while the nexts .word are used by the second instruction
			  // TODO: to be checked
			  //      To be safe there are added to both instructions
			  instrWithWords[instr.addr] = words_result;
			  instrWithWords[instr2.addr] = words_result;
			}
		    }
		}
	    }
	}
      
      input.close ();
    }
  line = "";

  /**********************************************************/
  /******         Program & Cfgs creation              ******/
  /**********************************************************/
  // Create the program
  cfglib::Program cfglib_program (config.program_name);
  
  // Create all Cfgs from symbol table
  // Rq: the key are the addresse of each function (thus in the order of appearance the .text section)
  typeTableFunctions::const_iterator it;
  for (it = symbol_table.functions.begin (); it != symbol_table.functions.end (); it++)
    cfglib_program.CreateNewCfg (it->second);

  // Create the SymbolTableAttribute
  SymbolTableAttribute ts_attribute;

  // add the sections
  for (size_t i = 0; i < symbol_table.sections.size (); i++)
    ts_attribute.addSection (symbol_table.sections[i].name, symbol_table.sections[i].addr, symbol_table.sections[i].size);

  // add the variables
  for (size_t i = 0; i < symbol_table.variables.size (); i++)
    ts_attribute.addVariable (symbol_table.variables[i].name, symbol_table.variables[i].addr, symbol_table.variables[i].size, symbol_table.variables[i].section_name);

  // specific MIPS: set GP
  if (isMIPSArchi) ts_attribute.setGP (symbol_table.MIPS_gp);
  // attach the attribute
  cfglib_program.SetAttribute (SymbolTableAttributeName, ts_attribute);

  /**********************************************************/
  /******     Last step: parsing of the objdump file   ******/
  /**********************************************************/

  function.name = "";
  
  input.open (objdumpname.c_str (), ios::in);
  // Go to the Disassembly of section .text:
  while (!input.eof () && ! Arch::isObjdumpTextMarker (line)) getline (input, line);

  // Parse it
  while (!input.eof ())
    {
      getline (input, line);
      if (line.length () == 0) continue; // skip empty lines

      if (Arch::isFunction (line))	// function
	{
	  // if it is not the first function
	  if (function.name != "")
	    {
	      // build the previously parsed function
	      build_heptane_cfg (cfglib_program, bb_start_addr, instructions, function, succs, instrWithWords, MetaInstructionsTable);

	      // and clean the variables for the current function
	      instructions.clear ();
	      bb_start_addr.clear ();
	      succs.clear ();
	      MetaInstructionsTable.clear();
	    }

	  function = Arch::parseFunction (line);
	}
      else if (Arch::isInstruction (line)) // instruction
	{
	  ObjdumpInstruction instr = Arch::parseInstruction (line);

	  // .word for ARM are not attached as instruction but as an attribute of an instruction
	  if (isARMArchi && Arch::isWord (instr)) continue;
	 
	  // The ARM multiple store/load (pop, push, ldm, stm) are rewritten using simple load/store instructions.
	  kernel(instructions, instr, MetaInstructionsTable);

	  if (Arch::isCall (instr))
	    {
	      t_address addr_next_bb = instr.addr + Arch::getInstructionSize () + Arch::getNBInstrInDelaySlot () * Arch::getInstructionSize ();
	      bb_start_addr.insert (addr_next_bb);
	      succs[instr.addr].insert (addr_next_bb);
	    }
	  else if (Arch::isReturn (instr))
	    {
	      t_address addr_next_bb = instr.addr + Arch::getInstructionSize () + Arch::getNBInstrInDelaySlot () * Arch::getInstructionSize ();
	      bb_start_addr.insert (addr_next_bb);
	    }
	  else if (Arch::isUnconditionalJump (instr))
	    {
	      t_address addr_next_bb = instr.addr + Arch::getInstructionSize () + Arch::getNBInstrInDelaySlot () * Arch::getInstructionSize ();
	      bb_start_addr.insert (addr_next_bb);

	      t_address addr_succ_bb = Arch::getJumpDestination (instr);
	      bb_start_addr.insert (addr_succ_bb);
	      succs[instr.addr].insert (addr_succ_bb);
	    }
	  else if (Arch::isConditionalJump (instr))
	    {
	      t_address addr_next_bb = instr.addr + Arch::getInstructionSize () + Arch::getNBInstrInDelaySlot () * Arch::getInstructionSize ();
	      bb_start_addr.insert (addr_next_bb);
	      succs[instr.addr].insert (addr_next_bb);

	      t_address addr_succ_bb = Arch::getJumpDestination (instr);
	      bb_start_addr.insert (addr_succ_bb);
	      succs[instr.addr].insert (addr_succ_bb);
	    }
	}
    }
  input.close ();

  // Build the last function
  build_heptane_cfg (cfglib_program, bb_start_addr, instructions, function, succs, instrWithWords, MetaInstructionsTable);
  // Finalize program construction
  finalize_program_construction (config, cfglib_program);

  // Export program in xml form
  exportCfg(cfglib_program);
}

/**
   Starting point of the heptane Extractor. It generates a xml file contianing the CFG of all the functions called by the program, with
   attributes attached to the CFGs, loops, basic block,...
   The xml file is used by the WCET analysis step.
*/
int
main (int argc, char **argv)
{
  string filename; // name of configuration file
  opt_verbose = false;
  
  if (argc == 3)
    {
      if ( string(argv[1]) == "-v")
	{
	  opt_verbose = true;
	  filename = string (argv[2]);
	}
      else
	cout << "Unknown option " << argv[1] << "... ignored " << endl;
    }
  else
    if (argc != 2) 
	Logger::addFatal ("Usage: HeptaneExtract [-v] configFile");
    else
      filename = string (argv[1]);
  // Parse the configuration file and generate all required files
  ConfigExtract config (filename, opt_verbose);

  // Build the CFG
  BuildCfg (config);

  // Cleaning
  Arch::kill ();
}
