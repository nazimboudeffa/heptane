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

#ifndef _IRISA_DOMINATOR_ANALYSIS_H
#define _IRISA_DOMINATOR_ANALYSIS_H

/* forward declarations and #includes */
#include <vector>
#include <set>
#include <list>
namespace cfglib
{
  class Cfg;
}
namespace cfglib
{
  class Node;
}

/* Debug of domination management methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_dom(x) x
#define dbg_dom(x)

/** this namespace is the global namespace */
namespace cfglib
{
  namespace helper
  {
    /** domination management */
    class DominatorComputer
    {
    public:
      /** \todo{MISSING FUNCTION API DOCUMENTATION"} */
      static void computeDominator (Cfg * cfg);

      // void compute (Program * prog);
    private:
      static bool equalDominator (Node * bb, std::set < Node * >&TempSet);
    };
  }
} // cfglib::
#endif // _IRISA_DOMINATOR_ANALYSIS_H
