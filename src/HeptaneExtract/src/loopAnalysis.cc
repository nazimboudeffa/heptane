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

#include "loopAnalysis.h"

/* forward declarations and #includes */
#include "dominatorData.h"

/* this namespace is the global namespace */
namespace cfglib {
  
#define dbg_loop(x)


  /**
     Detect natural loops in the cfg and create corresponding Loop objects. 
  */
  void LoopComputer::computeLoop(Cfg * cfg)
  {
    // cout << "LoopComputer::computeLoop for " << cfg->getStringName() << endl;
    std::vector < Node * >nodes = cfg->GetAllNodes();
    for (std::vector < Node * >::iterator i = nodes.begin(); i != nodes.end(); i++)
      {
	Node *node = *i;
	bool isLoopHead = false;
	std::vector < Node * >predecessors = cfg->GetPredecessors(node);
	std::vector < Node * >backEdgeSources;

	for (std::vector < Node * >::iterator j = predecessors.begin(); j != predecessors.end(); j++)
	  {
	    Node *pred = *j;
	    if (pred->HasAttribute(DominatorAttributeName))
	      {
		DominatorData dominators = (DominatorData &) pred->GetAttribute(DominatorAttributeName);
		
		/* If the current node dominates one of its predecessor, we found a 
		   back-edge (from the predecessor to the current node). This also marks a new loop 
		   whose head is the current node.
		*/
		if (dominators.findBB(node) && (node != pred))
		  {
		    isLoopHead = true;
		    backEdgeSources.push_back(pred);
		  }
	      }
	  }

	/* If the current node is the head of a loop */
	if (isLoopHead)
	  {
	    /* This loop has to be created. */
	    Loop *loop = cfg->CreateNewLoop();
	    loop->AddNode(node);

	    /* ----- trace lbesnard
	       vector < Instruction * >code = node->GetInstructions();
	       fprintf(stderr, "\nNode :%p\n", node);
	       for (size_t i = 0; i < code.size(); ++i)
	       cerr << "  " << code[i]->GetCode() << endl;
	       // ----- trace lbesnard 
	       */
	    
	    for (std::vector < Node * >::iterator j = backEdgeSources.begin(); j != backEdgeSources.end(); j++)
	      {
		Node *pred = *j;

		/* We can add each backedge to the loop. */
		loop->AddBackedge(cfg->FindEdge(pred, node));

		/* We can add the corresponding nodes to the loop. */
		//loop->AddNode (pred);

		/* And recursively start adding the other ones. */
		computeLoop(loop, cfg, pred);
	      }

	    loop_check(loop, node);
	  }
      }
  }


  // useful ???
  void LoopComputer::loop_check(Loop *loop, Node *node)
  {
    return;
    // -------------------
    struct NodeOp {
      virtual void operator () (Node *) = 0;
    };

    struct:public NodeOp {
      void operator () (Node * node) {
	vector < Instruction * >code = node->GetInstructions();
	fprintf(stderr, "Node :%p\n", node);
	for (size_t i = 0; i < code.size(); ++i)
	  cerr << code[i]->GetCode() << endl;
      }
    }
    printNode;

    struct {
      void operator () (vector < Node * >container, NodeOp & op) {
	for (size_t i = 0; i < container.size(); ++i)
	  op(container[i]);
      }
    }
    foreach;

    fprintf(stderr, "LOOP CREATION\n");
    fprintf(stderr, "\tLoop Head: %p\n", node);
    printNode(node);

    vector < Node * >loopNodes = loop->GetAllNodes();
    fprintf(stderr, "\tLoop Nodes: \n");
    foreach(loopNodes, printNode);
  }

  /** OLD VERSION: detect natural loops in the cfg and create corresponding Loop objects. */
  void LoopComputer::computeLoop_old(Cfg * cfg)
  {
    if (cfg)
      {
	std::vector < Node * >cfg_nodes = cfg->GetAllNodes();
	for (std::vector < Node * >::iterator it = cfg_nodes.begin(); it < cfg_nodes.end(); it++)
	  {
	    Node *bb = *it;

	    std::vector < Node * >successors = cfg->GetSuccessors(bb);
	    for (std::vector < Node * >::iterator it2(successors.begin()); it2 < successors.end(); ++it2)
	      {
		Node *suc = *it2;
		if (suc)
		  {
		    if (bb->HasAttribute(DominatorAttributeName))
		      {
			DominatorData dd = (DominatorData &) (bb->GetAttribute(DominatorAttributeName));
			/* if the successor of the current bb dominate it (i.e. iff succ is in dominator(bb)) 
			   we just found a Backedge and so a loop */
			if (dd.findBB(suc))
			  {
			    Loop *loop = cfg->CreateNewLoop();
			    /* we can add the backedge */
			    loop->AddBackedge(cfg->FindEdge(bb, suc));
			    /* suc is the Loop head, we add it first */
			    loop->AddNode(suc);

			    /* then we add recursively all the others Node */
			    computeLoop(loop, cfg, bb);

			    /* We remove the loop if it was previously met. */
			    std::vector < Loop * >loops = cfg->GetAllLoops();
			    std::vector < Loop * >::iterator known_loop;

			    bool metLoop = false;
			    for (known_loop = loops.begin(); known_loop != loops.end(); known_loop++)
			      if (loop->IsNestedIn(*known_loop) && (*known_loop)->IsNestedIn(loop))
				{
				  if (!metLoop)
				    metLoop = true;
				  else
				    {
				      // cfg->RemoveLoop(loop); does not exist (lbesnard)
				      break;
				    }
				}
			  }
		      }
		  }
	      }
	  }
      }
    else
      std::cout << "Error : No cfg available (loop computing)" << std::endl;
  }

  /** recursively add the predecessors to the loop */
  void LoopComputer::computeLoop(Loop * loop, Cfg * cfg, Node * current)
  {
    if (!current || !cfg || !loop)
      {
	std::cerr << "error computeLoop with invalid parameters" << std::endl;
	return;
      }

    /* this Node is yet in the loop (and so its predecessors) */
    if (loop->FindInLoop(current))
      {
	dbg_loop(std::cerr << "Yet in loop, we end here" << std::endl;);
	return;
      }

    loop->AddNode(current);
    std::vector < Node * >predecessors = cfg->GetPredecessors(current);
    for (std::vector < Node * >::iterator it(predecessors.begin()); it < predecessors.end(); it++)
      {
	Node *pred = *it;
	if (pred)
	  {
	    dbg_loop(std::cerr << "new predecessor" << std::endl; );
	    computeLoop(loop, cfg, pred);
	  }
      }
  }
}
