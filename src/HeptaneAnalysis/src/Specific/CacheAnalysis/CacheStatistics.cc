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

#include "Specific/CacheAnalysis/CacheStatistics.h"

#include "Generic/CallGraph.h"
#include "arch.h"

#include <limits>

#define CacheStatistics_trace(s) s

// -------------------------------------------------
// Checks every node has a frequency attribute
// previously computed by IPET analysis
// -------------------------------------------------
static bool
CheckFrequencyAttribute (Cfg * c, Node * n, void *param)
{
  string contextName;

  const ContextList & contexts = (ContextList &) c->GetAttribute (ContextListAttributeName);
  unsigned int nc = contexts.size ();
  unsigned int nb_ctx_found = 0;
  for (unsigned int ic = 0; ic < nc; ic++)
    {
      contextName = contexts[ic]->getStringId();
      if (n->HasAttribute (AnalysisHelper::getContextAttrFrequencyName(contextName)))
	nb_ctx_found++;
    }
  if (nb_ctx_found != 1 && nb_ctx_found != nc) return false;
  return true;
}

CacheStatistics::CacheStatistics (Program * p, const map < int, vector < CacheParam * > >&hierarchy_configuration, bool vperfectIcache, bool vperfectDcache):
Analysis (p), program (p), hierarchy_configuration (hierarchy_configuration)
{
   perfectIcache = vperfectIcache;
   perfectDcache = vperfectDcache;
}


bool
CacheStatistics::CheckInputAttributes ()
{
  // Check frequency attribute
  if ( ! AnalysisHelper::applyToAllNodesRecursive (p, CheckFrequencyAttribute, this))
    {
      string error_msg = "CacheStatistics: node should have the " + string (FrequencyAttributeName) + " attribute set, sorry ... ";
      Logger::addFatal (error_msg);
    }

  CheckInstrHaveCacheCategorizationAttribute ();
  return true;
}

// Compute the number of references, hits and misses for a cache (aCache)
void CacheStatistics::PerformAnalysis ( CacheParam * aCache, const vector < Cfg * >&cfgs, CallGraph &callgraph)
{
  Cfg *CurrentCfg;
  Node * CurrentNode;

  size_t refs = 0;
  size_t hits = 0;
  size_t misses = 0;

  // For each cfg
  for (size_t c = 0; c < cfgs.size (); ++c)
    {
      CurrentCfg = cfgs[c];
      if (!callgraph.isDeadCode (CurrentCfg))
	{
	  // For each context and node
	  const ContextList & contexts = (ContextList &) CurrentCfg->GetAttribute (ContextListAttributeName);
	  const vector < Node * >&nodes = CurrentCfg->GetAllNodes ();
	  for (size_t ct = 0; ct < contexts.size (); ++ct)
	    {
	      for (size_t n = 0; n < nodes.size (); ++n)
		{
		  CurrentNode = nodes[n];
		  if ( CurrentNode->isIsolatedNopNode() ) continue;

		  unsigned long frequency = getExecutionFrequency (contexts[ct]->getStringId(), CurrentNode);
		  if (frequency == 0) continue;

		  // For each instruction
		  const vector < Instruction * >&instr = CurrentNode->GetAsm ();
		  for (size_t i = 0; i < instr.size (); ++i)
		    {
		      if ( (aCache->type == DCACHE || aCache->type == PERFECTDCACHE) && ! Arch::isLoad (instr[i]->GetCode ()))
			continue;
		      string chmc = getCHMC (contexts[ct], instr[i], aCache);
		      size_t occurrences = getOccurrencesCount (frequency, contexts[ct], instr[i], aCache);
		      if (occurrences > 0)
			{
			  refs += occurrences;
			  if (chmc == "AH")
			    {
			      hits += occurrences; 
			      misses += 0;
			    }
			  else if (chmc == "AM")
			    {
			      misses += occurrences;
			      hits += 0;
			    }
			  else if (chmc == "NC")
			    {
			      misses += occurrences;
			      hits += 0;
			    }
			  else if (chmc == "FM")
			    {
			      size_t cab = countAccessedBlocks (contexts[ct], instr[i], aCache);
			      if (cab > occurrences)
				{
				  misses += occurrences; 
				  hits += 0;
				}
			      else
				{
				  hits += occurrences - cab;
				  misses += cab;
				}
			    }
			  else
			    {
			      assert (false);
			    }
			}
		    }

		}
	    }
	}
    }

  // Print results
  cout << "L" << aCache->level << "\t";
  cout << "type " << getCacheTypeName (aCache->type) << "\t";
  cout << "references " << refs << "\t";
  cout << "hits " << hits << "\t";
  cout << "misses " << misses << "\t";
  cout << endl;
}


