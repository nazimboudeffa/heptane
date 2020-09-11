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

// ---------------------------------------------
// 
//  Template for an analysis, to avoid
//  rewriting the same code again and again
//
// ---------------------------------------------

#ifndef ANALYSIS_HELPER_H
#define ANALYSIS_HELPER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>
#include "CfgLib.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"

// Useful analysis-independent types
// ---------------------------------
typedef unsigned long t_address;
#define INVALID_ADDRESS 0xffffffff

// Useful functions
// ----------------
typedef bool t_node_function (Cfg * c, Node *, void *param);
typedef bool t_node_function_six (Cfg * c, Node *, void *param, string in, string out, string type_analysis);

// Name of internal attribute used by node scanning function applyToAllNodesRecursive
#define VisitedInternalAttributeName "visited"


/**
 * Useful functions that help to implement many analyses
 *
 * - check program properties for WCET estimation
 * - attachement of contexts
 * - functions to apply the same function to every node in a cfg
 * - function to return the start address of a basic block
 *
 *  All methods are static, the program they operate on is given as a parameter
 * 
 */
class AnalysisHelper
{

 public:

  /** Constructor */
  AnalysisHelper ()
    {  };

  /** Destructor */
  virtual ~ AnalysisHelper ();

  /** Check program for WCET analyzability (no recursion, bounded loops, etc, see exact list in Analysis.cc) */
  static void ProgramCheck (Program * p);


  /** Compute context for each cfg like main#1#foo for the foo function
      called in main by the first call node.      
      The caller node has the call_number and the next node in the same cfg has the call_next_number.

      It first resets the shared attributes by calling resetContext(), required for the entry-points management.
  */
  static bool computeContext (Program * p);

  /** Returns the start address of a basic block */
  static t_address getStartAddress (Node * n);

  /** Apply function f to every node of CFG c. The parameter param is
      passed to function f. This method is not recursive. Whenever
      a node n in CFG c is a call to another CFG, f is called on n but
      not on the callees nodes. For a recursive application of function
      f, call method applyToAllNodesRecursive.
      applyToAllCfgNodes returns true if all applicatins of f return true,
      it returns false otherwise. Function f is called on all nodes
      regardless of the return code 
  */
  static bool applyToAllCfgNodes (Cfg * c, t_node_function * f, void *param);

  /** Call function f for every node of the entry point
      of program p and every node of any callee
      Function f is called once per node
      Returns false if one of the function returns false
      This method follows the CFG edges to determine which
      nodes are reachable. Therfore, function f
      should not modify the control flow. */
  static bool applyToAllNodesRecursive (Program * p, t_node_function * f, void *param);

  /** Compute the backedges for a program p.  */
  static set < Edge * > compute_backedges(Program * p, CallGraph *call_graph);

  /** return "true" if at least one caller of context c is in a loop, "false" otherwise */
  static bool CallerInLoop(Context * c);

  /** The attribute named attrName is assigned to all the instructions of a node (n) with the value A.*/
  static void AttributeAllInstructions(Node * n, string attrName, SerialisableStringAttribute A);

  /** @return "true" if all the instructions of a node (n) are attibuted by attrName, "false" otherwise */
  static bool checkInstructionsAttributed(Node * n, string attrName);

  /** Contextual attribute Names.
      @return the string <attr> + "#" + <contextName> */
   static string mkContextAttrName(string attr, string context);


   /** Contextual attribute Names.
       @return the string <attr> + "#" + nameOf(context) */
   static string mkContextAttrName(string attr, Context *context);



   /** @return the value of the attribute ( f(attr, contextName)) assigned to an instruction (instr).*/
   static int getInstrValueAttr(Instruction *instr, string attr, string contextName);

   /** @return the value of the attribute ( f(attr, contextName)) assigned to an edge (e).*/
   static int getEdgeValueAttr(Edge & e, string attr, string contextName);

   /** @return the value of the attribute ( f(attr, contextName)) assigned to a node (n).*/
   static int getNodeValueAttr(Node *n, string attr, string contextName);

   /** Contextual attribute Names.*/
   static string mkEdgeVariableNameSolver(string prefix, long source, long target, string vcontext );

   /** Contextual attribute Names.
       @return the symbol prefix_v_c_ic (name of the  variables used for the solvers)  */
   static string mkVariableNameSolver(string prefix, long v, string vcontext);

   /** Contextual attribute Names.
       @return the attribute name of the Frequency in a context (vcontext).*/
   static string  getContextAttrFrequencyName(string vcontext);

   static unsigned int getInstrIntAttr(Instruction* vinstr, string attrName, Context * context );
   static string getInstrStringAttr(Instruction* vinstr,  string attrName, Context *context);

   /** returns a copy of vid without identifier duplication. */
   static vector < string > unicity(vector < string > &vid);


   // Data flow anaylsis.
 static bool FilterBackedge(Node * current, Node *pred, set < Edge * >&backedges);
  
  /** Insert in vSet the contextual successors of vContNode, ignoring the backedges*/
 static void insertContextualSuccessorsExcludingBackEdges(ContextualNode  &current, set < ContextualNode > &vSet, set < Edge * >& backedges);

  /** Insert in vSet the contextual successors of vContNode. */
 static void insertContextualSuccessors(ContextualNode & vContNode, set < ContextualNode > &vSet);

  /**  @retnru the initial contextual node of the program.
       (ie first context of the entry point of the program, the start node of the entry point) */
 static set < ContextualNode > initWork( );
  
  /** Debugging: it prints the code of the instructions of a set of contextual nodes (work). */
 static void printSet(set < ContextualNode > &work, string vcaller);

 /** @return true if returnNode has a predecessor (previous_call) a call */
 static bool getCallerNode(Node *returnNode, Node **previous_call);

 private:

  /** Resets the shared attributes of a program (p): the contextual ones (removeContextualAttributes()) 
      and the no-contextual ones (StackInfoAttributeName, ContextListAttributeName, WCETAttributeName).
  */
  static void resetContext(Program * p);
  
  /** Resets the contextual shared attribute (atrName) assigned to the edges of a program (p).*/
  static void removeContextualEdgesAttribute (Program * p, string attrName);

  /** Resets the contextual shared attribute (atrName) assigned to the nodes of a program (p).*/
  static void removeContextualNodesAttribute (Program * p, string attrName);

  /** Resets the contextual shared attribute (atrName) assigned to the instructions of a program (p).*/
  static void removeContextualInstructionsAttribute (Program * p, string attrName);

  /** Removes the contextual attributes assigned to a program(p) */
  static void removeContextualAttributes(Program * p);

  /** Removes the context management for FrequencyAttributeName (currently not managed as the other contextual attributes). */
  static void removeFrequencyAttribute(Program * p);

};

#endif
