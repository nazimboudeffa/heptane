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

/* #includes and forward declarations. */
#include "dominatorData.h"
#include <string>
#include <set>
#include <cassert>
#include <iostream>

/** this namespace is the global namespace */
namespace cfglib
{
  /** Attribute interface. */

  /** Constructor */
  DominatorData::DominatorData ()
  { }

  /** virtual constructor. @return a copy of the current objet. */
  DominatorData *DominatorData::clone ()
  {
    return new DominatorData (*this);
  }

  /** Print function */
  void DominatorData::Print (std::ostream & os)
  {
    os << "(type=NonSerialisableAttribute,name=" << name << ")";
  }

  /* true if the Node is in this dominator set */
  bool DominatorData::findBB (Node * bb) const
  {
    for (std::vector < Node * >::const_iterator it = this->array_nodes.begin (); it < this->array_nodes.end (); it++)
      if ((*it) == bb) return true;
    return false;
  }

  /* add the Node to the dominator set */
  DominatorData & DominatorData::addBB (Node * b)
  {
    if (b)
      this->array_nodes.push_back (b);
    else
      assert (false);
    return *this;
  }

  /** replace the dominator set by a new list of Nodes.
      */
  DominatorData & DominatorData::setNewListBB (std::set < Node * >&s)
  {
    this->array_nodes.clear ();
    for (std::set < Node * >::iterator it = s.begin (); it != s.end (); it++)
      this->addBB (*it);
    return *this;
  }

  /** @return true if the dominator set is empty, false otherwise. */
  bool DominatorData::isEmpty () const
  {
    return this->array_nodes.empty ();
  }

  /** @return the number of Nodes in the dominator set */
  unsigned int DominatorData::numberOfBB () const
  {
    return this->array_nodes.size ();
  }

  /** begin() iterator on the vector of Nodes */
  std::vector < Node * >::iterator DominatorData::begin ()
  {
    return this->array_nodes.begin ();
  }

  /** end() iterator on the vector of Nodes */
  std::vector < Node * >::iterator DominatorData::end ()
  {
    return this->array_nodes.end ();
  }

}				// cfglib::
