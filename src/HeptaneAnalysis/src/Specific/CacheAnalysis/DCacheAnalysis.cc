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
   Data Cache analysis for write-through caches (interprocedural, context-sensitive, non-inclusive multi-level, LRU, PLRU, MRU,FIFO, RANDOM replacement policies)
 
   Generates a per-load-instruction categorization of load instructions wrt
   cache (ALWAYS_HIT, ALWAYS_MISS, FIRST_MISS, Not_Classified, Always_Unused)
   and an access classification for the next level (Always, Never, Uncertain, Uncertain-Never).
 
   The analysis is based on abstract interpretation. For every basic block,
   the following informations are computed:
 
   - Must ACS (in and out) : abstract cache states which determine if a memory block is always present in the cache at a given point.
   - PS ACS (in and out) : abstract cache states of the persistence analysis which determine if a memory block will not be evicted after it has been first loaded
   - May ACS (in and out) : abstract cache states which determine if a memory block may be in the cache at a given point.
 
   The analysis requires the context attribute and an access classification for the level under analysis.
 
   This analysis is intended to be used in architectures where a MISS
   is the worst-case (in case of conflict for a cache block, the
   load is categorized as a MISS or NC and considered as
   a MISS during the WCET computation (see IPET analysis).
 
   The analysis currently support LRU, PLRU, MRU, FIFO and RANDOM replacement policies.
 
   papers:
   - WCET analysis of multi-level set-associative data caches. B. Lesage, D. Hardy, I. Puaut. WCET 2009 (for the data caches)
   - Fast and Precise WCET Prediction by separated Cache and Path Analyses. H. Theiling, C. Ferdinand, and R. Wilhelm. RTSJ 2000 (for must and may analyses)
   - Scope-aware data cache analysis for WCET estimation. B. K. Huynh, L. Ju, and A. Roychoudhury. RTAS 2011. (for the persistence analysis)
   - Timing predictability of cache replacement policies. J. Reineke, D. Grund, C. Berg, and R. Wilhelm. RTSJ 2007 (for the replacement policies)
   - WCET analysis of multi-level non-inclusive set-associative instruction caches. D. Hardy, I. Puaut. RTSS 2008 (for the cache hierarchy)
*/

#include <cmath>
#include "Specific/CacheAnalysis/DCacheAnalysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Generic/Timer.h"
#include "arch.h"

// inlines...
#define getACSNode(T, NODE, IDATTR) ((AbstractCacheStateAttribute < T > &)NODE->GetAttribute(IDATTR))
#define getACSContextualNode(T, ELEM, IDATTR) getACSNode(T, ELEM.node,IDATTR)

/*************************************************************************************************************************
 CacheFactory functions
**************************************************************************************************************************/

/** Returns an empty Must cache */
AbstractCache < MUST > DCacheAnalysis::CacheFactoryMUST() const
{
  // default initialization (LRU)
  unsigned int nb_ways_analysis = nb_ways;
  unsigned int nb_ways_removed = 0;

  // Compute length of ACS for Must analysis depending on the replacement policy
  switch (replacement_policy)
    {
    case LRU:
      nb_ways_analysis = nb_ways;
      nb_ways_removed = 0;
      break;
    case PLRU:
      if (nb_ways == 1)
	{
	  nb_ways_analysis = 1;
	}
      else
	{
	  double n = log((double)nb_ways) / log(2.0) + 1;
	  nb_ways_analysis = static_cast < unsigned int >(n);
	}
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case MRU:
      nb_ways_analysis = (nb_ways == 1) ? 1 : 2;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case FIFO:
      nb_ways_analysis = 1;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case RANDOM:
      nb_ways_analysis = 1;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    default:
      Logger::addFatal("DCacheAnalysis: the replacement policy is not implemented");
    }

  return AbstractCache < MUST > (nb_sets, nb_ways_analysis, nb_ways_removed, cacheline_size);
}

/* Returns an empty PS cache */
AbstractCache < PS > DCacheAnalysis::CacheFactoryPS() const
{
  // default initialization (LRU)
  unsigned int nb_ways_analysis = nb_ways;
  unsigned int nb_ways_removed = 0;

  // Compute length of ACS for PS analysis depending on the replacement policy
  switch (replacement_policy)
    {
    case LRU:
      nb_ways_analysis = nb_ways;
      nb_ways_removed = 0;
      break;
    case PLRU:
      if (nb_ways == 1)
	{
	  nb_ways_analysis = 1;
	}
      else
	{
	  double n = log((double)nb_ways) / log((double)2) + 1;
	  nb_ways_analysis = static_cast < unsigned int >(n);
	}
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case MRU:
      nb_ways_analysis = (nb_ways == 1) ? 1 : 2;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case FIFO:
      nb_ways_analysis = 1;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    case RANDOM:
      nb_ways_analysis = 1;
      nb_ways_removed = nb_ways - nb_ways_analysis;
      break;
    default:
      Logger::addFatal("DCacheAnalysis: the replacement policy is not implemented");
    }

  return AbstractCache < PS > (nb_sets, nb_ways_analysis, nb_ways_removed, cacheline_size);
}

/* Build an empty May cache */
/*TODO: implementation of an infinite ACS for the PLRU and RANDOM May analysis (check MLS/Evict bound)*/
AbstractCache < MAY > DCacheAnalysis::CacheFactoryMAY() const
{
  // default initialization (LRU)
  unsigned int nb_ways_analysis = nb_ways;
  // Compute length of ACS for May analysis
  // depending on the replacement policy
  switch (replacement_policy)
    {
    case LRU:
      nb_ways_analysis = nb_ways;
      break;
    case PLRU:
      if (nb_ways <= 2)
	{
	  nb_ways_analysis = nb_ways;
	}
      else
	{
	  Logger::addFatal("DCacheAnalysis: The MAY analysis is not implemented for the PLRU replacement policy when #ways>2, sorry...");
	}
      break;
    case MRU:
      nb_ways_analysis = 2 * nb_ways - 2;
      break;
    case FIFO:
      nb_ways_analysis = 2 * nb_ways - 1;
      break;
    case RANDOM:
      Logger::addFatal("DCacheAnalysis: The MAY analysis is not implemented for the RANDOM replacement policy, sorry...");
      break;
    default:
      Logger::addFatal("DCacheAnalysis: The replacement policy is not implemented");
      exit(0);
    }

  return AbstractCache < MAY > (nb_sets, nb_ways_analysis, cacheline_size);
}

/*************************************************************************************************************************
 Basic functions
**************************************************************************************************************************/

// -------------------------------------------------
// Checks every Load instruction in the node
// contains a CAC attribute
// for the current cache level
//
// Remarks: L1 does not have CAC attribute
// -------------------------------------------------
static bool CheckInstrHaveDataAccessAttribute(Cfg * c, Node * n, void *param)
{
  int level = *((int *)param);
  string attributeName = CACAttributeNameData(level);
  assert(attributeName != "");

  if (!(c->HasAttribute(ContextListAttributeName)))
    {
      stringstream errorstr;
      errorstr << "DCacheAnalysis: contexts are not computed";
      Logger::addFatal(errorstr.str());
    }

  // for each context
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); ++context)
    {
      string id = AnalysisHelper::mkContextAttrName(attributeName, *context);
      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      if (! vi[i]->HasAttribute(id))
		{
		  return false; 
		}
	    }
	}
    }

  return true;
}

