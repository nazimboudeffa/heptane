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
#include "LoopTree.h"
#include "Specific/SimplePrint/SimplePrint.h"
#include "SharedAttributes/SharedAttributes.h"

// ----------------------
// SimplePrint class
// ----------------------

SimplePrint::SimplePrint (Program * p, bool printcallgraph, bool printloopnest, bool printcfg, bool printWCET):Analysis (p)
  {
    outputcallgraph = printcallgraph;
    outputloopnest = printloopnest;
    outputcfg = printcfg;
    outputwcetinfo = printWCET;
  };


// ----------------------------------------------------------------
// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
// Here, we simply check that there is a cfg to print
// ----------------------------------------------------------------
bool
SimplePrint::CheckInputAttributes ()
{
  Cfg *c = config->getEntryPoint ();
  assert (c != NULL);
  assert (!(c->IsEmpty ()));
  return true;
}

void SimplePrint::RemovePrivateAttributes ()
{ };



// -------------------------------------------
// Displays the contents of one node and
// prints the name of its successors in the CFG
// -------------------------------------------
bool
displayNode (Cfg * c, Node * n, void *param)
{

  if (n->IsCall ())
    {
      // Function call node
      cout << "------------------------------" << endl;
      cout << "Function call node";
      Cfg *callee = n->GetCallee ();
      assert (callee != NULL);
      cout << ", callee = " << callee->getStringName/*GetName*/();
      bool external = callee->IsExternal ();
      if (external)
	cout << " (external)" << endl;
      else
	cout << " (non external)" << endl;
    }

  else
    {
      // Get the corresponding BB and display it
      cout << "------------------------------" << endl;
      cout << "Basic block node" << endl;
    }

  // Print the node address
  cout << "Node address ";
  t_address add = AnalysisHelper::getStartAddress (n);
  cout << hex << add << endl;
  cout << "Node instructions" << endl;
  vector < Instruction * >vi = n->GetAsm ();
  for (unsigned int i = 0; i < vi.size (); i++)
    {
      cout << "    " << vi[i]->GetCode ();
      if (vi[i]->HasAttribute (AddressAttributeName))
	{
	  AddressAttribute attr = (AddressAttribute &) vi[i]->GetAttribute (AddressAttributeName);
	  t_address add = attr.getCodeAddress ();
	  cout << "\t address " << hex << add << endl;
	}
      else
	cout << endl;
    }

  // Print its successors in the CFG
  vector < Node * >sucs = c->GetSuccessors (n);
  if (sucs.size () != 0)
    cout << "Successors" << endl;
  else
    cout << "No successors" << endl;
  for (unsigned int i = 0; i < sucs.size (); i++)
    {
      if (sucs[i]->IsBB ())
	{
	  t_address addsuc = AnalysisHelper::getStartAddress (sucs[i]);
	  cout << "    Basic block of address " << hex << addsuc << endl;
	}
      else
	{
	  cout << "    Function call node" << endl;
	}
    }
  // Print its predecessors in the CFG
  vector < Node * >preds = c->GetPredecessors (n);
  if (preds.size () != 0)
    cout << "Predecessors" << endl;
  else
    cout << "No predecessors" << endl;
  for (unsigned int i = 0; i < preds.size (); i++)
    {
      if (preds[i]->IsBB ())
	{
	  t_address addpred = AnalysisHelper::getStartAddress (preds[i]);
	  cout << "    Basic block of address " << hex << addpred << endl;
	}
      else
	{
	  cout << "    Function call node" << endl;
	}
    }
  return true;
}

// ----------------------------------------------
// Performs the analysis
// Returns true if successful, false otherwise
// ----------------------------------------------
bool
SimplePrint::PerformAnalysis ()
{

  if (outputcallgraph)
    {
      CallGraph c (p);
      c.Print ();
    }

  if (outputloopnest)
    {
      LoopTree lt (*p);
      lt.Print ();
    }

  if (outputcfg)
    {
      vector < Cfg * >lc = p->GetAllCfgs ();
      for (unsigned int c = 0; c < lc.size (); c++)
	{
	  cout << "****************************************" << endl;
	  cout << "CFG of name " << lc[c]->getStringName() << endl;
	  vector < Node * >vn = lc[c]->GetEndNodes ();
	  cout << vn.size () << " end nodes" << endl;
	  if (AnalysisHelper::applyToAllCfgNodes (lc[c], displayNode, NULL) == false)
	    return false;
	}
    }

  if (outputwcetinfo)
    {
      string WCET;
      Cfg *c = config->getEntryPoint ();
      if (c->HasAttribute (WCETAttributeName))
	{
	  SerialisableStringAttribute ba = (SerialisableStringAttribute &) c->GetAttribute (WCETAttributeName);
	  WCET = ba.GetValue ();
	}
      else
	WCET="-1 (attach_WCET_info seems to be false. The value of the WCET is not available)";
      
      cout << "WCET: "  << WCET << endl;	
    }
  return true;
}


