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

#ifndef _IRISA_CFGLIB_UNSER_ATTRIBUTES_H
#define _IRISA_CFGLIB_UNSER_ATTRIBUTES_H

/* #includes and forward declarations. */
#include <string>
#include "Attributes.h"

/* Debug of attribute management methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_attr(x) x
#define dbg_attr(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
	
  /** NonSerialisableAttribute are local attributes that are not
   * expected to be serialised in XML files. Ther are built-in
   * NonSerialisableAttribute (integers, unisgned long, string,
   * BigInteger). New non serialisable attributes can be defined by
   * inheriting from class NonSerialisableAttribute */
  class NonSerialisableAttribute : public Attribute{
  };

  /** NonSerialisableStringAttribute is a library-supported non
      serialisable attribute of type string */
  class NonSerialisableStringAttribute : public NonSerialisableAttribute {
  private:
    std::string value;
  public:
    /*! Constructor */
    NonSerialisableStringAttribute(std::string const& val){value=val;};
    /*! default constructor. */
    NonSerialisableStringAttribute() {};    
    /*! virtual constructor */
    virtual NonSerialisableStringAttribute* clone() {
      return new NonSerialisableStringAttribute(*this);
    };
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&s) {
      s << "(type=NonSerialisableString,name="<<name<<",value="<<value<<")";
    };
    /*! get std::string value of the String Attribute */
    std::string GetValue() {return value;}
    /*! set the value of the String attribute */
    void SetValue(std::string val) {value = val;};
  } ;

  /** NonSerialisableIntegerAttribute is a library-supported non
      serialisable attribute of type integer */
  class NonSerialisableIntegerAttribute : public NonSerialisableAttribute {
  private:
    int value;
  public:
    /*! Constructor */
    NonSerialisableIntegerAttribute(int val){value=val;};
    /*! default constructor. */
    NonSerialisableIntegerAttribute() {};
    /*! virtual constructor */
    virtual NonSerialisableIntegerAttribute* clone()  {
      return new NonSerialisableIntegerAttribute(*this);
    };
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&s) {
      s << "(type=NonSerialisableInteger,name="<<name<<",value="<<value<<")";
    };
    /*! get int value of the Integer Attribute */
    int GetValue() {return value;}
    /*! set the value of the Integer attribute */
    void SetValue(int v) {value = v;};
  } ;

  /** NonSerialisableFloatAttribute is a library-supported non
      serialisable attribute of type float */
  class NonSerialisableFloatAttribute : public NonSerialisableAttribute {
  private:
    float value;
  public:
    /*! Constructor */
    NonSerialisableFloatAttribute(float val){value=val;};
    /*! default constructor. */
    NonSerialisableFloatAttribute() {};
    /*! virtual constructor */
    virtual NonSerialisableFloatAttribute* clone()  {
      return new NonSerialisableFloatAttribute(*this);
    };
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&s) {
      s << "(type=NonSerialisableFloat,name="<<name<<",value="<<value<<")";
    };
    /*! get int value of the Integer Attribute */
    float GetValue() {return value;}
    /*! set the value of the Integer attribute */
    void SetValue(float v) {value = v;};
  } ;

  /** NonSerialisableUnsignedLongAttribute is a library-supported non
      serialisable attribute of type unsigned long */
  class NonSerialisableUnsignedLongAttribute: public NonSerialisableAttribute {
  private:
    unsigned long value;
  public:
    /*! Constructor */
    NonSerialisableUnsignedLongAttribute(unsigned long val) {value=val;};  
    /*! default constructor. */
    NonSerialisableUnsignedLongAttribute() {};
    /*! virtual constructor */
    virtual NonSerialisableUnsignedLongAttribute* clone() {
      return new NonSerialisableUnsignedLongAttribute(*this);
    };
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&s) {
      s << "(type=NonSerialisableUnsignedLong,name="<<name<<",value="<<value<<")";
    };
    /*! get int value of the UnsignedLong Attribute */
    unsigned long GetValue() {return value;}
    /*! set the value of the UnsignedLong Attribute */
    void SetValue(unsigned long v) {value = v;};
  } ;
} // cfglib::
#endif // _IRISA_CFGLIB_UNSER_ATTRIBUTES_H
