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

#ifndef _IRISA_CFGLIB_ATTRIBUTES_H
#define _IRISA_CFGLIB_ATTRIBUTES_H

/* #includes and forward declarations. */
#include <string>
#include "Serialisable.h"

/* Debug of attribute management methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_attr(x) x
#define dbg_attr(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
  class Handle;
	class CloneHandle;
	
  /** Attributes can be attached to any object inheriting from class
   * Attributed. Built-in attributes are provided by cfglib, and new
   * ones can be defined by its users. Every attribute has a name and
   * a type. An attribute should implement a method clone (attributes
   * are duplicated when attached) and method Print for debugging
   * purposes. Attributes can be serialisable (subclass
   * SerialisableAttribute) or not (subclass
   * NonSerialisableAttribute). */
  class Attribute {
  private:
  protected:
    /*! There is only one attribute with a given name attached to an
        Attributed object. */
    std::string name ;
  public:
    
    /*! Default constructors and destructors */
    Attribute() {};
    virtual ~Attribute(){};

    /*! Attribute cloning function. Used by attribute attachment
     * method SetAttribute of class Attributed when attaching an
     * attribute to an object deriving from class Attributed */
    virtual Attribute* clone() = 0 ;
		virtual Attribute* clone(CloneHandle&) { return this->clone(); };
    
    /*! Attribute printing function (used for debug only) */
    virtual void Print(std::ostream&) = 0;

    /*! Set the unique "name" field in a list of Attribute.  This
     * methos is called during the unserialisation process and not
     * used otherwise */
    void SetName(std::string name)
    {	this->name = name ; }
  } ;
  
} // cfglib::
#endif // _IRISA_CFGLIB_ATTRIBUTES_H