// -------------------------------------------------
// Checks every Load/Store instruction in the node
// contains a data address attribute
// -------------------------------------------------
static bool CheckInstrHaveDataAddresses(Cfg * c, Node * n, void *param)
{
  vector < Instruction * >vi = n->GetAsm();
  for (size_t i = 0; i < vi.size(); i++)
    {
      string vinstr = vi[i]->GetCode();
      if (Arch::isLoad(vinstr) || Arch::isStore(vinstr))
	{
	  assert(c->HasAttribute(ContextListAttributeName));
	  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
	  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); ++context)
	    {
	      if (vi[i]->HasAttribute(AddressAttributeName) == false) return false;

	      AddressAttribute attributeValue = (AddressAttribute &) vi[i]->GetAttribute(AddressAttributeName);
	      string attributeName = AnalysisHelper::mkContextAttrName( AddressAttributeName, *context);

	      if (vi[i]->HasAttribute(attributeName))
		{
		  attributeValue = (AddressAttribute &) vi[i]->GetAttribute(attributeName);
		}
	      vector < AddressInfo > a = attributeValue.getListInfo();
	      //FIXME Not for contextual address information
	      //if(a.size()<2){return false;} // at least one address for the code and one for the accessed data

	      bool found = false;
	      for (size_t j = 0; j < a.size(); j++)
		{
		  // FIXME Pointers add the stack as a potentially accessed
		  // segment, but no contextual information is available.
		  //if (a[j].getSegment() == "stack")//if the accessed data is in the stack, the access has to be contextual
		  //{
		  //    if(vi[i]->HasAttribute(attributeName) == false){return false;}
		  //}

		  if (a[j].getSegment() != "code")
		    {
		      found = true;
		    }
		}
	      if (!found)
		{
		  return false;
		}
	    }
	}
    }
  return true;
}

// --------------------------------------------------------
// initialisation of the L1 access attribute
// to A (always) for all Load instructions of the basic block
// --------------------------------------------------------
static bool initL1AccessAttributeForData(Cfg * c, Node * n, void *param)
{
  SerialisableStringAttribute A("A");	// L1 cache is always acceded
  string CACattName = CACAttributeNameData(1);

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); ++context)
    {
      string currentContext = (*context)->getStringId();

      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      vi[i]->SetAttribute(AnalysisHelper::mkContextAttrName(CACattName, currentContext), A);
	    }
	}
    }
  return true;
}

// returns the data addresses accessed by an instruction in a context.
set < t_address > DCacheAnalysis::getDataAddress(Instruction * instruction, Context * context)
{
  set < t_address > accessedBlocks;
  string attributeName = AnalysisHelper::mkContextAttrName(AddressAttributeName, context);

  if (!instruction->HasAttribute(attributeName))
    {
      attributeName = AddressAttributeName;
    }  //stub: the accesses in the stack are contextual but the others

  AddressAttribute & attributeValue = (AddressAttribute &) instruction->GetAttribute(attributeName);
  vector < AddressInfo > a = attributeValue.getListInfo();

  for (size_t i = 0; i < a.size(); i++)
    {
      if (a[i].getSegment() != "code")
	{
	  vector < pair < string, string > >addressRanges = a[i].getAdrSize();
	  for (size_t j = 0; j < addressRanges.size(); j++)
	    {
	      t_address from = atol(addressRanges[j].first.c_str());
	      unsigned int range = atoi(addressRanges[j].second.c_str());
	      int unsigned ifrom = (from - (from % cacheline_size));
	      int unsigned ito = ((from + range - 1) - ((from + range - 1) % cacheline_size));
	      for (t_address k = ifrom; k <= ito; k += cacheline_size)
		{
		  accessedBlocks.insert(k);
		}
	    }
	}
    }
  return accessedBlocks;
}

//------------------------------------------------
// Block Count Attribute computation
//------------------------------------------------
bool static ComputeBlockCountAttribute(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string att_name = AnalysisHelper::mkContextAttrName( BlockCountAttributeName(ca->getLevelAnalysis()), currentContext);

      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      set < t_address > accessedBlocks = ca->getDataAddress(vi[i], (*context));
	      assert(accessedBlocks.size() > 0);
	      SerialisableIntegerAttribute blockCountAttribute(accessedBlocks.size());
	      vi[i]->SetAttribute(att_name, blockCountAttribute);
	    }
	}
    }
  return true;
}

