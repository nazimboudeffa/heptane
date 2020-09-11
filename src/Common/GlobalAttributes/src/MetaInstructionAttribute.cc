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

#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>

#include <cassert>
#include "GlobalAttributes.h"
#include "Logger.h"

using namespace cfglib;

MetaInstructionAttribute::MetaInstructionAttribute ()
{ }


// clone function
MetaInstructionAttribute *
MetaInstructionAttribute::clone ()
{
  MetaInstructionAttribute *result = new MetaInstructionAttribute ();
  result->instrObjdump = this->instrObjdump;
  return result;
}


// Serialisation function
ostream & MetaInstructionAttribute::WriteXml (std::ostream & os, Handle & hand)
{
  os << "<ATTR " << "type=\"" << MetaInstructionAttributeName << "\" name=\"" << name << "\">" << endl;
  //  os << "<ATTR " << "type=\"" << MetaInstructionAttributeName << "\">" << endl;
  os << "  <OBJDUMP_INSTR  " << "vinstr=\"" << instrObjdump << "\"/>" << std::endl;
  os << "</ATTR>" << endl;

  return os;
}

// Serialisation function
void
MetaInstructionAttribute::ReadXml (XmlTag const *tag, Handle & hand)
{
  string attr_name = tag->getName ();
  assert (attr_name == std::string ("ATTR"));

  ListXmlTag children = tag->getAllChildren ();
  assert (children.size () == 1 && children[0].getName () == "OBJDUMP_INSTR");

  ListXmlTag grandchildren = children[0].getAllChildren ();
  for (size_t gc = 0; gc < grandchildren.size (); gc++)
    {
      assert (grandchildren[gc].getName () == "OBJDUMP_INSTR");

      string saddr = grandchildren[gc].getAttributeString (std::string ("vinstr"));
      this->instrObjdump =  saddr;
    }
}

///----------------------------------------------------

SerialisableAttribute *
MetaInstructionAttribute::create ()
{
  return new MetaInstructionAttribute ();
}

void
MetaInstructionAttribute::Print (std::ostream & os)
{
  os << "Associated Objdump Instruction = " << instrObjdump << "\n";
}


string
MetaInstructionAttribute::getObjdumpInstr ()
{
  return this->instrObjdump;
}


void
MetaInstructionAttribute::setObjdumpInstr (string vi)
{
  this->instrObjdump = vi;
}
