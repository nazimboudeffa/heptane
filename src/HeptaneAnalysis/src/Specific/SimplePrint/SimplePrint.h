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

#ifndef SIMPLE_PRINT_H
#define SIMPLE_PRINT_H

#include "Analysis.h"

/**
 * Text printing of CFG structure
 */
class SimplePrint:public Analysis
{
  bool outputcallgraph, outputloopnest, outputcfg, outputwcetinfo;
public:

  SimplePrint (Program * p, bool printcallgraph, bool printloopnest, bool printcfg, bool printWCET);
  
  /** Checks if all required attributes are in the CFG
      @return true if successful, false otherwise.
  */
  bool CheckInputAttributes ();

  /** Performs the analysis
      @return true if successful, false otherwise.
  */
  bool PerformAnalysis ();

  /** Remove all private attributes*/
  void RemovePrivateAttributes ();

};

#endif
