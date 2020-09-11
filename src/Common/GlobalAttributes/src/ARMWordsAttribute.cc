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

ARMWordsAttribute::ARMWordsAttribute ()
{ }

// clone function
ARMWordsAttribute *
ARMWordsAttribute::clone ()
{
  ARMWordsAttribute *result = new ARMWordsAttribute ();
  result->words = this->words;
  return result;
}

void
ARMWordsAttribute::addWord (unsigned long addr, string type, unsigned long value)
{
  pair < string, unsigned long >current (type, value);
  words[addr] = current;

}

// Serialisation function
ostream & ARMWordsAttribute::WriteXml (std::ostream & os, Handle & hand)
{
  os << "<ATTR " << "type=\"" << ARMWordsAttributeName << "\" name=\"" << name << "\">" << endl;
  os << "<WORDS>" << endl;
  for (ARMWordsAttributeTable::const_iterator it = words.begin (); it != words.end (); it++)
    {
      pair < string, unsigned long > current = it->second;
      os << "<WORD addr=\"0x" << hex << it->first << dec << "\" type=\"" << current.first << "\" value=\"0x" << hex << current.second << dec << "\"/>" << endl;
    }

  os << "</WORDS>" << endl;
  os << "</ATTR>" << endl;

  return os;
}

// Serialisation function
void
ARMWordsAttribute::ReadXml (XmlTag const *tag, Handle & hand)
{
  string attr_name = tag->getName ();
  assert (attr_name == std::string ("ATTR"));
  ListXmlTag children = tag->getAllChildren ();
  assert (children.size () == 1 && children[0].getName () == "WORDS");

  ListXmlTag grandchildren = children[0].getAllChildren ();
  for (size_t gc = 0; gc < grandchildren.size (); gc++)
    {
      assert (grandchildren[gc].getName () == "WORD");

      string saddr = grandchildren[gc].getAttributeString (std::string ("addr"));
      string type = grandchildren[gc].getAttributeString (std::string ("type"));
      string svalue = grandchildren[gc].getAttributeString (std::string ("value"));

      unsigned long addr = strtoul (saddr.c_str (), NULL, 16);
      unsigned long value = strtoul (svalue.c_str (), NULL, 16);

      this->addWord (addr, type, value);
    }
}

///----------------------------------------------------

SerialisableAttribute *
ARMWordsAttribute::create ()
{
  return new ARMWordsAttribute ();
}

//TODO
void
ARMWordsAttribute::Print (std::ostream & os)
{
  os << ARMWordsAttributeName << "\n";
}


ARMWordsAttributeTable 
ARMWordsAttribute::getTable ()
{
  return words;
}
