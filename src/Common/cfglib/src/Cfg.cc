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

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "Helper.h"
#include "Handle.h"
#include "Cfg.h"
#include "Node.h"
#include "Program.h"

/*! this namespace is the global namespace */
namespace cfglib {
  /*! Basic constructor implementation.
     Cfg::Cfg(Program* program, std::string name) :
     program(program), name(ListOfString(name)), external(true), startNode(0) {
     }
   */

  Cfg::Cfg(Program * program, ListOfString name):program(program), name(name), external(true), startNode(0) { }

  /*! constructor for a free Cfg (used in clone function) */
  Cfg::Cfg(ListOfString name) : name(name) {}

  /*! Cloning function */
  Cfg *Cfg::Clone(CloneHandle & handle) {
    Cfg *C = new Cfg(name);
    handle.RegisterClone(this, C);

    C->program = (Program *) handle.GetClone(this->program);
    C->external = external;

    //clone every nodes
    for (std::vector < Node * >::iterator it = nodes.begin(); it != nodes.end(); it++)
      {
	Node *N = (*it)->Clone(handle);
	C->nodes.push_back(N);
      }

    //clone every edges
    for (std::vector < Edge * >::iterator it = edges.begin(); it != edges.end(); it++)
      {
	Edge *E = (*it)->Clone(handle);
	C->edges.push_back(E);
      }

    //loops
    for (std::vector < Loop * >::iterator it = loops.begin(); it != loops.end(); it++)
      {
	Loop *L = (*it)->Clone(handle);
	C->loops.push_back(L);
      }

    //handle start node and end nodes
    handle.ResolveClone(startNode, (void **)&(C->startNode));
    std::vector < Node * >curCFGendNodes = GetEndNodes();
    for (std::vector < Node * >::iterator it = curCFGendNodes.begin(); it != curCFGendNodes.end(); it++)
      {
	C->SetEndNode((Node *) handle.GetClone(*it));
      }

    //handle attributes
    this->CloneAttributesFor(C, handle);

    return C;
  }

  /*! Destructor implementation */
  Cfg::~Cfg()
  {
    /* recursively delete fields, we do not delete `startNode` and `endNodes` 
       because those Nodes are already in `nodes` */

    for (std::vector < Node * >::iterator it = this->nodes.begin(); it != this->nodes.end(); it++)
      {
	delete *it;
      }

    for (std::vector < Edge * >::iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	delete *it;
      }

    for (std::vector < Loop * >::iterator it(this->loops.begin()); it != this->loops.end(); ++it)
      {
	delete *it;
      }
  }

  /** retrieve the function name of this Cfg.
   */
  ListOfString Cfg::GetName() {
    return this->name;
  }

  // LBesnard.
  string Cfg::getStringName() {
    ListOfString::iterator it;
    if (name.empty())
      return " ";

    it = name.begin();
    std::string s = *it;
    it++;
    while (it != name.end())
      {
	s = s + ' ' + (*it);
	it++;
      }
    return s;
  }

  /*! tells if the CFG is external or not */
  bool Cfg::IsExternal() const {
    return this->external;
  }
  /*! retrieve a pointer to the including program */ Program *Cfg::GetProgram() {
    return this->program;
  }

  /*! Associate the program p to this CFG */
  void Cfg::SetProgram(Program * p) {
    program = p;
  }

  /*! Return every call node of the CFG */
  std::vector < Node * >Cfg::GetCallNodes(void) {
    std::vector < Node * >callNodes;

    for (std::vector < Node * >::iterator it = nodes.begin(); it != nodes.end(); it++)
      {
	if ((*it)->IsCall())
	  {
	    callNodes.push_back(*it);
	  }
      }
    return callNodes;
  }

  /*! Return true if the graph does not have any node */
  bool Cfg::IsEmpty() const {
    assert(this->edges.empty() || (!this->nodes.empty()));
    return this->nodes.empty();
  }
  /*! Creation of a new node. We add new nodes to
   * the CFG with this function which returns a
   * pointer to the new node. The first created
   * node is automatically set as the start node. */ Node *Cfg::CreateNewNode(enum node_type type) {
    external = false;
    Node *pBB_basic_block = new Node(this, type);
    if (this->nodes.empty())
      {
	this->startNode = pBB_basic_block;
      }
    this->nodes.push_back(pBB_basic_block);
    return pBB_basic_block;
  }

  /*! Adding an edge. Given two nodes, create an
   * edge from the first to the second. This
   * edge must not yet exist. */
  Edge *Cfg::CreateNewEdge(Node * origin, Node * destination) {
    Edge *pE_created = new Edge(origin, destination, this);
    this->edges.push_back(pE_created);
    return pE_created;
  }

