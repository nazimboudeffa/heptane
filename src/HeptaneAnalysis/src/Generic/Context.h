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

/**
 * \file
 * \brief Call context representation.
 * 
 * Classes related to call contexts representation and manipulation.
 *
 * Implementation notes:
 * - Contexts are never manipulated as objects, users only have access to
 * pointers to existing Context objects. 
 * Context are created and freed by a ContextTree which represents the tree of
 * contexts stemming from a given Cfg, the tree entry point, and following the cfg
 * call nodes.
 * - Context comparison therefore should be performed at pointer level. 
 * If two Context* are the same, they refer to the same context in the same
 * tree. Intuitively, contexts in distinct trees, as an example the Context* of
 * different programs, always compare to false. Context trees obtained through
 * copy or cloning share their data with the original and therefore can compare
 * their contexts.
 * - BL
 */

/* FIXME: Pointers to Context are solved upon ContextTree unserialisation.
 * Pointers to a Context can only be solved once the corresponding ContextTree
 * has been unserialised. Instead, the first pointer to a Context could
 * initialise the Context using its id using the appropriate constructor.
 * Further references to the same Context, including the ones in the
 * ContextTree, could then be initialised directly with a proper Context* value.
 * However, the unserialisation Handler does not support searching if an id has
 * been registered already or not.
 * - BL
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>
#include <ostream>

#include "CfgLib.h"
#include "Generic/cow_ptr.h"

using namespace cfglib;

typedef size_t context_id;

class ContextTree;
class Context;

/** \class ContextList \brief Context list attribute.
 *
 * A serialisable attribute capable of holding a list of contexts.
 */
/* Implementation notes: */
/* FIXME: Contents type is restricted to vector.  Unserialisation process
 * requires the address of pointers to be solved later, when the original
 * pointed object is unserialised from the xml. Only the vector stl container
 * provides the ability to get the address of its members.  - BL
 */
/* FIXME: Contents must be a pointer for unserialisation purposes.  When reading
 * a list of contexts from a xml file, contexts pointers' value may not be
 * available due to their context tree being read later. The list cells are
 * registered for late pointer resolution (so-called handles in the cfg
 * library). However, the attribute which is unserialised, and registered for
 * pointer resolution, is not the one attached to objects in the end. Instead a
 * clone is created and attached to objects. The clone itself is not registered
 * for pointer resolution and its original is destroyed.  To circumvent this
 * behaviour, the clone and original contents are swapped upon cloning hence
 * ensuring the registration of the clone contents as target for pointer
 * resolution. This also memory leaks due to pointer resolution targetting the
 * destroyed original attribute.  
 * - BL
 */
class ContextList:public SerialisableAttribute
{
 private:
  /* Should implement random access iterators. */
  typedef vector < Context * >container_type;

  /** Concrete context list. */
  container_type *contexts;

 public:
  /** Iterator on the list's contexts. */
  typedef container_type::const_iterator const_iterator;
  typedef container_type::iterator iterator;

  /** Base constructor. Initialise an empty context list. */
  ContextList ();

  /** Copy constructor.  */
  ContextList (const ContextList &);

  /** Desctructor. Free the context list container. */
  ~ContextList ();

  /** Create a new context list. */
  ContextList *create ();

  /** Clone a context list.
      Create a clone of the list, in case of attribute duplication.
      The pointer to the ContextList::container of the current context is given
      to the clone and a new container is allocated for the current context.
      This is required in case of attribute deserialisation, the contexts may
      have not been deserialised yet and the resolution of their pointers has been 
      delayed. However, only the current context container has been registered as
      requiring pointers to the unserialised contexts. As cloning occurs on
      attributes affectation, we must make sure that the pointers of attached
      attributes will be resolved.
  */
  ContextList *clone ();
  
  /** Clone a context list.
      Create a clone of the list, in case of cfg elements duplications.
      The contexts have to be registered for resolution once the contexts
      will have been cloned. 
  */
  ContextList *clone (CloneHandle &);

  /** Serialise attribute to xml.
      \verbatim
      Xml:
      <ATTR type="<List Attribute type Name>" name="<Attribute Name>">
      <CONTEXT id="<Context id>" />
      ...
      </ATTR>
      \endverbatim
      \param[in,out] os     Xml output stream.
      \param[in,out] handle Serialisable objects id collection.
   */
  ostream & WriteXml (ostream &, Handle &);

  /** Fetch attributes value from xml.
      \verbatim
      <ATTR type="<List Attribute type Name>" name="<Attribute Name>">
      <CONTEXT id="<Context id>" />
      	...
      </ATTR>
      \endverbatim
      \param[in]     tag    Xml tag to parse for values.
      \param[in,out] handle Serialisable objects id collection.
   */
  void ReadXml (const XmlTag *, Handle &);

  /** Print attribute.
      \verbatim
      Format:
      [ContextList <Attribute Name> - <Context Id>, <Context Id>...]
      \endverbatim
  */
  void Print (ostream &);

