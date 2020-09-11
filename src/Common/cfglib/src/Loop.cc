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

/* header file */
#include "Loop.h"

/* #includes and forward declarations */
#include <string>
#include <vector>
#include <cassert>
#include "Helper.h"
#include "Handle.h"
#include "Node.h"
#include "Cfg.h"

/*! this namespace is the global namespace */
namespace cfglib 
{

  /*! basic constructor */
  Loop::Loop() : head(0)
  {}

  /*! basic destructor. Do nothing as all objects are managed by the
   * Cfg. */
  Loop::~Loop()
  {}

  //TP
  Loop* Loop::Clone(CloneHandle& handle){
    Loop* L = new Loop();
    handle.RegisterClone (this, L);
	  
    //handle attributes
    this->CloneAttributesFor (L, handle);
	  
    L->head = (Node*) handle.GetClone(this->head);
    for(std::vector<Node*>::iterator it = nodes.begin();
	it != nodes.end();
	it++){
      L->nodes.push_back ((Node*) handle.GetClone(*it));
    }
	  
    for(std::vector<Edge*>::iterator it = backedges.begin();
	it != backedges.end();
	it++){
      L->backedges.push_back ((Edge*) handle.GetClone(*it));
    }
	  
    return L;
  }
  
  /*! Serialisation function. */
  std::ostream& Loop::WriteXml( std::ostream& os, Handle& hand)
  {
    os  << "<LOOP "
	<< " id=\"" << hand.identify((Serialisable *)this) << "\" ";
    os  << " head=\"" << hand.identify((Serialisable *)(this->head)) << "\" ";
    os  << " nodes=\"" ;
    // List of nodes
    for (	std::vector<Node*>::const_iterator it(this->nodes.begin());
		it != this->nodes.end();
		++it) {	
      os << hand.identify((Serialisable *)*it) ;
      os << ", " ;
    }
    os << "\" ";
    // List of backedges
    os  << " backedges=\"" ;
    for (	std::vector<Edge*>::const_iterator it(this->backedges.begin());
		it != this->backedges.end();
		++it) {	
      os << hand.identify((Serialisable *)*it) ;
      os << ", " ;
    }
    os << "\" ";
    os  << ">" << std::endl ;
    
    this->WriteXmlAttributes(os, hand) ; 

    os  << "</LOOP>" << std::endl ;
    return os ;
  }

  /*! Deserialisation function.
   *
   * The deserialisation is in two phase : first the creation of the
   * object (in several ways depending on the object) and second
   * call to O->ReadXml() which initialize the object with correct
   * values. */
  void Loop::ReadXml(XmlTag const* tag, Handle& hand)
  {
    assert(tag->getName() == string("LOOP"));
    string id = tag->getAttributeString("id");
    assert(id!="");
    hand.addID_serialisable(id,this);

    // Read loop head
    string head = tag->getAttributeString("head");
    assert(head != "");
    hand.addID_handle(head,(Serialisable **)&(this->head));
	
    // Read loop nodes
    string attr_nodes = tag->getAttributeString("nodes");
    assert(attr_nodes != "");
    std::vector<std::string> nodes_vector(cfglib::helper::split_string(attr_nodes, ", ")) ;
    this->nodes.resize(nodes_vector.size());
    for (unsigned int i=0;i<nodes_vector.size();i++) {
      (this->nodes)[i]=NULL;
      hand.addID_handle(nodes_vector[i],(Serialisable **)&(this->nodes[i]));
    }

    // Read loop backedges
    string attr_backedges = tag->getAttributeString("backedges");
    assert(attr_backedges != "");
    std::vector<std::string> backedges_vector(cfglib::helper::split_string(attr_backedges, ", ")) ;
    this->backedges.resize(backedges_vector.size());
    for (unsigned int i=0;i<backedges_vector.size();i++) {
      (this->backedges)[i]=NULL;
      hand.addID_handle(backedges_vector[i],(Serialisable **)&(this->backedges[i]));
    }  

    // the only Loop child element should be `ATTRS_LIST`
    ListXmlTag children = tag->getAllChildren();
    if (children.size()!=0) {
      assert(children.size()==1);
      assert(children[0].getName()==string("ATTRS_LIST"));
      this->ReadXmlAttributes(tag,hand);
    }
  }
  
