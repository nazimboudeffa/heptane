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

#include "SharedAttributes/SharedAttributes.h"



// CONSTRUCTOR / DESTRUCTOR
//-----------------------------------------
CodeLineAttribute::CodeLineAttribute ()
{
}

CodeLineAttribute::CodeLineAttribute (string & f, long l)
{
  file = f;
  line = l;
}

CodeLineAttribute::~CodeLineAttribute ()
{
}


// ACCESSING FUNCTIONS
//-----------------------------------------
string
CodeLineAttribute::getFile (void)
{
  return file;
}

long
CodeLineAttribute::getLine (void)
{
  return line;
}

void
CodeLineAttribute::setFile (string & f)
{
  file = f;
}

void
CodeLineAttribute::setLine (long l)
{
  line = l;
}


// FUNCTIONS
//-----------------------------------------
// Serialisation function
ostream & CodeLineAttribute::WriteXml (ostream & os, Handle & hand)
{
  os << "<ATTR type=\"" << CodeLineAttributeName << "\" " << "name=\"" << CodeLineAttributeName << "\">" << endl;
  os << " <SOURCE file=\"" << file << "\" " << "line=\"" << line << "\"/>" << endl;
  os << "</ATTR>" << endl;
  return os;
}

void
CodeLineAttribute::ReadXml (XmlTag const *tag, Handle & hand)
{
  string attr_name = tag->getName ();
  assert (attr_name == string ("ATTR"));
  ListXmlTag children = tag->getAllChildren ();

  for (unsigned int c = 0; c < children.size (); c++)
    {
      XmlTag child = children[c];

      this->file = child.getAttributeString (string ("file"));
      this->line = child.getAttributeInt (string ("line"));
    }

}


//cloning function
CodeLineAttribute *
CodeLineAttribute::clone ()
{
  CodeLineAttribute *result = new CodeLineAttribute ();
  result->setFile (file);
  result->setLine (line);

  return result;
}

SerialisableAttribute *
CodeLineAttribute::create ()
{
  return new CodeLineAttribute ();
}

//debug
void
CodeLineAttribute::Print (std::ostream & os)
{
  os << CodeLineAttributeName << "\n";
}
