/* ---------------------------------------------------------------------

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

   ------------------------------------------------------------------------ */

#include <stdlib.h>
#include <queue>
#include "Config.h"
#include "Specific/CacheAnalysis/ICacheAnalysis.h"
#include "Specific/CacheAnalysis/CacheStatistics.h"
#include "Specific/CacheAnalysis/DCacheAnalysis.h"
#include "Specific/SimplePrint/SimplePrint.h"
#include "Specific/DotPrint/DotPrint.h"
#include "Specific/IPETAnalysis/IPETAnalysis.h"
#include "Specific/DataAddressAnalysis/MIPSAddressAnalysis.h"
#include "Specific/DataAddressAnalysis/ARMAddressAnalysis.h"
#include "Specific/CodeLine/CodeLine.h"
#include "Specific/HtmlPrint/HtmlPrint.h"
#include "Specific/PipelineAnalysis/MIPSPipelineAnalysis.h"
#include "Specific/PipelineAnalysis/ARMPipelineAnalysis.h"
#include "arch.h"
#include "Specific/DummyAnalysis/DummyAnalysis.h"
#include "Generic/Timer.h"

Config *config = new Config ();	// global object.

#define ON "on"
#define OFF "off"

// ---------------------------------------------------
//
//  Constructor: set default values for configuration parameters
//
// ---------------------------------------------------
Config::Config ()
{
  nb_icache_levels = 0;
  nb_dcache_levels = 0;

  // Cache access latencies
  // By default, perfect memory hierarchy
  //  for (unsigned int i = 1; i < NB_MAX_CACHE_LEVEL + 1; i++)    cache_latencies[i] = 0;

  memory_load_latency = 0;
  memory_store_latency = 0;
  input_output_dir = "./";
  entrypoint=string("");
  initParameters();
}

/** UNUSED ---
    Constructor : set up load/store latency, no cache inserted yet 
    Config::Config (int load_lat, int store_lat)
    {
    nb_icache_levels = 0;
    nb_dcache_levels = 0;
    memory_load_latency = load_lat;
    memory_store_latency = store_lat;
    }
*/

Config::~Config ()
{
}

