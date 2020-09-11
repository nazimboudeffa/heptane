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

/* forward declarations and #includes */
#include <string>
#include "Attributed.h"
#include "CloneHandle.h"

namespace cfglib { class Numberize ; } // cf. "misc/handlers.h"
namespace cfglib { class Handle ; } // cf. "misc/handlers.h"

#ifndef _IRISA_CFGLIB_INSTRUCTIONS_H
#define _IRISA_CFGLIB_INSTRUCTIONS_H

/* Debug of instruction management methods */
// Uncomment one of these two lines to enter/leave debug mode
//#define dbg_instr(x) x
#define dbg_instr(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
	enum asm_type { 
		Code, /* instructions */
		Macro, /* Pseudo-instruction */
		Label, /* labels */
		Directive, /* directives and declarations */
		Other /* comments, etc. */
	} ;
  
  class Instruction : public Attributed
  {
  private:
    std::string code ;
    asm_type type ;
  public:
    /*! Constructor */
    Instruction();

    /*! Normal constructor */
    Instruction(std::string const& code, asm_type type);

    /*! destructor */
    ~Instruction();
    
    //TP
    /*! Cloning function */
    Instruction* Clone();
    Instruction* Clone(CloneHandle&);

    /*! Serialisation function. */
    std::ostream& WriteXml(std::ostream& os, Handle& hand);

    /*! Unserialisation function */
    virtual void ReadXml( XmlTag const* tag, Handle& hand) ;
			
    /*! get the assembly code line */
    std::string GetCode() ;

    /*! Returns true if the line is a Code line */
    bool IsCode() {return (type==Code);}

    /*! Returns true if the line is a Macro line */
    bool IsMacro() {return (type==Macro);}

    /*! Returns true if the line is a Directive */
    bool IsDirective() {return (type==Directive);}

    /*! Returns true if the line is a Labem */
    bool IsLabel() {return (type==Label);}
    
    /*! Returns true if the line is an other type of line (eg comment, empty line) */
    bool IsOther() {return (type==Other);}

  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_INSTRUCTIONS_H