//------------------------------------------------
// StubFunction: To put CACDATA=A CHMCDATA=AU to all non-load instruction
// Useful for the perfect cache assumption in the IPET ANALYSIS
//
// Put CHMC_DATA=AU for all non-load instructions
//------------------------------------------------
bool static StubFunction(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  SerialisableStringAttribute AUnref("AU");	// Always-unreferenced to consider WCET cost = 0

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string CHMCAttName = AnalysisHelper::mkContextAttrName( CHMCAttributeNameData(ca->getLevelAnalysis()), *context);
      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (! Arch::isLoad(vi[i]->GetCode()))
	    {
	      vi[i]->SetAttribute(CHMCAttName, AUnref);
	    }
	}
    }
  return true;
}


template < typename T > void DCacheAnalysis::compute_ACS_out(ContextualNode & current, Instruction *vinstr, AbstractCache < T > &ACS_out, string idAccessName) 
{
  if (Arch::isLoad(vinstr->GetCode()))
    {
      assert(vinstr->HasAttribute(idAccessName));
      string accessValue = ((SerialisableStringAttribute &) (vinstr->GetAttribute(idAccessName))).GetValue();
      if (accessValue != "N")
	{
	  set < t_address > add = getDataAddress(vinstr, current.context);
	  ACS_out.Update(add, accessValue);
	}
    }
}

/*
   @return the ACS_out, for an analysis T, of a ContextualNode (current). The initial ACS_out is the given by the ACS_in of the current analysis provided by inAnalysisName for the context.
   (inAnalysisName ::= ACSMUSTInName | ACSMAYInName | ACSPSInName )
*/
template < typename T > AbstractCache < T > DCacheAnalysis::compute_ACS_out(ContextualNode & current, string & inAnalysisName)
{
  string attributeAccessName = CACAttributeNameData(levelAnalysis);
  string idCurrentContext = current.context->getStringId();
  string idAccessName;

  AbstractCacheStateAttribute < T > &ca_attr_in = getACSContextualNode(T, current, inAnalysisName + idCurrentContext);
  AbstractCache < T > ACS_out = ca_attr_in.cache;

  idAccessName = AnalysisHelper::mkContextAttrName(attributeAccessName, idCurrentContext);
  vector < Instruction * >vi = current.node->GetAsm();
  for (size_t i = 0; i < vi.size(); i++)
    {
      compute_ACS_out<T> (current, vi[i], ACS_out, idAccessName); 
    }
  return ACS_out;
}



/*************************************************************************************************************************
 MUST ANALYSIS
**************************************************************************************************************************/

//-------------------------------------------------------
// Initialization of abstract cache for MUST analysis
//-------------------------------------------------------
static bool initACSMUST(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  AbstractCache < MUST > ACS_empty = ca->CacheFactoryMUST();
  AbstractCacheStateAttribute < MUST > att(ACS_empty);

  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      n->SetAttribute(in + currentContext, att);
      n->SetAttribute(out + currentContext, att);
    }

  return true;
}

/* FixPointMust1stStep analysis: Compute the ACS_out a set of nodes (work), without considering backedges.
   @return a set of nodes for which the ACS_in must be computed. */
set < ContextualNode > DCacheAnalysis::FixPointMust1stStep_ACS_out(set < ContextualNode > &work, set < Edge * >&backedges)
{
  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;
  set < ContextualNode > work_in;

  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      AbstractCache < MUST > ACS_out = compute_ACS_out < MUST > (current, in);

      AbstractCacheStateAttribute < MUST > &ca_attr_out = getACSContextualNode(MUST, current, out + current.context->getStringId());
      if (!ca_attr_out.cache.Equals(ACS_out))
	{
	  ca_attr_out.cache = ACS_out;
	  AnalysisHelper::insertContextualSuccessorsExcludingBackEdges(current, work_in, backedges);
	}
    }
  return work_in;
}

/* FixPointMust1stStep analysis: Compute the ACS_in of a set of nodes (work) without considering backedges.
   @return a set of nodes for which the ACS_out must be computed. */
set < ContextualNode > DCacheAnalysis::FixPointMust1stStep_ACS_in(set < ContextualNode > &work_in, set < Edge * >&backedges)
{
  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;
  set < ContextualNode > work_out;
  string idAttr;
  bool b;
  ContextualNode pred;

  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      ContextualNode current = *it;

      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	//it should not be the program's entry node

      AbstractCache < MUST > new_ACS_in;
      bool first = true;
      for (size_t i = 0; i < predecessors.size(); i++)
	{
	  pred = predecessors[i];
	  b = AnalysisHelper::FilterBackedge(current.node, pred.node, backedges);
	  if (b)
	    {
	      idAttr = out + pred.context->getStringId();
	      if (first)
		{
		  first = false;
		  new_ACS_in = getACSContextualNode(MUST, pred, idAttr).cache;
		}
	      else
		{
		  new_ACS_in.Join( getACSContextualNode(MUST, pred, idAttr).cache);
		}
	    }
	}

      AbstractCacheStateAttribute < MUST > &ca_attr_in = getACSContextualNode(MUST, current, in + current.context->getStringId());

      if (!ca_attr_in.cache.Equals(new_ACS_in))
	{
	  ca_attr_in.cache = new_ACS_in;
	  work_out.insert(current);
	}
    }
  return work_out;
}


/* First Step of the MUST analysis: Fixed point computation of MUST Abstract Cache States (ACS) without considering backedges.
   Remarks: Introduced for a precise classification of access performed inside loops.
   This approach avoids a bottom state in the ACS as defined in Ferdinand's Thesis.
*/
bool DCacheAnalysis::FixPointMust1stStep()
{
  set < ContextualNode > work_in, work;

  AnalysisHelper::applyToAllNodesRecursive(p, initACSMUST, (void *)this);
  set < Edge * >backedges = AnalysisHelper::compute_backedges(p, call_graph); // getting the backedges.
  work = AnalysisHelper::initWork();
  while (!work.empty())
    {
      work_in = FixPointMust1stStep_ACS_out(work, backedges);
      work.clear();
      work = FixPointMust1stStep_ACS_in(work_in, backedges);
      work_in.clear();
    }

  return true;
}

