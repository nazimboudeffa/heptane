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

#include <string>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <vector>
#include "Node.h"
#include "Handle.h"
#include "Instruction.h"
#include "Cfg.h"

/*! this namespace is the global namespace */
namespace cfglib 
{
  /* This class represents the most common node : basic block.  */

  /* forward declarations and #includes */
  namespace helper { class Handle; } // cf. Handle.h"
  
  /*! constructor of a Node, common to BB and Code nodes. */
  Node::Node(Cfg* cfg) : cfg(cfg), type(BB), is_return(false), callee_cfg(NULL)
  {
    dbg_node(std::cout << "Node constructor called" << std::endl ;); 
  }

  /*! constructor. */
  Node::Node(Cfg* cfg, node_type type) : cfg(cfg), type(type), is_return(false), callee_cfg(NULL)
  {
    dbg_node(std::cout << "Node constructor called" << std::endl ;); 
  }

  /* destructor. */
  Node::~Node() 
  { 
    for (std::vector<Instruction*>::iterator it = this->instructions.begin(); it != this->instructions.end(); it++)
      {
	delete *it ;
      }
  }

  //TP
  /*! constructor for a free Cfg (used in clone function) */
  Node::Node(){ }

  //TP
  /*! cloning function */
  Node* Node::Clone(CloneHandle& handle)
  {
    Node* n = new Node();
    handle.RegisterClone (this, n);

    //handle attributes
    this->CloneAttributesFor (n, handle);

    handle.ResolveClone(this->cfg, (void**)&(n->cfg));
    n->is_return = is_return;
    n->type = type;
    n->callee_name = callee_name;
    handle.ResolveClone(this->callee_cfg, (void**)&(n->callee_cfg));
     
    //clone instructions
    for(std::vector<Instruction*>::iterator it = instructions.begin();	it != instructions.end(); it++){
      Instruction* I = (*it)->Clone(handle);
      n->instructions.push_back(I);
    }
     
    return n;
  }

  /*! Virtual constructor. */
  Node* Node::create(Cfg* cfg, node_type type) const
  {
    return new Node(cfg, type) ; 
  }

  /*! Returns the Cfg this Node belong to */
  Cfg* Node::GetCfg()
  {	
    return this->cfg ; 
  }

  /*! Returns true if the node is a Call node */
  bool Node::IsCall() 
  {
    return (this->type==Call);
  }

  /*! Returns true if the node is a basic block */
  bool Node::IsBB() 
  {
    return (this->type==BB);
  }
	
  /* two local helper function for ReadXml and WriteXml */
  std::string string_of_type(node_type type) 
  {
    if (BB == type) { return std::string("BasicBlock") ; }
    if (Call == type) {	return std::string("FunctionCall") ; }
    assert(false) ; 
  }
  
  node_type type_of_string(std::string stype)
  {
    if (std::string("BasicBlock") == stype) { return BB ; }
    if (std::string("FunctionCall") == stype) {return Call ; }
    std::cerr << "the string \"" << stype << "\" is not a valid Node type, " << "should be \"BasicBlock\" or \"FunctionCall\"" << std::endl ;
    assert(false) ;
  }
  
  /*! Serialisation function. Implements the
   * `Node` interface. */
  std::ostream& Node::WriteXml(std::ostream& os, Handle& hand)
  {       
    os << "  <NODE " 
       << " id=\"" << hand.identify(this) << "\" " 
       << " type=\"" << string_of_type(this->type) << "\" "  ;
    if (Call == this->type)
      {	os << " called=\"" << this->callee_name << "\" " ; }
    os << ">" << std::endl ;
    
    for (std::vector<Instruction*>::const_iterator it(this->instructions.begin()) ; it != this->instructions.end() ; it++)
      { 
	(*it)->WriteXml(os, hand) ;
      }        
    
    this->WriteXmlAttributes(os, hand) ;
    
    os << "  </NODE>" << std::endl ;
    return os ;
  }
  
  /*! Unserialisation function */
  void Node::ReadXml(XmlTag const* tag, Handle& hand) 
  {
    assert(tag->getName()==string("NODE"));
    string id = tag->getAttributeString("id");
    assert(id!="");
    hand.addID_serialisable(id,this) ;
    string attr_type = tag->getAttributeString("type");
    assert(attr_type!="");
    assert(this->type == type_of_string(attr_type));
    
    if (this->type == Call) { 
      string attr_called = tag->getAttributeString("called");
      assert(attr_called != "");
      this->callee_name = attr_called ;
    }
    
    /* Instanciation of all Instruction components */
    ListXmlTag children = tag->getAllChildren();
    for (unsigned int c=0;c<children.size();c++) {
      XmlTag child = children[c];
      if (child.getName()==string("ATTRS_LIST")) continue;
      assert(child.getName()==string("INSTRUCTION"));
      Instruction* inst = new Instruction() ;
      inst->ReadXml(&child, hand) ;
      this->instructions.push_back(inst) ;
    }
	  
    /* Attributes parsing */
    this->ReadXmlAttributes(tag, hand) ;
  }
  
