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
#include "Specific/DotPrint/DotPrint.h"
#include "SharedAttributes/SharedAttributes.h"


// -----------------------------------------------
// BackEdge: helper function to determine if an
// edge is a back edge
// NB: this function is not efficiently implemented
//     because the library does not allow to know
//     quickly is an edge is a back-edge or not
// -----------------------------------------------
static bool
BackEdge (Cfg * c, Node * s, Node * d)
{

  Edge *e = c->FindEdge (s, d);
  assert (e != NULL);
  vector < Loop * >vl = c->GetAllLoops ();

  for (unsigned int l = 0; l < vl.size (); l++)
    {
      vector < Edge * >vbe = vl[l]->GetBackedges ();
      for (unsigned int be = 0; be < vbe.size (); be++)
	{
	  if (vbe[be] == e)
	    {
	      return true;
	    }
	}
    }
  return false;
}

/*
  Temporary add-on (for DATE 2012 paper, Isabelle)
  Kept here not to loose the code

#include "Specific/CRPD/CRPDfi.h"
static void displayCRPD(Cfg *c, Node *n, ofstream& os) 
{
  // Add-on: display CRPD
  if (n->HasAttribute("CRPD_final")) 
    {
      NonSerialisableIntegerAttribute crpda = (NonSerialisableIntegerAttribute&)n->GetAttribute("CRPD_final");
      int crpd = crpda.GetValue();
      os << " : " << dec << crpd ;
    }
  // Display execution time interval
  const ContextList& contexts = (ContextList&) c->GetAttribute(ContextListAttributeName);
  unsigned int nc = contexts.size();
  for (unsigned int ic=0;ic<nc;ic++) 
    {
      Context *ctx = contexts[ic];    
      if (n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx))) 
	{
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx)));
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMaxName, ctx)));
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(StartMinName,ctx)));
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(StartMaxName,ctx)));
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(EndMinName,ctx)));
	assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(EndMaxName,ctx)));
	NonSerialisableIntegerAttribute a_execmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx));
	NonSerialisableIntegerAttribute a_execmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(ExecMaxName,ctx));
	NonSerialisableIntegerAttribute a_startmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(StartMinName, ctx));
	NonSerialisableIntegerAttribute a_startmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(StartMaxName, ctx));
	NonSerialisableIntegerAttribute a_endmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(EndMinName,ctx));
	NonSerialisableIntegerAttribute a_endmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(EndMaxName, ctx));
	os << "\\nExec" << ctx->getStringId() << " [" << dec << a_execmin.GetValue() << "," << a_execmax.GetValue() << "] ";
	os << "\\nInterval" << ctx->getStringId() << " [" << dec << a_startmin.GetValue() << "," << a_startmax.GetValue() << "," 
	   << a_endmin.GetValue() << "," << a_endmax.GetValue() << "] ";
	}
      // Display frequency and execution time info from IPET
      char tmp[80];
      sprintf(tmp,"%d",ic);
      
      string freqAttr=AnalysisHelper::getContextAttrFrequencyName(contexts[ic]->getStringId());
      if (n->HasAttribute(freqAttr)) {
	SerialisableUnsignedLongAttribute freq_a = (SerialisableUnsignedLongAttribute&) n->GetAttribute(freqAttr);
	os << "\\nFreq" << ctx->getStringId() << " " << freq_a.GetValue();
      }
      if (n->HasAttribute(AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, ctx)))
	{
	  NonSerialisableIntegerAttribute exec_first_a = (NonSerialisableIntegerAttribute&) n->GetAttribute(AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, ctx));
	  NonSerialisableIntegerAttribute exec_next_a = (NonSerialisableIntegerAttribute&) n->GetAttribute(AnalysisHelper::mkContextAttrName(InternalAttributeWCETfirst, ctx));
	  assert(exec_first_a.GetValue() == exec_next_a.GetValue());
	  os << " Dur" << ctx->getStringId() << " " << exec_first_a.GetValue();
	}
      if (n->HasAttribute(InternalAttributeId)) 
	{
	  NonSerialisableIntegerAttribute attr_id = (NonSerialisableIntegerAttribute&) n->GetAttribute(InternalAttributeId);
	  os << "  Nid" << attr_id.GetValue();
	}
    }
  // End of Addon
}

static void displayCRPDLoop(Cfg *c, Loop *n, ofstream& os) 
{
  // Display execution time interval
  const ContextList& contexts = (ContextList&) c->GetAttribute(ContextListAttributeName);
  unsigned int nc = contexts.size();
  for (unsigned int ic=0;ic<nc;ic++) 
    {
      Context *ctx = contexts[ic];    
      if (n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx))) 
	{
	  assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx)));
	  assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(ExecMaxName, ctx)));
	  assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(StartMinName, ctx)));
	  assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(StartMaxName, ctx)));
	  assert(n->HasAttribute(AnalysisHelper::mkContextAttrName(EndMinName, ctx)));
	  assert(n->HasAttribute(AnalysisHelper::(EndMaxName, ctx)));
	  NonSerialisableIntegerAttribute a_execmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(ExecMinName, ctx));
	  NonSerialisableIntegerAttribute a_execmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::mkContextAttrName(ExecMaxName, ctx));
	  NonSerialisableIntegerAttribute a_startmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::(StartMinName,ctx));
	  NonSerialisableIntegerAttribute a_startmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::(StartMaxName,ctx));
	  NonSerialisableIntegerAttribute a_endmin = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::(EndMinName, ctx));
	  NonSerialisableIntegerAttribute a_endmax = (NonSerialisableIntegerAttribute&)n->GetAttribute(AnalysisHelper::(EndMaxName, ctx));
	  os << "\\nExec" << ctx->getStringId() << " [" << dec << a_execmin.GetValue() << "," << a_execmax.GetValue() << "] ";
	  os << "\\nInterval" << ctx->getStringId() << " [" << dec << a_startmin.GetValue() << "," << a_startmax.GetValue() << "," 
	     << a_endmin.GetValue() << "," << a_endmax.GetValue() << "] ";
	}
    }
  // End of Addon
}
*/

