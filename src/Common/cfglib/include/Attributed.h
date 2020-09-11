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

#ifndef _IRISA_CFGLIB_ATTRIBUTED_H
#define _IRISA_CFGLIB_ATTRIBUTED_H

/* #includes and forward declarations */
#include <string>
#include <map>
#include "Attributes.h"
#include "Serialisable.h"
#include "CloneHandle.h"
namespace cfglib { class Handle ; }

/*! this namespace is the global namespace */
namespace cfglib 
{

  /*! Attributed. All objects to which we can add
   * attributes inherit this class. */
  class Attributed : public Serialisable {
  private:
    typedef std::map<std::string, Attribute*> attributes_container;
    attributes_container attributes;
  public:
	
    /*! Returns true if the attributed object has an attribute of name 'symbol' attached
     *  Must be called before any attempt to call method GetAttribute
     */
    bool HasAttribute(std::string symbol) ;

    /*! Get a attribute given its name. The method makes a copy of the attribute
     * by calling its method "clone" before the attribute is stored. All attributed
     * must have a 'clone' method implemented.
     * Method HasAttribute has to be called before trying to retrieve an attribute
     * to make sure the attribute with name 'symbol' exists (else, an assertion in the code fails).
     * In the case the attribute contains pointers on other attributes, the attribute copy
     * constructor has to be defined.
     * Example of use of attributes (with n an attributed object)
     *       IntegerAttribute ia(6);		// Declare an integer attribute
     *       n.SetAttribute("myinteger",ia);    // Attach a copy of the attribute to object n
     *
     *       IntegerAttribute ia;               // Declare an attribute
     *	     assert(n.HasAttribute("myinteger");// Make sure n has an attribute attached
     *       ia=n.GetAttribute("myinteger");    // Retrieve a copy of the attribute in ia
     *       int val = ia.GetValue();           // Get it's value (here, a simple integer)
     */	
    Attribute &GetAttribute(std::string symbol) ;

    //TP
    /*! return every symbols used in the attribute map */
    std::vector<string> getAttributeList(void);
    void CloneAttributesFor (Attributed*, CloneHandle&);
    
    
    /*! Add or replace a named attribute. A pointer to a copy
     * of the attribute is stored.
     * In case an already existing attribute is replaced,
     * the attribute is deleted before the new one is
     * installed. */
    void SetAttribute(std::string const& symbol, Attribute &attribute) ;
    
    /*! Remove an attribute (frees its memory) 
     * (if not removed, an attribute stays attached and consumes memory up
     * to the program termination)
     */
    void RemoveAttribute(std::string const& symbol) ;

    /*! Print information on the non serialisable attributes, by calling
     * their Print method. Used for debug only, to check that all
     * NonSerialisableAttributes are removed at the end of every analysis.
     */
    void PrintNonSerialisableAttributes(std::ostream& os);
    
    /*! Serialise all attributes */
    std::ostream& WriteXmlAttributes(std::ostream& os,
				     Handle& hand_ser) const ;

    /*! Unserialise all attributes */
    void ReadXmlAttributes(XmlTag const* tag, 
			   Handle& hand) ;
    
    /*! virtual destructor. */
    virtual ~Attributed();
  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_ATTRIBUTED_H
