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

#ifndef _IRISA_CFGLIB_CLONE_HANDLE_H
#define _IRISA_CFGLIB_CLONE_HANDLE_H

/*! #includes and forward declarations */
#include <map>
#include <set>
#include "Factory.h"
/*! #includes and forward declarations */
#include "Serialisable.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  /*! this class manipulate clone of objects during
   * cloning. Used to update clone of pointers to the clone of their original
	 * target.*/
  class CloneHandle {
  private:

		/*! map used to asociate an object and its clone. */
		std::map<void*, void*> clones;

		/*! map used to store pending references to clones of an object. */
		mutable std::map<void*, set<void**> > pending_requests;

		void ResolvePendingRequests (void* object) const;
    
  public:
    CloneHandle() ;
		~CloneHandle();

		/*! Declare a new cloneable object and its corresponding clone.*/
		void RegisterClone (void* object, void* clone);
		/*! Declare a new handle to a cloneable object. */
		void ResolveClone  (void* object, void** ptr) const;

		/*! Search for the clone of a cloneable object. */
		bool HasRegisteredClone (void* object) const;
		/*! Find the clone of a cloneable object. Null if it does not exist yet.*/
		void* GetClone (void* object) const;

  } ;
  

} // cfglib::
#endif // _IRISA_CFGLIB_HANDLE_H