// -------------------------------------------
// Displays the contents of one node and
// prints the name of its successors in the CFG
// -------------------------------------------
static bool
displayNode (Cfg * c, Node * n, ofstream & os)
{

  if (n->HasAttribute (InternalAttributeNameOK))
    {
      os << "node" << n << ";" << endl;
      return true;		// Node already treated
    }

  os << "node" << n << " [label = \"";

  // Node address and type
  t_address add = AnalysisHelper::getStartAddress (n);
  if (add != INVALID_ADDRESS)
    {
      os << "@0x" << hex << add << " ";
    }
  if (n->IsBB ())
    {
      os << "(BB)";
    }
  else
    {
      os << "(Call ";
      Cfg *callee = n->GetCallee ();
      assert (callee != NULL);
      os << callee->getStringName/*GetName*/ ();
      if (callee->IsExternal ())
	os << ": external)";
      else
	os << ")";
    }
  // Add-on: display CRPD information
  // displayCRPD(c,n,os);
  // os <<"\\n";
  NonSerialisableIntegerAttribute OK (0);
  n->SetAttribute (InternalAttributeNameOK, OK);

  os << "\"";

  Node *entry = c->GetStartNode ();
  if (entry == n)
    os << " ,color=\"blue\"";

  // Display loops heads in green
  vector < Loop * >vl = c->GetAllLoops ();
  for (unsigned int i = 0; i < vl.size (); i++)
    {
      Node *head = vl[i]->GetHead ();
      if (head == n)
	{
	  os << " ,color=\"green\"";
	  break;
	}
    }

  os << "];" << endl;

  return true;
}

static bool
displaySucs (Cfg * c, Node * n, ofstream & os)
{

  // Print its successors in the CFG
  vector < Node * >sucs = c->GetSuccessors (n);
  for (unsigned int i = 0; i < sucs.size (); i++)
    {
      os << "node" << n << " -> " << "node" << sucs[i];
      if (BackEdge (c, n, sucs[i]))
	os << "[color=\"red\"]";
      os << ";" << endl;
    }
  return true;
}

