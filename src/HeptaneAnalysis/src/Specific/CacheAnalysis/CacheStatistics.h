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

#ifndef CACHE_STATISTICS_H
#define CACHE_STATISTICS_H

#include "Generic/Analysis.h"
#include "Generic/Config.h"

class CacheStatistics:public Analysis
{
public:
  CacheStatistics (Program *, const map < int, vector < CacheParam * > >&hierarchy_configuration, bool b1, bool b2);

  /** Check the presence of the required CHMC and frequency attributes. */
  bool CheckInputAttributes ();

  /** Compute the number of references, hits and misses on the specified cache level.  */
  bool PerformAnalysis ();

  /** Compute the number of references, hits and misses for a cache (aCache).  */
  void PerformAnalysis ( CacheParam * aCache, const vector < Cfg * >&cfgs, CallGraph &callgraph);

  /** Removes the private attributes*/
  void RemovePrivateAttributes ();
  
  
  /** Count the number of cache levels in the memory hierarchy. */
  int countCacheLevels ();

  /** Return the parameters of the caches at level level in the memory hierarchy. */
    vector < CacheParam * >getCacheLevelParameters (int level);

    /** Return the execution frequency of a node on the WCEP. */
  unsigned long getExecutionFrequency (string context_lid, Node * node);

  /** Return the CHMC of instruction inst. */
  string getCHMC (Context * context, Instruction * inst, CacheParam * cache);


  /** Compute an upper bound on the number of occurrences on the specified cache.*/
  size_t getOccurrencesCount (unsigned long frequency, Context *, Instruction *, CacheParam *);

  /** Count the number of different cache blocks that may be accessed by inst. */
  size_t countAccessedBlocks (Context *, Instruction *, CacheParam * cache);

  /** Check for a cache of the specified type at the specified level. */
  bool hasCache (t_cache_type cache_type, int level);

  /** Return the cache of the specified type at the specified level. NULL if no such cache exists.*/
  CacheParam *getCache (t_cache_type cache_type, int level);

  /** Return printable screen name for cache type. */
  string getCacheTypeName (t_cache_type cache_type);

  /** Defined the vectors has_icache_info and has_dcache_info for all the caches of the configuration.
      - has_icache_info[i] is true if the Icache Analysis has been applied for the i-th instruction cache,
      - has_dcache_info[i] is true if the Dcache Analysis has been applied for the i-th data cache.
      It is used after to print or not the informations of the caches.
  */
  void CheckInstrHaveCacheCategorizationAttribute ();
private:
    Program * program;
    const map < int, vector < CacheParam * > > &hierarchy_configuration;
    vector < bool >has_icache_info;
    vector < bool >has_dcache_info;
    bool perfectIcache, perfectDcache;
};

#endif
