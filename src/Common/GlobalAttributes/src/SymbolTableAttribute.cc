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
#include "arch.h"
#include "Logger.h"

using namespace cfglib;

SymbolTableAttribute::SymbolTableAttribute ()
{
  _gp = 0;
}

// clone function
SymbolTableAttribute *
SymbolTableAttribute::clone ()
{
  SymbolTableAttribute *result = new SymbolTableAttribute ();

  result->sections = this->sections;
  result->variables = this->variables;
  result->_gp = this->_gp;

  return result;
}

void
SymbolTableAttribute::addSection (std::string name, unsigned long addr, int size)
{
  std::pair < unsigned long, int >tmp (addr, size);
  sections[name] = tmp;
}

void
SymbolTableAttribute::addVariable (std::string name, unsigned long addr, int size, std::string section_name)
{
  triplet tmp (addr, size, section_name);
  variables[name] = tmp;
}

void
SymbolTableAttribute::setGP (unsigned long val)
{
  _gp = val;
}

unsigned long
SymbolTableAttribute::getGP ()
{
  assert (_gp != 0);
  return _gp;
}

unsigned long
SymbolTableAttribute::getCodeStartAddr ()
{
  assert (sections.find (".text") != sections.end ());
  return sections[".text"].first;
}

void
SymbolTableAttribute::getInfo (unsigned long addr, string * var_name, unsigned long *start_addr, int *size, string * section_name)
{
  //search in variables
  for (map < std::string, triplet >::const_iterator it = variables.begin (); it != variables.end (); it++)
    {
      triplet current = it->second;
      if (current.first <= addr && addr < current.first + current.second)
	{
	  *var_name = it->first;
	  *start_addr = current.first;
	  *size = current.second;
	  *section_name = current.third;
	  return;
	}
    }

  //search in .sdata (no variable name in that case). Added Lbesnard : or .bss (ARM) (no variable name in that case also)
  for (map < std::string, std::pair < unsigned long, int > >::const_iterator it = sections.begin (); it != sections.end (); it++)
    {
      // if ((it->first == ".sdata") replaced by for ARM/MIPS access. Arm: word reference .bss
      // Oct 2017 adding .rodata (read only data area, used for the switch statements.
      if ((it->first == ".sdata") || (it->first == ".bss") || (it->first == ".rodata"))
	{
	  pair < unsigned long, int >current = it->second;
	  if (current.first <= addr && addr < current.first + current.second)
	    {
	      *var_name = "";
	      *start_addr = current.first;
	      *size = current.second;
	      *section_name = it->first;
	      return;
	    }
	}
      
    }

  ostringstream oss;
  oss << "0x" << hex << addr << dec;
  Logger::addError ("address " + oss.str () + " not found");
}


// Serialisation function
ostream & SymbolTableAttribute::WriteXml (std::ostream & os, Handle & hand)
{
  os << "<ATTR " << "type=\"" << SymbolTableAttributeName << "\" name=\"" << name << "\">" << std::endl;

  //---- sections ----
  os << "<SECTIONS>" << std::endl;
  for (map < std::string, std::pair < unsigned long, int > >::const_iterator it = sections.begin (); it != sections.end (); it++)
    {
      std::pair < unsigned long, int >
	current = it->second;
      os << "<SECTION name=\"" << it->first << "\" addr=\"0x" << hex << current.first << dec << "\" size=\"" << current.second << "\"/>" << std::endl;
    }
  os << "</SECTIONS>" << std::endl;

  //---- variables ----
  os << "<VARIABLES>" << std::endl;
  for (map < std::string, triplet >::const_iterator it = variables.begin (); it != variables.end (); it++)
    {
      triplet
	current = it->second;
      os << "<VARIABLE name=\"" << it->first << "\" addr=\"0x" << hex << current.first << dec << "\" size=\"" << current.
	second << "\" section=\"" << current.third << "\"/>" << std::endl;
    }
  os << "</VARIABLES>" << std::endl;

  //-- extra -----
  if (Arch::getArchitectureName () == "MIPS")	//MIPS specific
    {
      os << "<GP value=\"0x" << hex << _gp << dec << "\"/>" << std::endl;
    }

  os << "</ATTR>" << std::endl;
  return os;
}

// Serialisation function
void
SymbolTableAttribute::ReadXml (XmlTag const *tag, Handle & hand)
{
  string attr_name = tag->getName ();
  assert (attr_name == std::string ("ATTR"));
  ListXmlTag children = tag->getAllChildren ();

  for (size_t c = 0; c < children.size (); c++)
    {
      XmlTag child = children[c];
      std::string child_name = child.getName ();
      if (child_name == "SECTIONS")
	{
	  ListXmlTag grandchildren = child.getAllChildren ();
	  for (size_t gc = 0; gc < grandchildren.size (); gc++)
	    {
	      assert (grandchildren[gc].getName () == "SECTION");

	      string name = grandchildren[gc].getAttributeString (std::string ("name"));
	      string saddr = grandchildren[gc].getAttributeString (std::string ("addr"));
	      string ssize = grandchildren[gc].getAttributeString (std::string ("size"));

	      unsigned long addr = strtoul (saddr.c_str (), NULL, 16);
	      int size = atoi (ssize.c_str ());

	      this->addSection (name, addr, size);
	    }
	}
      else if (child_name == "VARIABLES")
	{
	  ListXmlTag grandchildren = child.getAllChildren ();
	  for (size_t gc = 0; gc < grandchildren.size (); gc++)
	    {
	      assert (grandchildren[gc].getName () == "VARIABLE");

	      string name = grandchildren[gc].getAttributeString (std::string ("name"));
	      string saddr = grandchildren[gc].getAttributeString (std::string ("addr"));
	      string ssize = grandchildren[gc].getAttributeString (std::string ("size"));
	      string section = grandchildren[gc].getAttributeString (std::string ("section"));

	      unsigned long addr = strtoul (saddr.c_str (), NULL, 16);
	      int size = atoi (ssize.c_str ());

	      this->addVariable (name, addr, size, section);
	    }
	}
      else
	{
	  if (Arch::getArchitectureName () == "MIPS" && child_name == "GP")
	    {
	      string sgp_value = child.getAttributeString (std::string ("value"));
	      unsigned long gp_value = strtoul (sgp_value.c_str (), NULL, 16);

	      this->setGP (gp_value);
	    }
	  else
	    {
	      assert (false);
	    }
	}
    }
}

///----------------------------------------------------

SerialisableAttribute *
SymbolTableAttribute::create ()
{
  return new SymbolTableAttribute ();
}

//TODO
void
SymbolTableAttribute::Print (std::ostream & os)
{
  os << SymbolTableAttributeName << "\n";
}