/* MustAnalysis analysis: Compute the ACS_out a set of nodes (work).
   @return a set of nodes for which the ACS_in must be computed (all the nodes have to be visited at least once).*/
set < ContextualNode > DCacheAnalysis::MustAnalysis_ACS_out(set < ContextualNode > &work, set < ContextualNode > &visited)
{
  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;
  set < ContextualNode > work_in;
  bool b;

  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      AbstractCache < MUST > ACS_out = compute_ACS_out < MUST > (current, in);
      AbstractCacheStateAttribute < MUST > &ca_attr_out = getACSContextualNode( MUST, current, out + current.context->getStringId());
      b = !ca_attr_out.cache.Equals(ACS_out);
      if (b) ca_attr_out.cache = ACS_out;
      
      //To force the visit of all nodes
      if (visited.find(current) == visited.end())
	{
	  visited.insert(current);
	  b = true;
	}
      if (b) AnalysisHelper::insertContextualSuccessors(current, work_in);
    }
  return work_in;
}

/* MustAnalysis: Compute the ACS_in of a set of nodes (work). 
   @return a set of nodes for which the ACS_out must be computed (all the nodes have to be visited at least once). */
set < ContextualNode > DCacheAnalysis::MustAnalysis_ACS_in(set < ContextualNode > &work_in, set < ContextualNode > &visited)
{
  bool b;
  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;
  set < ContextualNode > work_out;

  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      ContextualNode current = *it;

      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	//it should not be the program's entry node

      AbstractCache < MUST > new_ACS_in = getACSContextualNode( MUST, predecessors[0], out + predecessors[0].context->getStringId()).cache;
      for (size_t i = 1; i < predecessors.size(); i++)
	{
	  new_ACS_in.Join(getACSContextualNode (MUST, predecessors[i], out + predecessors[i].context->getStringId()).cache);
	}

      AbstractCacheStateAttribute < MUST > &ca_attr_in = getACSContextualNode(MUST, current, in + current.context->getStringId());
      b = ! ca_attr_in.cache.Equals(new_ACS_in);
      if (b) ca_attr_in.cache = new_ACS_in;
      // To force the visit of all nodes
      b = b || (visited.find(current) == visited.end());
      if (b) work_out.insert(current);
    }
  return work_out;
}



/* MUST ANALYSIS.
   Fixed point computation of MUST Abstract Cache States (ACS).
   All nodes have to be visited at least once. */
bool DCacheAnalysis::MustAnalysis()
{
  set < ContextualNode > work, visited, work_in;

  FixPointMust1stStep();
  work = AnalysisHelper::initWork();
  while (!work.empty())
    {
      // AnalysisHelper::printSet(work, "DCacheAnalysis"); // debug
      work_in = MustAnalysis_ACS_out(work, visited);
      work.clear();
      work = MustAnalysis_ACS_in(work_in, visited);
      work_in.clear();
    }

  return true;
}

//-------------------------------------------------------
// MUST CHMC classification
//  Determines if an access can be classified as AH
//  Removes the MUST analysis attributes
//-------------------------------------------------------
bool static ClassifCHMCMust(Cfg * c, Node * n, void *param)
{
  SerialisableStringAttribute AHatt("AH");	// Always-hit
  SerialisableStringAttribute AUnref("AU");	// Always-unreferenced to consider WCET cost = 0
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  string CHMCAttName = CHMCAttributeNameData(ca->getLevelAnalysis());
  string in = ACSMUSTInName;
  string out = ACSMUSTOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string CACattName = AnalysisHelper::mkContextAttrName(CACAttributeNameData(ca->getLevelAnalysis()), currentContext);

      assert(n->HasAttribute(in + currentContext));
      AbstractCache < MUST > ca_must = getACSNode(MUST, n, in + currentContext).cache;

      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      assert(vi[i]->HasAttribute(CACattName));
	      string accessValue = ((SerialisableStringAttribute &) (vi[i]->GetAttribute(CACattName))).GetValue();
	      string id = AnalysisHelper::mkContextAttrName(CHMCAttName, currentContext);
	      if (accessValue == "N")	//if not accessed: AU
		{
		  vi[i]->SetAttribute(id, AUnref);
		}
	      else
		{
		  set < t_address > add = ca->getDataAddress(vi[i], (*context));
		  if (ca_must.AllPresent(add))	//if all present: AH
		    {
		      vi[i]->SetAttribute(id, AHatt);
		    }

		  ca_must.Update(add, accessValue);	//simulate the access for the next instruction
		}
	    }
	}

      // Remove the ACS_in and ACS_out attributes
      n->RemoveAttribute(in + currentContext);
      n->RemoveAttribute(out + currentContext);
    }
  return true;
}

/*************************************************************************************************************************
 MAY ANALYSIS
**************************************************************************************************************************/

//-------------------------------------------------------
// Initialization of abstract cache for MAY analysis
//-------------------------------------------------------
bool static initACSMAY(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  AbstractCache < MAY > ACS_empty = ca->CacheFactoryMAY();
  AbstractCacheStateAttribute < MAY > att(ACS_empty);

  string in = ACSMAYInName;
  string out = ACSMAYOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      n->SetAttribute(in + currentContext, att);
      n->SetAttribute(out + currentContext, att);
    }

  return true;
}

/* MayAnalysis analysis: Compute the ACS_out a set of nodes (work).
   @return a set of nodes for which the ACS_in must be computed (all the nodes have to be visited at least once).*/