  /** @return an iterator to the first context in the list. */
  const_iterator begin () const;
  iterator begin ();
  /** @return an iterator to the past-end-element in the list. */
  const_iterator end () const;
  iterator end ();

  /** Add a context at the end of the list, after its last element. */
  void push_back (Context *);

  /** @return the number of contexts in the list. */
  size_t size () const;

  /** @return the element at position \a n in the list. */
  Context *operator[] (size_t n) const;

  /** Assignment operator. */
  ContextList & operator= (const ContextList &);
};

/**
 * \class ContextTree
 * \brief Context tree attribute for a program.
 *
 * A context tree holds the different contexts stemming from a
 * given entry point. 
 *
 * It is responsible for all the possible contexts it allocates, 
 * i.e. should the tree be deleted, if there is no other tree 
 * using the same data, it will be free from the memory.
 *
 * A tree is also responsible for keeping its contexts caches consistent with
 * the tree structure. Modifications of the relations between contexts in the
 * tree should therefore be reflected on the relevant contexts. 
 * See ContextTree::updateContextsCaches().
 *
 * A tree and its copies through affection or construction, or clones all share
 * the same data. If a copy of a tree is modified, so will be the original. If
 * distinct tree are required, they must be constructed separately.
 * See ContextTree::data
 */
class ContextTree:public SerialisableAttribute
{
 public:

  /** Base constructor. 
      Allocate an empty context tree and its data.
      Used for the attributes factory and attributes deserialisation.
  */
  ContextTree ();
  /** Destructor. */
  ~ContextTree ();

  /** Create a new context tree. */
  ContextTree *create ();

  /** Clone a context tree. 
      Create a tightly coupled clone of the tree, in case of attribute duplication.
      \return a clone of the current context tree.
  */
  ContextTree *clone ();
  /** Clone a context tree.
      Create a clone of the tree through a deep copy, in case of cfg elements duplication
      \return a clone of the current context tree.
  */
  ContextTree *clone (CloneHandle &);

  /** Set the tree entry point and initialize its contents.
   * \warning Callgraph from the provided entry point must be acyclic.
   */
  void initialise (Cfg * entry_point);

  /** @return the root context (ie a pointer to the tree root context).
  */
  Context *getRoot () const;

  /** @return  a pointer to the context of id \a id if it exists, out of bound exception otherwise.
      \param[in] id The id of the searched context.
   */
  Context *getContext (context_id id) const;

  /** @return the context attained by calling \a call in context \a c.
      \param[in] Context The caller context
      \param[in] call    The called node.
      \return a pointer to the callee context, or NULL if it does not exist.
      A callee context should always exist for a properly initialised tree, as long
      as the Program has been left unchanged.
   */
  Context *getCalleeContext (const Context * c, Node * call);

  /** @return the number of allocated contexts. */
  size_t getContextsCount () const;

  /**
     Serialise attribute to xml. 
     Serialise the tree data and write the corresponding xml to the \a os output stream.
     Xml:
     <ATTR type="<Context tree attribute name>" name="<Attribute name>" 
     root_uid="<Root context id>">
     <CONTEXTS count="<Number of allocated contexts>"> <CONTEXT id="0" current_function="1" />
 	...
	<CONTEXT id="3" current_function="14" predecessor_id="2" call="42"/> </CONTEXTS>
	<LINKS>
	  <SOURCE id="0" outgoing_count="2"> <DESTINATION call="41" callee_id="1" />
    	   ...
    	  </SOURCE>
 	    ...
 	<SOURCE id="3" outgoind_count="0" />
	</LINKS>
    </CONTEXTS>
    </ATTR>

    \param[in,out] os Xml output stream.
    \param[in,out] handle Serialisable objects id collection.
  */

  ostream & WriteXml (ostream &, Handle &);


  /**
   Fetch attribute value from xml.
\verbatim
Xml: 
 <ATTR type="ContextTree" name="name">
 <CONTEXTS count="4">
 <CONTEXT id="0" current_function="1" />
 ...
 <CONTEXT id="3" current_function="14" predecessor_id="2" call="42"/>
 </CONTEXTS>
 <LINKS>
 <SOURCE id="0" outgoing_count="2">
 <DESTINATION call="41" callee_id="1" />
 ...
 </SOURCE>
 ...
 <SOURCE id="3" outgoind_count="0" />
 </LINKS>
 </ATTR>
\endverbatim
 \param[in]     tag    Xml tag to parse for values.
 \param[in,out] handle Serialisable objects id collection.
 */
  void ReadXml (const XmlTag *, Handle &);

  /** Print attribute. */
  void Print (ostream &);

 private:
  /** Add a new context through \param call from a specified context.
      Create and register the callee context obtained by going through call node
      \param call in context \param caller_context.
      Both \param caller_context and \param call are assumed to point to valid
      objects. \param caller_context is further assumed to be a context of the current tree.
 */
  Context * createCalleeContext (Context * c, Node * call);