bool
CacheStatistics::PerformAnalysis ()
{
  CallGraph callgraph (program);
  const vector < Cfg * >&cfgs = program->GetAllCfgs ();
  CacheParam * aCache;
  bool btodo = true;

  // For each cache level and cache in the memory hierarchy and if the infos are available.
  map < int, vector < CacheParam * > >::const_iterator it_levels = hierarchy_configuration.begin ();
  map < int, vector < CacheParam * > >::const_iterator it_end = hierarchy_configuration.end ();
  for (; it_levels != it_end; ++it_levels)
    {
      const vector < CacheParam * >&caches = it_levels->second;
      
      for (size_t s = 0; s < caches.size (); ++s)
	{
	  aCache = caches[s];
	  assert (aCache->level == it_levels->first);
	  if (aCache->type == ICACHE || aCache->type == PERFECTICACHE)
	    {
	      if (perfectIcache) btodo=false; else btodo=has_icache_info[aCache->level -1];
	      // printf(" ICACHE and has_icache_info[ %d ] = %d \n", aCache->level-1, btodo);
	    }
	  else
	    { 
	      if (perfectDcache) btodo=false; else btodo=has_dcache_info[aCache->level -1];
	      // printf(" DCACHE and has_dcache_info[ %d ] = %d \n", aCache->level-1, btodo);
	    }
	  if (btodo)
	    PerformAnalysis (aCache, cfgs, callgraph ); 
	  else
	    CacheStatistics_trace(PerformAnalysis (aCache, cfgs, callgraph)); // to see the effect of perfect caches.
	}
    }
  return true;
}


void
CacheStatistics::RemovePrivateAttributes ()
{
}

// Count the number of cache levels in the memory hierarchy.
int
CacheStatistics::countCacheLevels ()
{
  assert ((size_t) numeric_limits < int >::max () > hierarchy_configuration.size ());
  return (int) hierarchy_configuration.size ();
}


// Return the parameters of the caches at level level in the memory hierarchy.
vector < CacheParam * >CacheStatistics::getCacheLevelParameters (int level)
{
  if (level > this->countCacheLevels ()) return vector < CacheParam * >();
  return hierarchy_configuration.find (level)->second;
}


// Return the execution frequency of a node, in its local context number
// context_lid,  on the WCEP.
unsigned long
CacheStatistics::getExecutionFrequency (string contextName, Node * node)
{
  unsigned long frequency = ((SerialisableUnsignedLongAttribute &) node->GetAttribute (AnalysisHelper::getContextAttrFrequencyName(contextName))).GetValue ();
  return frequency;
}


// Return the CHMC of instruction inst in the specified context  with regards to
// a given cache.
string
CacheStatistics::getCHMC (Context * context, Instruction * inst, CacheParam * cache)
{
  string chmc_name;
  switch (cache->type)
    {
    case ICACHE:
    case PERFECTICACHE:
      chmc_name = AnalysisHelper::mkContextAttrName( CHMCAttributeNameCode (cache->level), context);
      break;
    case DCACHE:
    case PERFECTDCACHE:
      assert (Arch::isLoad (inst->GetCode ()));
      chmc_name = AnalysisHelper::mkContextAttrName( CHMCAttributeNameData (cache->level), context);
      break;
    default:
      assert (false);
      return "AU";
    };
  assert (inst->HasAttribute (chmc_name));
  SerialisableStringAttribute & chmc = (SerialisableStringAttribute &) inst->GetAttribute (chmc_name);
  return chmc.GetValue ();
}

// Compute and upper bound on the number of occurrences on the specified cache
// of instruction inst. inst is considered in the specified context. frequency
// is the execution frequency of inst on the WCEP.
size_t
CacheStatistics::getOccurrencesCount (unsigned long frequency, Context * context, Instruction * inst, CacheParam * cache)
{
  if (frequency == 0) return 0;
  if (cache->level == 1) return frequency;
  assert (hasCache (cache->type, cache->level - 1));

  CacheParam *previous_cache = getCache (cache->type, cache->level - 1);
  size_t previous_occurrences = getOccurrencesCount (frequency, context, inst, previous_cache);

  if (previous_occurrences == 0)
    return 0;

  string previous_chmc = getCHMC (context, inst, previous_cache);
  if (previous_chmc == "AH") return 0;
  if (previous_chmc == "AM") return previous_occurrences;
  if (previous_chmc == "NC") return previous_occurrences;
  if (previous_chmc == "FM") return std::min (previous_occurrences, countAccessedBlocks (context, inst, previous_cache));

  stringstream errorstr;
  errorstr << "CacheStatistics: Faulty CHMC for " << getCacheTypeName (cache->type) <<
    " cache level " << cache->level - 1 << " is '" << previous_chmc << "'" << endl;
  Logger::addFatal (errorstr.str ());
  return 0;
}