// ---------------------------------------------------
//
//  Init architectural configuration parameters from an XML file
//
// ---------------------------------------------------
void
Config::FillArchitectureFromXml (string xml_file)
{
  XmlDocument xmldoc (xml_file);
  ListXmlTag lt;
  bool target_found = false;
  string arch_endianness;



  // Directory section
  // -----------------
  lt = xmldoc.searchChildren ("INPUTOUTPUTDIR");
  assert (lt.size () <= 1);
  if (lt.size () == 1)
    input_output_dir = lt[0].getAttributeString ("name");

  // Architecture section
  // ----------------------

  lt = xmldoc.searchChildren ("ARCHITECTURE");
  if (lt.size () != 1)
    Logger::addFatal ("Config: there should be a single ARCHITECTURE tag in your XML");
  // Caches and memory properties
  bool has_memory = false;
  ListXmlTag ltarch = lt[0].getAllChildren ();
  perfectDcache = false;
  perfectIcache = false;
  for (unsigned int i = 0; i < ltarch.size (); i++)
    {
      string currName = ltarch[i].getName ();

      // Get the architecture file from xml
      if (currName == "TARGET")
	{
	  target_found = true;
	  arch_endianness = ltarch[i].getAttributeString ("ENDIANNESS");
	  if (arch_endianness != "BIG" && arch_endianness != "LITTLE")
	    Logger::addFatal ("Config error: Unsupported endianness (should be BIG or LITTLE): " + arch_endianness);
	  arch_name = ltarch[i].getAttributeString ("NAME");
	  if ((arch_name != "MIPS") && (arch_name != "ARM"))
	    {
	      string msg_error = "Config: unsupported target architecture: " + arch_name;
	      Logger::addFatal (msg_error);
	    }
	}
      // Initialization of cache parameters
      else if (currName == "CACHE")
	{
	  CacheParam *cp = new CacheParam (ltarch[i]);
	  if ((cp->type == ICACHE) && perfectIcache )
	    {
	      Logger::addFatal ("Config: picache/icache directives are incompatible");
	    }
	  else
	    if (( cp->type == DCACHE) && perfectDcache )
	      {
		Logger::addFatal ("Config: pdcache/dcache directives are incompatible");
	      }
	    else
	      { 
		t_cache_type tCache = cp->type;
		if ((tCache == ICACHE || tCache == PERFECTICACHE) && cp->level > nb_icache_levels) nb_icache_levels = cp->level;
		if ((tCache == DCACHE || tCache == PERFECTDCACHE) && cp->level > nb_dcache_levels) nb_dcache_levels = cp->level;
		if (tCache == PERFECTICACHE)
		  {
		    if (perfectIcache) Logger::addFatal ("Config: configuration file should have atmost one perfect instruction cache directive");
		    if (cp->level != 1)
		      {
			Logger::addWarning ("Config: the level for a  perfect instruction cache must be 1, set to 1");
			cp->level = 1;
		      }
		    perfectIcache = true;
		  }
		else
		  if ( tCache == PERFECTDCACHE)
		    {
		      if (perfectDcache)  Logger::addFatal ("Config: configuration file should have atmost one perfect data cache directive");
		      if (cp->level != 1)
			{
			  Logger::addWarning ("Config: the level for a  perfect instruction cache must be 1, set to 1");
			  cp->level = 1;
			}
		      perfectDcache = true;
		    }
		cache_params[cp->level].push_back (cp);
	      }
	}      
      else // Initialization of memory parameters.
	if (currName == "MEMORY")
	  {
	    if (has_memory)
	      { 
		Logger::addFatal ("Config: configuration file should have one unique MEMORY tag ..."); 
	      }
	    has_memory = true;
	    this->memory_load_latency = ltarch[i].getAttributeInt ("load_latency");
	    this->memory_store_latency = ltarch[i].getAttributeInt ("store_latency");
	    if (this->memory_load_latency < 0 || this->memory_store_latency < 0)
	      Logger::addFatal ("Config: Incorrect memory load and/or store latencies");
	  }
	else
	  if (currName != "comment")
	    {
	      string error_msg = "Config: unknown architectural element in config file " + currName;
	      Logger::addFatal (error_msg);
	    }
	  else; // Comments are ignored.
    }

  if (!perfectIcache && !perfectDcache && (nb_icache_levels != nb_dcache_levels))
    Logger::addFatal ("Config: configuration file should have the same levels for caches, except the perfect caches.");
  
  if (!has_memory)
    Logger::addFatal ("Config: configuration file should have one unique MEMORY tag");
  if (!target_found)
    Logger::addFatal ("Config: configuration file should have a TARGET tag");

  // Creating the dependant architecture instance (MIPS, ARM, ...)
  Arch::init (arch_name, arch_endianness == "BIG");

  // Initialization of latencies per cache level
  /*  for (unsigned int i = 1; i <= cache_params.size (); i++)
    for (unsigned int j = 0; j < cache_params[i].size (); j++)
      if (j == 0)
	cache_latencies[i] = cache_params[i][j]->latency;
      else 
	if ( ! perfectDcache &&  ! perfectIcache)
	  {
	    if (cache_params[i][j]->latency != cache_latencies[i])
	      Logger::addFatal ("Config: all caches at the same level should have the same latency, except the perfect caches.");
	  }
  */
}

/** TO BE REVISITED/ADPATED 
    Fill the architecture parameters by adding a cache to the current configuration
void
Config::AddCacheLevel (int nbsets, int nbways, int cachelinesize, int level, t_replacement_policy replacement_policy, t_cache_type type, int latency)
{
  // called in main program, configuration object by program, unused currently.
  CacheParam *cp = new CacheParam ();
  cp->nbsets = nbsets;
  cp->nbways = nbways;
  cp->cachelinesize = cachelinesize;
  if (level <= 0)
    Logger::addFatal ("Config: invalid cache level number");
  cp->level = level;
  if (replacement_policy != LRU && replacement_policy != PLRU && replacement_policy != FIFO && replacement_policy != MRU && replacement_policy != RANDOM)
    {
      Logger::addFatal ("Config: unknown replacement policy");
    }
  if (type != ICACHE && type != DCACHE)
    {
      stringstream s_type;
      s_type << type;
      string msg_error = "Config: invalid cache type: " + s_type.str ();
      Logger::addFatal (msg_error);
    }
  cp->type = type;
  if (latency >= memory_load_latency)
    {
      stringstream s_latency;
      s_latency << latency;
      string msg_error = "Config: invalid cache latency: " + s_latency.str ();
      Logger::addFatal (msg_error);
    }

  cp->latency = latency;
  if (cp->type == ICACHE && cp->level > nb_icache_levels)
    nb_icache_levels = cp->level;
  if (cp->type == DCACHE && cp->level > nb_dcache_levels)
    nb_dcache_levels = cp->level;
  cache_params[cp->level].push_back (cp);
  for (unsigned int i = 1; i <= cache_params.size (); i++)
    {
      for (unsigned int j = 0; j < cache_params[i].size (); j++)
	{
	  if (j == 0)
	    {
	      cache_latencies[i] = cache_params[i][j]->latency;
	    }
	  else if (cache_params[i][j]->latency != cache_latencies[i])
	    {
	      Logger::addFatal ("Config: all caches at the same level should have the same latency");
	    }
	}
    }
}
 */

