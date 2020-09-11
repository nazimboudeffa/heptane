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
#include "CloneHandle.h"
#include "Helper.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  CloneHandle::CloneHandle()
  {
		clones[NULL] = NULL; 
	}

	CloneHandle::~CloneHandle() { assert(pending_requests.empty());}
  
  void CloneHandle::RegisterClone(void* object, void* clone)
	{
		assert (clones.count(object) == 0);
		clones[object] = clone;
		this->ResolvePendingRequests (object);
  }

	void CloneHandle::ResolveClone (void* object, void** ptr) const
	{
		if (clones.count (object) == 1) {
			(*ptr) = clones.find(object)->second;
		}
		else {
			pending_requests[object].insert (ptr);
		}
	}

	bool CloneHandle::HasRegisteredClone (void* object) const
	{
		return (clones.count(object) != 0);
	}

	void* CloneHandle::GetClone (void* object) const
	{
		assert (clones.count(object));
		return clones.find(object)->second;
	}


	void CloneHandle::ResolvePendingRequests (void* object) const
	{
		assert (clones.count(object) == 1);
		if (pending_requests.count (object)) {
			void* clone = clones.find(object)->second;
			set<void**>& request_ptrs = pending_requests[object];
			
			for (set<void**>::iterator ptr_it = request_ptrs.begin();
				   ptr_it != request_ptrs.end();
					 ++ptr_it)
			{
				void** ptr = *ptr_it;
				(*ptr) = clone;
			}

		  pending_requests.erase (object);
		}
	}


} // cfglib::
