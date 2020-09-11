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
  for all attributes shared by the different analysis steps

*/

#include "Config.h"

#ifndef SHARED_ATTRIBUTES_H
#define SHARED_ATTRIBUTES_H

/** Include all attribute definitions for attributes shared between the CFG extractor and the WCET analyzer.
    The other attributes, defined above, are used by the WCET analyzer only */
#include "GlobalAttributes.h"

// Context list attribute
#define ContextListAttributeName "ContextList"
#define ContextTreeAttributeName "ContextTree"
#include "Generic/Context.h"


// StackInfoAttribute
#define StackInfoAttributeName	"stackinfo"
#include "Specific/DataAddressAnalysis/StackInfoAttribute.h"


/** Cache categorization attribute prefix
 * value = AH,AM,FH,FM,AU
 * -----------------------------
 * produced by CacheAnalysis and DataCacheAnalysis
 * and used by IPETcalculation
 *
 * The final attribute name is of the form
 * CHMCLxCode or CHMCLxData with x the
 * cache level (from 1 to the last level cache)
 */

// Attribute name
#define CHMCAttributeName "CHMC"

// Macros to build attribute names
#define CHMCAttributeNameCode(l) string(string(CHMCAttributeName)+string("L") +	\
					static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Code"))
#define CHMCAttributeNameData(l) string(string(CHMCAttributeName)+string("L") +	\
					static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Data"))

// Attribute type
// StringAttribute (directly supported by CFGLIB)

///////////////////////////////////////////////////////////////


/** Cache access attribute prefix
 * value = Always, Never or Unknown
 * -----------------------------
 * produced by ICacheAnalysis/DCacheAnalysis and used by next level CacheAnalysis
 * 
 * CACAttributeName is only a prefix, the final attribute
 * name is CACLxCode or CACLxData with x the cache level
 */

#define CACAttributeName "CAC"

// Macros to build attribute names
#define CACAttributeNameCode(l) string(string(CACAttributeName)+string("L") + \
				       static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Code"))
#define CACAttributeNameData(l) string(string(CACAttributeName)+string("L") + \
				       static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Data"))


/** Cache age attribute
 * value = INT 
 * Representing the age in the
 * cache of the access
 * -----------------------------
 * produced by ICacheAnalysis
 *
 * AGEAttributeName is only a prefix, the final attribute
 * name is AGELxCode_y with x the cache level and y the
 * cache analysis (MUST or PS)
 *
 * Rq1: The attribute is attached only if the reference is in the cache
 * Rq2: The attribute for the PS analysis is attached only if the attribute
 *      is not attached by the MUST analysis
 *
 * TODO: to be defined for data also
 */

#define AGEAttributeName "AGE"

#define AGEMustAttributeNameCode(l) string(string(AGEAttributeName)+string("L") + \
					   static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Code_MUST"))

#define AGEPSAttributeNameCode(l) string(string(AGEAttributeName)+string("L") + \
					 static_cast<ostringstream*>( &(ostringstream() << (l)))->str() + string("Code_PS"))

// With bypass
//#define BeforeBPattribute "BeforeBP#"
//#define DataBeforeBPattribute "BeforeBP#Data"


/** Data Accessed Block Count attribute prefix
 * ------------------------------------
 *  produced by DataCacheAnalysis and used by IPETanalysis
 *
 * this attribute name is only a prefix, the final attribute
 * name is DataBlockCountLx with x the cache level
 */
#define DataAccessedBlockCountAttributeName "DataBlockCount"

#define BlockCountAttributeName(l) string(string(DataAccessedBlockCountAttributeName)+string("L") + \
					  static_cast<ostringstream*>( &(ostringstream() << (l)))->str())

/** Frequency attribute
 * ---------------------
 *
 * Frequency of execution of nodes along WCEP
 * Currently, attached to nodes only by IPET computation
 */

// Attribute name
#define FrequencyAttributeName "frequency"


// Attribute type
// UnsignedLongAttribute (directly supported by CFGLIB)

/** WCET attribute
 * ---------------------
 *
 * WCET of entry point
 */

// Attribute name
#define WCETAttributeName "WCET"

// Attribute type
// BinIntegerAttribute (directly supported by CFGLIB)

/** Hit ratio attribute
 * ---------------------
 *
 * Hit ratio, attached to the entry point by IPET estimation
 */

// Attribute name
#define HitRatioAttributeName "HitRatio"

// Attribute type
// SerialisableFloatAttribute (directly supported by CFGLIB)


/** Code line attribute
 * ---------------------
 *
 * gives the location of an instruction in a source file.
 *
 */
#define CodeLineAttributeName "CodeLine"
#include "Specific/CodeLine/CodeLineAttribute.h"

/** Timing attribute for pipeline analysis
 * ----------------------------------------
 * 
 */

#define NodeExecTimeFirstAttributeName "NodeExecTimeFirst"
#define NodeExecTimeNextAttributeName "NodeExecTimeNext"
#define DeltaFFAttributeName "DeltaFF"
#define DeltaFNAttributeName "DeltaFN"
#define DeltaNFAttributeName "DeltaNF"
#define DeltaNNAttributeName "DeltaNN"
#define CallDeltaFirstAttributeName "CallDeltaFirst"
#define CallDeltaNextAttributeName "CallDeltaNext"
#define ReturnDeltaFirstAttributeName "ReturnDeltaFirst"
#define ReturnDeltaNextAttributeName "ReturnDeltaNext"


#endif // SHARED_ATTRIBUTES_H
