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

#ifndef _IRISA_CFGLIB_LOOP_H
#define _IRISA_CFGLIB_LOOP_H

/* #includes and forward declarations */
#include <vector>
#include <iostream>
#include "Attributed.h"
#include "CloneHandle.h"
namespace cfglib {	class Node ; }
namespace cfglib {	class Edge ; }
namespace cfglib {	class Handle ; }

/* Debug of loop management methods */
// Uncomment one of these two lines to enter/leave debug mode
// #define dbg_loop(x) x
#define dbg_loop(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
  /*! Natural loops. */
  class Loop : public Attributed
  {
  private:
    std::vector<Node*> nodes ;
    std::vector<Edge*> backedges ;
    Node* head ;

  public:
    /*! Constructor */
    Loop() ;

    /*! destructor */
    ~Loop() ;

    //TP
    /*! cloning function */
    Loop* Clone(CloneHandle&);
			
    /*! Serialisation function. */
    std::ostream& WriteXml( std::ostream& os, Handle& hand);

    /*! Deserialisation function. */
    virtual void ReadXml( XmlTag const *tag, Handle& hand) ;

    /*! Add nodes to the loop, the first Node added must be
     * the head of the loop */
    void AddNode(Node*) ;

    /*! Add backedge to the loop. The Nodes of the backedge
     * should be added independently. FIXME: in the reductible
     * loop case I could infer backedges from the head
     * information and this function will be therefore
     * redundant. */
    void AddBackedge(Edge*) ;

    /*! Get the entry node of the loop */
    Node* GetHead() const;

    /*! Say if the node is in this loop. */
    bool FindInLoop(Node* node) const;

    /*! Get the nodes of the Loop. The vector's first Node is
     * the head of the loop, other nodes are in no particular
     * order. Returns all nodes of the loop, even those belonging
     * to nested loops. To have only the nodes not belonging to
     * nested loops, call method GetAllNodesNotNested */
    std::vector<Node*> GetAllNodes() const ;

    /*! Get the nodes of the Loop. The vector's first Node is
     * the head of the loop, other nodes are in no particular
     * order. Returns only the nodes not belonging to
     * nested loops. To have all nodes, call method GetAllNodes */
    std::vector<Node*> GetAllNodesNotNested() const ;

    /*! Get the Loop Backedges. */
    std::vector<Edge*> GetBackedges() const ;

    /*! Returns true if the loop (this) is nested 
      in loop (l) */
    bool IsNestedIn(Loop *l) const;

    /* Peel one iteration off this loop. This method
       only updates the CFG to reflect the new structure.
       Instructions are NOT modified. */
    void Peel();
  };

} // cfglib::
#endif // _IRISA_CFGLIB_LOOP_H
