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

#ifndef ANALYSIS_H
#define ANALYSIS_H

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
#include "Generic/AnalysisHelper.h"

using namespace std;
using namespace cfglib;


/**
 * Common interface of every analysis step
 *
 * Any new analysis step should inherit from this class.
 * This class defines basic methods useful for any
 * class of analysis (eg: scanning of all CFG nodes),
 * as well as methods for the 3 steps of any analysis:
 *
 * - verification of presence of input attributes (CheckInputAttributes)
 * - analysis itself (PerformAnalysis)
 * - removal of non serialisable attributes (RemovePrivateAttributes)
 * 
 * These 3 methods are pure virtual, such that they are implemented
 * in any anaysis deriving from this class.
 */
class Analysis
{
protected:
  /** Program on which the analysis is applied */
  Program * p;
  string name;

public:

  /** Constructor */
  Analysis (Program * pp)
  {
    p = pp;
    name = "Undefined";
  };

  /** Destructor */
  virtual ~ Analysis ();

private:

  // -----------------------------------------------------------------
  // Virtual functions: need to be implemented for a specific analysis
  // -----------------------------------------------------------------

  /** Checks if all required attributes are in the CFG
     Returns true if successful, false otherwise.
     This method must be implemented for any analysis.
   */
  virtual bool CheckInputAttributes () = 0;

  /** Performs the analysis
     Returns true if successful, false otherwise
   */
  virtual bool PerformAnalysis () = 0;

  /** Remove all private attributes */
  virtual void RemovePrivateAttributes () = 0;

public:

  /** Simply calls CheckInputAttributes, PerformAnalysis and RemovePrivateAttributes for the current analysis */
  bool CheckPerformCleanup (bool printTime);
  void setName(string v)  { name = v;};
  string getName()  { return name;}
};

#endif
