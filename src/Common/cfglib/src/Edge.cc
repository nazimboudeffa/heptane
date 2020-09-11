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
#include "Edge.h"

/* #includes and forward declarations */
#include <string>
#include <iostream>
#include "Cfg.h"
#include "Handle.h"
#include <cassert>

/*! this namespace is the global namespace */
namespace cfglib 
{
  /* #includes and forward declarations */
  class Cfg ; 

  /*! basic constructor */
  Edge::Edge(Node* const origin, Node* const destination, Cfg* cfg) 
    : cfg(cfg), origin(origin), destination(destination)
  {}
  
  /*! uninitialised constructor */
  Edge::Edge() : cfg(0), origin(0), destination(0)
  {}


  //TP
  /*! cloning function */
  Edge* Edge::Clone(CloneHandle& handle){
    Edge* E = new Edge();
    handle.RegisterClone (this, E);

    handle.ResolveClone (this->origin, (void**) &(E->origin));
    handle.ResolveClone (this->destination, (void**) &(E->destination));

    //handle attributes
    this->CloneAttributesFor (E, handle);
    
    return E;
  }


  /*! give the source Node of this Edge. */
  Node* Edge::GetSource()
  { return this->origin ; }

  /*! give the destination Node of this Edge. */
  Node* Edge::GetTarget()
  { return this->destination ; }

  /*! give the cfg of this Edge. */
  Cfg* Edge::GetCfg()
  { return this->cfg ; }

  /*! Serialisation function. */
  std::ostream& Edge::WriteXml( std::ostream& os, Handle& hand)
  {
    os  << "    <EDGE "
	<< " id=\"" << hand.identify(this) << "\" ";
    os  << " origin=\"" << hand.identify(this->origin) << "\" ";
    os  << " destination=\"" << hand.identify(this->destination) << "\" ";
    os  << ">"
	<< std::endl ;
    
    this->WriteXmlAttributes(os, hand) ; 
    
    os  << "    </EDGE>" << std::endl ;
    return os ;
  }

  /*! Deserialisation function.
   *
   * The deserialisation is in two phase : first the creation of
   * the object (Cfg.GetNewEdge) and second call to O->ReadXml()
   * which initialize the object with correct values. */
  void Edge::ReadXml(XmlTag const* tag, Handle& hand)
  {
    assert(tag->getName() == std::string("EDGE"));
    string id = tag->getAttributeString("id");
    assert(id != "");
    hand.addID_serialisable(id,this);
    
    string origin = tag->getAttributeString("origin");
    assert(origin !="");
    string dest = tag->getAttributeString("destination");
    assert(dest !="");
    
    this->origin = NULL;
    this->destination = NULL;
    hand.addID_handle(origin,(Serialisable **)&(this->origin));
    hand.addID_handle(dest,(Serialisable **)&(this->destination));
    
    // the only Edge child element should be `ATTRS_LIST`
    ListXmlTag children = tag->getAllChildren();
    if (children.size()!=0) {
      assert(children.size()==1);
      assert(children[0].getName()==string("ATTRS_LIST"));
      this->ReadXmlAttributes(tag,hand);
    }
  }

} // cfglib::
