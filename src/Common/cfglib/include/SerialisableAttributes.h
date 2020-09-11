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

#ifndef _IRISA_CFGLIB_SER_ATTRIBUTES_H
#define _IRISA_CFGLIB_SER_ATTRIBUTES_H

/* #includes and forward declarations. */
#include <string>
#include <list>
#include "Serialisable.h"
#include "Attributes.h"

/* Debug of attribute management methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_attr(x) x
#define dbg_attr(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
  class Handle;
	
  /*! Attribute interface. */
  class SerialisableAttribute : public Attribute, public Serialisable {
  public:
    /*! Atrribute factory */
    virtual SerialisableAttribute *create() = 0 ; 
  } ;
  
  class SerialisableStringAttribute : public SerialisableAttribute {
  private:
    std::string value;
  public:
    /*! Constructor */
    SerialisableStringAttribute() {};
    SerialisableStringAttribute(std::string const& val) : value(val){};
    
    /*! default constructor. */
    ~SerialisableStringAttribute() {};
    
    /*! virtual constructor */
    virtual SerialisableStringAttribute* clone() ;
    
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&);

    /*! Serialisation function */
    std::ostream& WriteXml( std::ostream&, Handle&);
    
    /*! deserialisation function */
    virtual void ReadXml(XmlTag const*, cfglib::Handle&) ;
    
    /*! get std::string value of the String Attribute */
    std::string GetValue() {return value;};

    /*! set the value of the String attribute */
    void SetValue(std::string v) {value =v;};

    /*! Factory */
    SerialisableStringAttribute *create(); 
  } ;

  class SerialisableIntegerAttribute : public SerialisableAttribute {
  private:
    int value;
  public:
    /*! Constructor */
    SerialisableIntegerAttribute() {};
    SerialisableIntegerAttribute(int val): value(val){};

    /*! default constructor. */
    ~SerialisableIntegerAttribute() {};

    /*! virtual constructor */
    SerialisableIntegerAttribute* clone() ;

    /*! Attribute printing function (debug only) */
    void Print(std::ostream&);

    /*! Serialisation function */
    std::ostream& WriteXml( std::ostream&, Handle&) ;
    
    /*! deserialisation function */
    virtual void ReadXml(XmlTag const*, cfglib::Handle&) ;
    
    /*! get int value of the Integer Attribute */
    int GetValue() {return value;};
    
    /*! set the value of the Integer attribute */
    void SetValue(int v) {value = v;};

    /*! Factory */
    SerialisableIntegerAttribute *create(); 
  } ;
 
  class SerialisableFloatAttribute : public SerialisableAttribute {
  private:
    float value;
  public:
    /*! Constructor */
    SerialisableFloatAttribute() {};
    SerialisableFloatAttribute(float val): value(val){};

    /*! default constructor. */
    ~SerialisableFloatAttribute() {};

    /*! virtual constructor */
    SerialisableFloatAttribute* clone() ;

    /*! Attribute printing function (debug only) */
    void Print(std::ostream&);

    /*! Serialisation function */
    std::ostream& WriteXml( std::ostream&, Handle&) ;
    
    /*! deserialisation function */
    virtual void ReadXml(XmlTag const*, cfglib::Handle&) ;
    
    /*! get int value of the Integer Attribute */
    float GetValue() {return value;};
    
    /*! set the value of the Integer attribute */
    void SetValue(float v) {value = v;};

    /*! Factory */
    SerialisableFloatAttribute *create(); 
  } ;
 
  class SerialisableUnsignedLongAttribute : public SerialisableAttribute {
  private:
    unsigned long value;
  public:
    /*! Constructor */
    SerialisableUnsignedLongAttribute() {};
    SerialisableUnsignedLongAttribute(unsigned long val): value(val){};
    
    /*! default constructor. */
    ~SerialisableUnsignedLongAttribute() {} ;
    
    /*! cloning function */
    SerialisableUnsignedLongAttribute* clone() ;
    
    /*! Attribute printing function (debug only) */
    void Print(std::ostream&);

    /*! Serialisation function */
    std::ostream& WriteXml( std::ostream&, Handle&);
    
    /*! deserialisation function */
    virtual void ReadXml(XmlTag const*, cfglib::Handle&) ;
    
    /*! get int value of the UnsignedLong Attribute */
    unsigned long GetValue() {return value;};

    /*! set the value of the UnsignedLong Attribute */
    void SetValue(unsigned long v) {value =v;};

     /*! Factory */
    SerialisableUnsignedLongAttribute *create(); 
  } ;
 
  class SerialisableListAttribute : public SerialisableAttribute {
    private:
	  list<SerialisableAttribute*> value;
	public:
	  /*! Constructor */
	  SerialisableListAttribute(){value=list<SerialisableAttribute*>();};
	  
	  SerialisableListAttribute(SerialisableListAttribute &v);
	  
	  SerialisableListAttribute(list<SerialisableAttribute*> l){
	    value=l;
	  };
	  
	  /*! default constructor. */
	  ~SerialisableListAttribute(){
	    for(list<SerialisableAttribute*>::iterator iter=value.begin();iter!=value.end();iter++){
		  delete (*iter);
		}
	  };
  
      /*! virtual constructor */
      SerialisableListAttribute* clone() ;
			SerialisableListAttribute* clone(CloneHandle& handle);
	  
	  /*! Attribute printing function (debug only) */
      void Print(std::ostream&);

	  /*! Serialisation function */
      std::ostream& WriteXml( std::ostream&, Handle&);
    
      /*! deserialisation function */
      virtual void ReadXml(XmlTag const*, cfglib::Handle&) ;
  
  
      /*! get int value of the SerialisableList Attribute */
      list<SerialisableAttribute*> GetValue() {return value;};
    
      /*! set the value of the SerialisableList attribute */
      void SetValue(list<SerialisableAttribute*> v) {
	    value = v;
	  };
  
      /*! Factory */
      SerialisableListAttribute *create();
	  

  
  };

} // cfglib::
#endif // _IRISA_CFGLIB_SER_ATTRIBUTES_H
