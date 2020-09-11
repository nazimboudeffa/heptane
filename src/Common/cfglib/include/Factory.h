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

#ifndef _IRISA_CFGLIB_FACTORY_H
#define _IRISA_CFGLIB_FACTORY_H

/* forward declarations and #includes */
#include <map>
namespace cfglib { class Attribute ; }
namespace cfglib { class AttributeFactory ; }
#include "SerialisableAttributes.h"
#include "PointerAttributes.h"

/* Debug of factory management methods */
// Uncomment one of these two lines to enter/leave debug mode
//#define dbg_factory(x) x
#define dbg_factory(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
  class AttributesFactory {
  private:
    std::map<std::string, SerialisableAttribute*> map ;
    /*! constructor */
    AttributesFactory();
    ~AttributesFactory();

   public:
    
    /*! creation of attribute of a given type. */
    SerialisableAttribute *CreateNewAttribute(std::string const& type) ;
    
    /*! association between a type identifier and a
     * constructor function. */
    void SetAttributeType(std::string const&, SerialisableAttribute*) ;

    /*! Access to global attribute factory */
    static AttributesFactory* GetInstance();

    /*! Declare end of access of attribute factory */
    static void KillInstance();
  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_FACTORY_H