  /*! Add nodes to the loop, the first Node added must be the head of the loop */
  void Loop::AddNode(Node* new_node) 
  {
    if (this->head)
      {	this->nodes.push_back(new_node) ; }
    else
      {	this->head = new_node ; 
	this->nodes.push_back(new_node);
      }
  }

  /*! Add backedge to the loop. The Nodes of the backedge
   * should be added independently. FIXME: in the reductible
   * loop case I could infer backedges from the head
   * information and this function will be therefore
   * redundant. */
  void Loop::AddBackedge(Edge* backedge)
  {
    this->backedges.push_back(backedge) ; 
  }

  /*! Get the entry node of the loop */
  Node* Loop::GetHead() const
  {
    return this->head ;
  }
  
  /*! Get the nodes of the Loop. The vector's first Node is
   * the head of the loop, other nodes are in no particular
   * order. Returns all nodes of the loop, even those belonging
   * to nested loops. To have only the nodes not belonging to
   * nested loops, call method GetAllNodesNotNested */
  std::vector<Node*> Loop::GetAllNodes() const 
  {
    return this->nodes;
  }

  /*! Get the nodes of the Loop. The vector's first Node is
   * the head of the loop, other nodes are in no particular
   * order. Returns only the nodes not belonging to
   * nested loops. To have all nodes, call method GetAllNodes 
   * FIXME: speed-up this method!
   */
  std::vector<Node*> Loop::GetAllNodesNotNested() const {
    vector<Node*> res;
    assert(this->head!=NULL);
    Cfg *c=this->head->GetCfg();
    vector<Loop*> vl = c->GetAllLoops();
    for (unsigned int n=0;n<this->nodes.size();n++) {
      Node *my_node = this->nodes[n];
      bool is_in_nested_loop = false;
      for (unsigned int l=0;l<vl.size();l++) {
	if (vl[l]!=this) {
	  if (vl[l]->FindInLoop(my_node) && 
	      vl[l]->IsNestedIn((Loop*)this)) {
	    is_in_nested_loop = true;
	    break;
	  }
	}
      }
      if (! is_in_nested_loop) res.push_back(my_node);
    }
    return res;
  }

  /*! say if the node is in this loop
   * FIXME: speed-up this method!
   */
  bool Loop::FindInLoop(Node* node) const
  {	
    if (node == this->head) 
      {	
	dbg_loop(std::cerr << "node is head" << std::endl ;);
	return true ; 
      }
    
    for ( std::vector<Node*>::const_iterator it = this->nodes.begin(); it != this->nodes.end(); it++) 
      {
	if ((*it) == node) 
	  {	
	    dbg_loop(std::cerr << "node is in loop" << std::endl;);
	    return true; 
	  }
      }
    return false;
  }

  /*! Get the loop Backedges */
  std::vector<Edge*> Loop::GetBackedges() const
  {
    return this->backedges ;
  }

  /*! Returns true if the loop (this) is nested in loop (l)
   * FIXME: speed-up this method!
   */
  bool Loop::IsNestedIn(Loop *l) const {
    if (l==NULL) return true; // NULL represents the cfg itself
    vector<Node*> vnl = l->GetAllNodes();
    for (unsigned int i=0;i<this->nodes.size();i++)
      {
	bool found=false;
	for (unsigned int il=0;il<vnl.size();il++) {
	  if ((this->nodes)[i]==vnl[il]) {
	    found = true; break;
	  }
	}
	if (!found) return false;
      }
    return true;
  }


