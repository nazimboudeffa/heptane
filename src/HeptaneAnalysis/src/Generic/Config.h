/* ------------------------------------------------------------------------------------

   Copyright IRISA, 2003-2017

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

   ------------------------------------------------------------------------------------ */

#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>
#include "CfgLib.h"
#include "Analysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Logger.h"

using namespace std;
using namespace cfglib;

// ----------------------------------------------------------------
//
// Constants (IPET variations, replacement policies, etc)
//
// ----------------------------------------------------------------

/** WCET calculation method
 *  -----------------------
 */
/** Solver */
#define LP_SOLVE 0
#define CPLEX 1

/** Supported architectures */

/* Supported architectures */
#define MIPS_ARCHITECTURE 0
// #define PPC_ARCHITECTURE 1
#define ARM_ARCHITECTURE 2

// Cache architecture constants
// ----------------------------
// Replacement policies
typedef enum
{ LRU, PLRU, RANDOM, FIFO, MRU, RR, PSEUDO_RR, UNKNOWN } t_replacement_policy;
// Types of caches supoorted
typedef enum
  { ICACHE, DCACHE, PERFECTICACHE, PERFECTDCACHE } t_cache_type;
// Max number of cache levels in the hierarchy
#define NB_MAX_CACHE_LEVEL 3

// Configuration of a single cache, whatever its position in the hierarchy
// -----------------------------------------------------------------------
class CacheParam
{
public:
  int nbsets, nbways, cachelinesize, level;
  t_replacement_policy replacement_policy;
  t_cache_type type;
  int latency;
public:
    CacheParam ()
  {;
  }
  CacheParam (XmlTag const &tag);
  ~CacheParam ();
};


/** -----------------------------------------------------------------------
 * Configuration class
 *
 * Contains all information on the architecture and analyses to
 * be applied.
 *
 * By default (default constructor), no analysis is applied and
 * architectural parameters are set to default values.
 *
 ----------------------------------------------------------------------- */
class Analysis;
class ParamAnalysis;
class ConfigAnalysis;
class WCETAnalysis;
class ConfigICache;
class Config
{

  /// Caches, with a vector for every cache level in the hierarchy
  map < int, vector < CacheParam * > >cache_params;

private:
  int nb_icache_levels;
  int nb_dcache_levels;

  // Latencies for the cache hierarchy, with cache levels number from 1 (L1 cache to N, last level cache)
  // Represents the latencies to access each cache level
  // In the final WCET computation, the latencies of all accessed cache levels are cumulated
  // map < int, int >cache_latencies;	// NB: all caches at the same level (dcache, icache) have the same latency

  // Memory load and store latencies in RAM
  // NB: for loads, a hit in the last level cache is systematically added, 
  //      i.e. 17 in case of hit in 1cycle, mem read of 16 cycles
  // NB: for stores, the latency is the total store latency, used as is in WCET computation 
  //     For instance, a very small value assumes a write-through cache with a write buffer
  int memory_load_latency, memory_store_latency;

  string arch_name; ///< architecture name (MIPS or ARM)
  Program *p;
  string entrypoint;
  int IPET_method_Applied;
  int MaxLevelCacheAnalysis; // the max level of the ICacheAnalysis, DCacheAnalysis (useful for cleaning the shared attributes)
  bool perfectIcache, perfectDcache;

public:

  /// Analyzed program location
  string input_output_dir;

  /** Constructors - simply sets up the analysis parameters to their default values (architecture, analyses). */
    Config ();
  /** Constructor : set up load/store latency, no cache inserted yet */
  //  Config (int load_lat, int store_lat);

  /** Destructor */
   ~Config ();

   void initParameters();
  /** Fill the architecture parameters from a given configuration file */
  void FillArchitectureFromXml (string xml_file);

  /** Execute the analyses as specified in the given configuration file */
  void ExecuteFromXml (string xml_file, bool printTime);

  /** TO BE REVISISTED
      Fill the architecture parameters by adding a cache to the current configuration
  void AddCacheLevel (int nbsets, int nbways, int cachelinesize, int level, t_replacement_policy replacement_policy, t_cache_type type, int latency); */


  // Latency for the Instruction cache access 
  // NB: cache levels are number from 1 (L1 cache) to N (last level cache)
  int getICacheLatency (int level);