ParamAnalysis * Config::getParameters(string directive, string input_output_dir, XmlTag analysis)
{
  // directive ::= Printers | Analysis | ENTRYPOINT
  if (directive == "ENTRYPOINT") { return new ParamEntryPoint(analysis);}

  // Printers ::= DOTPRINT | SIMPLEPRINT | HTMLPRINT | CODELINE | CACHESTATISTICS 
  if (directive == "DOTPRINT") { return new ParamDotPrint (input_output_dir, analysis); /* Read paramaters from Xml */ }
  if (directive == "SIMPLEPRINT") { return new ParamSimplePrint (analysis); }
  if (directive == "DUMMYANALYSIS") { return new ParamDummyAnalysis (analysis); }
  if (directive == "CODELINE") {return new ParamCodeLine (analysis);}
  if (directive == "HTMLPRINT") { return new ParamHtmlPrint (analysis); }
  if (directive == "CACHESTATISTICS") { return new ParamCacheStatistics (analysis);}

  // Analysis ::= ICACHE | PIPELINE | IPET | DATAADDRESS | DCACHE | DUMMYANALYSIS
  if (directive == "ICACHE") { return new ParamICache (analysis); }
  if (directive == "DATAADDRESS") { return new ParamDataAddress (analysis); }
  if (directive == "DCACHE") { return new ParamDCache (analysis); }
  if (directive == "PIPELINE") { return  new ParamPipeline (analysis); }
  if (directive == "IPET") { return  new ParamIPET (analysis); }
  // Fatal error otherwise.
  string error_msg = "Config: unknown analysis type " + directive;
  Logger::addFatal (error_msg);
  return NULL;
}

int  Config::getPerfectIcacheLatency()
{
  if (perfectIcache)
    {
      if (cache_params[1][0]->type == PERFECTICACHE) return cache_params[1][0]->latency;
      if (cache_params[1][1]->type == PERFECTICACHE) return cache_params[1][1]->latency;
    }
  return -1;
}

int  Config::getPerfectDcacheLatency()
{
  if (perfectDcache)
    {
      if (cache_params[1][0]->type == PERFECTDCACHE) return cache_params[1][0]->latency;
      if (cache_params[1][1]->type == PERFECTDCACHE) return cache_params[1][1]->latency;
    }
  return -1;
}