  /*! Get instructions vector */
  std::vector<Instruction*> Node::GetInstructions()
  {
    return this->instructions ;
  }

  /*! Return the list of Code instructions of the Node */
  vector <Instruction *> Node::GetAsm() 
  {
    vector <Instruction *> res;
    for (unsigned int i=0;i<this->instructions.size();i++) {
      if (this->instructions[i]->IsCode()) {
	res.push_back(this->instructions[i]);
      }
    }
    return res;
  }

  /*! Create new instruction. Instruction must be added in the
   * same order than for execution. */
  Instruction* Node::CreateNewInstruction( std::string const& code, asm_type type, bool ret)
  {
    dbg_instr(std::cerr << "CreateNewInstruction called" << std::endl ;);
    Instruction* inst = new Instruction(code, type) ;
    this->instructions.push_back(inst) ;
    this->is_return = (this->is_return || ret) ;
    if (ret) this->cfg->SetEndNode(this) ;
    return inst ;
  }
  
  /*! Return true if the BB ends with a return instruction. */
  bool Node::IsReturn() 
  {
    //FIXME Salto should be the one setting this parameter
    // see mips_dependent.cc is_return_instruction
    vector<Node*> cfg_ends = this->GetCfg()->GetEndNodes();
		
    this->is_return = (find (cfg_ends.begin(), cfg_ends.end(), this) != cfg_ends.end());	
    return (this->is_return && (this->type == BB)) ; 
  }
  
  /*! apply to node of type Call to know which function it calls.
   * return 0 on error or if the Node is not a Call. */
  Cfg* Node::GetCallee()
  {
    if (this->type != Call){ return 0 ; }
    if (callee_cfg == NULL) 
      {
	Program *p = this->cfg->GetProgram();
	this->callee_cfg = p->GetCfgByName(this->callee_name);
	return this->callee_cfg;
      }
    return this->callee_cfg;
  }
  
  /*! Returns the name of the callee of the node is a Call node.
    returns the empty string if the node is not a Call node */
  std::string Node::GetCalleeName() 
  {
    if (this->type != Call) return "";
    return this->callee_name;
  }

  /*! Set the node as a call node and update the required
    information. A warning message is printed on stderr if the
    called function has no Cfg. When the called function does not
    have a Cfg, regardless of the generation of a warning message,
    an empty Cfg is generated. */
  void Node::SetCall(string called, bool emit_warning) 
  {	
    ListOfString names;
    this->type = Call;
    this->callee_name = called;
    Program *p = this->cfg->GetProgram();
    Cfg* c = p->GetCfgByName(called);

    names.push_front(called);
    // Detect use of function pointers
    if (called == "_unknown_") {
      if (emit_warning) 
	{
	  std::cerr << "WARNING : a FunctionCallNode from Cfg " << this->cfg->getStringName() << " refers to a Cfg unknown offline (use of indirect calls) " << std::endl ;
	  std::cerr << "Creating an empty external Cfg  " << std::endl ; 
	}
      c = p->CreateNewCfg(names);
    }
    // Detect calls to extern functions
    else if (c == 0)
      {
	if (emit_warning) {
	  std::cerr << "WARNING : a FunctionCallNode refers to a not existing Cfg " << called << std::endl ;
	  std::cerr << "Creating an empty external Cfg (assumes callee is a library function) " << std::endl ;
	}
	c = p->CreateNewCfg(names);
      }
    this->callee_cfg = c;
  }

  /**
     @return true if the node is composed of a instruction nop, and isolated (without pred/succ).
     In ARM, it is possible (there is no delay slot, but ....)
  */
  bool Node::isIsolatedNopNode()
  {
    if (! this->IsBB()) return false;
    if (instructions.size() != 1) return false;
    string asm_code = instructions[0]->GetCode ();
    if (asm_code != "nop") return false;
    if (GetCfg()->GetPredecessors(this).size() != 0) return false;
    return (GetCfg()->GetSuccessors(this).size() ==0);
  }// cfglib::


  bool Node::isIsolatedNopNodeLB()
  {
    if (! this->IsBB()) return false;
    if (instructions.size() != 1) return false;
    string asm_code = instructions[0]->GetCode ();
    if (asm_code != "nop") return false;
    return (GetCfg()->GetPredecessors(this).size() == 0);
    // return (GetCfg()->GetSuccessors(this).size() ==0);
  }// cfglib::

  string Node::getIdentifier() 
  {
    Program *p = this->cfg->GetProgram();
    Handle hand = p->hand;
    return hand.getId(this);
  }

}