  /*! Remove an edge and delete it from memory
   * removes the edge only, not the source/target node */
  void Cfg::RemoveEdge(Edge * e) {
    for (std::vector < Edge * >::iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it) == e)
	  {
	    delete e;
	    this->edges.erase(it);
	    return;
	  }
      }
  }

  /*! Remove an edge but do not delete it
   * removes the edge only, not the source/target node */
  void Cfg::RemoveEdgeNoDelete(Edge * e) {
    for (std::vector < Edge * >::iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it) == e)
	  {
	    this->edges.erase(it);
	    return;
	  }
      }
  }

  /*! Put a removed edge in the cfg
   * to use with edges removed by RemoveEdgeNoDelete */
  void Cfg::putEdge(Edge * E) {
    this->edges.push_back(E);
  }

  /*! internal function for adding a Edge not-initialised */
  Edge *Cfg::CreateNewEdge() {
    Edge *pE_created = new Edge();
    this->edges.push_back(pE_created);
    return pE_created;
  }

  /*! internal function for deserialisation of Loops */
  Loop *Cfg::CreateNewLoop() {
    Loop *new_loop = new Loop();
    this->loops.push_back(new_loop);
    return new_loop;
  }

  /*! Return a list of all nodes */
  std::vector < Node * >Cfg::GetAllNodes()
  {
    return this->nodes;
  }

  /*! Return a list of all edges */
  std::vector < Edge * >Cfg::GetAllEdges()
  {
    return this->edges;
  }

  /*! Return a list of all loops */
  std::vector < Loop * >Cfg::GetAllLoops()
  {
    return this->loops;
  }

  /*! Get the Edge between the two argument nodes. Return NULL if
   * there is no such Edge */
  Edge *Cfg::FindEdge(Node const *origin, Node const *destination)const {
      for (std::vector < Edge * >::const_iterator it = this->edges.begin(); it != this->edges.end(); it++) {
	if (((*it)->destination == destination) && ((*it)->origin == origin))
	  {
	    return (*it);
      }} return 0;
  }

  /*! Return the start Node of this CFG. There
   * is only one start node for each CFG. Return
   * 0 if there is no node on this CFG */
  Node *Cfg::GetStartNode() {
    return this->startNode;
  }

  /*! Change the start node of this CFG. */
  void Cfg::SetStartNode(Node * node) {
    this->startNode = node;
  }

  /*! Return a list of end nodes of this CFG. */
  std::vector < Node * >Cfg::GetEndNodes()
  {
    return this->endNodes;
  }

  /*! Set a node as an end node. */
  void Cfg::SetEndNode(Node * node) {
    this->endNodes.push_back(node);
  }


  void Cfg::removeEndNode(Node * node) {
    for (std::vector < Node * >::iterator it =  this->endNodes.begin(); it !=  this->endNodes.end(); it++)
      {
	if ((*it) == node) 
	  {
	     this->endNodes.erase(it);
	     return;
	  }
      }
  }

  /*! Return the incoming edges of a node. */
  std::vector < Edge * >Cfg::GetIncomingEdges(Node * node)
  {
    std::vector < Edge * >incomingEdges;
    for (std::vector < Edge * >::iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it)->destination == node)
	  {
	    incomingEdges.push_back(*it);
	  }
      }
    return incomingEdges;
  }

  /*! Return the predecessors of a node. */
  std::vector < Node * >Cfg::GetPredecessors(Node * node)
  {
    std::vector < Node * >pred;
    for (std::vector < Edge * >::const_iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it)->destination == node)
	  {
	    pred.push_back((*it)->origin);
	  }
      }
    return pred;
  }

  /*! Return the edges outgoing from a node. */
  std::vector < Edge * >Cfg::GetOutgoingEdges(Node * node)
  {
    std::vector < Edge * >outEdges;
    for (std::vector < Edge * >::iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it)->origin == node)
	  {
	    outEdges.push_back(*it);
	  }
      }
    return outEdges;
  }

  /*! Return the successors of a node. */
  std::vector < Node * >Cfg::GetSuccessors(Node * node)
  {
    std::vector < Node * >succ;
    for (std::vector < Edge * >::const_iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	if ((*it)->origin == node)
	  {
	    succ.push_back((*it)->destination);
	  }
      }
    return succ;
  }

  /*! Return the origin of an edge */
  Node *Cfg::GetSourceNode(Edge * edge) {
    return edge->GetSource();
  }

  /*! Return the destination of an edge */
  Node *Cfg::GetTargetNode(Edge * edge) {
    return edge->GetTarget();
  }

  /*! Loop creation. The loops managed here are
   * natural loops, with only one entry point.
   * */
  Loop *Cfg::CreateNewLoop(Node * headNode, std::vector < Node * >const &otherNodes) {
    Loop *new_loop = new Loop();
    new_loop->AddNode(headNode);
    for (std::vector < Node * >::const_iterator it(otherNodes.begin()); it != otherNodes.end(); ++it)
      {
	new_loop->AddNode(*it);
      }
    this->loops.push_back(new_loop);
    return new_loop;
  }

  /*! Serialisation code */
  std::ostream & Cfg::WriteXml(std::ostream & os, Handle & hand)
  {

    //  os << "<CFG id=\"" << hand.identify((Serialisable *)this) << "\" " << " name=\"" << this->name << "\" " << " startnode=\"" << hand.identify((Serialisable *)this->startNode) << "\" "; modified (LBesnard)
    os << "<CFG id=\"" << hand.identify((Serialisable *) this) << "\" " << " name=\"" << this->getStringName() << "\" " << " startnode=\"" << hand.identify((Serialisable *) this->startNode) << "\" ";
    os << " endnodes=\"";

    // List of end nodes
    for (std::vector < Node * >::const_iterator it = this->endNodes.begin(); it != this->endNodes.end(); ++it)
      {
	os << hand.identify((Serialisable *) * it);
	os << ", ";
      }
    os << "\" ";
    os << ">" << std::endl;

    /* Serialize each Node and Edge */
    for (std::vector < Node * >::const_iterator it = this->nodes.begin(); it != this->nodes.end(); it++)
      {
	(*it)->WriteXml(os, hand);
	os << std::endl;
      }
    for (std::vector < Edge * >::const_iterator it = this->edges.begin(); it != this->edges.end(); it++)
      {
	(*it)->WriteXml(os, hand);
	os << std::endl;
      }

    /* Serialise each Loop */
    for (std::vector < Loop * >::const_iterator it(this->loops.begin()); it != this->loops.end(); ++it)
      {
	(*it)->WriteXml(os, hand) << std::endl;
      }

    // Serialise cfg attributes
    this->WriteXmlAttributes(os, hand);

    os << "</CFG>" << std::endl;

    return os;
  }

  /*! Deserialisation function. */
  void Cfg::ReadXml(XmlTag const *tag, Handle & hand) {
    /* verifying if the xml_node really an XML Element */
    assert(tag->getName() == string("CFG"));
    string cfgname = tag->getAttributeString("name");
    assert(cfgname != "");
    string cfgid = tag->getAttributeString("id");
    assert(cfgid != "");
    hand.addID_serialisable(cfgid, this);
    string startid = tag->getAttributeString("startnode");
    assert(startid != "");

    // Read cfg endnodes
    string attr_endnodes = tag->getAttributeString("endnodes");
    ListXmlTag children = tag->getAllChildren();
    // check there are some end nodes, except for external CFGs
    if (children.size() != 0)
      {
	if (attr_endnodes == "")
	  {
	    cout << "Cfg does not have endnodes, cfg " << cfgname << endl;
	    cout << "Going on, ..." << endl;
	    //exit(-1);
	  }
	//assert(attr_endnodes != "");
      }
    std::vector < std::string > endnodes_vector(cfglib::helper::split_string(attr_endnodes, ", "));
    this->endNodes.resize(endnodes_vector.size());
    for (unsigned int i = 0; i < endnodes_vector.size(); i++)
      {
	this->endNodes[i] = NULL;
	hand.addID_handle(endnodes_vector[i], (Serialisable **) & (this->endNodes[i]));
      }

    /* We instanciate all Cfg objects : Nodes, Edges and Loops */
    for (unsigned int c = 0; c < children.size(); c++)
      {
	XmlTag child = children[c];
	if (child.getName() == string("NODE"))
	  {
	    string attr_type = child.getAttributeString("type");
	    assert(attr_type == "BasicBlock" || attr_type == "FunctionCall");
	    Node *bb;
	    if (attr_type == "BasicBlock")
	      {
		bb = this->CreateNewNode(BB);
		std::string attr_id = child.getAttributeString("id");
		if (attr_id == startid)
		  this->SetStartNode(bb);
	      }
	    else
	      bb = this->CreateNewNode(Call);
	    bb->ReadXml(&child, hand);
	  }
	else if (child.getName() == string("EDGE"))
	  {
	    Edge *edge = this->CreateNewEdge();
	    edge->ReadXml(&child, hand);
	  }
	else if (child.getName() == string("LOOP"))
	  {
	    Loop *loop = this->CreateNewLoop();
	    loop->ReadXml(&child, hand);
	  }
	else if (child.getName() == string("ATTRS_LIST"))
	  {
	    // Unserialise attributes
	    this->ReadXmlAttributes(tag, hand);
	  }
	else
	  {
	    std::cerr << "XML parsing WARNING : unrecognised child of <CFG>" << child.getName() << std::endl;
	  }
      }
  }

}				// cfglib::
