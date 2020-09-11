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

#ifndef ICACHE_ANALYSIS_H
#define ICACHE_ANALYSIS_H

#include <vector>
#include <map>
#include <string>
#include "Generic/Analysis.h"
#include "Specific/CacheAnalysis/Cache.h"
#include "Specific/CacheAnalysis/CacheAnalysis.h"

#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"

/**
   Instruction Cache analysis (interprocedural, context-sensitive, non-inclusive multi-level, LRU, PLRU, MRU,FIFO, RANDOM replacement policies)

   Generates a per-instruction categorization of instructions wrt
   cache (ALWAYS_HIT, ALWAYS_MISS, FIRST_MISS, Not_Classified, Always_Unused)
   and an access classification for the next level (Always, Never, Uncertain, Uncertain-Never).

   The analysis is based on Ferdinand's work, based on abstract interpretation. For every basic block,
   the following informations are computed:
 
   - Must ACS (in and out) : abstract cache states which determine if a memory block is always present
     in the cache at a given point.
   - PS ACS (in and out) : abstract cache states of the persistence analysis which determine if a memory
     block will not be evicted after it has been first loaded
   - May ACS (in and out) : abstract cache states which determine if a memory block may be in the cache
     at a given point.
	
   The analysis requires the context attribute and an access classification for the level under analysis.

   This analysis is intended to be used in architectures where a MISS
   is the worst-case (in case of conflict for a cache block, the
   instruction is categorized as a MISS or NC and considered as
   a MISS during the WCET computation (see IPET analysis).

   The analysis currently support LRU, PLRU, MRU, FIFO and RANDOM replacement policies.
   
   papers:
   - Fast and Precise WCET Prediction by separated Cache and Path Analyses. H. Theiling, C. Ferdinand, and R. Wilhelm. RTSJ 2000 (for must and may analyses)
   - Scope-aware data cache analysis for WCET estimation. B. K. Huynh, L. Ju, and A. Roychoudhury. RTAS 2011. (for the persistence analysis)
   - Timing predictability of cache replacement policies. J. Reineke, D. Grund, C. Berg, and R. Wilhelm. RTSJ 2007 (for the replacement policies)
   - WCET analysis of multi-level non-inclusive set-associative instruction caches. D. Hardy, I. Puaut. RTSS 2008 (for the cache hierarchy)
*/

class ICacheAnalysis: public Analysis
{
private:

  /** Cache parameters */
  int nb_sets, nb_ways, cacheline_size;
  t_replacement_policy replacement_policy;

  /** Analyses to be performed */
  bool perform_must_analysis;
  bool perform_persistence_analysis;
  bool perform_may_analysis;
  bool perfectIcache;

  /** to keep the age of accesses outside the analysis */
  bool keep_age;

  /** multilevel analysis: current level */
  int levelAnalysis;

  /** Program call graph (used for detection of dead code to speed up the analysis). */
  CallGraph *call_graph;

  /** First Step of the MUST analysis: Fixed point computation of MUST Abstract Cache States (ACS) without considering backedges. */
  bool FixPointMust1stStep ();

  /** Fixed point computation of MUST Abstract Cache States (ACS). */
  bool MustAnalysis ();

  /** Fixed point computation of MAY Abstract Cache States (ACS). */
  bool MayAnalysis ();

  /** Fixed point computation of PS Abstract Cache States (ACS). */
  bool PSAnalysis ();


  template < typename T > void compute_ACS_out(ContextualNode & current, Instruction *vinstr, AbstractCache < T > &ACS_out, string idAccessName);

  /** @return the ACS_out, for an analysis T, of a ContextualNode (current). 
      The initial ACS_out is the ACS_in of the current analysis (given by the attribute inAnalysisName in the context of current).
      Then the ACS_out is updated for each Load instructions of the node.
      Remark: inAnalysisName ::= ACSMUSTInName | ACSMAYInName | ACSPSInName. */
  template<typename T> AbstractCache < T > compute_ACS_out(ContextualNode &current, string &inAnalysisName);