Analysis* Config::mkAnalyzerObject(string directive, Program *p, ParamAnalysis *pa)
{
  // directive ::= Printers | Analysis

  // Printers ::= DOTPRINT | SIMPLEPRINT | HTMLPRINT | CODELINE | CACHESTATISTICS
  if (directive == "DOTPRINT") { return new DotPrint (p, ((ParamDotPrint *) pa)->directory); }
  if (directive == "SIMPLEPRINT")
    {
      ParamSimplePrint *ps = (ParamSimplePrint *) pa;
      return new SimplePrint (p, ps->printcallgraph, ps->printloopnest, ps->printcfg, ps->printWCETinfo);
    }
  if (directive == "DUMMYANALYSIS") { /* ParamDummyAnalysis *ps=(ParamDummyAnalysis*)pa; */ return new DummyAnalysis (p); }
  if (directive == "CODELINE") { return new CodeLine (p, input_output_dir + "/" + ((ParamCodeLine *) pa)->binary_file, arch_name, ((ParamCodeLine *) pa)->binary_addr2line); }
  if (directive == "HTMLPRINT") { return new HtmlPrint (p, input_output_dir + "/" + ((ParamHtmlPrint *) pa)->html_file, ((ParamHtmlPrint *) pa)->colorize); }
  if (directive == "CACHESTATISTICS") 
    { 
      return new CacheStatistics (p, GetCaches (), perfectIcache, perfectDcache);
    }

  // Analysis ::= ICACHE | DATAADDRESS | DCACHE | PIPELINE | IPET  | DUMMYANALYSIS
  if (directive == "ICACHE")
    {
      ParamICache *ps = (ParamICache *) pa;
      CacheParam *cp = GetCacheAtLevel (ps->level, ICACHE);
      // FIXME: nice error handling
      assert (cp != NULL);
      if ( perfectIcache &&  ps->level != 1)
	Logger::addFatal ("ICacheAnalysis : bad level for perfect instruction cache");
      if (ps->level > MaxLevelCacheAnalysis) MaxLevelCacheAnalysis=ps->level;
      return new ICacheAnalysis (p, cp->nbsets, cp->nbways, cp->cachelinesize, cp->replacement_policy, ps->level, ps->apply_must, ps->apply_persistence, ps->apply_may, ps->keep_age, perfectIcache);
    }

  if (directive == "DATAADDRESS") 
    {
      ParamDataAddress *ps = (ParamDataAddress *) pa;
      if (arch_name == "MIPS") return new MIPSAddressAnalysis (p, ps->sp); else return new ARMAddressAnalysis (p, ps->sp);
    }

  if (directive == "DCACHE")
    {
      ParamDCache *ps = (ParamDCache *) pa;
      CacheParam *cp = GetCacheAtLevel (ps->level, DCACHE);
      // FIXME: nice error handling
      assert (cp != NULL);
      if ( perfectDcache &&  ps->level != 1)
	Logger::addFatal ("DCacheAnalysis : bad level for perfect data cache");

      if (ps->level > MaxLevelCacheAnalysis) MaxLevelCacheAnalysis=ps->level;
      return new DCacheAnalysis (p, cp->nbsets, cp->nbways, cp->cachelinesize, cp->replacement_policy, ps->level, ps->apply_must, ps->apply_persistence, ps->apply_may, perfectDcache);
    }
  if (directive == "PIPELINE")
    {
      int nbicache = getNbICacheLevels ();
      // assumed verified nbicache == nbdcache for not perfect caches.
      if (arch_name == "MIPS")  return new MIPSPipelineAnalysis (p, nbicache); else return new ARMPipelineAnalysis (p, nbicache); 
    }
  if (directive == "IPET")
    {
      ParamIPET *ps = (ParamIPET *) pa;
      return new IPETAnalysis (p, ps->solver, ps->pipeline, ps->attach_WCET_info, ps->generate_node_freq, getNbICacheLevels (), getNbDCacheLevels (), cache_params);
    }

  // Already testesd before in getParameters() ?
  string error_msg = "Config: unknown analysis type " + directive;
  Logger::addFatal (error_msg);
  return NULL;
}

void Config::initParameters()
{
  MaxLevelCacheAnalysis=-1; // reset for next ICache/Dcache analysis.
  //  IPET_method_Applied = -1;
}

