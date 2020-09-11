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

#ifndef _IRISA_CFGLIB_NODE_H
#define _IRISA_CFGLIB_NODE_H

/* #includes and forward declarations : */
#include <libxml/tree.h>
#include "Attributed.h"
#include "CloneHandle.h"
#include "Instruction.h"

/* Debug of node management methods */
// Uncomment one of these two lines to enter/leave debug mode
//#define dbg_node(x) x
#define dbg_node(x)

/*! this namespace is the global namespace */
namespace cfglib 
{
  /* #includes and forward declarations : */
  class Cfg ; // cf. cfg/cfg.h

  /*! possible Node types */
  enum node_type { BB, Call } ;
  
  /*! This abstract class represents a Node. Nodes are basic
   * components of a CFG. Nodes can be of type `BB` or `Call`
   * A `Call` node contains instructions including the call instruction.
   */
  class Node : public Attributed {
  public: 
    
  private:
    /*! Cfg this node belong to */
    //TP
    Cfg* cfg ;

    /*! type of the current Node */
    node_type type ;

    /*! true if the BB end with a return instruction */
    bool is_return ;

    /*! name of the Cfg this Call node calls */
    std::string callee_name;

    /*! Cfg this Call node calls */
    Cfg* callee_cfg;

  public:
    /*! Return the Cfg this Node belong to. */
    Cfg* GetCfg();

    /*! Cfg can create Nodes, et caetera. */
    friend class Cfg;
    //TP
    friend class Loop;

  private:
    /*! Instruction list */
    std::vector<Instruction*> instructions ;

    /*! Constructor. This constructor is private
     * and meaned to be only called via friend
     * `Cfg::CreateNode()` */
    Node(Cfg* cfg);

    /*! Constructor. This constructor is private
     * and meaned to be only called via friend
     * `Cfg::CreateNode()` */
    Node(Cfg* cfg, node_type type);

    //TP
    /*! constructor for a free Cfg (used in clone function) */
    Node();

    /*! Virtual constructor. */
    virtual Node* create(Cfg* cfg, node_type type) const;
    
  public:
    /*! basic destructor */
    virtual ~Node () ;

    //TP
    /*! cloning function */
    Node* Clone(CloneHandle&);

    /*! Serialisation function. Implements the
     * `Node` interface. */
    virtual std::ostream& WriteXml(std::ostream& os, Handle& hand) ;

    /*! Unserialisation */
    virtual void ReadXml( XmlTag const* tag, Handle& hand); 

    /*! Returns the node type */
    node_type GetType() {return type;}

    /*! Returns true if the node is a Call node */
    bool IsCall();

    /*! Returns true if the node is a basic block */
    bool IsBB();

    /*! Get instructions vector */
    std::vector<Instruction*> GetInstructions();

    /*! Get the number of asm lines (instructions + others)
      of the node */
    int GetNbInstructions() {return instructions.size();}

    /*! Return the list of Code instructions of the Node */
    std::vector <Instruction *>GetAsm();

    /*! Create new instruction. Instruction must
     * be added in the same order than for execution. */
    Instruction* CreateNewInstruction(
				   std::string const& code, 
				   asm_type asm_type,
				   bool ret);
    
    /*! Return true if the BB ends with a return instruction. */
    bool IsReturn() ;

    /*! Applies to node of type Call to know which function it
     * calls.  return 0 on error or if the Node is not a Call. */
    Cfg* GetCallee() ;
    
    /*! Returns the name of the callee of the node is a Call node.
      returns the empty string if the node is not a Call node */
    std::string GetCalleeName();

    /*! Set the node as a call node and update the required
      information. A warning message is printed on stderr if the
      called function has no Cfg. When the called function does not
      have a Cfg, regardless of the generation of a warning message,
      an empty Cfg is generated. */
    void SetCall(string called, bool emit_warning) ;

    /*!
       @return 
       - true if the node is composed of a instruction nop, and isolated (without pred/succ).
       In ARM, it is possible (there is no delay slot,but ....)
       - false totherwise.
    */
    bool isIsolatedNopNode();

    bool isIsolatedNopNodeLB();
    string getIdentifier();
  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_NODE_H
