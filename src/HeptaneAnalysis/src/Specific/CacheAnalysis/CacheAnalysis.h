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

/**
 AbstractCacheStateAttribute definition with the names used by the Instruction and Data cache analysis
  */

#ifndef CACHE_ANALYSIS_H
#define CACHE_ANALYSIS_H


/*************************************************************************************************************************
 Names of internal attributes
 **************************************************************************************************************************/

// ACSMAY_in and ACSMAY_out, of type AbstractCacheStateAttribute<MAY>, attached to nodes
#define ACSMAYInName "ACSMAY_in"
#define ACSMAYOutName "ACSMAY_out"

// ACSMUST_in and ACSMUST_out, of type AbstractCacheStateAttribute<MUST>, attached to nodes
#define ACSMUSTInName "ACSMUST_in"
#define ACSMUSTOutName "ACSMUST_out"

// ACSPS_in and ACSPS_out, of type AbstractCacheStateAttribute<PS>, attached to nodes (PS: persistent)
#define ACSPSInName "ACSPS_in"
#define ACSPSOutName "ACSPS_out"


/*************************************************************************************************************************
 AbstractCache state attribute
 **************************************************************************************************************************/

// ------------------------------------------------------
// AbstractCache state attribute
// for now, implements no serialisation function (empty)
// just want to attach the abstract cache states to the nodes,
// not to serialise them
//
// generic type T stands for MUST, MAY or PS (see Cache.h for more details)
//
// -----------------------------------------------------
/**
 * Abstract cache state attribute (attached to CFG nodes)
 */
template < typename T > class AbstractCacheStateAttribute:public NonSerialisableAttribute
{
public:
  /** corresponding AbstractCache<T> */
  AbstractCache < T > cache;

  /** Constructor */
  AbstractCacheStateAttribute < T > (const AbstractCache < T > &ca)
  {
    cache = ca;
  };

  /** Cloning function */
  AbstractCacheStateAttribute < T > *clone ()
  {
    return new AbstractCacheStateAttribute < T > (cache);
  };

  /** Printing function */
  void Print (std::ostream & os)
  {
    os << "(type NonSerialisableAbstractCacheStateAttribute, name " << name << ")";
  };
};

#endif
