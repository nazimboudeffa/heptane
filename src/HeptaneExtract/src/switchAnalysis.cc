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
#include <assert.h>
#include "GlobalAttributes.h"
#include "switchAnalysis.h"
#include "ConfigExtract.h"

/* this namespace is the global namespace */
namespace cfglib {


  bool SwitchComputer::GetAddressInstr (cfglib::Instruction * vi, t_address  * addr)
  {
    if (! vi->HasAttribute (AddressAttributeNameExtract)) return false;
    AddressAttribute attr_first = (AddressAttribute &) vi->GetAttribute (AddressAttributeNameExtract);
    *addr = attr_first.getCodeAddress ();
    return true;
  }

  /**
     Utility function to get first/last code address of a node (n)
  */
  bool SwitchComputer::GetFirstLastAddresses (cfglib::Node * n, t_address * first, t_address * last)
  {
    vector < cfglib::Instruction * >vi = n->GetAsm ();
    if (GetAddressInstr(vi[0], first))
      return GetAddressInstr(vi[vi.size () - 1], last);
    return false;
  }
  
  int SwitchComputer::getIndexNodeWithFirstAddr( vector < cfglib::Node * > vn, t_address vaddr)
  {
    t_address first_address, last_address;
    for (size_t i = 0; i < vn.size(); ++i)
      {
	if (GetFirstLastAddresses (vn[i], &first_address, &last_address))
	  if ((vaddr >= first_address) && (vaddr <= last_address)) return i;
      }
    return -1;
  }

  bool SwitchComputer::getAddressJump(Node * n, t_address *vaddr)
  {
    t_address addrinstr;
    vector < cfglib::Instruction * >vi = n->GetAsm ();
   // vi.size();
   // MIPS vi[vi.size() -2] (Branch delay slot ), ARM vi[vi.size() -1]
    for (int i=1; i<=2;i++)
      {
	cfglib::Instruction * vinstr = vi[vi.size() - i ];
	string l = vinstr->GetCode();
	if (GetAddressInstr(vinstr, &addrinstr))
	  {
	    // to use the parsing, the objdump format is rebuilt.
	    ostringstream oss;
	    oss << "0x" << hex << addrinstr;
	    ObjdumpInstruction instr = Arch::parseInstruction(oss.str () + " " + oss.str () + " " + l);
	    if (Arch::isUnconditionalJump (instr) || Arch::isConditionalJump (instr))
	      {
		*vaddr = Arch::getJumpDestination (instr);
		return true;
	      }
	  }
      }
   return false;
  }
  
  /** Modifies a Cfg (cfg) for the switch statements.
      For a switch S, element of lswitch, S is the address of the begining of the switch statement.
      If S is an address of the Cfg, (block BB0)
          * BB0 contains the jump to the end of the switch, (so this block -BBend- of the end switch is known).
	  * BB1 (next of BB0 in the Cfg) contains the indirect jump via a table (.rodata for MIPS, set of .words for ARM)
	  The modification consists in adding edge from BB1 to BBi, where BBi is a block BB2 until BBend-1.
  */
  void SwitchComputer::computeSwitch( Cfg * acfg, vector < t_address> &lswitch, bool isMIPSarchi)
  {
    t_address switchAddr,end_switch_address ;
    vector < cfglib::Node * >vn;
    cfglib::Node * n,* node_indirect_jump;
    std::vector <  t_address >::iterator i;
    int m, jj, nn;
    
    for (i = lswitch.begin(); i != lswitch.end(); i++)
      {
	switchAddr = *i;
	vn = acfg->GetAllNodes ();
	nn = getIndexNodeWithFirstAddr(vn, switchAddr);
	if ( nn != -1 )
	  {
	    n = vn[nn];
	    // cout << " Switch found in " << acfg->getStringName() << endl;
	    if (getAddressJump(n, &end_switch_address))
	      {
		m = getIndexNodeWithFirstAddr(vn, end_switch_address );
		if ( m == -1 )
		  {
		    // cout << " End switch node not found !!!" <<  endl;
		  }
		else
		  {
		    // -- ARM   (one block)
		    // 80ec:	979ff103 	ldrls	pc, [pc, r3, lsl #2]      -- indirect jump
		    // 80f0:	ea00002f 	b	81b4 <num_to_lcd+0xdc>    -- end switch
		    // 80f4:	00008134 	.word	0x00008134                -- table of pointers
		    // 80f8:	0000813c 	.word	0x0000813c
		    // -- MIPS (two blocs)
		    //      ...
		    // 400028:	2c430010 	sltiu	v1,v0,16
		    // 40002c:	10600029 	beqz	v1,4000d4 <num_to_lcd+0xbc> -- end switch
		    // 400030:	00000000 	nop
		    //   --SECOND BLOC.
		    // 400034:	00021880 	sll	v1,v0,0x2
		    // 400038:	3c020040 	lui	v0,0x40
		    // 40003c:	24420178 	addiu	v0,v0,376    -- v0 @ in .rodata (table of pointers)
		    // 400040:	00621021 	addu	v0,v1,v0
		    // 400044:	8c420000 	lw	v0,0(v0)
		    // 400048:	00000000 	nop
		    // 40004c:	00400008 	jr	v0   -- indirect jump
		    // 400050:	00000000 	nop
		    int incr;
		    if (isMIPSarchi) incr=1; else incr=0;
		    node_indirect_jump = vn[nn + incr];
		    // adding edge "node_indirect_jump  -> vn[i]", for i in [nn+1+incr, m-1]
		    for (jj = nn+1+incr; jj <= m-1; jj++)
		      {
			acfg->CreateNewEdge (node_indirect_jump, vn[jj]);
		      }
		    // the BBlock with indirect jump is removed from the "end nodes"
		    acfg->removeEndNode(node_indirect_jump);
		    node_indirect_jump->IsReturn();
		  }
	      }
	  }
      }
}
}

	