// Count the number of different cache blocks that may be accessed by inst in
// the specified context on cache level cache.
size_t
CacheStatistics::countAccessedBlocks (Context * context, Instruction * inst, CacheParam * cache)
{
  size_t count = 0;
  string attribute_name = AddressAttributeName;
  if (cache->type == DCACHE)
    { 
      //FIXME Contextual stack adresses.
      attribute_name = AnalysisHelper::mkContextAttrName(AddressAttributeName, context);
      if (!inst->HasAttribute (attribute_name))
	{
	  attribute_name = AddressAttributeName;
	}
    }

  AddressAttribute address_attr = (AddressAttribute &) inst->GetAttribute (attribute_name);

  vector < AddressInfo > addresses = address_attr.getListInfo ();
  for (size_t i = 0; i < addresses.size (); ++i)
    {
      if ((cache->type == ICACHE && addresses[i].getSegment () == "code") || (cache->type == DCACHE && addresses[i].getSegment () != "code"))
	{
	  const vector < pair < string, string > >&regions = addresses[i].getAdrSize ();
	  for (size_t r = 0; r < regions.size (); ++r)
	    {
	      t_address from = atol (regions[r].first.c_str ());
	      unsigned int range = atoi (regions[r].second.c_str ());
	      for (t_address add = from - (from % cache->cachelinesize);
		   add <= (from + range - 1) - ((from + range - 1) % cache->cachelinesize); add += cache->cachelinesize)
		{
		  ++count;
		}
	    }
	}
    }

  assert (cache->type != ICACHE || count == 1);

  return count;
}


// Check for a cache of the specified type at the specified level.
bool
CacheStatistics::hasCache (t_cache_type cache_type, int level)
{
  const vector < CacheParam * >&level_parameters = getCacheLevelParameters (level);
  bool has_cache = false;

  // Search for instruction or dcache in the hierarchy configuration
  for (size_t c = 0; c < level_parameters.size (); ++c)
    {
      if (level_parameters[c]->type == cache_type)
	{
	  assert (!has_cache);
	  has_cache = true;
	}
    }

  return has_cache;
}

// Return the cache of the specified type at the specified level.
// NULL if no such cache exists.
CacheParam *
CacheStatistics::getCache (t_cache_type cache_type, int level)
{
  const vector < CacheParam * >&level_parameters = getCacheLevelParameters (level);
  CacheParam *cache = NULL;

  // Search for instruction or dcache in the hierarchy configuration
  for (size_t c = 0; c < level_parameters.size (); ++c)
    {
      if (level_parameters[c]->type == cache_type)
	{
	  assert (cache == NULL);
	  cache = level_parameters[c];
	}
    }

  return cache;
}

// Return printable screen name for cache type.
string
CacheStatistics::getCacheTypeName (t_cache_type cache_type)
{
  switch (cache_type)
    {
    case ICACHE:
      return "ICACHE";
    case DCACHE:
      return "DCACHE";
    case PERFECTICACHE:
      return "PERFECTICACHE";
    case PERFECTDCACHE:
      return "PERFECTDCACHE";
    }
  return "UNKNOWN_CACHE";
}



/* Defining has_icache_info[l] has_dcache_info[l] : Check every instruction have a cache
   categorization for the l-th cache, for all caches of the configuration.*/
void CacheStatistics::CheckInstrHaveCacheCategorizationAttribute ()
{

  Context * vcontext;
  Cfg * c;
  Node * n;

  CallGraph callgraph (program);
  const vector < Cfg * >&cfgs = program->GetAllCfgs ();
  size_t  nbcfg = cfgs.size ();
  int level_count = countCacheLevels ();  // number of caches in the configuration..
  
  for (int l = 1; l <= level_count; ++l)
    {
      bool has_icache = hasCache (ICACHE, l);
      bool has_dcache = hasCache (DCACHE, l);
      
      has_icache_info.push_back(has_icache);
      has_dcache_info.push_back(has_dcache);
      for (size_t i = 0; i < nbcfg; i++)
	{
	  c = cfgs[i];
	  if (!callgraph.isDeadCode (c))
	    {
	      const ContextList & contexts = (ContextList &) c->GetAttribute (ContextListAttributeName);
	      unsigned int nc = contexts.size ();
	      const vector < Node * >&nodes = c->GetAllNodes ();
	      for (size_t j = 0; j < nodes.size (); j++)
		{
		  n = nodes[j];	  
		  // if (currentNode->IsBB())
		  if (! n->isIsolatedNopNode())
		    for (unsigned int ic = 0; ic < nc; ic++)
		      {
			const vector < Instruction * >&vi = n->GetAsm ();
			vcontext = contexts[ic];
			// Check for the presence of the required CHMC classifications.
			for (unsigned int i = 0; i < vi.size (); i++)
			  {
			    Instruction *instr = vi[i];
			    if (has_icache)
			      {
				if (! instr->HasAttribute (AnalysisHelper::mkContextAttrName(CHMCAttributeNameCode (l), vcontext))) 
				  has_icache_info[l-1] = false;
			      }
			 
			    if (has_dcache && Arch::isLoad (instr->GetCode ()))
			      {
				if (! instr->HasAttribute (AnalysisHelper::mkContextAttrName( CHMCAttributeNameData (l), vcontext)))
				  has_dcache_info[l-1] = false;
			      }
			  }
		      }
		}
	    }
       }
   }

  /* for debug (lbesnard)
   for (int l = 0; l < level_count; ++l)
   {
   cout << " has_icache_info [" << l << "] = " <<  has_icache_info[l] << endl;
   cout << " has_dcache_info [" << l << "] = " <<  has_dcache_info[l] << endl;
   }
  */
  
}