set < ContextualNode > DCacheAnalysis::MayAnalysis_ACS_out(set < ContextualNode > &work, set < ContextualNode > &visited)
{
  string in = ACSMAYInName;
  string out = ACSMAYOutName;
  string attributeAccessName = CACAttributeNameData(levelAnalysis);

  set < ContextualNode > work_in;

  //compute ACS_out
  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;

      AbstractCache < MAY > ACS_out = compute_ACS_out < MAY > (current, in);
      AbstractCacheStateAttribute < MAY > &ca_attr_out =getACSContextualNode(MAY, current, out + current.context->getStringId());
      bool b = ! ca_attr_out.cache.Equals(ACS_out);
      if (b) ca_attr_out.cache = ACS_out;

      //To force the visit of all nodes
      if (visited.find(current) == visited.end())
	{
	  visited.insert(current);
	  b = true;
	}

      if (b) AnalysisHelper::insertContextualSuccessors(current, work_in);

    }
  return work_in;
}

/* MayAnalysis: Compute the ACS_in of a set of nodes (work). 
   @return a set of nodes for which the ACS_out must be computed (all the nodes have to be visited at least once). */
set < ContextualNode > DCacheAnalysis::MayAnalysis_ACS_in(set < ContextualNode > &work_in, set < ContextualNode > &visited)
{
  string in = ACSMAYInName;
  string out = ACSMAYOutName;
  set < ContextualNode > work;

  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      ContextualNode current = *it;
      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	//it should not be the program's entry node

      AbstractCache < MAY > new_ACS_in =getACSContextualNode (MAY, predecessors[0], out + predecessors[0].context->getStringId()).cache;
      for (size_t i = 1; i < predecessors.size(); i++)
	{
	  new_ACS_in.Join(getACSContextualNode(MAY, predecessors[i], out + predecessors[i].context->getStringId()).cache);
	}

      AbstractCacheStateAttribute < MAY > &ca_attr_in =getACSContextualNode ( MAY, current, in + current.context->getStringId());
      bool b = ! ca_attr_in.cache.Equals(new_ACS_in);
      if (b) { ca_attr_in.cache = new_ACS_in; }

      // To force the visit of all nodes
      b = b || (visited.find(current) == visited.end());
      if (b) work.insert(current);
    }
  return work;
}

/*
  MAY ANALYSIS.
  Fixed point computation of MAY Abstract Cache States (ACS).
  All nodes have to be visited at least once.
*/
bool DCacheAnalysis::MayAnalysis()
{
  set < ContextualNode > visited, work_in, work;

  AnalysisHelper::applyToAllNodesRecursive(p, initACSMAY, (void *)this);
  work = AnalysisHelper::initWork();
  while (!work.empty())
    {
      work_in = MayAnalysis_ACS_out(work, visited);
      work.clear();
      work = MayAnalysis_ACS_in(work_in, visited);
      work_in.clear();
    }
  return true;
}

//-------------------------------------------------------
// MAY CHMC classification
//  Determines if an access can be classified as AM
//  Removes the MAY analysis attributes
//-------------------------------------------------------
bool static ClassifCHMCMay(Cfg * c, Node * n, void *param)
{
  SerialisableStringAttribute AMatt("AM");	// Always-miss
  SerialisableStringAttribute AUnref("AU");	// Always-unreferenced to consider WCET cost = 0
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  string CHMCAttName = CHMCAttributeNameData(ca->getLevelAnalysis());
  string in = ACSMAYInName;
  string out = ACSMAYOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string CACattName = AnalysisHelper::mkContextAttrName( CACAttributeNameData(ca->getLevelAnalysis()), currentContext);

      assert(n->HasAttribute(in + currentContext));
      AbstractCache < MAY > ca_may = getACSNode(MAY, n, in + currentContext).cache;

      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      string id = AnalysisHelper::mkContextAttrName( CHMCAttName, currentContext);
	      if (!vi[i]->HasAttribute(id))	//if the chmc attribute was not set by the MUST or the PS analysis
		{
		  assert(vi[i]->HasAttribute(CACattName));
		  string accessValue = ((SerialisableStringAttribute &) (vi[i]->GetAttribute(CACattName))).GetValue();

		  if (accessValue == "N")	//if not accessed: AU
		    {
		      vi[i]->SetAttribute( id, AUnref);
		    }
		  else
		    {
		      set < t_address > add = ca->getDataAddress(vi[i], (*context));
		      if (!ca_may.OnePresent(add))	//if all absent: AM
			{
			  vi[i]->SetAttribute(id, AMatt);
			}

		      ca_may.Update(add, accessValue);	//simulate the access for the next instruction
		    }
		}
	    }
	}

      // Remove the ACS_in and ACS_out attributes
      n->RemoveAttribute(in + currentContext);
      n->RemoveAttribute(out + currentContext);
    }
  return true;
}

/*************************************************************************************************************************
 PS ANALYSIS
*************************************************************************************************************************/

//-------------------------------------------------------
// Initialization of abstract cache for persistent analysis
//
// Returns:
//  set<ContextualNode>: all outer loop's head of the program
//
// Remark:
//  the ACS is attach only in loop or when the
//  caller point (or transitive) is in a loop
//-------------------------------------------------------

//possible improvement: computation of identical loops in only one context

