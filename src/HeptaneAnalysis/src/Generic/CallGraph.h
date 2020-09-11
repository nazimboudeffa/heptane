/* -----------------------------------------------------------------------------------
   Copyright IRISA, 2003-2017

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

   ---------------------------------------------------------------------------------- */

#ifndef CALL_GRAPH_H
#define CALL_GRAPH_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Analysis.h"

using namespace std;
using namespace cfglib;

/**
   Node in call graph Defines a cfg ('cfg')+ list of CallGraphElem for
   functions called by this Cfg ('callees'). There is one unique
   instance of CallGraphElem per Cfg that is directly or indirectly
   called by the program entry point. Every called function appears
   only once in the vector of callees 'callees', regardless of the
   number of call points from 'cfg' to 'callee'.
*/
class CallGraphElem
{
public:
  /** Flag used for detection of cycles within the callgraph */
  bool visited;

  /** Cfg. There is a single CallGraphElem with a given cfg in the
     callgraph of a program */
  Cfg *cfg;

  /** CallGraphElems for functions called by 'cfg' */
    vector < CallGraphElem * >callees;

  /** Constructor */
    CallGraphElem (Cfg * c)
  {
    cfg = c;
    visited = false;
  };

  /** Destructor */
  ~CallGraphElem ()
  {
  };

  /** Returns true if cfg 'cfg' calls (directly or indirectly) cfg 'c' */
  bool Calls (Cfg * c);

  /** Returns true if there is a cycle in the callgraph starting from node 'cfg' */
  bool detectCycles (Cfg * c);
};

/**
   Call graph.

   Flat structure of CallGraphElems. There is one CallGraphElem per
   cfg directly or indirectly called by the program entry point.

   Call graph cyclicity is detected when constructing the callgraph in
   the class constructor and reflected in member 'cyclic'. Subsequent
   calls to isCyclic simply return the value of 'cyclic'.
 */
class CallGraph
{
  /** True if the call graph is cyclic. Initialized in the class
     constructor
   */
  bool cyclic;

  /** Pointer to the Cfg of program entry point */
  Cfg *root;

  /** Vector of CallGraphElem, whose size if the number of
     functions directly of indirectly called by the program entry point */
    vector < CallGraphElem * >elems;

    /** Called to detect function calls in a newly allocated
	CallGraphElem e.  e is already inserted in the list of
     CallGraphElem 'elems'. This function scans the nodes of e->cfg to
     detect function calls. If function calls are found, new
     CallGraphElem are inserted in 'elems' if not already present. In
     this case, BuildFromCallGraphElem is recursively called for the
     callee. */
  void BuildFromCallGraphElem (CallGraphElem * e);

public:

  /** Constructor. 
      Initializes the callgraph structure, which is not modified after. 
      Calgraph cyclicity is detected during callgraph construction. */
    CallGraph (Program * p);

  /** Destructor. */
   ~CallGraph ();

   /** Cyclicity test. Simply returns 'cyclic', initialized when constructing the callgraph */
  bool isCyclic ();

  /** Dead code detection. Scans the call graph structure to check if cfg 'c' belongs to vector 'elems' */
  bool isDeadCode (Cfg * c);

  /** Text printing of callgraph structure + callgraph cyclicity of standard output */
  void Print ();
};

#endif
