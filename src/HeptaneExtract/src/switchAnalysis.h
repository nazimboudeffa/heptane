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

#ifndef _IRISA_SWITCH_ANALYSIS_H
#define _IRISA_SWITCH_ANALYSIS_H

/* forward declarations and #includes */
namespace cfglib
{
  class Cfg;
}                             // cf. "cfg/cfg.h"
namespace cfglib
{
  class Node;
}				// cf. "cfg/cfg.h"
#include <CfgLib.h>
/* Debug of loop analysis methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_loop(x) x
#define dbg_switch(x)

/** this namespace is the global namespace */
namespace cfglib
{ 
  /** loop analysis : detect natural loops in the cfg and create corresponding Loop objects. */
  class SwitchComputer
  {
  public:
    /** detect natural loops in the cfg and create corresponding Loop objects */
    static  void computeSwitch( Cfg * acfg, vector < t_address> &lswitch, bool isMIPSarchi);
  private:
    static bool GetFirstLastAddresses (cfglib::Node * n, t_address * first, t_address * last);
    static bool GetAddressInstr (cfglib::Instruction * vi, t_address  * addr);
    static int getIndexNodeWithFirstAddr( vector < cfglib::Node * > vn, t_address vaddr);
    static bool getAddressJump(Node * n, t_address *vaddr);
  };
}    // cfglib::
#endif
