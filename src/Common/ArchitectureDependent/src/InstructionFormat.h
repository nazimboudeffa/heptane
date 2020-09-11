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
 
    InstructionFormat abstract class definition
    and the concrete classes
 
    The concrete classes are used in arch_dep implementation classes
    like MIPS.cc for the formats map
 
    Semantic of the notation:
        rd:     register destination
        rs:     register source
	rds:	register source and destination
        mem:    memory pattern -> (for mips: offset (base)) (for arm: [*******])
        int:    integer value in decimal
        hex:    integer value in hexa (0x...)
        addr:   integer value corresponding to an address
	rdlist:	list with variable size containing destination registers
	rslist:	list with variable size containing source registers
	shifter:operand that shift the content of a register -> (for arm: lsl, lsr, asr ror, rrx)
 
 Rq1: The order of appearance of the notation follows 
      the syntaxic order of the instruction
      i.e.: add $rd <- $rs1 + $rs2 will have the following class rd_rs_rs
 
*****************************************************************/

#ifndef INSTRUCTION_FORMAT_H
#define INSTRUCTION_FORMAT_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <stdlib.h>

#include "arch.h"

using namespace std;

//--------------------------------------------
//	Abstract class: InstructionFormat
//--------------------------------------------
class InstructionFormat
{
    public :
    
    /*! Returns true if operands match the format*/
    virtual bool isFormat(const vector<string>& operands)=0;
    
    /*! Returns the input resources */
    virtual vector<string> getResourceInputs(const vector<string>& operands)=0;
    
    /*! Returns the output resources */
    virtual vector<string> getResourceOutputs(const vector<string>& operands)=0;
    
    /*! Virtual destructor*/
    virtual ~InstructionFormat(){;};
};

//--------------------------------------------
//	Empty (no operand)
//--------------------------------------------
class Empty : public InstructionFormat
{
    public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

/**********************************************
 *
 *               1 OPERAND
 *
 **********************************************/

//--------------------------------------------
//	rd
//--------------------------------------------
class rd : public InstructionFormat
{
private:
    string rin; ///< static input resources
public:

    /*! If there are several ressources, separate them with comas, like "LO, HI" */
    rd(const string& in);

    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	hex
//--------------------------------------------
class Hex : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	addr
//--------------------------------------------
class Addr : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs
//--------------------------------------------
class rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//	rd_mem
//--------------------------------------------
class rd_mem : public InstructionFormat
{
    public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_int
//--------------------------------------------
class rd_int : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs_mem
//--------------------------------------------
class rs_mem : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs_addr
//--------------------------------------------
class rs_addr : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_rs
//--------------------------------------------
class rd_rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs_rd
//--------------------------------------------
class rs_rd : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs_rs
//--------------------------------------------
class rs_rs : public InstructionFormat
{
private:
    string rout; //static output resources
public:

    /*! If there are several ressources, separate them with comas, like "LO, HI" */
    rs_rs(const string& out);

    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_hex
//--------------------------------------------
class rd_hex : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

/**********************************************
 *
 *               3 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//	rs_zero_hex
//--------------------------------------------
class rs_zero_hex : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rs_rs_addr
//--------------------------------------------
class rs_rs_addr : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	zero_rs_rs
//--------------------------------------------
class zero_rs_rs : public InstructionFormat
{
private:
    string rout; //static output resources
public:

    /*! If there are several ressources, separate them with comas, like "LO, HI" */
    zero_rs_rs(const string& out);

    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rd_rs_rs
//--------------------------------------------
class rd_rs_rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_rs_int
//--------------------------------------------
class rd_rs_int : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_rs_hex
//--------------------------------------------
class rd_rs_hex : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};




/***************************************************************


	    ADDITIONNALS FORMATS FOR ARM


**************************************************************/


/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//	rs_int
//--------------------------------------------
class rs_int :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_hex
//--------------------------------------------
class rs_hex :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rds_int
//--------------------------------------------
class rds_int :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rds_hex
//--------------------------------------------
class rds_hex :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};



/**********************************************
 *
 *               3 OPERANDS
 *
 **********************************************/


//--------------------------------------------
//	rd_mem_int
//--------------------------------------------
class rd_mem_int :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_mem_int
//--------------------------------------------
class rs_mem_int :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rd_mem_rs
//--------------------------------------------
class rd_mem_rs :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_mem_rs
//--------------------------------------------
class rs_mem_rs :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_rs_shifter
//--------------------------------------------
class rs_rs_shifter : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rd_rs_shifter
//--------------------------------------------
class rd_rs_shifter : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};



/**********************************************
 *
 *               4 OPERANDS
 *
 **********************************************/

//--------------------------------------------
//	rd_rd_rs_rs
//--------------------------------------------
class rd_rd_rs_rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rd_rs_rs_rs
//--------------------------------------------
class rd_rs_rs_rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rds_rds_rs_rs
//--------------------------------------------
class rds_rds_rs_rs : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rd_mem_rs_shifter
//--------------------------------------------
class rd_mem_rs_shifter : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_mem_rs_shifter
//--------------------------------------------
class rs_mem_rs_shifter : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rd_rs_rs_shifter
//--------------------------------------------
class rd_rs_rs_shifter : public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


/**********************************************
 *
 *               ONE OPERAND
 *
 **********************************************/
//--------------------------------------------
//	rslist 
// 
//  Warning : because of ARM, this format returns "mem" in the Ouputs
//--------------------------------------------
class rslist :  public InstructionFormat
{
private:
    string rin; //static input resource
    string rout; //static output resource
public:

    /*! If there are several ressources, separate them with comas, like "LO, HI" */
    rslist(const string& in = "", const string& out = "");

    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

//--------------------------------------------
//	rdlist 
// 
//  Warning : because of ARM, this format returns "mem" in the Inputs
//--------------------------------------------
class rdlist :  public InstructionFormat
{
private:
    string rin; //static input resource
    string rout; //static output resource
public:

    /*! If there are several ressources, separate them with comas, like "LO, HI" */
    rdlist(const string& in = "", const string& out = "");

    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};



/**********************************************
 *
 *               2 OPERANDS
 *
 **********************************************/
//--------------------------------------------
//	rs_rslist 
// 
//  Warning : because of ARM, this format returns "mem" in the Ouputs
//--------------------------------------------
class rs_rslist :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rds_rslist 
// 
//  Warning : because of ARM, this format returns "mem" in the Ouputs
//--------------------------------------------
class rds_rslist :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rs_rdlist 
// 
//  Warning : because of ARM, this format returns "mem" in the Inputs
//--------------------------------------------
class rs_rdlist :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};


//--------------------------------------------
//	rds_rdlist 
// 
//  Warning : because of ARM, this format returns "mem" in the Inputs
//--------------------------------------------
class rds_rdlist :  public InstructionFormat
{
public:
    bool isFormat(const vector<string>& operands);
    vector<string> getResourceInputs(const vector<string>& operands);
    vector<string> getResourceOutputs(const vector<string>& operands);
};

#endif
