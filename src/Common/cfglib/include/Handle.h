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

#ifndef _IRISA_CFGLIB_HANDLE_H
#define _IRISA_CFGLIB_HANDLE_H

/*! #includes and forward declarations */
#include <map>
#include <set>
#include "Factory.h"
/*! #includes and forward declarations */
#include "Serialisable.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  /*! this class manipulate unique identifier during
   * unserisalisation */
  class Handle {
  private:

    /*! maps used for unserisalisation*/

    /*! map used to associate an id to a Serialisable*/
    std::map<std::string, Serialisable*> id_serialisable;
    /*! map used to resolve the undefined referenced Serialisable associated to an id*/
    std::map<std::string,std::set<Serialisable**> > id_handle;

    /*! maps used for serisalisation to attribute a unique identifier to a serialisable object*/
    std::map<Serialisable const*, int> identifiers ;
    
  public:
    Handle() ;
    
    /*! Declare a new handle to a Serialisable object. Declared handles
     * will be updated when the method resolveHandles() will be
     * called. The first argument is a string id and the second is a pointer to the
     * memory place of the handle Serialisable*
     * ATTENTION : ptr must be use only with no ordered structure !!! */
    //void decl_node_handle(Node*&, std::string const& id);
    void addID_handle(std::string const& id,Serialisable** ptr);

    /*! Declare a Serialisable and its identifier. Objects are
     * declared by the library, an also by library
     * user. */
    //void decl_node_object(Node*, std::string const&);
    void addID_serialisable(std::string const& , Serialisable*);

    /*! during serialisation give a unique identifier
     * for each Serialisable object. Identity of objects
     * is given by their address (you pass a pointer), be
     * careful to not modify dynamic containers during
     * serialisation. */
    std::string identify(Serialisable const* obj);


    /*! Replace all handles with their final value.
     * not intended for library user, this function is
     * used only by unserialisation predefined routine. */
    void resolveHandles();

    std::string getId(Serialisable const* obj);

  } ;
  

} // cfglib::
#endif // _IRISA_CFGLIB_HANDLE_H