void
Config::ExecuteFromXml (string xml_file, bool printTime)
{
  XmlDocument xmldoc (xml_file);
  ListXmlTag lt;
  bool b;
  string ep;

  // Directory section
  // -----------------
  lt = xmldoc.searchChildren ("INPUTOUTPUTDIR");
  assert (lt.size () <= 1);
  if (lt.size () == 1) { input_output_dir = lt[0].getAttributeString ("name");}

  // Search for analysis section
  // --------------------------
  lt = xmldoc.searchChildren ("ANALYSIS");
  if (lt.size () != 1) { Logger::addFatal ("Config: there should be a single ANALYSIS tag in your XML");}

  Timer timer_AllAnalysis;
  float time = 0.0;
  timer_AllAnalysis.initTimer();

  // Just call the appropriate function to fill-in analysis dependent parameters from the xml
  // --------------------------------------------------------
  ListXmlTag ltanalysis = lt[0].getAllChildren ();
  p= NULL;
  for (unsigned int i = 0; i < ltanalysis.size (); i++)
    {
      ParamAnalysis *pa = NULL;
      Analysis *a = NULL;
      string analysis_name = ltanalysis[i].getName ();

      // Read the parameters (pa) from the XML and create the associated analysis object (a)

      // Skip comments in xml
      if (analysis_name == "comment") { continue;}

      // Getting the parameters of the directive.
      pa = getParameters(analysis_name, input_output_dir, ltanalysis[i]);
      assert (pa != NULL);

      // Call the analysis
      // -----------------
      // Decide on which program the analysis should be applied and check the program suitability for WCET before going on
      b = false;
      if (pa->input_file != "")
	{
	  if (p != NULL) delete p;
	  p = Program::unserialise_program_file (input_output_dir + "/" + pa->input_file);
	  AnalysisHelper::ProgramCheck (p);
	  b = true;
	}
      if (analysis_name == "ENTRYPOINT")
	{
	  ep = ((ParamEntryPoint*) pa)->entrypoint ;
	  if (!p->SetEntryPoint(ep)) Logger::addFatal ("Config: Bad entry point name " + ep );
	  b = true;
	}
      if (b)
	{
	  AnalysisHelper::computeContext(p);
	  initParameters();
	  Logger::print( "\n*** Begin analysis for entry point: " + ep);
	}
      
      // Clone the program should the analysis results are not kept
      Program *pgm = NULL;
      if (! pa->keep_results) pgm = p->Clone (); else { pgm = p; entrypoint=ep;}
      assert (pgm != NULL);

      if ( analysis_name != "ENTRYPOINT")
	{

	  // create the associated analysis object (a) for the current analysis.
	  a = mkAnalyzerObject(analysis_name, p, pa);
	  a->setName(analysis_name);
	  assert (a != NULL);
	  
	  // Assumed: the first print order is the SIMPLEPRINT.
	  if ( (analysis_name == "SIMPLEPRINT") && printTime) 
	    {
	      timer_AllAnalysis.addTimer(time);
	      stringstream infostr;
	      infostr << " =======> Total time for the analyses = "  << time;
	      Logger::addInfo(infostr.str());
	      Logger::print();
	    }


	  // Apply the analysis, and log its results
	  Logger::clean ();
	  bool res = a->CheckPerformCleanup (printTime);
	  if (!res) Logger::addFatal ("Config: call to analysis failed");
	  Logger::print ();
	  if (Logger::getErrorState ()) exit (-1);
	  
	  // For debug only
	  if ((analysis_name == "IPET") && Logger::isDebugMode ())
	    {
	      string WCET;
	      if (res)
		{
		  // Retrieve the WCET, stored as an attribute attached to the program entry point
		  Cfg *c = pgm->GetEntryPoint ();
		  // Modified LBesnard, May 2016. When attach_WCET_info is set to false, the value of the WCET is not kept.
		  if (c->HasAttribute (WCETAttributeName))
		    {
		      SerialisableStringAttribute ba = (SerialisableStringAttribute &) c->GetAttribute (WCETAttributeName);
		      WCET = ba.GetValue ();
		    }
		  else
		    WCET="-1 (attach_WCET_info seems to be false. The value of the WCET is not available)";
		}
	      else
		WCET = "-1";
	      
	      cout << endl;
	    }
	}
      
      // Dump the result to XML if asked for
      string ofile = pa->output_file;
      if (ofile != "")
	{
	  string xml_file = input_output_dir + "/" + ofile;
	  pgm->serialise_program (xml_file);
	}
      
      if (!pa->keep_results) { delete pgm; }

      // Cleaning for the next step.
      delete pa; pa = NULL;
      delete a; a = NULL;
    }
}

// ---------------------------------------------------
//
//  Basic accessors
//
// ---------------------------------------------------
int
Config::getICacheLatency (int level)
{
  assert (level >= 1 && (level <= nb_icache_levels));
  t_cache_type tCache = cache_params[level][0]->type;
  if (tCache == ICACHE || tCache == PERFECTICACHE) return cache_params[level][0] ->latency;
  return cache_params[level][1] ->latency;
}

int
Config::getMemoryLoadLatency () const
{
  return memory_load_latency;
}

int
Config::getMemoryStoreLatency () const
{
  return memory_store_latency;
}

int
Config::getNbICacheLevels () const
{
  return nb_icache_levels;
}

int
Config::getNbDCacheLevels () const
{
  return nb_dcache_levels;
}

Cfg*
Config::getEntryPoint () const
{
  return  p->GetEntryPoint();
} 


