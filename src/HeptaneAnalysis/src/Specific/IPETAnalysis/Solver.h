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

#ifndef IPET_SOLVER_H
#define IPET_SOLVER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "SharedAttributes/SharedAttributes.h"
// #include <libxml/parser.h>  removed because it induces "memory leaks".

using namespace std;
class IPETAnalysis;

/**
   Encapsulation of ILP solver (lp_solve and CPLEX so far)
*/
class Solver
{
 protected:
  IPETAnalysis * analysis;
 public:
  friend class IPETAnalysis;
  Solver (IPETAnalysis * a):analysis (a)
  { };
  virtual ~ Solver ()
  { };

  /** generate_objective_function MAXIMIZE sum(ids*cst)
    - os: stream where to output the constrain system
    - ids: ids of variables in the linear system
    - cst: constant in the linear system
  */
  virtual void generate_objective_function (ostringstream & os, vector < string > ids, vector < long >cst) = 0;

  /** generate_declarations(strf,vid)
      Declare types of variables as integers (needed by lp_solve).
  */
  virtual void generate_declarations (ostringstream & os, vector < string > ids) = 0;

  /** Generate a flow constraint: vid[0] = sum(vid[1..n]) */
  virtual void generate_flow_constraint (ostringstream & os, vector < string > vid) = 0;

  /** Generate an inequality : Sum(vids) <= N */
  virtual void generate_inequality (ostringstream & os, vector < string > vid, int N) = 0;

  /** Generate an inequality sum (ids*cst) <= N
      - os: stream where to output the constrain system
      - ids: ids of variables in the linear system
      - cst: constant in the linear system
  */
  virtual void generate_linear_inequality (ostringstream & os, vector < string > vid, vector < long >cst, int N) = 0;

  /** Generate an inequality: Sum(vids) = N */
  virtual void generate_equality (ostringstream & os, vector < string > vid, int N) = 0;

  /** Solve the constraint system */
  virtual bool solve (string file_name, string fout) = 0;

  /** Parse solver output */
  virtual bool parse_output (string file_name, string & wcet) = 0;

  
  /** It assigns the frequency (freq) to the node (Basic block) associated with a variable (VariableName).
      The frequency is the result provided by a linear programming solver (cplex or lp_solve) for such a variable.
      The variable is a symbol n_NID_cCNB", where NID is the cCNB is the name of a context (currently it is not the contextual context used in other analysis).
      The names are generated in the method IPETAnalysis::generateNodeIds().
  */
  void setFrequencyAttribute(string VariableName, string freq);
};

/**
 * lp_solve ILP solver
 */
class LpsolveSolver:public Solver
{
 public:
  LpsolveSolver (IPETAnalysis * a):Solver (a)
  { };

  ~LpsolveSolver ()
  { };
  void generate_objective_function (ostringstream & os, vector < string > ids, vector < long >cst);
  void generate_declarations (ostringstream & os, vector < string > ids);
  void generate_flow_constraint (ostringstream & os, vector < string > vid);
  void generate_inequality (ostringstream & os, vector < string > vid, int N);
  void generate_linear_inequality (ostringstream & os, vector < string > vid, vector < long >cst, int N);
  void generate_equality (ostringstream & os, vector < string > vid, int N);
  bool solve (string file_name, string fout);
  bool parse_output (string file_name, string & wcet);
};

/**
 * CPLEX ILP solver
 */
class CPLEXSolver:public Solver
{
 public:
  CPLEXSolver (IPETAnalysis * a):Solver (a)
  { };

  ~CPLEXSolver ()
  { };
  void generate_objective_function (ostringstream & os, vector < string > ids, vector < long >cst);
  void generate_declarations (ostringstream & os, vector < string > ids);
  void generate_flow_constraint (ostringstream & os, vector < string > vid);
  void generate_inequality (ostringstream & os, vector < string > vid, int N);
  void generate_linear_inequality (ostringstream & os, vector < string > vid, vector < long >cst, int N);
  void generate_equality (ostringstream & os, vector < string > vid, int N);
  bool solve (string file_name, string fout);
  bool parse_output (string file_name, string & wcet);
};

#endif
