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

#ifndef LOOP_TREE_H
#define LOOP_TREE_H

#include <assert.h>
#include "Loop.h"
#include "Program.h"
#include "Node.h"
#include "Cfg.h"
#include "Helper.h"

/**
 * Node in LoopTree
 */
class LoopTreeNode {
private:
  cfglib::Cfg *cfg;   // Cfg of the loop tree
  cfglib::Loop *loop; // Loop (NULL for the root of the tree)
  vector<LoopTreeNode*> subloops;
public:
  LoopTreeNode (cfglib::Cfg *c, cfglib::Loop* l);
  ~LoopTreeNode();
  void buildSubTree(vector<cfglib::Loop*> &vl);
  void Print(int indent);

  // Get number of subloops
  unsigned int getNbSubloops() {return subloops.size();};

  // Get nth subloop
  LoopTreeNode* getSubloop(unsigned int i) const
    {if (i<subloops.size()) return subloops[i]; else return NULL;};

  // Get the Cfg where the loop is located
  cfglib::Cfg *getCfg() {return cfg;}

  // Get the loop
  cfglib::Loop *getLoop() {return loop;}
  
};

/**
 * LoopTree structure (tree of LoopTreeNode)
 */
class LoopTree {
private:
  vector<LoopTreeNode*> loopTree;
public:
 
  // Build the loop nest tree
  LoopTree(cfglib::Program &p);
  
  // Remove everything
  ~LoopTree();
  

  // Get number of elements
  unsigned int getNbTrees() {return loopTree.size();};

  // Get nth loop tree
  LoopTreeNode* getLoopTree(unsigned int i) {
    if (i<loopTree.size()) return loopTree[i]; else return NULL;};

  // Print
  void Print();
};

#endif