// ---------------------------------------------------
//
//  To obtained configuration of the (assumed unique)
//  cache of type 'type' at level 'level' in the
//  cache hierarchy
//
// ---------------------------------------------------
CacheParam *
Config::GetCacheAtLevel (int level, t_cache_type type)
{
  vector < CacheParam * >vcp = cache_params[level];
  int idx = 0;
  bool found = false;
  // Check there is a single cache of type 'type' at the requested level
  if (vcp.size () == 0)
    {
      stringstream s_level;
      s_level << level;
      string error_msg = "Config: no cache of the asked type at level " + s_level.str ();
      Logger::addFatal (error_msg);
    }
  for (unsigned int i = 0; i < vcp.size (); i++)
    {
      if (vcp[i]->type == type)
	{
	  idx = i;
	  if (found)
	    {
	      Logger::addFatal ("Config: more than one cache of the same type at the same level");
	    }
	  found = true;
	}
    }
  return vcp[idx];
}

// --------------------------------
//
// Reading of cache parameters
//
// --------------------------------
CacheParam::CacheParam (XmlTag const &tag)
{
  string stype = tag.getAttributeString ("type");
  assert (stype == "icache" || stype == "dcache" || stype =="pdcache" || stype =="picache");

  if ((stype == "icache") || (stype == "dcache"))
    {
      if (stype == "icache") this->type = ICACHE; else this->type = DCACHE;
      this->nbsets = tag.getAttributeInt ("nbsets");
      assert (this->nbsets != 0);
      this->nbways = tag.getAttributeInt ("nbways");
      assert (this->nbways != 0);
      this->cachelinesize = tag.getAttributeInt ("cachelinesize");
	assert (this->cachelinesize != 0);
	this->level = tag.getAttributeInt ("level");
	assert (this->level > 0 && this->level <= NB_MAX_CACHE_LEVEL);
	this->latency = tag.getAttributeInt ("latency");
	assert (this->latency >= 0);
	string RP = tag.getAttributeString ("replacement_policy");
	assert (RP == "LRU" || RP == "PLRU" || RP == "FIFO" || RP == "MRU" || RP == "RANDOM" || RP == "RR" || RP == "PSEUDO_RR" || RP == "UNKNOWN");
	if (RP == "LRU")
	  this->replacement_policy = LRU;
	else if (RP == "PLRU")
	  this->replacement_policy = PLRU;
	else if (RP == "FIFO")
	  this->replacement_policy = FIFO;
	else if (RP == "MRU")
	  this->replacement_policy = MRU;
	else if (RP == "RANDOM")
	  this->replacement_policy = RANDOM;
	else if (RP == "RR")
	  this->replacement_policy = RR;
	else if (RP == "PSEUDO_RR")
	  this->replacement_policy = PSEUDO_RR;
	else if (RP == "UNKNOWN")
	  this->replacement_policy = UNKNOWN;
      }
    else // perfect data cache / perfect instruction cache
      {
	if (stype == "picache") this->type = PERFECTICACHE; else this->type = PERFECTDCACHE;
	this->latency = tag.getAttributeInt ("latency");
	assert (this->latency >= 0);
	this->level = tag.getAttributeInt ("level");
	assert (this->level == 1);
      }
}

// ---------------------------------------------------
//
//  To obtained configuration of all the caches
//
// ---------------------------------------------------
const map < int, vector < CacheParam * > >&
Config::GetCaches ()
{
  return cache_params;
}

int Config::getMaxLevelCacheAnalysis()
{
  return MaxLevelCacheAnalysis;
}

// -------------------------------------------
//
// Reading of common analysis parameters from Xml
//
// ------------------------------------------
ParamAnalysis::ParamAnalysis (XmlTag const &tag)
{
  keep_results = false;
  this->input_file = tag.getAttributeString ("input_file");
  this->output_file = tag.getAttributeString ("output_file");
  string keep_s = tag.getAttributeString ("keepresults");
  this->keep_results = (keep_s == ON);
}

ParamAnalysis::~ParamAnalysis ()
{
}

// -------------------------------------------
//
// Reading of analysis dependant parameters
// from Xml + execution
//
// ------------------------------------------

// Printing functions (dot/text)
// ------------------------------
ParamDotPrint::ParamDotPrint (string dir, XmlTag const &tag):
  ParamAnalysis (tag)
{
  directory = dir;
}

ParamSimplePrint::ParamSimplePrint (XmlTag const &tag):
  ParamAnalysis (tag)
{
  string s = tag.getAttributeString ("printcallgraph");
  assert (s == ON || s == OFF || s == "");
  this->printcallgraph = (s == ON);

  s = tag.getAttributeString ("printloopnest");
  assert (s == ON || s == OFF || s == "");
  this->printloopnest = (s == ON);

  s = tag.getAttributeString ("printcfg");
  assert (s == ON || s == OFF || s == "");
  this->printcfg = (s == ON);

  s = tag.getAttributeString ("printWCETinfo");
  assert (s == ON || s == OFF || s == "");
  this->printWCETinfo = (s == ON);
}

