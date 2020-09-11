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

/*! #includes and forward declarations */
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <assert.h>
#include "Handle.h"
#include "Helper.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  using namespace cfglib::helper ;

  Handle::Handle()
  { }
  
  /*! Declare a Serialisable and its identifier. Objects are
   * declared by the library, not intended for library
   * user. */
  void Handle::addID_serialisable(std::string const& id, Serialisable* attr){
		assert (id_serialisable.find(id) == id_serialisable.end());
		id_serialisable[id]=attr;

		// Replace known handles with their final values.
		if (id_handle.find(id) != id_handle.end()) {
			set<Serialisable**> handles = id_handle[id];
			for(set<Serialisable**>::iterator itset = handles.begin();itset!=handles.end();itset++){
				Serialisable** ptr = *itset;
				*ptr=attr;
			}
			id_handle.erase(id);
		}
  }

  /*! Declare a new handle to a Serialisable object. Declared handles
   * will be updated when the method resolveHandles() will be
   * called. The first argument is a string id and the second is a pointer to the memory place of the handle Serialisable*
   * ATTENTION : ptr must be use only with no ordered structure !!! */
  void Handle::addID_handle(std::string const& id,Serialisable** ptr){
		if(id_serialisable.find(id) != id_serialisable.end()){
			*ptr=id_serialisable[id];
		} 
		else{
			set<Serialisable**> tmp;

			if(id_handle.count(id)==0){
				tmp = set<Serialisable**>();
				tmp.insert(ptr);
				id_handle[id]=tmp;
			}
			else{
				tmp=id_handle[id];
				tmp.insert(ptr);
				id_handle[id]=tmp;
			}
		}
  }

  /*! put in place all handles with their final value.
   * not intended for library user, this function is
   * used only by unserialisation predefined routine. */
  void Handle::resolveHandles(){

    map<string,set<Serialisable**> >::iterator itmap;
    for(itmap=id_handle.begin();itmap!=id_handle.end();itmap++){
      string currentID = (*itmap).first;
      set<Serialisable**> currentToSolve = (*itmap).second; // the set of handles for the current id
      set<Serialisable**>::iterator itset;
      
      //update of all handles of the current id
      for(itset = currentToSolve.begin();itset!=currentToSolve.end();itset++){
	assert(id_serialisable.count(currentID)!=0);
	Serialisable** ptr = *itset;
	*ptr=id_serialisable[currentID];
      }
    }
  }
  

  /*! during serialisation give a unique identifier
   * for each Serialisable object. Identity of objects
   * is given by their address (you pass a pointer), be
   * careful to not modify dynamic containers during
   * serialisation. */
  std::string Handle::identify(Serialisable const* obj){

    if(identifiers.count(obj)==1) return int_to_string(identifiers[obj]);
    int id = identifiers.size();
    identifiers[obj]=id;
    return int_to_string(id);
  }

  std::string Handle::getId(Serialisable const* obj){
    return int_to_string(identifiers[obj]);
  }
} // cfglib::