  void Loop::Peel(void)
  {
    // FIXME: what about attributes?
    // FIXME: what about instructions? Duplicate? Ignore?

    // If there is no head, this loop is empty, peeling is pointless.
    if (!head)
      return;

    // FIXME: maybe check the max_iter attribute here?

    Cfg* cfg = head->GetCfg();

    // Duplicate nodes. Keep a mapping of old-to-new.
    map<Node*, Node*> temp;
    for(vector<Node*>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
      Node* node = *it;
      if (temp.find(node) != temp.end()) {
	// cout << "Loop.Peel: skipping duplicate node " << " in function " << cfg->GetName() << endl; replaced by (LBesnard)
	cout << "Loop.Peel: skipping duplicate node " << " in function " << cfg->getStringName() << endl;
        if (node != head) cout << "(not the header)" << endl;
        continue;
      }
      // cout << "Copying Node " << node << " " << node->GetId() << endl;
      Node* copy = cfg->CreateNewNode(node->type);
      if (node->IsCall()) { copy->SetCall(node->GetCalleeName(), false); }
      temp[node] = copy;
      
      // Can a node be a Cfg start node? If so, it is no longer in the
      // peeled loop.
      if (node == cfg->GetStartNode()) { cfg->SetStartNode(copy); }

      // Copy instructions, copy the pointers.
      int num_insn = node->instructions.size();
      copy->instructions.resize(num_insn);
      for(int i=0; i < num_insn; ++i) {
	Instruction* insn = node->instructions[i];
	copy->instructions[i] = insn->Clone();
      }
    }

    // Duplicate edges. Make a copy of all Cfg edges to avoid messing with the iterator.
    vector<Edge*> all_edges = cfg->GetAllEdges(); // !!! COPY !!!
    
    // FIXME: can avoid this copy by inserting newly created edges in a
    // temporary list, and copy that list to the Cfg in a second pass.
    // But we need more API on class Cfg: either public iterators or a
    // reference to the list of edges.
    for(vector<Edge*>::iterator it_edges = all_edges.begin();
	it_edges != all_edges.end();
	it_edges++) {
      Edge* e = *it_edges;
      // Check if origin and destination are in our loop. We do this by
      // checking the map. We also keep the iterators.
      map<Node*, Node*>::iterator origin_it = temp.find(e->origin);
      map<Node*, Node*>::iterator destination_it = temp.find(e->destination);

      // Incoming edges to the loop (from outside to head): redirect to the
      // peeled iteration.
      if (e->destination == head && origin_it == temp.end()) {
        //	cout << "Incoming edge " << e->origin << " -> " << e->destination << endl;
	e->destination = temp[head];
	continue;
      }

      // Exit edges (from inside to outside): create new edge that links the
      // copy to the same destination.
      if (origin_it != temp.end() && destination_it == temp.end()) {
        //	cout << "exit edge " << e->origin << " -> " << e->destination << endl;
	cfg->CreateNewEdge(origin_it->second, e->destination);
      }

      // Internal edge (both origin and destination are part of the loop):
      // handle the backedge as a special case.
      if (origin_it != temp.end() && destination_it != temp.end()) {
	if (e->destination == head) {  // this is a backedge
          //	  cout << "backedge " << e->origin << " -> " << e->destination << endl;
	  cfg->CreateNewEdge(origin_it->second, e->destination);
	}
	else {
          //	  cout << "internal edge " << e->origin << " -> " << e->destination << endl;
	  cfg->CreateNewEdge(origin_it->second, destination_it->second);
	}
      }
    }

    // Add edge at end of peeled iteration to skip loop body (in case the loop
    // iterates only once).
    // Maybe not, this is already handled by exit edges!  (TODO)

    // Rebuild loop structure of nested loops.
    vector<Loop*> loops = cfg->GetAllLoops();  // !!! COPY !!!
    for(vector<Loop*>::iterator loop_it = loops.begin();
	loop_it != loops.end();
	++loop_it)
      {
	// Find nested loops
	Loop* l = *loop_it;
	if (l == this)
	  continue;
	// A loop is nested in myself if I contain its head (and it's not me)
	if (!FindInLoop(l->head))
	  continue;

	//cout << "found nested loop." << endl;

	// l is a nested loop, duplicate the structure
	Loop* new_loop = cfg->CreateNewLoop();
	// Need to add the head first.
	new_loop->AddNode(temp[l->head]);
	vector<Node*> nested_nodes = l->GetAllNodes();  // !!! COPY !!!
	for(vector<Node*>::iterator nested_it = nested_nodes.begin();
	    nested_it != nested_nodes.end();
	    ++nested_it)
	  {
	    Node* nested_node = *nested_it;
	    // head is already added
	    if (nested_node == l->head)
	      continue;
	    assert(temp[nested_node]);
	    new_loop->AddNode(temp[nested_node]);
	  }

	// duplicate l loop attributes
	const vector<string>& old_attr = l->getAttributeList();
	for(vector<string>::const_iterator old_attr_it = old_attr.begin();
	    old_attr_it != old_attr.end();
	    ++old_attr_it)
	  {
	    //FIXME: what about attributes whose semantic is altered by copy ?
	    const string& attr_name = *old_attr_it;
	    Attribute& attr = l->GetAttribute(attr_name);
	    new_loop->SetAttribute(attr_name, attr);
	
	  }

	// duplicate l back-edges
	// FIXME GetBackedges -> GetBackEdges
	// FIXME AddBackedge -> AddBackEdge
	vector<Edge*> back_edges = l->GetBackedges();
	for(vector<Edge*>::const_iterator back_edge_it = back_edges.begin();
	    back_edge_it != back_edges.end();
	    ++back_edge_it)
	  {
	    Edge* e = *back_edge_it;
	    Edge* new_back_edge = cfg->FindEdge(temp[e->origin], temp[e->destination]);
	    new_loop->AddBackedge (new_back_edge);
	  }

      }


    // Rebuild loop structure of nesting loops.
    loops = cfg->GetAllLoops();  // !!! COPY !!!
    for (map<Node*, Node*>::iterator copy_it = temp.begin();
	 copy_it != temp.end();
	 ++copy_it)
      {
	Node* new_node = copy_it->second;

	for(vector<Loop*>::iterator loop_it = loops.begin();
	    loop_it != loops.end();
	    ++loop_it)
	  {
	    Loop* l = *loop_it;

	    // New nodes do not belong to the current loop.
	    if (l == this)
	      continue;

	    // Neither old nor new nodes belong to not nesting loops.
	    if (!this->IsNestedIn (l))
	      continue;

	    assert (l->FindInLoop(copy_it->first));
	    assert (!l->FindInLoop(new_node));
	    l->AddNode (new_node);
	  }
      }

    // Subtract one to the max number of iterations of this loop.
    // FIXME: should this be done here? Is maxiter really part of a Cfg/Loop?
    if (HasAttribute("maxiter")) {
      SerialisableIntegerAttribute ia =
	(SerialisableIntegerAttribute&)GetAttribute("maxiter");
      int max_iter = ia.GetValue();
      --max_iter;
      SerialisableIntegerAttribute ia2(max_iter);
      SetAttribute("maxiter", ia2);
    }

    // Let's try to duplicate attributes.
    // Duplicated attributed elements include:
    //  - Nodes
    //  - Instructions -> dealt with by instruction cloning function
    //  - Loops -> dealt with at loop duplication time
    for (map<Node*, Node*>::iterator copy_it = temp.begin();
	 copy_it != temp.end();
	 ++copy_it)
      {
	Node* old_node = copy_it->first;
	Node* new_node = copy_it->second;

	// Duplicate attributes attached to the old node.
	const vector<string>& old_attr = old_node->getAttributeList();
	for(vector<string>::const_iterator old_attr_it = old_attr.begin();
	    old_attr_it != old_attr.end();
	    ++old_attr_it)
	  {
	    const string& attr_name = *old_attr_it;
	    Attribute& attr = old_node->GetAttribute(attr_name);
	    new_node->SetAttribute(attr_name, attr);
	  }
      }
    
  }


} // cfglib::