// CodeLine+HtmlPrint
// CodeLine attaches line information to basic blocks
// Htmlprint generates a Html after WCET estimation to identify the critical path
// -----------------------------------------------------------------------------
ParamCodeLine::ParamCodeLine (XmlTag const &tag):
  ParamAnalysis (tag)
{
  this->binary_file =  tag.getAttributeString ("binaryfile");
  this->binary_addr2line = tag.getAttributeString ("addr2lineCommand");
}

ParamHtmlPrint::ParamHtmlPrint (XmlTag const &tag):
  ParamAnalysis (tag)
{
  string s = tag.getAttributeString ("colorize");
  assert (s == ON || s == OFF || s == "");
  this->colorize = (s == ON);
  this->html_file = tag.getAttributeString ("html_file");
}

// Cache analysis (instr/data)
// -----------------------------------------
ParamICache::ParamICache (XmlTag const &tag):
  ParamAnalysis (tag)
{
  this->level = tag.getAttributeInt ("level");
  assert (level > 0 && level <= NB_MAX_CACHE_LEVEL);

  string s = tag.getAttributeString ("must");
  assert (s == "on" || s == "off");
  this->apply_must = (s == "on");

  s = tag.getAttributeString ("persistence");
  assert (s == "on" || s == "off");
  this->apply_persistence = (s == "on");

  s = tag.getAttributeString ("may");
  assert (s == "on" || s == "off");
  this->apply_may = (s == "on");

  s = tag.getAttributeString ("keep_age");
  if (s == "") s = "off";
  assert (s == "on" || s == "off");
  this->keep_age = (s == "on");
}

ParamDCache::ParamDCache (XmlTag const &tag):
  ParamAnalysis (tag)
{
  this->level = tag.getAttributeInt ("level");
  assert (level > 0 && level <= NB_MAX_CACHE_LEVEL);

  string s = tag.getAttributeString ("must");
  assert (s == "on" || s == "off");
  this->apply_must = (s == "on");

  s = tag.getAttributeString ("persistence");
  assert (s == "on" || s == "off");
  this->apply_persistence = (s == "on");

  s = tag.getAttributeString ("may");
  assert (s == "on" || s == "off");
  this->apply_may = (s == "on");
}

// Data address extraction
// -----------------------
ParamDataAddress::ParamDataAddress (XmlTag const &tag):
  ParamAnalysis (tag)
{
  this->sp = tag.getAttributeHexa("sp");
}

// Pipeline analysis
// -----------------
ParamPipeline::ParamPipeline (XmlTag const &tag):
  ParamAnalysis (tag)
{
}

// WCET calculation
// ----------------
ParamIPET::ParamIPET (XmlTag const &tag):
  ParamAnalysis (tag)
{
  string s;
  s = tag.getAttributeString ("attach_WCET_info");
  assert (s == ON || s == OFF || s == "");
  this->attach_WCET_info = (s == ON);

  s = tag.getAttributeString ("generate_node_freq");
  assert (s == ON || s == OFF || s == "");
  this->generate_node_freq = (s == ON);

  s = tag.getAttributeString ("solver");
  assert (s == "cplex" || s == "lp_solve");
  if (s == "cplex")
    this->solver = CPLEX;
  else
    this->solver = LP_SOLVE;

  s = tag.getAttributeString ("pipeline");
  assert (s == ON || s == OFF);
  pipeline = ( s == ON );
}


// Entry point analysis
ParamEntryPoint::ParamEntryPoint (XmlTag const &tag):
  ParamAnalysis (tag)
{
  entrypoint = tag.getAttributeString ("entrypointname");
  if ( entrypoint == "" ) Logger::addFatal ("Config: entry point name not set");
}


// Cache statistics functions (code/data)
// --------------------------------------
ParamCacheStatistics::ParamCacheStatistics (XmlTag const &tag):
  ParamAnalysis (tag)
{
}


// DummyAnalysis
// --------------------------------------
ParamDummyAnalysis::ParamDummyAnalysis (XmlTag const &tag):
  ParamAnalysis (tag)
{
}
