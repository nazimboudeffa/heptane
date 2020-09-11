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

#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <utility>
#include <set>
#include <map>
#include <string>
#include <cassert>
#include "Specific/IPETAnalysis/Solver.h"
#include "Specific/IPETAnalysis/IPETAnalysis.h"
#include "Utl.h"


/*
  It assigns the frequency (freq) to the node (Basic block) associated with a variable (VariableName).
  The frequency is the result provided by a linear programming solver (cplex or lp_solve) for such a variable.
  The variable is a symbol n_NID_cCNB", where NID is the cCNB is the name of a context (currently it is not the contextual context used in other analysis).
  The names are generated in the method IPETAnalysis::generateNodeIds().
*/
void Solver::setFrequencyAttribute(string VariableName, string freq)
{
  Node *n;
  string ctxName;

  n = analysis->node_ids[VariableName];
  if (n != NULL)
    {
      ctxName = VariableName; // VariableName looks like "n_NID_cCNB"
      ctxName.erase (0, ctxName.find_last_of ("_")); // now, only contains cCNB
      string attr = FrequencyAttributeName + ctxName;
      if (n->HasAttribute (attr))
	{
	  Logger::addFatal ("LpsolveSolver: Variable " + VariableName + " already has a frequency ...");
	}

      SerialisableUnsignedLongAttribute frequency (atol ((char *) freq.c_str()));
      TRACE(cout << "attr = " <<  attr << ", variable = " << VariableName << endl);
      n->SetAttribute (attr, frequency);
    }
}


// Constraint generation functions (specific to lp_solve so far)
// -------------------------------------------------------------

// generate_objective_function MAXIMIZE sum(ids*cst)
//
// - os: stream where to output the constrain system
// - ids: ids of variables in the linear system
// - cst: constant in the linear system
// -------------------------------------------------------------
void
LpsolveSolver::generate_objective_function (ostringstream & os, vector < string > ids, vector < long >cst)
{
  os << "MAX: " << endl;
  assert (ids.size () == cst.size ());
  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    {
      os << cst[i] << "*" << ids[i] << " ";
      if (i < nvars - 1)
	os << " + ";
      else
	os << ";" << endl;
    }
}

// Generate an inequality sum (ids*cst) <= N
// - os: stream where to output the constrain system
// - ids: ids of variables in the linear system
// - cst: constant in the linear system
void
LpsolveSolver::generate_linear_inequality (ostringstream & os, vector < string > ids, vector < long >cst, int N)
{
  assert (ids.size () == cst.size ());
  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    {
      os << cst[i] << "*" << ids[i] << " ";
      if (i < nvars - 1)
	os << " + ";
      else
	os << " <= ";
    }
  os << N << ";" << endl;
}

// generate_declarations(strf,vid)
//
// Declare types of variables as integers (needed by lp_solve)
void
LpsolveSolver::generate_declarations (ostringstream & os, vector < string > ids)
{
  os << "int ";
  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    {
      os << ids[i];
      if (i < nvars - 1)
	{
	  os << ", ";
	  if ( i%10 == 0) os << endl;
	}
      else
	os << ";" << endl;
    }
}

// Generate a flow constraint
// vid[0] = sum(vid[1..n])
void
LpsolveSolver::generate_flow_constraint (ostringstream & os, vector < string > vid)
{
  assert (vid.size () > 0);
  if (vid.size () > 1)
    {
      unsigned int nbedges = vid.size ();
      for (unsigned int i = 1; i < nbedges; i++)
	{
	  os << vid[i];
	  if (i < nbedges - 1)
	    os << " + ";
	}
      os << " - " << vid[0] << " = 0;" << endl;
    }
}

// Generate an inequality
// Sum(vids) <= N
void
LpsolveSolver::generate_inequality (ostringstream & os, vector < string > vid, int N)
{
  assert (vid.size () > 0);
  unsigned int nvar = vid.size ();
  for (unsigned int i = 0; i < nvar; i++)
    {
      os << vid[i];
      if (i < nvar - 1)
	os << " + ";
    }
  os << " <= " << N << ";" << endl;
}

// Generate an inequality
// Sum(vids) = N
void
LpsolveSolver::generate_equality (ostringstream & os, vector < string > vid, int N)
{
  assert (vid.size () > 0);
  unsigned int nvar = vid.size ();
  for (unsigned int i = 0; i < nvar; i++)
    {
      os << vid[i];
      if (i < nvar - 1)
	os << " + ";
    }
  os << " = " << N << ";" << endl;
}

bool
LpsolveSolver::solve (string file_name, string fout)
{

  string lp_solve_command = "lp_solve < " + file_name + " > " + fout;
  if (system (lp_solve_command.c_str ()) != 0)
    {
      stringstream errorstr;
      errorstr << "LpsolveSolver: Error: when executing command: " << lp_solve_command;
      Logger::addFatal (errorstr.str ());
      return false;
    }
  return true;
}

bool
LpsolveSolver::parse_output (string file_name, string & wcet)
{
  bool wcet_found = false;
  ifstream fin;
  fin.open (file_name.c_str ());

  // Scan the result
  char line[256];
  fin.getline (line, 256);
  while (!fin.fail ())
    {
      string s1, s2, s3, s4, s5;
      istringstream streamed_line (line);
      streamed_line >> s1 >> s2 >> s3 >> s4 >> s5;

      if (s1 == "Value" && s2 == "of")
	{
	  wcet_found = true;
	  float fwcet = atof (s5.c_str ());
	  char buf[256];
	  sprintf (buf, "%.0f", fwcet);
	  wcet = string (buf);
	}
      if (analysis->generate_node_frequencies)
	if (s1 != "Actual" && s1 != "Value" && s1 != "" && s2 != "")
	  {
	    // Found the value of basic block frequency (variable name in s1, value in s2)
	    setFrequencyAttribute(s1, s2);
	  }
      fin.getline (line, 256);
    }

  fin.close ();

  if (!wcet_found)
    return false;
  return true;
}

