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

using namespace cfglib;

///----------------------------------------

AddressInfo::AddressInfo ()
{
  adrsize = vector < pair < string, string > >();
  name = "";
  type = "";
  segment = "";
  precision = false;
}

void
AddressInfo::setName (string n)
{
  name = n;
}

void
AddressInfo::setType (string t)
{
  type = t;
}

void
AddressInfo::setSegment (string seg)
{
  segment = seg;
}

void
AddressInfo::setPrecision (bool p)
{
  precision = p;
}

void
AddressInfo::addAdrSize (string adr, string size)
{
  pair < string, string > tmp = pair < string, string > (adr, size);
  adrsize.push_back (tmp);
}

vector < pair < string, string > >AddressInfo::getAdrSize ()
{
  return adrsize;
}


string
AddressInfo::getType ()
{
  return type;
}

string
AddressInfo::getName ()
{
  return name;
}

string
AddressInfo::getSegment ()
{
  return segment;
}

bool
AddressInfo::getPrecision ()
{
  return precision;
}

void
AddressInfo::print ()
{
  cout << "---- AddressInfo\n";
  int sizeAdrSize = adrsize.size ();
  for (int i = 0; i < sizeAdrSize; i++)
    {
      string addr = adrsize[i].first;
      cout << "\t adrSize = (0x" << std::hex << atol(addr.c_str()) << ", " << adrsize[i].second << std::dec << ")" << endl;
    }
  cout << "\t type=" << type << "\n";
  cout << "\t name=" << name << "\n";
  cout << "\t segment=" << segment << "\n";
  cout << "\t precision=" << precision << endl;
}

///----------------------------------------------------

AddressAttribute::AddressAttribute ()
{
  listInfo = vector < AddressInfo > ();
}

vector < AddressInfo > AddressAttribute::getListInfo ()
{
  return listInfo;
}


//
void
AddressAttribute::setInfo (vector < AddressInfo > list)
{
  listInfo = list;
}



//add an object AddressInfo (equivalent to an access to memory) to the AddressInfo list of this
void
AddressAttribute::addInfo (AddressInfo info)
{
  listInfo.push_back (info);
}

// clone function
AddressAttribute *
AddressAttribute::clone ()
{
  AddressAttribute *result = new AddressAttribute ();
  result->setInfo (this->getListInfo ());
  return result;
}


// Serialisation function
ostream & AddressAttribute::WriteXml (std::ostream & os, Handle & hand)
{
  os << "<ATTR " << "type=\"" << AddressAttributeName << "\" name=\"" << name << "\">" << std::endl;

  int sizeAddressInfo = listInfo.size ();
  for (int i = 0; i < sizeAddressInfo; i++)
    {
      AddressInfo current = listInfo[i];
      os << "  <ACCES "
	<< "type=\"" << current.getType () << "\" seg=\"" << current.getSegment () << "\" varname=\"" << current.
	getName () << "\" precision=\"" << current.getPrecision () << "\">" << std::endl;

      vector < pair < string, string > >adrSize = current.getAdrSize ();
      pair < string, string > currentPair;
      int
	size = adrSize.size ();
      for (int j = 0; j < size; j++)
	{
	  currentPair = adrSize[j];
	  os << "      <ADDRSIZE  begin=\"0x" << std::hex << atol (currentPair.first.c_str ()) << std::dec << "\" size=\"" << currentPair.second << "\"/>" << std::endl;
	}

      os << "  </ACCES>" << std::endl;
    }

  os << "</ATTR>" << std::endl;
  return os;
}



// Serialisation function
void
AddressAttribute::ReadXml (XmlTag const *tag, Handle & hand)
{

  string attr_name = tag->getName ();
  assert (attr_name == std::string ("ATTR"));
  ListXmlTag children = tag->getAllChildren ();

  for (unsigned int c = 0; c < children.size (); c++)
    {

      AddressInfo info;

      XmlTag child = children[c];	//balise access
      string type = child.getAttributeString (std::string ("type"));
      string varname = child.getAttributeString (std::string ("varname"));
      string segment = child.getAttributeString (std::string ("seg"));
      int precision = child.getAttributeInt (std::string ("precision"));

      info.setType (type);
      info.setName (varname);
      info.setSegment (segment);
      info.setPrecision (precision != 0);
      
      ListXmlTag grandchildren = child.getAllChildren ();
      for (unsigned int gc = 0; gc < grandchildren.size (); gc++)
	{
	  XmlTag grandchild = grandchildren[gc];
	  string adrBegin = grandchild.getAttributeString (std::string ("begin"));
	  string size = grandchild.getAttributeString (std::string ("size"));

	  //convert adrBegin in decimal if the value is in hexa
	  if (adrBegin.length () >= 2 && adrBegin[0] == '0' && adrBegin[1] == 'x')
	    {
	      long dec_value = strtoul (adrBegin.c_str (), NULL, 16);
	      std::ostringstream sdec_value;
	      sdec_value << dec_value;
	      adrBegin = sdec_value.str ();
	    }
	  info.addAdrSize (adrBegin, size);
	}
      this->listInfo.push_back (info);
    }
}


long
AddressAttribute::getCodeAddress ()
{
  for (unsigned int i = 0; i < listInfo.size (); i++)
    {
      if (listInfo[i].getSegment () == "code")
	{
	  return atol (listInfo[i].getAdrSize ()[0].first.c_str ());
	}
    }
  return (long) -1;
}

///----------------------------------------------------

SerialisableAttribute *
AddressAttribute::create ()
{
  return new AddressAttribute ();
}




//TODO
void
AddressAttribute::Print (std::ostream & os)
{
  os << AddressAttributeName << "\n";
}
