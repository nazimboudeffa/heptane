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

#ifndef _IRISA_CFGLIB_EDGE_H
#define _IRISA_CFGLIB_EDGE_H
/* #includes and forward declarations */
#include "Attributed.h"
#include "CloneHandle.h"

/*! this namespace is the global namespace */
namespace cfglib 
{
  /* #includes and forward declarations */
  class Node ; // cf. cfg/node.h
  class Cfg ;  // cf. cfg/cfg.h
  class Handle ; // cf. misc/handlers.h


  /*! This class represents an Edge. 
   * TODO: I do not know if this class implements yet
   * the `Attributed` interface.
   */
  class Edge : public Attributed {
    friend class Cfg;
    //TP
    friend class Loop;
  private: 
    /*! Cfg this edge belong to. */
    Cfg* cfg ;

    /*! */
    Node* origin ;

    /*! */
    Node* destination ;

    /*! basic constructor */
    Edge(Node* source, Node* target, Cfg* cfg);

    /*! uninitialised constructor */
    Edge();

  public:

    /*! return a pointer on the origin Node .
     * Internal helper function used by Cfg. */
    Node* GetSource();

    /*! return a pointer on the destination Node.
     * Internal helper function used by Cfg. */
    Node* GetTarget();
	

    //TP
    /*! cloning function */
    Edge* Clone(CloneHandle&);

    /*! give the cfg of this Edge. */
    Cfg* GetCfg();

    /*! Serialisation function. */
    virtual std::ostream& WriteXml( std::ostream& os, Handle& hand) ;

    /*! Deserialisation function.
     *
     * The deserialisation is in two phase : first the
     * creation of the object (in several ways depending
     * on the object) and second call to O->ReadXml()
     * which initialize the object with correct values. */
    virtual void ReadXml(XmlTag const* tag, Handle& handle);


  };

} // cfglib::
#endif // _IRISA_CFGLIB_EDGE_H
