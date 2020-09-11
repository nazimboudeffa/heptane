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

#ifndef _IRISA_LOOP_ANALYSIS_H
#define _IRISA_LOOP_ANALYSIS_H

/* forward declarations and #includes */
namespace cfglib
{
  class Cfg;
}                             // cf. "cfg/cfg.h"
namespace cfglib
{
  class Node;
}				// cf. "cfg/cfg.h"
namespace cfglib
{
  class Loop;
}				// cf. "cfg/loop.h"

/** this namespace is the global namespace */
namespace cfglib
{ 
  /** loop analysis : detect natural loops in the cfg and create corresponding Loop objects. */
  class LoopComputer
  {
  public:
    /** Detect natural loops in the cfg and create corresponding Loop objects */
    static void computeLoop (Cfg * cfg);
  private:
    /** Recursively add the predecessors to the loop */
    static void computeLoop (Loop * loop, Cfg * cfg, Node * current);

    static void loop_check(Loop * loop, Node *node);
    static void computeLoop_old(Cfg * cfg); // old version of computeLoop().
  };
}

#endif
