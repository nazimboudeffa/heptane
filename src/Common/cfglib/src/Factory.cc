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

/* header file */
#include "Factory.h"

/* forward declarations and #includes */
#include <cassert>
#include <map>
#include <iostream>
#include "Attributes.h"

/*! this namespace is the global namespace */
namespace cfglib 
{
  /*! Global attribute factory */
  AttributesFactory *global_factory = NULL;

  /* constructor. initialise this factory with basic types
   * `string`, `integer` `unsignedlong` and `attribute_list`. */
  AttributesFactory::AttributesFactory() {
    SetAttributeType("string", new SerialisableStringAttribute()) ;
    SetAttributeType("integer", new SerialisableIntegerAttribute()) ;
    SetAttributeType("float", new SerialisableFloatAttribute()) ;
    SetAttributeType("unsignedlong", new SerialisableUnsignedLongAttribute()) ;
    SetAttributeType("list", new SerialisableListAttribute()) ;
    REGISTER_POINTER_ATTRIBUTE(Cfg, SerialisableCfgAttribute);
    REGISTER_POINTER_ATTRIBUTE(Node, SerialisableNodeAttribute);
    REGISTER_POINTER_ATTRIBUTE(Loop, SerialisableLoopAttribute);
    REGISTER_POINTER_ATTRIBUTE(Instruction, SerialisableInstructionAttribute);
  }

  AttributesFactory::~AttributesFactory() {
    std::map<std::string, SerialisableAttribute*>::iterator i;
    for (i = map.begin(); i != map.end(); ++i) {
      delete i->second;
    }
  }

  SerialisableAttribute *AttributesFactory::CreateNewAttribute(std::string const& type) {
    std::map<std::string, SerialisableAttribute* >::const_iterator it(this->map.find(type));
    if (it != this->map.end()) 
      return (it->second)->create();
    std::cerr << "AttributesFactory::CreateNewAttribute failed because " << "type \"" << type << "\" " << "is not recognised" << std::endl ;
    return 0;
  }
  
  void AttributesFactory::SetAttributeType( std::string const& type, SerialisableAttribute* constructor) 
  {
    std::map<std::string, SerialisableAttribute*>::iterator it(this->map.find(type)) ;
    if (this->map.end() != it) {	
      this->map.erase(it) ; 
    }
    this->map[type] = constructor;
  }
  
  /*! Access to global attribute factory */
  AttributesFactory* AttributesFactory::GetInstance() {
    if (global_factory==NULL) {
      global_factory=new AttributesFactory();
    }
    return global_factory;
  }

  /*! Declare end of access of attribute factory */
  void AttributesFactory::KillInstance() {
    if (global_factory!=NULL) delete global_factory;
  }

} // cfglib::