set < ContextualNode > initACSPS(Program * p, DCacheAnalysis * a)
{
  string in = ACSPSInName;
  string out = ACSPSOutName;
  set < ContextualNode > result;

  AbstractCache < PS > abstractCache = a->CacheFactoryPS();
  AbstractCacheStateAttribute < PS > ca(abstractCache);

  vector < Cfg * >cfgs = p->GetAllCfgs();

  for (size_t i = 0; i < cfgs.size(); i++)
    {
      if (!a->getCallGraph()->isDeadCode(cfgs[i]))
	{
	  assert(cfgs[i]->HasAttribute(ContextListAttributeName));

	  const ContextList & contexts = (ContextList &) cfgs[i]->GetAttribute(ContextListAttributeName);
	  for (ContextList::const_iterator context_it = contexts.begin(); context_it != contexts.end(); context_it++)
	    {
	      Context *context = *context_it;
	      string context_id = context->getStringId();

	      if (AnalysisHelper::CallerInLoop(context))	//if the current context is called in a loop
		{
		  vector < Node * >nodes = cfgs[i]->GetAllNodes();
		  for (size_t j = 0; j < nodes.size(); j++)
		    {
		      // Attach the attribute to all nodes in the cfg.
		      nodes[j]->SetAttribute(in + context_id, ca);
		      nodes[j]->SetAttribute(out + context_id, ca);
		    }
		}
	      else
		{
		  vector < Loop * >loops = cfgs[i]->GetAllLoops();
		  vector < Loop * >loopsOuter;

		  //get only the outers loops of the current cfg
		  for (size_t k = 0; k < loops.size(); k++)
		    {
		      bool found = false;
		      for (size_t j = 0; j < loops.size() && !found; j++)
			{
			  if (k != j && loops[k]->IsNestedIn(loops[j]))
			    {
			      found = true;
			    }	//loop k is nested in loop j
			}
		      if (!found)
			{
			  loopsOuter.push_back(loops[k]);
			}	//loop k is not nested in other loop
		    }
		  //now loopOuter contains only the loop not nested

		  for (size_t j = 0; j < loopsOuter.size(); ++j)
		    {
		      ContextualNode cn(context, loopsOuter[j]->GetHead());
		      result.insert(cn);

		      vector < Node * >nodes = loopsOuter[j]->GetAllNodes();
		      for (size_t k = 0; k < nodes.size(); ++k)
			{
			  // Attach the attribute to all nodes in the loop.
			  nodes[k]->SetAttribute(in + context_id, ca);
			  nodes[k]->SetAttribute(out + context_id, ca);
			}
		    }
		}
	    }
	}
    }
  return result;
}

/* PSAnalysis: Compute the ACS_out of a set of nodes (work).
   @return a set of nodes for which the ACS_in must be computed (all the nodes have to be visited at least once). */
set < ContextualNode > DCacheAnalysis::PSAnalysis_ACS_out(set < ContextualNode > &work, set < ContextualNode > &visited)
{
  string in = ACSPSInName;
  string out = ACSPSOutName;
  set < ContextualNode > work_in;
  string attributeAccessName = CACAttributeNameData(levelAnalysis);

  for (set < ContextualNode >::iterator it = work.begin(); it != work.end(); it++)
    {
      ContextualNode current = *it;
      AbstractCache < PS > ACS_out = compute_ACS_out < PS > (current, in);

      AbstractCacheStateAttribute < PS > &ca_attr_out = getACSContextualNode(PS, current, out + current.context->getStringId());
      bool b = ! ca_attr_out.cache.Equals(ACS_out);
      if (b) ca_attr_out.cache = ACS_out;

      // To force the visit of all nodes
      if (visited.find(current) == visited.end())
	{
	  b = true;
	  visited.insert(current);
	}

      if (b)
	{
	  vector < ContextualNode > succ = GetContextualSuccessors(current);
	  for (size_t i = 0; i < succ.size(); i++)
	    {
	      // A successor is added only if it is present in the loop
	      if (succ[i].node->HasAttribute(in + succ[i].context->getStringId())) 
		{
		  work_in.insert(succ[i]);
		}
	    }
	}
    }
  return work_in;
}

/* PSAnalysis: Compute the ACS_in of a set of nodes (work). 
   @return a set of nodes for which the ACS_out must be computed (all the nodes have to be visited at least once). */
set < ContextualNode > DCacheAnalysis::PSAnalysis_ACS_in(set < ContextualNode > &work_in, set < ContextualNode > &visited)
{
  string in = ACSPSInName;
  string out = ACSPSOutName;
  set < ContextualNode > work;

  for (set < ContextualNode >::iterator it = work_in.begin(); it != work_in.end(); it++)
    {
      ContextualNode current = *it;

      const vector < ContextualNode > &predecessors = GetContextualPredecessors(current);
      assert(predecessors.size() != 0);	//it should not be the program's entry node

      AbstractCache < PS > new_ACS_in;
      bool first = true;
      for (size_t i = 0; i < predecessors.size(); i++)
	{
	  if (predecessors[i].node->HasAttribute(out + predecessors[i].context->getStringId()))
	    {
	      if (first)
		{
		  first = false;
		  new_ACS_in =getACSContextualNode(PS, predecessors[i], out + predecessors[i].context->getStringId()).cache;
		}
	      else
		{
		  new_ACS_in.Join(getACSContextualNode( PS, predecessors[i], out + predecessors[i].context->getStringId()).cache);
		}
	    }
	}

      AbstractCacheStateAttribute < PS > &ca_attr_in =getACSContextualNode(PS, current, in + current.context->getStringId());
      bool b = ! ca_attr_in.cache.Equals(new_ACS_in);
      if (b) ca_attr_in.cache = new_ACS_in;

      // To force the visit of all nodes
      b = b || (visited.find(current) == visited.end());
      if (b) work.insert(current);

    }
  return work;
}

/* PS ANALYSIS.
    Fixed point computation of PS Abstract Cache States (ACS).
    All the nodes have to be visited at least once.
*/
bool DCacheAnalysis::PSAnalysis()
{
  set < ContextualNode > work, work_in, visited;

  work = initACSPS(p, this);
  while (!work.empty())
    {
      work_in = PSAnalysis_ACS_out(work, visited);
      work.clear();
      work = PSAnalysis_ACS_in(work_in, visited);
      work_in.clear();
    }

  return true;
}

//-------------------------------------------------------
// PS CHMC classification
//  Determines if an access can be classified as FM
//  Removes the PS analysis attributes
//-------------------------------------------------------