// generate_objective_function MAXIMIZE sum(ids*cst)
//
// - os: stream where to output the constrain system
// - ids: ids of variables in the linear system
// - cst: constant in the linear system
// -------------------------------------------------------------
void
CPLEXSolver::generate_objective_function (ostringstream & os, vector < string > ids, vector < long >cst)
{
  assert (ids.size () == cst.size ());

  os << "enter toto" << endl << endl << "Maximize" << endl << "obj: ";

  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    {
      os << cst[i] << " " << ids[i] << " ";
      if (i < nvars - 1)
	{
	  if (cst[i + 1] >= 0)
	    os << " + ";
	  else
	    os << " ";
	}
      else
	{
	  os << endl;
	}
    }
  os << endl << "Subject To" << endl;
}

// Generate an inequality sum (ids*cst) <= N
// - os: stream where to output the constrain system
// - ids: ids of variables in the linear system
// - cst: constant in the linear system
void
CPLEXSolver::generate_linear_inequality (ostringstream & os, vector < string > ids, vector < long >cst, int N)
{
  assert (ids.size () == cst.size ());
  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    {
      os << cst[i] << " " << ids[i] << " ";
      if (i < nvars - 1)
	{
	  if (cst[i + 1] >= 0)
	    os << " + ";
	  else
	    os << " ";
	}
      else
	{
	  os << " <= ";
	}
    }
  os << N << endl;
}

// generate_declarations(strf,vid)
//
// Declare types of variables as integers (needed by lp_solve)
void
CPLEXSolver::generate_declarations (ostringstream & os, vector < string > ids)
{
  os << "General " << endl;
  unsigned int nvars = ids.size ();
  for (unsigned int i = 0; i < nvars; i++)
    os << ids[i] << endl;

  os << "End" << endl << endl << "optimize" << endl;
}

// Generate a flow constraint
// vid[0] = sum(vid[1..n])
void
CPLEXSolver::generate_flow_constraint (ostringstream & os, vector < string > vid)
{
  assert (vid.size () > 0);
  if (vid.size () > 1)
    {
      unsigned int nbedges = vid.size ();
      for (unsigned int i = 1; i < nbedges; i++)
	{
	  os << vid[i];
	  if (i < nbedges - 1)
	    os << " + ";
	}
      os << " - " << vid[0] << " = 0" << endl;
    }
}

// Generate an inequality
// Sum(vids) <= N
void
CPLEXSolver::generate_inequality (ostringstream & os, vector < string > vid, int N)
{
  assert (vid.size () > 0);
  unsigned int nvar = vid.size ();
  for (unsigned int i = 0; i < nvar; i++)
    {
      os << vid[i];
      if (i < nvar - 1)
	{
	  os << " + ";
	}
    }
  os << " <= " << N << endl;
}

// Generate an inequality
// Sum(vids) = N
void
CPLEXSolver::generate_equality (ostringstream & os, vector < string > vid, int N)
{
  assert (vid.size () > 0);
  unsigned int nvar = vid.size ();
  for (unsigned int i = 0; i < nvar; i++)
    {
      os << vid[i];
      if (i < nvar - 1)
	os << " + ";
    }
  os << " = " << N << endl;
}

bool
CPLEXSolver::solve (string file_name, string fout)
{
  ofstream file;
  file.open (file_name.c_str (), ios_base::app);
  file << "write " << fout << " sol" << endl;
  file.close ();

  string lp_solve_command = "cplex < " + file_name + " > /dev/null";
  if (system (lp_solve_command.c_str ()) != 0)
    {
      stringstream errorstr;
      errorstr << "CPLEXSolver: error when executing command: " << lp_solve_command;
      Logger::addFatal (errorstr.str ());
      return false;
    }
  return true;
}

bool CPLEXSolver::parse_output (string file_name, string & wcet)
{
  string line;
  bool found_wcet, found_SolutionStatus;

  ifstream readfile;
  readfile.open(file_name.c_str(), ios::in);
  if (! readfile.is_open()) return false; // No solution
  
  // Getting the wcet and the "solutionStatusString" 
  found_wcet=false;
  found_SolutionStatus = false;
  while(!readfile.eof() && ( !found_wcet || !found_SolutionStatus))
    {
      getline(readfile,line);
      if(line.find("solutionStatusString") != EOS)
	{
	  if(line.find("integer optimal solution") == EOS)
	    {
	      readfile.close();
	      stringstream errorstr;
	      errorstr << "CPLEXSolver: when parsing file: " << file_name << " solution not correct";
	      Logger::addFatal (errorstr.str ()); 
	      return false;
	    }
	  found_SolutionStatus = true;
	}
      else
	if(line.find("objectiveValue")!= EOS)
	  {
	    wcet = Utl::extractStringValue(line);
	    found_wcet=true;
	  }
    }
  
  assert(found_wcet && found_SolutionStatus);
  
  if (analysis->generate_node_frequencies)
    while(!readfile.eof())
      {
	getline(readfile,line);
	if(line.find("<variables>") != EOS)
	  {
	    while(!readfile.eof())
	      {
		getline(readfile,line);
		if( line.find("<variable name=") != EOS)
		  {
		    // Example: <variable name="x1" index="0" value="40"/>
		    string s1, s2, s3, s4;
		    istringstream streamed_line (line);
		    streamed_line >> s1 >> s2 >> s3 >> s4;
		    setFrequencyAttribute(Utl::extractStringValue(s2),  Utl::extractStringValue(s4));
		  }
	      }
	  }
      }
    
  readfile.close();
  return true;
}
