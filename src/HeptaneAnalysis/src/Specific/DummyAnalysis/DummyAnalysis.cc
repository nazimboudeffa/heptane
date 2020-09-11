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
			    // #include "Generic/CallGraph.h"
#include "Specific/DummyAnalysis/DummyAnalysis.h"
#include "SharedAttributes/SharedAttributes.h"

// ----------------------
// DummyAnalysis class
// ----------------------

DummyAnalysis::DummyAnalysis (Program * p):Analysis (p)
{ }

// ----------------------------------------------------------------
// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
// Here, we simply check that there is a cfg to print
// ----------------------------------------------------------------
bool
DummyAnalysis::CheckInputAttributes ()
{
  //Stub
  return true;
}

// ----------------------------------------------
// Performs the analysis
// Returns true if successful, false otherwise
// ----------------------------------------------
bool
DummyAnalysis::PerformAnalysis ()
{
  //Stub
  cout << "DummyAnalysis executed" << endl;
  return true;
}

/* Remove all private attributes */
void
DummyAnalysis::RemovePrivateAttributes ()
{ }