//possible improvement:
//  PS_UNION_MUST -> [C ; if() then A else B ; C]: allows to keep c in the cache when the loop does not fit in the cache

bool static ClassifCHMCPS(Cfg * c, Node * n, void *param)
{
  SerialisableStringAttribute FMatt("FM");	// First-miss
  SerialisableStringAttribute AUnref("AU");	// Always-unreferenced to consider WCET cost = 0
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  string CHMCAttName = CHMCAttributeNameData(ca->getLevelAnalysis());
  string in = ACSPSInName;
  string out = ACSPSOutName;

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string CACattName = AnalysisHelper::mkContextAttrName( CACAttributeNameData(ca->getLevelAnalysis()), currentContext);

      if (n->HasAttribute(in + currentContext))
	{
	  AbstractCache < PS > ca_ps = getACSNode(PS, n, in + currentContext).cache;

	  vector < Instruction * >vi = n->GetAsm();
	  for (size_t i = 0; i < vi.size(); i++)
	    {
	      if (Arch::isLoad(vi[i]->GetCode()))
		{
		  string id=AnalysisHelper::mkContextAttrName( CHMCAttName, currentContext);
		  if (!vi[i]->HasAttribute(id))	//if the chmc attribute was not set by the MUST analysis
		    {
		      assert(vi[i]->HasAttribute(CACattName));
		      string accessValue = ((SerialisableStringAttribute &) (vi[i]->GetAttribute(CACattName))).GetValue();

		      if (accessValue == "N")	//if not accessed: AU
			{
			  vi[i]->SetAttribute(id, AUnref);
			}
		      else
			{
			  set < t_address > add = ca->getDataAddress(vi[i], (*context));
			  if (ca_ps.AllPresent(add))	//if all present: FM
			    {
			      vi[i]->SetAttribute(id , FMatt);
			    }

			  ca_ps.Update(add, accessValue);	//simulate the access for the next instruction
			}
		    }
		}
	    }

	  //remove the ACS_in and ACS_out attributes
	  n->RemoveAttribute(in + currentContext);
	  n->RemoveAttribute(out + currentContext);
	}
    }
  return true;
}

/*************************************************************************************************************************
 NC, AH and CAC computations
*************************************************************************************************************************/

void static ClassifCHMC(Cfg * c, Node * n, string vclassif, string CHMCAttName )
{
   SerialisableStringAttribute NCatt(vclassif);

   assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string curAttr = AnalysisHelper::mkContextAttrName(CHMCAttName, currentContext);
      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      if (!vi[i]->HasAttribute(curAttr))	// if not set by the MUST, PS or MAY analysis
		{
		  vi[i]->SetAttribute(curAttr, NCatt);
		}
	    }
	}
    }
 }

//------------------------------------------------
// NC classification computation
//------------------------------------------------
bool static ClassifCHMCNC(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  string CHMCAttName = CHMCAttributeNameData(ca->getLevelAnalysis());
  ClassifCHMC ( c, n , "NC", CHMCAttName);
  return true;
}

//------------------------------------------------
// AH classification computation
//------------------------------------------------
bool static ClassifCHMCAH(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  string CHMCAttName = CHMCAttributeNameData(ca->getLevelAnalysis());
  ClassifCHMC ( c, n , "AH", CHMCAttName);
  return true;
}


//------------------------------------------------
// CAC computation for the next level
//------------------------------------------------
bool static ClassifCACNext(Cfg * c, Node * n, void *param)
{
  DCacheAnalysis *ca = (DCacheAnalysis *) param;
  int vlevel = ca->getLevelAnalysis();
  string CHMCAttName = CHMCAttributeNameData(vlevel);

  assert(c->HasAttribute(ContextListAttributeName));
  const ContextList & contexts = (ContextList &) c->GetAttribute(ContextListAttributeName);
  for (ContextList::const_iterator context = contexts.begin(); context != contexts.end(); context++)
    {
      string currentContext = (*context)->getStringId();
      string CACattName = AnalysisHelper::mkContextAttrName( CACAttributeNameData(vlevel), currentContext);
      string CACattNextName = AnalysisHelper::mkContextAttrName( CACAttributeNameData(vlevel + 1), currentContext);
      string cur_CHMCAttName = AnalysisHelper::mkContextAttrName( CHMCAttName, currentContext);
      vector < Instruction * >vi = n->GetAsm();
      for (size_t i = 0; i < vi.size(); i++)
	{
	  if (Arch::isLoad(vi[i]->GetCode()))
	    {
	      assert(vi[i]->HasAttribute(cur_CHMCAttName));
	      string chmcValue = ((SerialisableStringAttribute &) (vi[i]->GetAttribute(cur_CHMCAttName))).GetValue();

	      assert(vi[i]->HasAttribute(CACattName));
	      string accessValue = ((SerialisableStringAttribute &) (vi[i]->GetAttribute(CACattName))).GetValue();

	      vi[i]->SetAttribute(CACattNextName, ca->cac_computation[chmcValue][accessValue]);
	    }
	}
    }
  return true;
}

/*************************************************************************************************************************
 Generic analysis functions
*************************************************************************************************************************/

