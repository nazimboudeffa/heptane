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

/*

  This file contains type definitions and naming conventions
  for all attributes shared by the different analysis binaries
  (CFG extractor and WCETanalysis).

  Other attributes (used locally in the CFG extractor and
  WCET analyzer) should NOT use the same attribute names.

  This file is included in both the CFG extractor and the
  WCET analyzer.

*/

#ifndef GLOBAL_ATTRIBUTES_H
#define GLOBAL_ATTRIBUTES_H

using namespace std;

//////////////////////////////////////////////////////////////

/*! Loop bound attribute
 * ---------------------
 *
 * Maximum number of iterations for loops
 * This attribute is set by the CFG extractor
 * (annotations embedded in source code or external xml file)
 * and is retrieved by any subsequent analysis step
*/

// Attribute name
#define MaxiterAttributeName "maxiter"

/*
 * Address Attributes
 * ------------------
 * Set of addresses acceded by each instruction
 * Attached by the CFG extractor, used by the
 * WCET analyzer
 */

#define AddressAttributeName "address"
#include "AddressAttribute.h"


/*
 * SymbolTable Attribute
 * ------------------
 * Symbol Table information
 * Attached by the CFG extractor, used by the
 * WCET analyzer
 */
#define SymbolTableAttributeName "symbolTable"
#include "SymbolTableAttribute.h"


/*
 * ARMWords Attribute
 * ------------------
 * ARM .word information
 * Attached by the CFG extractor, used by the
 * WCET analyzer
 */
#define ARMWordsAttributeName "ARM_WORDS"
#include "ARMWordsAttribute.h"

/** Attached to ARM to added instruction for the translation of pusp, pop, ldm, stm instructions.
    Attached by the CFG extractor */
#define MetaInstructionAttributeName "MetaInstruction"
#include "MetaInstructionAttribute.h"

#endif // GLOBAL_ATTRIBUTES_H
