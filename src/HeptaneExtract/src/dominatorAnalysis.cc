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

#include "dominatorAnalysis.h"

/* #includes and forward declarations */
#include <algorithm>
#include <iostream>
#include <assert.h>
#include "dominatorData.h"

/* this namespace is the global namespace */
namespace cfglib
{
  namespace helper
  {

#define MAX_PASSES 20

#if 0
    void DominatorComputer::compute (Program * prog)
    {
      if (prog)
	{
	  for (std::vector < Cfg * >::iterator it = prog->begin (); it != prog->end (); it++)
	    computeDominator (*it);
	}
      else
	std::cout << "Error : no Cfg available (dominator analysis)" << std::endl;
    }
#endif


    bool DominatorComputer::equalDominator (Node * bb, std::set < Node * >&TempSet) /* private */
    {

      if (!bb)
	{
	  std::cout << "Error : in equalDominator function (dominator analysis)" << std::endl;
	  return false;
	}

      assert (bb->HasAttribute (DominatorAttributeName));
      DominatorData dd = (DominatorData &) (bb->GetAttribute (DominatorAttributeName));

      if (dd.isEmpty () && TempSet.empty ()) return true;
      if (dd.numberOfBB () != TempSet.size ()) return false;
      for (std::vector < Node * >::iterator it = dd.begin (); it < dd.end (); it++)
	{
	  bool find = false;
	  for (std::set < Node * >::iterator it2 = TempSet.begin (); it2 != TempSet.end (); it2++)
	    if ((*it) == (*it2)) find = true;
	  
	  if (!find) return false;
	}
      return true;
    }

    void DominatorComputer::computeDominator (Cfg * cfg) /* public */
    {
      if (cfg)
	{

	  // std::cout << "DominatorComputer::computeDominator  cfg= " << cfg->getStringName() << endl; 
	  Node *bb = NULL;

	  std::vector < Node * >all_nodes (cfg->GetAllNodes ());
	  std::set < Node * >all_nodes_set;
	  for (std::vector < Node * >::iterator it = all_nodes.begin (); it < all_nodes.end (); it++)
	    all_nodes_set.insert (*it);
	  // init = cfg->getBB(0);

	  // Create data BBs of the cfg
	  /* initialisation of dominator data to all nodes */
	  for (std::vector < Node * >::iterator it = all_nodes.begin (); it < all_nodes.end (); it++)
	    {
	      DominatorData dd;
	      dd.setNewListBB (all_nodes_set);
	      (*it)->SetAttribute (DominatorAttributeName, dd);
	    }

	  /* dominator node of the start node is only the start node itself */
	  {
	    assert (cfg->GetStartNode ()->HasAttribute (DominatorAttributeName));
	    DominatorData dd = (DominatorData &) (cfg->GetStartNode ()->GetAttribute (DominatorAttributeName));
	    std::set < Node * >empty;
	    dd.setNewListBB (empty);
	    dd.addBB (cfg->GetStartNode ());
	    cfg->GetStartNode ()->SetAttribute (DominatorAttributeName, dd);
	  }

	  // Computing...
	  // computation is finished when fixed point is reached or after at most MAX_PASSES
	  bool change = true;
	  int steps = MAX_PASSES;
	  while (change && steps)
	    {
	      change = false;

	      for (std::vector < Node * >::iterator it (all_nodes.begin ()); it < all_nodes.end (); it++)
		{
		  if ((*it) != cfg->GetStartNode ())
		    {
		      bb = (*it);	// each node but the startNode

		      // TempSet is initialised to the full set of Nodes in Cfg
		      std::set < Node * >TempSet;
		      for (std::vector < Node * >::iterator it2 (all_nodes.begin ()); it2 < all_nodes.end (); it2++)
			{
			  Node *bb2 = *it2;
			  if (bb2) TempSet.insert (bb2);
			}

		      std::vector < Node * >predecessors (cfg->GetPredecessors (bb));
		      for (std::vector < Node * >::iterator it2 (predecessors.begin ()); it2 < predecessors.end (); it2++)
			{
			  Node *pred = (*it2);

			  std::set < Node * >ListDom;
			  dbg_dom (std::cout << "Dominateur pred : " << std::endl; );
			  assert (pred->HasAttribute (DominatorAttributeName));
			  DominatorData dompred = (DominatorData &) (pred->GetAttribute (DominatorAttributeName));
			  /* ListDom = dd = dominator(pred) */
			  for (std::vector < Node * >::iterator it4 = dompred.begin (); it4 < dompred.end (); it4++)
			    ListDom.insert (*it4);
			  
			  /* ListInter = intersection(TempSet, ListDom) */
			  std::list < Node * >ListInter;
			  std::set_intersection (TempSet.begin (),
						 TempSet.end (),
						 ListDom.begin (), ListDom.end (), std::front_insert_iterator < std::list < Node * > >(ListInter));

			  /* tempset = ListInter */
			  TempSet.clear ();
			  for (std::list < Node * >::iterator it6 = ListInter.begin (); it6 != ListInter.end (); it6++)
			    if (*it6) TempSet.insert (*it6);
			}

		      TempSet.insert (bb);

		      if (!equalDominator (bb, TempSet))
			{
			  dbg_dom (std::cout << "Change : true " << std::endl;);
			  change = true;
			  assert (bb->HasAttribute (DominatorAttributeName));
			  DominatorData dd = (DominatorData &) (bb->GetAttribute (DominatorAttributeName));
			  dd.setNewListBB (TempSet);
			  bb->SetAttribute (DominatorAttributeName, dd);
			}
		    }
		}
	      steps--;
	    }
	  //        std::cerr << steps ;
	  //        if (0 == steps) 
	  //        {       std::cerr<< " MAXSTEP reached during dominator analysis" << std::endl ; } 
	  //        else { std::cerr << " steps passe during dominatorAnalysis"<<std::endl ; }
	  //        if (change) 
	  //        {       std::cerr << "fixed point not reached during dominatorAnalysis " << std::endl ; }
	}
      else
	std::cout << "Error : No CFG available (dominator analysis)" << std::endl;
    }

  }
}  // cfglib::