static bool
displayLoop (Cfg * c, Loop * l, ofstream & os, vector < Loop * >&vl)
{

  // Print the loop if there is no loop in the list above the loop in the list
  if (l != NULL)
    {
      for (unsigned int nl = 0; nl < vl.size (); nl++)
	{
	  if (vl[nl] != l)
	    {
	      if (l->IsNestedIn (vl[nl]))
		{
		  return true;
		}
	    }
	}
    }

  // Remove the loop from the list of loops to be printed
  for (std::vector < Loop * >::iterator it = vl.begin (); it != vl.end (); it++)
    {
      if ((*it) == l)
	{
	  vl.erase (it);
	  break;
	}
    }

  // Print the loop nodes
  if (l != NULL)
    {
      vector < Node * >vn = l->GetAllNodes ();
      os << "subgraph " << "cluster_loop" << l << " {" << endl;
      int maxiter;
      SerialisableIntegerAttribute ai = (SerialisableIntegerAttribute &) l->GetAttribute (MaxiterAttributeName);
      maxiter = ai.GetValue ();
      os << "graph [label = \"loop [" << dec << maxiter << "]";
      // displayCRPDLoop(c,l,os);
      os << "\"];" << endl;
      for (unsigned int i = 0; i < vn.size (); i++)
	{
	  displayNode (c, vn[i], os);
	}
    }

  // Display subloops of loop l
  vector < Loop * >viter = vl;

  for (unsigned int nl = 0; nl < viter.size (); nl++)
    {
      bool stillthere = false;
      for (unsigned int tmp = 0; tmp < vl.size (); tmp++)
	if (vl[tmp] == viter[nl])
	  stillthere = true;
      if (stillthere && viter[nl]->IsNestedIn (l))
	{
	  displayLoop (c, viter[nl], os, vl);
	}
    }

  if (l != NULL)
    os << "}" << endl;

  return true;
}


static void
displayCfg (Cfg * c, ofstream & os)
{
  os << "subgraph cluster_" << c->getStringName() << " {" << endl;
  os << "graph [label = \"" << c->getStringName() << "\"];" << endl;

  vector < Loop * >vl = c->GetAllLoops ();
  displayLoop (c, NULL, os, vl);
  assert (vl.size () == 0);

  vector < Node * >vn = c->GetAllNodes ();
  for (unsigned int i = 0; i < vn.size (); i++)
    {
      displayNode (c, vn[i], os);
    }
  os << "}" << endl;
}

static void
displayAllSucs (ofstream & os, Program * p)
{
  vector < Cfg * >lc = p->GetAllCfgs ();
  for (unsigned int c = 0; c < lc.size (); c++)
    {
      vector < Node * >vn = lc[c]->GetAllNodes ();
      for (unsigned int i = 0; i < vn.size (); i++)
	{
	  displaySucs (lc[c], vn[i], os);
	}
    }
}


// ----------------------
// DotPrint class
// ----------------------

DotPrint::DotPrint (Program * p, string dir):Analysis (p)
  {
    directory = dir;
  };

// ----------------------------------------------------------------
// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
// Here, nothing specific to do 
// ----------------------------------------------------------------
bool
DotPrint::CheckInputAttributes ()
{
  return true;
}

// ----------------------------------------------
// Performs the analysis
// Returns true if successful, false otherwise
// ----------------------------------------------
bool
DotPrint::PerformAnalysis ()
{
  CallGraph * call_graph = new CallGraph(p);
  Cfg * currentCfg;

  string filename = this->directory + "/" + p->GetName () + ".dot";
  ofstream os (filename.c_str ());
  os << "digraph G {" << endl;
  vector < Cfg * > lc = p->GetAllCfgs ();

  for (unsigned int c = 0; c < lc.size (); c++)
    {
      currentCfg = lc[c];
      if (! call_graph->isDeadCode(currentCfg))
	displayCfg (currentCfg, os);
    }
  displayAllSucs (os, p);
  os << "}" << endl;
  os.close ();
  // Isabelle: changed format to a pdf output, was not managing colors
  // properly with jpg export on version 2.32 (default color seemed to be white, ...)
  string command = "dot -Tpdf " + filename + " -o " + this->directory + "/" + p->GetName () + ".pdf";
  system (command.c_str ());
  return true;
}

// Remove all private attributes
void
DotPrint::RemovePrivateAttributes ()
{
  vector < Cfg * >lc = p->GetAllCfgs ();
  for (unsigned int c = 0; c < lc.size (); c++)
    {
      vector < Node * >ln = lc[c]->GetAllNodes ();
      for (unsigned int n = 0; n < ln.size (); n++)
	if (ln[n]->HasAttribute (InternalAttributeNameOK))
	  ln[n]->RemoveAttribute (InternalAttributeNameOK);
    }
}