  /** FixPointMust1stStep analysis: Compute the ACS_out a set of nodes (work), without considering backedges.
      @return a set of nodes for which the ACS_in must be computed. */
  set < ContextualNode > FixPointMust1stStep_ACS_out(set < ContextualNode >&work_in, set < Edge * >& backedges );

  /** FixPointMust1stStep analysis: Compute the ACS_in of a set of nodes (work) without considering backedges.
      @return a set of nodes for which the ACS_out must be computed. */
  set < ContextualNode > FixPointMust1stStep_ACS_in(set < ContextualNode >&work, set < Edge * >& backedges );

  /** MustAnalysis analysis: Compute the ACS_out a set of nodes (work).
      @return a set of nodes for which the ACS_in must be computed (all the nodes have to be visited at least once).*/
  set < ContextualNode > MustAnalysis_ACS_out(set < ContextualNode >&work, set < ContextualNode > &visited);

  /** MustAnalysis: Compute the ACS_in of a set of nodes (work_in). 
      @return a set of nodes for which the ACS_out must be computed (all the nodes have to be visited at least once). */
  set < ContextualNode > MustAnalysis_ACS_in(set < ContextualNode >&work_in, set < ContextualNode > &visited);

  /** MayAnalysis analysis: Compute the ACS_out a set of nodes (work).
      @return a set of nodes for which the ACS_in must be computed (all the nodes have to be visited at least once).*/
  set < ContextualNode > MayAnalysis_ACS_out(set < ContextualNode > &work);

  /** MayAnalysis: Compute the ACS_in of a set of nodes (work_in). 
      @return a set of nodes for which the ACS_out must be computed (all the nodes have to be visited at least once). */
  set < ContextualNode > MayAnalysis_ACS_in(set < ContextualNode > &work_in);

  /** PSAnalysis: Compute the ACS_out of a set of nodes (work).
       @return a set of nodes for which the ACS_in must be computed. */
  set < ContextualNode > PSAnalysis_ACS_out(set < ContextualNode >&work);

  /** PSAnalysis: Compute the ACS_in of a set of nodes (work_in). 
      @return a set of nodes for which the ACS_out must be computed. */
  set < ContextualNode > PSAnalysis_ACS_in(set < ContextualNode >&work_in);

public:

  /** @return an empty Must cache */
    AbstractCache < MUST > CacheFactoryMUST () const;
  /** @return an empty PS cache */
    AbstractCache < PS > CacheFactoryPS () const;
  /** @return an empty May cache */
    AbstractCache < MAY > CacheFactoryMAY () const;

  /** Map used to determine the next level CAC based on current CAC and CHMC */
    map < string, map < string, SerialisableStringAttribute > >cac_computation;

  /** Constructor. Sets up cache parameters */
    ICacheAnalysis (Program * p, int nbsets, int nbways, int cachelinesize,
		    t_replacement_policy r, int cacheLevel, bool apply_must, bool apply_persistence, bool apply_may, bool keepage, bool picache);

  /** Destructor. */
   ~ICacheAnalysis ()
  {
    delete call_graph;
  };

  // Useful methods
   /** Check attribute method */
   bool CheckInputAttributes ();
   /** Perform Analysis method */
   bool PerformAnalysis ();
   
   /** Remove all private attributes*/
   void RemovePrivateAttributes ();   

  /** Accessors */
  int getNbSets () const
  {
    return nb_sets;
  };

  int getNbWays () const
  {
    return nb_ways;
  };

  int getLineSize () const
  {
    return cacheline_size;
  };

  int getLevelAnalysis () const
  {
    return levelAnalysis;
  };

  t_replacement_policy getReplacementPolicy () const
  {
    return replacement_policy;
  };

  CallGraph *getCallGraph () const
  {
    return call_graph;
  };

  bool getKeepAge () const
  {
    return keep_age;
  };

};

#endif