  /** Keep the caches in the contexts of the tree to date with the tree structure.
      Updates the cache structures in the contexts which the tree is responsible
      for in case of modifications of the tree structure. This is used as an
      example after tree initialisation (ContextTree::initialise) or
      deserialisation (ContextTree::ReadXml).
      
      \warning Filling some caches requires a fully unserialised cfg, e.g. the address of call nodes in memory.
  */
  void updateContextsCaches ();

  /**
   * \struct TreeImpl
   * \brief Contexts allocated by a tree.
   *
   * Holds an array of contexts and the caller-callee links between them.
   * Frees its contexts upon destruction.
   * 
   * Contains the data required by ContextTree, held in a shared pointer.
   *
   * Calls and callee contexts of a context n are stored in calls[n] and
   * callee_contexts[n] respectively. The jth call node met by context n is in
   * calls[n][j] and the corresponding callee context is in
   * callee_contexts[n][j].
   */
  struct TreeImpl
  {
    ~TreeImpl ();

    /** Root context. */
    Context *root;
    /** Allocated contexts. */
    std::vector < Context * >contexts;
    /** Call nodes met by each context. */
    std::vector < vector < Node * > >calls;
    /** Callee contexts for each caller context. */
    std::vector < vector < Context * > >callee_contexts;
  };

  /** Shared pointer to the actual tree data. */
  heptane_shared_ptr < TreeImpl > contents;
};

/**
 * \class Context
 * \brief Call context representation.
 *
 * Represents a context node in a context tree.
 *
 * Both Context::successors acts as a cache for the context's callees. The
 * context tree is responsible of keeping cache contents up-to-date.
 * See ContextTree::updateContextsCaches().
 */
class Context:public Serialisable
{
 public:
  /** Constructor, used during deserialisation.
      Allocate an empty context, only its Id is specified.
      \param[in] id The id of the new context.
  */
  explicit Context (context_id  id );

  /** Constructor.
      Allocates a new context.
      \see{ContextTree::ContextTree(cfglib::Cfg*)}
      \see{Context::getCalleeContext()}
      \param[in]     id          Context id.
      \param[in]     predecessor Caller context, if any.
      \param[in]     caller      Last call node, if any.
      \param[in]     function    Current context function.
  */
  Context ( context_id id ,Context* predecessor ,Node* caller ,Cfg* function );
  /** Destructor. */
  ~Context ();
  
  /** Clone context.
      Create a clone of the context, in case of cfg elements duplication.
      This way we point to the nodes and cfg clones.
      \return a clone of the current context.
   */
  Context *clone (CloneHandle &);

  /** @return context id. */
  context_id getId () const;
  /** @return the context id as a string. */
  string getStringId () const;
  /** @return the last called function in the context. */
  Cfg *getCurrentFunction () const;
  /** @return the last call met in the context. */
  Node *getCallerNode () const;
  /** @return the caller context. */
  Context *getCallerContext () const;
  /** @return the callee context when calling the \a call node.

      Search for the context attained by calling node \a call in the current
      context.  The local callee context dictionnary, Context::successors, is
      investigated. */
  Context *getCalleeContext (Node * call) const;

  /** Print the context. 
      \param[in,out] os The output stream where the context should be print.
   */
  void print (ostream &) const;

  /**
  Serialise attribute to xml.

  Output the minimum required information. As Context::successors
  is a mere cache for the callee contexts it is not serialised.
  The owner context tree is responsible for keeping this information, 
  see ContextTree::WriteXml.
\verbatim
  Xml:
  <CONTEXT 
     id="<Context id>" 
     current_function="<Current function>" 
   	predecessor_id="<Predecessor context id>" #If any
 		call="<Last call node id>"                #If any
   />
	\endverbatim
  \param[in,out] os     Xml output stream.
  \param[in,out] handle Serialisable objects id collection.
  */
  ostream & WriteXml (ostream &, Handle &);



/**
  Fetch attributes value from xml. 
 
  Note Context::successors, as a cache, is not  unserialised here. It is the
  responsibility of the ContextTree.
 \verbatim
  Xml:
 	<CONTEXT 
     id="<Context id>" 
     current_function="<Current function>" 
   	predecessor_id="<Predecessor context id>" #If any
 		call="<Last call node id>"                #If any
   />
 \endverbatim
  \param[in]     tag    Xml tag to parse for values.
  \param[in,out] handle Serialisable objects id collection.
  */
  void ReadXml (const XmlTag *, Handle &);

 private:
  friend class ContextTree;

  Cfg *current_function;
  Node *caller; ///< Last met call node.
  Context *predecessor; ///< Caller node context.
  typedef std::map < Node *, Context * >successors_map;
  mutable successors_map successors; ///< Callee contexts for each call node in the current function.

  const context_id id; ///< Id.
  string id_string;///< String id.
};

#endif