//------------------------------------------------
// Perform Analysis method
//------------------------------------------------
bool DCacheAnalysis::PerformAnalysis()
{
  if (levelAnalysis == 1)
    {
      // L1 CAC initialization (CAC=A for each access)
      AnalysisHelper::applyToAllNodesRecursive(p, initL1AccessAttributeForData, NULL);
    }

  float time = 0.0;
  //------------------------
  // MUST analysis
  //------------------------
  if (perform_must_analysis)
    {
      Timer timer_must;
      timer_must.initTimer();
      MustAnalysis();
      AnalysisHelper::applyToAllNodesRecursive(p, ClassifCHMCMust, (void *)this);
      timer_must.addTimer(time);
      stringstream infostr;
      infostr << "DcacheAnalysis: MUST done: " << time;
      Logger::addInfo(infostr.str());
    }
  //------------------------
  // PS analysis
  //------------------------
  if (perform_persistence_analysis)
    {
      time = 0.0;
      Timer timer_ps;
      timer_ps.initTimer();
      PSAnalysis();
      AnalysisHelper::applyToAllNodesRecursive(p, ClassifCHMCPS, (void *)this);
      timer_ps.addTimer(time);
      stringstream infostr;
      infostr << "DcacheAnalysis: PS done: " << time;
      Logger::addInfo(infostr.str());
    }
  //------------------------
  // MAY analysis
  //------------------------
  if (perform_may_analysis)
    {
      time = 0.0;
      Timer timer_may;
      timer_may.initTimer();
      MayAnalysis();
      AnalysisHelper::applyToAllNodesRecursive(p, ClassifCHMCMay, (void *)this);
      timer_may.addTimer(time);
      stringstream infostr;
      infostr << "DcacheAnalysis: MAY done: " << time;
      Logger::addInfo(infostr.str());
    }

  if (perfectDcache)
    AnalysisHelper::applyToAllNodesRecursive(p, ClassifCHMCAH, (void *)this); // AH classification
  else
    AnalysisHelper::applyToAllNodesRecursive(p, ClassifCHMCNC, (void *)this); // NC classification
  
  //------------------------
  // Block Count Attribute
  //------------------------
  //normalement les valeurs ne devraient pas être utilisées dans l'IPET dans le cas du perfect dcache (mais c'est du code de Benjamin à l'origine donc j'ai un doute) 
  AnalysisHelper::applyToAllNodesRecursive(p, ComputeBlockCountAttribute, (void *)this); 
  

  //------------------------
  // Stub Function for the perfect cache assumption in the IPET analysis  
  // Ici les commentaires ne veulent rien dire (même le nom de la fonction). Ce que ça fait c'est mettre la CHMC à AU pour toutes les instructions autres que load 
  AnalysisHelper::applyToAllNodesRecursive(p, StubFunction, (void *)this);

  //------------------------
  // CAC next level
  //------------------------
  AnalysisHelper::applyToAllNodesRecursive(p, ClassifCACNext, (void *)this);

  return true;
}

//------------------------------------------------
// Check attribute method
//------------------------------------------------
bool DCacheAnalysis::CheckInputAttributes()
{

  if (AnalysisHelper::applyToAllNodesRecursive(p, CheckInstrHaveDataAddresses, this) == false)
    {
      string error_msg = "DCacheAnalysis: load and Store instructions should have the " + string(AddressAttributeName) + " attribute set, sorry ... ";
      Logger::addFatal(error_msg);
      return false;
    }

  if (levelAnalysis != 1)
    {
      if (AnalysisHelper::applyToAllNodesRecursive(p, CheckInstrHaveDataAccessAttribute, &levelAnalysis) == false)
	{
	  stringstream errorstr;
	  errorstr << "DCacheAnalysis: instructions should have the " << CACAttributeNameData(levelAnalysis) << " attribute set, sorry ... ";
	  Logger::addFatal(errorstr.str());
	  return false;
	}
    }
  return true;
}

//------------------------------------------------
// Constructor:
// Set up cache parameters for the analysis
// and cac_computation map initialization
//------------------------------------------------
 DCacheAnalysis::DCacheAnalysis(Program * p, int nbsets, int nbways, int cachelinesize, t_replacement_policy r, int levelCache, 
				bool apply_must, bool apply_persistence, bool apply_may, bool pdcache):Analysis (p)
{
  perfectDcache = pdcache;
  nb_sets = nbsets;
  nb_ways = nbways;
  cacheline_size = cachelinesize;
  replacement_policy = r;
  levelAnalysis = levelCache;

  if (perfectDcache)
    {
      perform_must_analysis = false;
      perform_persistence_analysis = false;
      perform_may_analysis = false;
    }
  else
    {
      if (r != LRU)
	{
	  Logger::addFatal("DCacheAnalysis: only LRU replacement policy is implemented for the computation of DCACHE CHMC sorry...");
	}

      perform_must_analysis = apply_must;
      perform_persistence_analysis = apply_persistence;
      perform_may_analysis = apply_may;
    }

  this->call_graph = new CallGraph(p);

  //-------------------------------------
  // cac_computation initialization
  //-------------------------------------

  SerialisableStringAttribute A("A");	// access is Always made to the cache level
  SerialisableStringAttribute N("N");	// access is Never made to the cache level
  SerialisableStringAttribute U("U");	// access is Unknown to the cache level
  SerialisableStringAttribute UN("UN");	// access is Unknown to the cache level

  cac_computation["AM"]["A"] = A;
  cac_computation["AM"]["U"] = U;
  cac_computation["AM"]["UN"] = UN;
  //cac_computation["AM"]["N"]=N;

  cac_computation["AH"]["A"] = N;
  cac_computation["AH"]["U"] = N;
  cac_computation["AH"]["UN"] = N;
  //cac_computation["AH"]["N"]=N;

  cac_computation["FM"]["A"] = UN;
  cac_computation["FM"]["U"] = UN;
  cac_computation["FM"]["UN"] = UN;
  //cac_computation["FM"]["N"]=N;

  cac_computation["NC"]["A"] = U;
  cac_computation["NC"]["U"] = U;
  cac_computation["NC"]["UN"] = UN;
  //cac_computation["NC"]["N"]=U;

  cac_computation["AU"]["N"] = N;
}

//------------------------------------------------
// Remove all temporary attributes for a node
//------------------------------------------------
static bool CleanupNodeInternalAttributes(Cfg * c, Node * n, void *param)
{
  return true;
}

//------------------------------------------------
// Remove all temporary attributes
//------------------------------------------------
void DCacheAnalysis::RemovePrivateAttributes()
{
  AnalysisHelper::applyToAllNodesRecursive(p, CleanupNodeInternalAttributes, NULL);
}


