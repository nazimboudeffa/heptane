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

/* #includes and forward declarations */
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include "Attributed.h"
#include "Handle.h"
#include "Factory.h"
#include "SerialisableAttributes.h"
#include "NonSerialisableAttributes.h"

namespace cfglib
{
  class Attribute;
}

/*! this namespace is the global namespace */
namespace cfglib
{
  /*! Destructor */
  Attributed::~Attributed ()
  {
    // Deallocate attributes
    for (attributes_container::iterator it (this->attributes.begin ()); it != this->attributes.end (); ++it)
      {
	delete it->second;
      }
  }

  /*! Returns true if the attributed object has an attribute of name 'symbol' attached
   *  Must be called before any attempt to call method GetAttribute
   */
  bool Attributed::HasAttribute (std::string symbol)
  {
    attributes_container::iterator it (this->attributes.find (symbol));
    return (it != this->attributes.end ());
  }

  /*! Get a attribute given its name. The method makes a copy of the attribute
   * by calling its method "clone" before the attribute is stored. All attributed
   * must have a 'clone' method implemented.
   * Method HasAttribute has to be called before trying to retrieve an attribute
   * to make sure the attribute with name 'symbol' exists (else, an assertion in the code fails).
   * In the case the attribute contains pointers on other attributes, the attribute copy
   * constructor has to be defined.
   * Example of use of attributes (with n an attributed object)
   *       IntegerAttribute ia(6);              // Declare an integer attribute
   *       n.SetAttribute("myinteger",ia);      // Attach a copy of the attribute to object n
   *
   *       IntegerAttribute ia;                 // Declare an attribute
   *         assert(n.HasAttribute("myinteger");// Make sure n has an attribute attached
   *       ia=n.GetAttribute("myinteger");      // Retrieve a copy of the attribute in ia
   *       int val = ia.GetValue();             // Get it's value (here, a simple integer)
   */
  Attribute & Attributed::GetAttribute (std::string symbol)
  {
    attributes_container::iterator it (this->attributes.find (symbol));
    if (it == this->attributes.end ())
      {
	cout << "cfglib::GetAttribute, no attribute found, attribute name " << symbol << endl;
      }
    assert (it != this->attributes.end ());
    Attribute *res = it->second;
    return (*res);
  }

  std::vector < string > Attributed::getAttributeList (void)
  {
    std::vector < string > attrList;
    for (attributes_container::iterator it = attributes.begin (); it != attributes.end (); it++)
      {
	attrList.push_back (it->first);
      }
    return attrList;
  }


  void Attributed::CloneAttributesFor (Attributed * target, CloneHandle & handle)
  {
    for (attributes_container::iterator it = this->attributes.begin (); it != this->attributes.end (); ++it)
      {
	Attribute *clone = it->second->clone (handle);
	attributes_container::iterator previous_pos = target->attributes.find (it->first);
	if (previous_pos != target->attributes.end ())
	  {
	    delete previous_pos->second;
	    previous_pos->second = clone;
	  }
	else
	  {
	    target->attributes[it->first] = clone;
	  }
      }

  }

  /*! Add or replace a named attribute. A pointer to a copy
   * of the attribute is stored.
   * In case an already existing attribute is replaced,
   * the attribute is deleted before the new one is
   * installed. */
  void Attributed::SetAttribute (std::string const &symbol, Attribute & attribute)
  {
    // cout << " SetAttribute = " << symbol << endl;
    // Make a copy of the attribute
    Attribute *new_attribute = attribute.clone ();
    // Delete the former attribute with same name, if any
    attributes_container::iterator it (this->attributes.find (symbol));
    if (it != this->attributes.end ())
      {
	assert (it->second != NULL);
	delete it->second;
      }
    // Store the new attribute
    (this->attributes)[symbol] = new_attribute;
  }

  /*! Remove an attribute (frees its memory) 
   * (if not removed, an attribute stays attached and consumes memory up
   * to the program termination)
   */
  void Attributed::RemoveAttribute (std::string const &symbol)
  {
    // cout << " removeAttribute = " << symbol << endl;
    attributes_container::iterator it (this->attributes.find (symbol));
    if (it != this->attributes.end ())
      {
	delete it->second;
	this->attributes.erase (it);
      }
  }

  /*! Print information on the non serialisable attributes, by calling
   * their Print method. Used for debug only, to check that all
   * NonSerialisableAttributes are removed at the end of every analysis.
   */
  void Attributed::PrintNonSerialisableAttributes (std::ostream & os)
  {
    if (this->attributes.size () != 0)
      {
	for (attributes_container::const_iterator it (this->attributes.begin ()); it != this->attributes.end (); ++it)
	  {
	    if (NonSerialisableAttribute * nsa = dynamic_cast < NonSerialisableAttribute * >(it->second))
	      {
		nsa->Print (os);
		os << std::endl;
	      }
	  }
      }
  }


  /*! Serialise all attributes */
  std::ostream & Attributed::WriteXmlAttributes (std::ostream & os, Handle & hand_ser) const
  {
    if (this->attributes.size () != 0)
      {
	os << "<ATTRS_LIST>" << std::endl;
	for (attributes_container::const_iterator it (this->attributes.begin ()); it != this->attributes.end (); ++it)
	  {
	    it->second->SetName (it->first);
	    if (SerialisableAttribute * sa = dynamic_cast < SerialisableAttribute * >(it->second))
	      {
		sa->WriteXml (os, hand_ser);
	      }
	  }

	os << "</ATTRS_LIST>" << std::endl;
      }
    return os;
  }

  /*! Unserialise all attributes */
  void Attributed::ReadXmlAttributes (XmlTag const *tag, Handle & hand)
  {
    ListXmlTag children = tag->searchChildren ("ATTRS_LIST");
    if (children.size () == 0)
      return;			// No attributes
    assert (children.size () == 1);
    children = children[0].getAllChildren ();
    for (unsigned int c = 0; c < children.size (); c++)
      {
	XmlTag child = children[c];
	assert (child.getName () == std::string ("ATTR"));
	string attrtype = child.getAttributeString (std::string ("type"));
	AttributesFactory *af = AttributesFactory::GetInstance ();
	SerialisableAttribute *new_attribute = af->CreateNewAttribute (attrtype);
	if (new_attribute != NULL)
	  {
	    // all `ATTR`s elements have a `name` attribute
	    // this name is normally unique amongst all the
	    // `ATTR`s of one `ATTRS_LISTS
	    string attrname = child.getAttributeString (std::string ("name"));
	    assert (attrname != "");
	    new_attribute->ReadXml (&child, hand);
	    this->SetAttribute (attrname, *new_attribute);
	    delete new_attribute;
	  }
	else
	  {
	    std::cerr << "Error: unknown attribute type: " << attrtype << std::endl;
	    std::cerr << "Attribute ignored in XML file" << std::endl;
	  }
      }
  }

}				// cfglib::