  int getMemoryLoadLatency () const; 
  int getMemoryStoreLatency () const;

  // Get number of cache levels (for icaches and dcaches)
  // These two values should be the same, except in exotic cache configurations
  int getNbICacheLevels () const;
  int getNbDCacheLevels () const;
  Cfg* getEntryPoint () const;

  int getMaxLevelCacheAnalysis();

  //returns all cache configurations
  const map < int, vector < CacheParam * > >&GetCaches ();

  friend class ConfigICache;
private:
  int  getPerfectIcacheLatency();
  int  getPerfectDcacheLatency();

  /** Get cache Configuration at a given cache level */
  CacheParam * GetCacheAtLevel (int level, t_cache_type type);

  /** @return the parameters associated with a directive (directive ::= Printers | anaylsis ).
      When the analysis is unknown, a fatal error is emitted.
  */
  ParamAnalysis * getParameters(string directive, string input_output_dir, XmlTag analysis);

  /** @return the specific analyzer object associated with the directive ( directive ::= Printers | anaylsis ).
      When the analysis is unknown, a fatal error is emitted.
  */
  Analysis * mkAnalyzerObject(string directive, Program *p, ParamAnalysis *pa);

};

// Externals: pointer on configuration (global)
extern Config *config;


// -------------------------------------------------------
//
// Parameter objects for the analyses to be applied
//
// -------------------------------------------------------

// Base class, essentially implementing the reading
// of common parameters from Xml
// ------------------------------------------------
class ParamAnalysis
{
public:
  string input_file;
  string output_file;
  bool keep_results;
    ParamAnalysis ();
    ParamAnalysis (XmlTag const &tag);
   ~ParamAnalysis ();
};

// Printing functions (dot/text)
// ------------------------------
class ParamDotPrint:public ParamAnalysis
{
public:
  string directory;
  ParamDotPrint (string dir, XmlTag const &tag);
};
class ParamSimplePrint:public ParamAnalysis
{
public:
  bool printcallgraph;
  bool printloopnest;
  bool printcfg;
  bool printWCETinfo;
    ParamSimplePrint (XmlTag const &tag);
};

// CodeLine+HtmlPrint
// CodeLine attaches line information to basic blocks
// Htmlprint generates a Html after WCET estimation to identify the critical path
// -----------------------------------------------------------------------------
class ParamCodeLine:public ParamAnalysis
{
public:
  string binary_file;
  string binary_addr2line; ///< addr2line comand (full path)
  ParamCodeLine (XmlTag const &tag);
};
class ParamHtmlPrint:public ParamAnalysis
{
public:
  bool colorize;
  string html_file;
    ParamHtmlPrint (XmlTag const &tag);
};

// Cache analysis (instr/data/unified)
// -----------------------------------------
class ParamICache:public ParamAnalysis
{
public:
  int level;
  bool apply_must, apply_persistence, apply_may, keep_age;
    ParamICache (XmlTag const &tag);
};
class ParamDCache:public ParamAnalysis
{
public:
  int level;
  bool apply_must, apply_persistence, apply_may;
    ParamDCache (XmlTag const &tag);
};

// Data address extraction
// -----------------------
class ParamDataAddress:public ParamAnalysis
{
public:
  int sp;
  ParamDataAddress (XmlTag const &tag);
};

// Pipeline analysis
// -----------------
class ParamPipeline:public ParamAnalysis
{
public:
  ParamPipeline (XmlTag const &tag);
};

// WCET calculation
// ----------------
class ParamIPET:public ParamAnalysis
{
public:
  int solver;
  bool attach_WCET_info;
  bool generate_node_freq;
  ParamIPET (XmlTag const &tag);
  bool pipeline;
};

// Entry point analysis
class ParamEntryPoint:public ParamAnalysis
{
 public:
  string entrypoint;
  ParamEntryPoint (XmlTag const &tag);
};


// Cache statistics functions (code/data)
// --------------------------------------
class ParamCacheStatistics:public ParamAnalysis
{
public:
  ParamCacheStatistics (XmlTag const &tag);
};

// DummyAnalysis
// --------------------------------------
class ParamDummyAnalysis:public ParamAnalysis
{
public:
  ParamDummyAnalysis (XmlTag const &tag);
};

#endif
