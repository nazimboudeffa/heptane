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

#include "Analysis.h"
#include "Generic/CallGraph.h"


static CallGraphElem *
findInElems (vector < CallGraphElem * >elems, Cfg * c)
{
  for (unsigned int i = 0; i < elems.size (); i++)
    {
      if (elems[i]->cfg == c)
	return elems[i];
    }
  return NULL;
}

/** Returns true if there is a cycle in the callgraph starting from node 'cfg' */
bool
CallGraphElem::detectCycles (Cfg * c)
{
  this->visited = true;
  for (unsigned int j = 0; j < this->callees.size (); j++)
    {
      if (this->callees[j]->visited && (this->callees[j]->cfg == c))
	return true;
      if (!this->callees[j]->visited)
	{
	  if (this->callees[j]->detectCycles (c))
	    return true;
	}
    }
  return false;
}

/** Returns true if cfg 'cfg' calls (directly or indirectly) cfg 'c' */
bool
CallGraphElem::Calls (Cfg * c)
{
  for (unsigned int i = 0; i < callees.size (); i++)
    {
      if (callees[i]->cfg == c)
	return true;
    }
  for (unsigned int i = 0; i < callees.size (); i++)
    {
      if (callees[i]->Calls (c))
	return true;
    }
  return false;
}


/** Called to detect function calls in a newly allocated
  CallGraphElem e.  e is already inserted in the list of
  CallGraphElem 'elems'. This function scans the nodes of e->cfg to
  detect function calls. If function calls are found, new
  CallGraphElem are inserted in 'elems' if not already present. In
  this case, BuildFromCallGraphElem is recursively called for the
  callee. */
void
CallGraph::BuildFromCallGraphElem (CallGraphElem * e)
{
  Cfg *c = e->cfg;
  vector < Node * >vn = c->GetAllNodes ();
  for (unsigned int i = 0; i < vn.size (); i++)
    {
      Node *n = vn[i];
      if (n->IsCall ())
	{
	  Cfg *callee = n->GetCallee ();
	  assert (callee != NULL);
	  bool external = callee->IsExternal ();
	  // Insert if not already present in the CallGraph structure
	  CallGraphElem *inelem = findInElems (e->callees, callee);
	  if (inelem == NULL)
	    {
	      CallGraphElem *enew = new CallGraphElem (callee);
	      (e->callees).insert ((e->callees).end (), enew);
	      if (findInElems (elems, callee) == NULL)
		{
		  elems.insert (elems.end (), enew);
		  if (external == false)
		    this->BuildFromCallGraphElem (enew);
		}
	    }
	}
    }
}

/** Constructor. Initializes the callgraph structure, which is not
  modified after. Calgraph cyclicity is detected during callgraph
  construction. */
CallGraph::CallGraph (Program * p)
{
  // Build call graph
  Cfg *c = config->getEntryPoint ();
  this->root = c;
  CallGraphElem *el = new CallGraphElem (c);
  elems.insert (elems.end (), el);
  this->BuildFromCallGraphElem (el);

  // Test the cyclicity
  cyclic = false;
  for (unsigned int e = 0; e < elems.size (); e++)
    {
      // Reset visited bit
      for (unsigned int ve = 0; ve < elems.size (); ve++)
	elems[ve]->visited = false;
      // Recursively test for cycles
      if (elems[e]->detectCycles (elems[e]->cfg))
	{
	  cyclic = true;
	  break;
	}
    }
}

/** Destructor. */
CallGraph::~CallGraph ()
{
  for (unsigned int e = 0; e < elems.size (); e++)
    {
      delete elems[e];
    }
}

/** Cyclicity test. Simply returns 'cyclic', initialized when
  constructing the callgraph */
bool
CallGraph::isCyclic ()
{
  return cyclic;
}


/** Dead code detection. Scans the call graph structure to check if
    cfg 'c' belongs to vector 'elems' */
bool
CallGraph::isDeadCode (Cfg * c)
{
  assert (cyclic == false);
  if (c == root) return false;
  for (unsigned int i = 0; i < elems.size (); i++)
    {
      if (elems[i]->Calls (c))
	return false;
    }
  return true;
}

/** Text printing of callgraph structure + callgraph cyclicity
  of standard output */
void
CallGraph::Print ()
{
  for (unsigned int e = 0; e < elems.size (); e++)
    {
      //      string caller = elems[e]->cfg->GetName ();
      string caller = elems[e]->cfg->getStringName ();
      if (elems[e]->callees.size () != 0)
	{
	  for (unsigned int i = 0; i < elems[e]->callees.size (); i++)
	    {
	      cout << "Function " << caller << " calls function " << (elems[e]->callees)[i]->cfg->getStringName /*GetName*/ () << endl;
	    }
	}
      else
	cout << "Function " << caller << " is a leave " << endl;
    }

  if (cyclic)
    cout << "(Call graph is cyclic)" << endl;
  else
    cout << "(Call graph is acyclic)" << endl;
}
