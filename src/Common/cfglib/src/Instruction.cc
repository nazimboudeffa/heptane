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

#include <string>
#include <iostream>
#include <assert.h>
#include "Handle.h"
#include "Helper.h"
#include "Instruction.h"

/*! this namespace is the global namespace */
namespace cfglib
{
  /*! This class represents the most common node : basic block.  */

  /* forward declarations and #includes */

  /*! constructor */
  Instruction::Instruction ()
  {
    dbg_instr (std::cerr << "Instruction basic constructor called" << std::endl; );
  }

  Instruction::Instruction (std::string const &code, asm_type type):code (code), type (type)
  {
    dbg_instr (std::cerr << "Instruction constructor called" << std::endl; );
  }

  /*! destructor. Basically do nothing now (all memory
   * allocation seems to be language managed) */
  Instruction::~Instruction ()
  {
  }

  //TP
  //TODO Register the instruction and its clone in the handle, if required.
  Instruction *Instruction::Clone (CloneHandle & handle)
  {
    Instruction *I = new Instruction (this->code, this->type);
    //handle attributes
    this->CloneAttributesFor (I, handle);
    return I;
  }

  //TP
  Instruction *Instruction::Clone (void)
  {
    Instruction *I = new Instruction (this->code, this->type);

    //handle attributes
    std::vector < string > attrList = getAttributeList ();
    for (std::vector < string >::iterator it = attrList.begin (); it != attrList.end (); it++)
      {
	Attribute & attr = GetAttribute (*it);
	I->SetAttribute ((*it), attr);
      }
    /*std::vector<int> int_attrList = getIntAttributeList();
       for(std::vector<int>::iterator it = int_attrList.begin();
       it != int_attrList.end();
       it++){
       Attribute& attr = GetAttribute(*it);
       I->SetAttribute((*it), attr);
       } */

    return I;
  }

  std::string asm_string_from_type (asm_type type)
  {
    if (type == Code) return "Code";
    if (type == Macro) return "Macro";
    if (type == Directive) return "Directive";
    if (type == Label) return "Label";
    if (type == Other) return "Other";
    assert (false);
  }

  asm_type asm_type_from_string (std::string type)
  {
    if (type == "Code") return Code;
    if (type == "Macro") return Macro;
    if (type == "Directive") return Directive;
    if (type == "Label") return Label;
    if (type == "Other") return Other;
    std::cerr << "Malformed input file " << std::endl;
    assert (false);
  }

  /*! Serialisation function. Implements the
   * `Node` interface. */
  std::ostream & Instruction::WriteXml (std::ostream & os, Handle & hand)
  {
    using helper::escape_xml;
    os << "  <INSTRUCTION "
      << " id=\"" << hand.identify (this) << "\" "
      << " asm_type=\"" << escape_xml (asm_string_from_type (this->type)) << "\" "
      << " code=\"" << (escape_xml (this->code)) << "\" " << " >" << std::endl;

    this->WriteXmlAttributes (os, hand);

    os << "  </INSTRUCTION>" << std::endl;
    return os;
  }

  /*! Unserialisation function */
  void Instruction::ReadXml (XmlTag const *tag, Handle & hand)
  {
    assert (tag->getName () == string ("INSTRUCTION"));
    string code = tag->getAttributeString ("code");
    this->code = code;
    string asm_type = tag->getAttributeString ("asm_type");
    assert (asm_type != "");
    this->type = asm_type_from_string (asm_type);
    string id = tag->getAttributeString ("id");
    assert (id != "");
    hand.addID_serialisable (id, this);
    this->ReadXmlAttributes (tag, hand);
  }

  std::string Instruction::GetCode ()
  {
    return this->code;
  }

} // cfglib::
