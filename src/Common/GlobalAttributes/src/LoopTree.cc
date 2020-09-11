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

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include "LoopTree.h"

using namespace cfglib;

LoopTreeNode::LoopTreeNode (Cfg * c, Loop * l)
{
  loop = l;
  cfg = c;
}

LoopTreeNode::~LoopTreeNode ()
{
  for (unsigned int i = 0; i < this->subloops.size (); i++)
    {
      delete subloops[i];
    }
}

void
LoopTreeNode::buildSubTree (vector < Loop * >&vl)
{


  // Remove the loop from the list of loops
  for (std::vector < Loop * >::iterator it = vl.begin (); it != vl.end (); it++)
    {
      if ((*it) == loop)
	{
	  vl.erase (it);
	  break;
	}
    }

  // Create the subloops
  // NB: using a copy of vl, because vl is modified by recursive calls that may
  // modify vl ...
  vector < Loop * >viter = vl;
  for (unsigned int nl = 0; nl < viter.size (); nl++)
    {

      bool stillthere = false;
      for (unsigned int tmp = 0; tmp < vl.size (); tmp++)
	if (vl[tmp] == viter[nl])
	  stillthere = true;

      if (viter[nl]->IsNestedIn (loop) && stillthere)
	{

	  // Insert if there are no loop between vl[nl] and loop
	  bool insert = true;
	  for (unsigned int nup = 0; nup < vl.size (); nup++)
	    {
	      if (viter[nl] != vl[nup] && viter[nl]->IsNestedIn (vl[nup]))
		{
		  insert = false;
		}
	    }
	  if (insert)
	    {
	      LoopTreeNode *subnode = new LoopTreeNode (cfg, viter[nl]);
	      this->subloops.push_back (subnode);
	      subnode->buildSubTree (vl);
	    }
	}
    }
}

void
LoopTreeNode::Print (int indent)
{
  // Print the node itself

  for (int id = 0; id < indent; id++)
    cout << " ";
  if (loop == NULL)
    {
      // cout << "ROOT of CFG " << cfg->GetName() << endl; replaced by (LBesnard)
      cout << "ROOT of CFG " << cfg->getStringName () << endl;
    }
  else
    {
      if (loop->HasAttribute ("maxiter"))
	{
	  SerialisableIntegerAttribute ii = (SerialisableIntegerAttribute &) loop->GetAttribute ("maxiter");
	  cout << "Loop of maxiter " << dec << ii.GetValue () << endl;
	}
      else
	cout << "Loop without maxiter" << endl;
    }
  // Print the loop subloops
  for (unsigned int i = 0; i < this->subloops.size (); i++)
    {
      this->subloops[i]->Print (indent + 2);
    }
}

// Returns true if two loops have an empty intersection
static bool
EmptyIntersect (Loop * l1, Loop * l2)
{
  vector < Node * >vn1 = l1->GetAllNodes ();
  vector < Node * >vn2 = l2->GetAllNodes ();
  for (unsigned int i1 = 0; i1 < vn1.size (); i1++)
    {
      for (unsigned int i2 = 0; i2 < vn2.size (); i2++)
	{
	  if (vn1[i1] == vn2[i2])
	    return false;
	}
    }
  return true;
}

// Print
void
LoopTree::Print ()
{
  for (unsigned int i = 0; i < loopTree.size (); i++)
    {
      loopTree[i]->Print (0);
    }
}


// Remove everything
LoopTree::~LoopTree ()
{
  for (unsigned int i = 0; i < loopTree.size (); i++)
    {
      delete loopTree[i];
    }
}

// Build the loop nest tree
LoopTree::LoopTree (Program & p)
{

  vector < Cfg * >lcfg = p.GetAllCfgs ();

  // Check loops are properly nested
  for (unsigned int i = 0; i < lcfg.size (); i++)
    {
      Cfg *c = lcfg[i];
      vector < Loop * >vl = c->GetAllLoops ();
      for (unsigned int l1 = 0; l1 < vl.size (); l1++)
	{
	  for (unsigned int l2 = 0; l2 < vl.size (); l2++)
	    {
	      if (!vl[l1]->IsNestedIn (vl[l2]) && !vl[l2]->IsNestedIn (vl[l1]) && !EmptyIntersect (vl[l1], vl[l2]))
		{
		  cerr << "Warning: loops are not properly nested. Don't trust the pictures!..." << endl;
		  cout << "Warning: loops are not properly nested. Don't trust the pictures!..." << endl;
		}
	    }
	}
    }

  for (unsigned int i = 0; i < lcfg.size (); i++)
    {
      // Build the loop tree for cfg number i
      LoopTreeNode *tree_i = new LoopTreeNode (lcfg[i], NULL);
      loopTree.push_back (tree_i);
      vector < Loop * >vl = lcfg[i]->GetAllLoops ();
      tree_i->buildSubTree (vl);
      assert (vl.size () == 0);	// Loops should not remain after tree construction
    }
}
