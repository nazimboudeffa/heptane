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

#ifndef _IRISA_DOMINATORDATA_H
#define _IRISA_DOMINATORDATA_H

/* #includes and forward declarations. */
#include "CfgLib.h"
#include <string>
#include <set>
namespace cfglib
{
  class Node;
}				// cf. "cfg/node.h"

/* Dominator attribute name */
#define DominatorAttributeName "DOMINATOR"

/** this namespace is the global namespace */
namespace cfglib
{
  /** Attribute interface. */
  class DominatorData:public NonSerialisableAttribute
  {
  private:
    std::vector < Node * >array_nodes;

  public:
    /** default constructor. */
    DominatorData ();

    /** virtual constructor */
    virtual DominatorData *clone ();

    /** virtual constructor */
    void Print (std::ostream & os);

    /** true if the Node is in this dominator set */
    bool findBB (Node * bb) const;

    /** add the Node to the dominator set. WARNING: this
	function do not verify if the Node is yet in the set.*/
    DominatorData & addBB (Node * b);

    /** replace the dominator set by a new list of Nodes */
    DominatorData & setNewListBB (std::set < Node * >&s);

    /** true if the dominator set is empty */
    bool isEmpty () const;

    /** number of Nodes in the dominator set */
    unsigned int numberOfBB () const;

    /** begin() iterator on the vector of Nodes */
    std::vector < Node * >::iterator begin ();

    /** end() iterator on the vector of Nodes */
    std::vector < Node * >::iterator end ();

  };
}				// cfglib::
#endif				// _IRISA_DOMINATORDATA_H
