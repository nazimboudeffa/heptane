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

#include "Specific/DataAddressAnalysis/StackInfoAttribute.h"
#include "SharedAttributes/SharedAttributes.h"

StackInfoAttribute::StackInfoAttribute(){;}

StackInfoAttribute *
StackInfoAttribute::clone ()
{
  StackInfoAttribute *result = new StackInfoAttribute ();
  result->setFrameSizeWithoutCaller (this->getFrameSizeWithoutCaller ());
  result->setFrameSizeWithCaller (this->getFrameSizeWithCaller ());
  result->context2sp = this->context2sp;
  return result;
}

int
StackInfoAttribute::getFrameSizeWithoutCaller ()
{
  return frameSizeWithoutCaller;
}

int
StackInfoAttribute::getFrameSizeWithCaller ()
{
  return frameSizeWithCaller;
}

void
StackInfoAttribute::setFrameSizeWithoutCaller (int val)
{
  frameSizeWithoutCaller = val;
}

void
StackInfoAttribute::setFrameSizeWithCaller (int val)
{
  frameSizeWithCaller = val;
}

void
StackInfoAttribute::addContext (long sp, string ctx)
{
  assert (context2sp.find (ctx) == context2sp.end ());
  context2sp[ctx] = sp;
}

long
StackInfoAttribute::getSP (string ctx)
{
  assert (context2sp.find (ctx) != context2sp.end ());
  return context2sp[ctx];
}


SerialisableAttribute* StackInfoAttribute::create()
{ return new StackInfoAttribute() ;}

// Serialisation function
ostream& StackInfoAttribute::WriteXml(std::ostream& os, Handle& hand) {
  os	<< "<ATTR "
	<< "type=\"" << StackInfoAttributeName <<"\" name=\"" << name << "\" framesizewithcaller=\"" << frameSizeWithCaller <<
    "\"" << " framesizewithoutcaller=\"" << frameSizeWithoutCaller << "\">" << std::endl;
  
  for(map<string,long>::const_iterator it=context2sp.begin();it!=context2sp.end();it++)
    {
      os << "\t<SPCTX sp=\"0x" << hex << it->second << dec << "\" ctx=\"" << it->first << "\"/>" << endl;
    }
  
  os 	<< "</ATTR>" << std::endl ;
  return os;
}

// Serialisation function
void StackInfoAttribute::ReadXml(XmlTag const*tag,Handle& hand) {
    
  string attr_name = tag->getName();
  assert(attr_name == std::string("ATTR"));
    
  this->frameSizeWithCaller     = tag->getAttributeInt(std::string("framesizewithcaller"));
  this->frameSizeWithoutCaller  = tag->getAttributeInt(std::string("framesizewithoutcaller"));
    
  ListXmlTag children = tag->getAllChildren();
  for (unsigned int c=0;c<children.size();c++)
    {
      XmlTag  child = children[c]; //balise access
        
      string child_name = child.getName();
      assert(child_name == std::string("SPCTX"));
        
      long sp     = child.getAttributeHexa(std::string("sp"));
      string ctx  = child.getAttributeString(std::string("ctx"));
        
      this->context2sp[ctx]=sp;
    }
}

//TODO
void StackInfoAttribute::Print(std::ostream& os) {os << StackInfoAttributeName << "\n";}
