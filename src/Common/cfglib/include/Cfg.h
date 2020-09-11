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

#ifndef _IRISA_CFGLIB_CFG_H
#define _IRISA_CFGLIB_CFG_H

/* #includes and forward declarations : */
#include <vector>
#include <map>
#include <string>
#include "Attributed.h"
#include "Node.h"
#include "Edge.h"
#include "Loop.h"
#include "Program.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  /*! This class represents a Control Flow Graph. */
  class Cfg : public Attributed {
  private:
    /*! program this cfg belongs to (only one) */
    Program* program ;

    /*! Name of the function this CFG represents. */
    //    std::string name; (LBesnard) replaced by
    ListOfString name;

    /*! If true, the CFG has no nodes (only edges). It represents a function whose code is given elsewhere (e.g. library code) */
    bool external;
    
    /*! Nodes of this Cfg */
    std::vector<Node*> nodes ;

    /*! start Node (entry point) */
    Node* startNode ;

    /*! end Nodes of this Cfg */
    std::vector<Node*> endNodes ;

    /*! Edges of this Cfg */
    std::vector<Edge*> edges ;

    /*! Loops of this Cfg */
    std::vector<Loop*> loops ;

  private:
    /*! Create an uninitialised Edge */
    Edge* CreateNewEdge() ;

  public:
    /*! Basic constructor. */
    // Cfg(Program* program, std::string name); 
    Cfg(Program* program, ListOfString name);

    //TP
    /*! constructor for a free Cfg (used in clone function) */
    Cfg(ListOfString name);

    /*! destructor */
    ~Cfg();

    //TP
    /*! Cloning function 
     *  The program in which this CFG is associated have to be set manualy
     *  (use the Cfg::SetProgram(Program* P) function) */
    Cfg* Clone(CloneHandle&);

    /*! retrieve the function name of this Cfg */
    ListOfString GetName() ;

    /*! tells if the CFG is external or not */
    bool IsExternal() const;

    /*! retrieve a pointer to the including program */
    Program* GetProgram() ;

    //TP
    /*! Associate the program p to this CFG */
    void SetProgram(Program* p);

    /*! Return every call node of the CFG */
    std::vector<Node*> GetCallNodes(void);
    
    /*! Return true if the graph does not have any node */
    bool IsEmpty() const ;

    /*! Creation of a new node. We add
     * new nodes to the CFG with this function
     * which returns a pointer to the new node. The
     * first created node is automatically set as
     * the start node.  
     */
    Node* CreateNewNode(enum node_type type);

    /*! Adding an edge. Given two nodes, create an
     * edge from the first to the second. This
     * edge must not yet exist. */
    Edge* CreateNewEdge(Node* origin, 
			Node* destination);
    
    /*! Remove an edge and delete it from memory
     * removes the edge only, not the source/target node */
    void RemoveEdge(Edge *e);
    
    /*! Remove an edge but do not delete it
     * removes the edge only, not the source/target node */
    void RemoveEdgeNoDelete(Edge *e);
    
    /*! Put a removed edge in the cfg
     * to use with edges removed by RemoveEdgeNoDelete */
    void putEdge(Edge* E);
    
    /*! Return a list of all nodes */
    std::vector<Node*> GetAllNodes();

    /*! Return a list of all edges */
    std::vector<Edge*> GetAllEdges();

    /*! Return a list of all loops */
    std::vector<Loop*> GetAllLoops();

    /*! Get the Edge between the two argument nodes. Return
     * NULL if there is no such Edge */
    Edge* FindEdge(Node const* origin, Node const* destination) const ;

    /*! Return the start Node of this CFG. There
     * is only one start node for each CFG. Return
     * 0 if there is no node on this CFG */
    Node* GetStartNode();

    /*! Change the start node of this CFG. */
    void SetStartNode(Node* node);

    /*! Return a list of end nodes of this CFG. */
    std::vector<Node*> GetEndNodes();

    /*! Set an existing node as an end node. */
    void SetEndNode(Node* node);

    void removeEndNode(Node * node);

    /*! Return the incoming edges of a node. */
    std::vector<Edge*> GetIncomingEdges(Node* node);

    /*! Return the predecessors of a node. */
    std::vector<Node*> GetPredecessors(Node* node);

    /*! Return the edges outgoing from a node. */
    std::vector<Edge*> GetOutgoingEdges(Node* node);

    /*! Return the successors of a node. The successors vector
     * is empty if you pass an end Node. Function call are
     * simple Nodes, the successor of a function call Node
     * is the next basic block in the caller Cfg. */
    std::vector<Node*> GetSuccessors(Node* node);

    /*! Return the origin of an edge */
    Node* GetSourceNode(Edge* edge);
    
    /*! Return the destination of an edge */
    Node* GetTargetNode(Edge* edge);

    /*! Loop creation. The loops managed here are
     * natural loops, with only one entry point.
     * The first argument is the head Node (entry point in
     * the loop), then come the list of other Nodes in the
     * loop */
    Loop* CreateNewLoop(Node* startNode, std::vector<Node*> const& otherNodesList) ;

    /*! Create an uninitialised Loop. Use loop->AddNode()
     * beginning with the head Node to fill the loop. */
    Loop* CreateNewLoop() ;

    /*! Loop suppression. */
    void RemoveLoop(Loop* loop);

    /*! Serialisation temporary function FIXME: */
    std::ostream& WriteXml(std::ostream& oss, Handle& hand)  ;

    /*! Deserialisation function. */
    virtual void ReadXml(XmlTag const* tag, Handle& hand);
    
    /** Prints the associated names of the CFG */
    // void printNames(); void printNames(std::ostream& os) ;
    string getStringName();
  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_CFG_H


