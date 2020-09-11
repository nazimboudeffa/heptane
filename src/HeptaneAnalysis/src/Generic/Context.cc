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
 * \brief Call context implementation.
 */
#include <assert.h>
#include <queue>
#include "Generic/Context.h"
#include "SharedAttributes/SharedAttributes.h"



/** Base constructor. 
 * 
 * Allocate an empty context tree and its data.
 * Used for the attributes factory and attributes deserialisation.
 */
ContextTree::ContextTree ():
contents (new TreeImpl ())
{
}

/**
 * Destructor.
 */
ContextTree::~ContextTree ()
{
}

/**
 * Create a new context tree.
 *
 * Allocate an empty context tree.
 * Used for attributes deserialisation.
 *
 * \return a pointer to an empty context tree.
 */
ContextTree *
ContextTree::create ()
{
  return new ContextTree ();
}

/**
 * Clone a context tree. 
 *
 * Create a tightly coupled clone of the tree, in case of attribute duplication.
 *
 * \return a clone of the current context tree.
 */
ContextTree *
ContextTree::clone ()
{
  ContextTree *clone = new ContextTree (*this);
  return clone;
}

/**
 * Clone a context tree.
 *
 * Create a clone of the tree through a deep copy, in case of cfg elements duplication.
 *
 * \return a clone of the current context tree.
 */
ContextTree *
ContextTree::clone (CloneHandle & handle)
{
  ContextTree *clone = new ContextTree ();
  handle.RegisterClone (this, clone);

  //Clone contexts.
  for (size_t c = 0; c < contents->contexts.size (); ++c)
    {
      clone->contents->contexts.push_back (this->contents->contexts[c]->clone (handle));
    }

  //Update call nodes to their corresponding clones.
  clone->contents->calls.resize (this->contents->calls.size ());
  for (size_t c = 0; c < contents->calls.size (); ++c)
    {
      clone->contents->calls[c].resize (this->contents->calls[c].size ());
      for (size_t n = 0; n < contents->calls[c].size (); ++n)
	{
	  handle.ResolveClone (this->contents->calls[c][n], (void **) &(clone->contents->calls[c][n]));
	}
    }

  //Update callee contexts to the corresponding clones.
  clone->contents->callee_contexts.resize (this->contents->callee_contexts.size ());
  for (size_t c = 0; c < contents->callee_contexts.size (); ++c)
    {
      clone->contents->callee_contexts[c].resize (this->contents->callee_contexts[c].size ());
      for (size_t t = 0; t < contents->callee_contexts[c].size (); ++t)
	{
	  handle.ResolveClone (this->contents->callee_contexts[c][t], (void **) &(clone->contents->callee_contexts[c][t]));
	}
    }

  //Update tree structure caches in the clone contexts.
  clone->updateContextsCaches ();


  //Set the clone root.
  if (this->contents->root)
    clone->contents->root = (Context *) handle.GetClone (this->contents->root);

  return clone;
}


/** 
 * Set the tree entry point and initialise its contents.
 * \warning Callgraph from the provided entry point must be acyclic.
 */
void
ContextTree::initialise (Cfg * entry_point)
{
  //Initialise the tree root context as the context of its entry point.
  // TODO Cast NULL to proper value
  contents->root = new Context (0, NULL, NULL, entry_point);
  contents->contexts.push_back (contents->root);
  contents->calls.resize (contents->contexts.size ());
  contents->callee_contexts.resize (contents->contexts.size ());

  // Explore the call tree context per context.
  queue < Context * >exploration_queue;
  exploration_queue.push (this->getRoot ());

  while (!exploration_queue.empty ())
    {
      Context *caller_context = exploration_queue.front ();
      Cfg *caller = caller_context->getCurrentFunction ();

      const vector < Node * >&call_nodes = caller->GetCallNodes ();
      for (vector < Node * >::const_iterator call_it = call_nodes.begin (); call_it != call_nodes.end (); ++call_it)
	{
	  Node *call = *call_it;
	  Context *callee_context = this->createCalleeContext (caller_context, call);
	  exploration_queue.push (callee_context);
	}

      exploration_queue.pop ();
    }

  //Update tree structure caches in the contexts.
  this->updateContextsCaches ();
}


/**
 * Add a new context through \param call from a specified context.
 *
 * Create and register the callee context obtained by going through call node
 * \param call in context \param caller_context.
 * Both \param caller_context and \param call are assumed to point to valid
 * objects. \param caller_context is further assumed to be a context of the
 * current tree.
 */
Context *
ContextTree::createCalleeContext (Context * caller_context, Node * call)
{
  assert (caller_context);
  assert (call);
  assert (contents->contexts[caller_context->getId ()] == caller_context);
  assert (caller_context->getCurrentFunction () == call->GetCfg ());
  //Allocate the new context.
  Context *callee_context = new Context (contents->contexts.size (), caller_context, call, call->GetCallee ());

  //Allocate required entries for the new context.
  contents->contexts.push_back (callee_context);
  contents->calls.resize (contents->contexts.size ());
  contents->callee_contexts.resize (contents->contexts.size ());
  assert (callee_context->getId () == contents->contexts.size () - 1);

  //Create links.
  contents->calls[caller_context->getId ()].push_back (call);
  contents->callee_contexts[caller_context->getId ()].push_back (callee_context);

  return callee_context;
}


/**
 * Keep the caches in the contexts of the tree to date with the tree structure.
 *
 * Updates the cache structures in the contexts which the tree is responsible
 * for in case of modifications of the tree structure. This is used as an
 * example after tree initialisation (ContextTree::initialise) or
 * deserialisation (ContextTree::ReadXml).
 *
 * \warning Filling some caches requires a fully unserialised cfg, e.g. the
 * address of call nodes in memory.
 */
void
ContextTree::updateContextsCaches ()
{
  //Update each context successors list.
  for (size_t c = 0; c < contents->contexts.size (); ++c)
    {
      for (size_t n = 0; n < contents->calls[c].size (); ++n)
	{
	  assert (contents->callee_contexts[c][n]);
	  assert (contents->calls[c][n]);
	  contents->contexts[c]->successors[contents->calls[c][n]] = contents->callee_contexts[c][n];
	}
    }
}


/**
 * Get the root context. 
 * \return a pointer to the tree root context.
 */
Context *
ContextTree::getRoot () const
{
  return contents->root;
}

/**
 * Get the context of number \a id.
 * 
 * \param[in] id The id of the searched context.
 *
 * \return a pointer to the context of id \a id if it exists, out of bound exception otherwise.
 */
Context *
ContextTree::getContext (context_id id) const
{
  assert (id < contents->contexts.size ());
  return contents->contexts[id];
}

/**
 * Get the context attained by calling \a call in context \a context. 
 *
 * \param[in] context The caller context.
 * \param[in] call    The called node.
 *
 * \return a pointer to the callee context, or NULL if it does not exist.
 * A callee context should always exist for a properly initialised tree, as long
 * as the Program has been left unchanged.

 */
Context *
ContextTree::getCalleeContext (const Context * context, Node * call)
{
  size_t id = context->getId ();
  size_t call_index = 0;
  //Search for the edge position based on the call node.
  while (call_index < contents->calls[id].size () && contents->calls[id][call_index] != call)
    {
      ++call_index;
    }

  //Known context, return it.
  if (call_index < contents->calls[id].size ())
    {
      assert (contents->callee_contexts.size () > id);
      assert (contents->callee_contexts[id].size () > call_index);
      return contents->callee_contexts[id][call_index];
    }
  // FIXME: Missing context could be generated on the fly using
  // createCalleeContext;

  assert (false);
  return NULL;
}

/**
 * Get the number of allocated contexts.
 * \return the number of allocated contexts.
 */
size_t
ContextTree::getContextsCount () const
{
  return contents->contexts.size ();
}

/**
 * Serialise attribute to xml. 
 *
 * Serialise the tree data and write the corresponding xml to the \a os output stream.
 * Xml:
 * <ATTR type="<Context tree attribute name>" name="<Attribute name>"
 *			 root_uid="<Root context id>"
 * >
 *	<CONTEXTS count="<Number of allocated contexts>">
 *				<CONTEXT id="0" current_function="1" />
 *				...
 *				<CONTEXT id="3" current_function="14" predecessor_id="2" call="42"/>
 *		</CONTEXTS>
 *		<LINKS>
 *				<SOURCE id="0" outgoing_count="2">
 *						<DESTINATION call="41" callee_id="1" />
 *						...
 *				</SOURCE>
 *				...
 *				<SOURCE id="3" outgoind_count="0" />
 *		</LINKS>
 *  </CONTEXTS>
 * </ATTR>
 *
 * \param[in,out] os     Xml output stream.
 * \param[in,out] handle Serialisable objects id collection.
 */
ostream & ContextTree::WriteXml (ostream & os, Handle & handle)
{
  os << "<ATTR " << "type=\"" << ContextTreeAttributeName << "\" " << "name=\"" << this->name << "\" " << "";
  if (contents->root)
    {
      os << "root_uid=\"" << this->contents->root->getId () << "\" ";
    }
  os << ">" << endl;
  
  //Output contexts.
  os << "\t" << "<CONTEXTS " << "count=\"" << this->contents->contexts.size () << "\" " << ">" << endl;

  for (size_t c = 0; c < contents->contexts.size (); ++c)
    {
      assert (contents->contexts[c]);
      assert (contents->contexts[c]->getId () == c);
      Context *
	context = contents->contexts[c];
      context->WriteXml (os, handle);
    }

  os << "\t" << "</CONTEXTS>" << endl;
  //End Output contexts

  //Output context relationships
  os << "\t" << "<LINKS>" << endl;

  assert (contents->calls.size () == contents->callee_contexts.size ());
  for (size_t c = 0; c < contents->contexts.size (); ++c)
    {
      Context *
	context = contents->contexts[c];
      os << "\t\t" << "<SOURCE " << "id=\"" << handle.identify (context) << "\" " << "outgoing_count=\"" << contents->calls[c].size () << "\" ";

      if (contents->calls[c].size () == 0)
	{
	  os << "/>" << endl;
	}
      else
	{
	  assert (contents->calls[c].size () == contents->callee_contexts[c].size ());
	  os << ">" << endl;
	  for (size_t n = 0; n < contents->calls[c].size (); ++n)
	    {
	      Node *
		call = contents->calls[c][n];
	      Context *
		callee_context = contents->callee_contexts[c][n];
	      os << "\t\t\t"
		<< "<DESTINATION "
		<< "call=\"" << handle.identify (call) << "\" " << "callee_id=\"" << handle.identify (callee_context) << "\" " << "/>" << endl;
	    }

	  os << "\t\t" << "</SOURCE>" << endl;
	}
    }

  os << "\t" << "</LINKS>" << endl;


  os << "</ATTR>" << endl;

  return os;
}

/**
 * Fetch attribute value from xml.
 *
 * Xml: 
 * <ATTR type="ContextTree" name="name">
 *		<CONTEXTS count="4">
 *				<CONTEXT id="0" current_function="1" />
 *				...
 *				<CONTEXT id="3" current_function="14" predecessor_id="2" call="42"/>
 *		</CONTEXTS>
 *		<LINKS>
 *				<SOURCE id="0" outgoing_count="2">
 *						<DESTINATION call="41" callee_id="1" />
 *						...
 *				</SOURCE>
 *				...
 *				<SOURCE id="3" outgoind_count="0" />
 *		</LINKS>
 * </ATTR>
 *
 * \param[in]     tag    Xml tag to parse for values.
 * \param[in,out] handle Serialisable objects id collection.
 */
void
ContextTree::ReadXml (const XmlTag * tag, Handle & handle)
{
  //Remove prior data.
  contents->contexts.clear ();
  contents->calls.clear ();
  contents->callee_contexts.clear ();

  //Fetch attribute name.
  string name = tag->getAttributeString ("name");
  this->name = name;
  assert (name != "");

  ListXmlTag children = tag->getAllChildren ();
  assert (children.size () == 2);
  XmlTag contexts_tag = children[0];
  assert (contexts_tag.getName () == "CONTEXTS");
  XmlTag links_tag = children[1];
  assert (links_tag.getName () == "LINKS");

  //Fetch and recreate contexts.
  size_t contexts_cnt = contexts_tag.getAttributeInt ("count");
  ListXmlTag contexts = contexts_tag.getAllChildren ();
  assert (contexts.size () == contexts_cnt);
  contents->contexts.resize (contexts_cnt);
  for (size_t c = 0; c < contexts_cnt; ++c)
    {
      XmlTag context_tag = contexts[c];
      assert (context_tag.getName () == "CONTEXT");

      //Create context and fill fields.
      Context *context = new Context (c);
      context->ReadXml (&context_tag, handle);

      //Store created context.
      contents->contexts[c] = context;
    }

  //Retrieve root
  context_id root_id = tag->getAttributeInt ("root_uid");
  contents->root = contents->contexts[root_id];


  //Fetch and recreate stored links.
  contents->calls.resize (contexts_cnt);
  contents->callee_contexts.resize (contexts_cnt);
  ListXmlTag sources_tag = links_tag.getAllChildren ();
  for (size_t c = 0; c < contexts_cnt; ++c)
    {
      XmlTag source_tag = sources_tag[c];
      assert (source_tag.getName () == "SOURCE");

      string id = source_tag.getAttributeString ("id");
      size_t destinations_cnt = source_tag.getAttributeInt ("outgoing_count");

      contents->calls[c].resize (destinations_cnt);
      contents->callee_contexts[c].resize (destinations_cnt);

      ListXmlTag destinations_tag = source_tag.getAllChildren ();
      for (size_t n = 0; n < destinations_cnt; ++n)
	{
	  XmlTag destination_tag = destinations_tag[n];

	  string callee_id = destination_tag.getAttributeString ("callee_id");
	  string call_id = destination_tag.getAttributeString ("call");

	  assert (callee_id != "");
	  assert (call_id != "");

	  handle.addID_handle (callee_id, (Serialisable **) & (contents->callee_contexts[c][n]));
	  handle.addID_handle (call_id, (Serialisable **) & (contents->calls[c][n]));
	}
    }

  //Update tree structure caches in the contexts.
  this->updateContextsCaches ();
}

/**
 * Print attribute.
 */
void
ContextTree::Print (ostream & os)
{
  os << "ContextTree (" << contents->contexts.size () << " contexts)" << endl;
}


/**
 * TreeImpl destructor.
 *
 * Free all allocated contexts.
 */
ContextTree::TreeImpl::~TreeImpl ()
{
  for (size_t c = 0; c < contexts.size (); ++c)
    {
      delete contexts[c];
    }
}

/**
 * Constructor, used during deserialisation.
 *
 * Allocate an empty context, only its Id is specified.
 *
 * \param[in] id The id of the new context.
 */
Context::Context (context_id id):
current_function (NULL), caller (NULL), predecessor (NULL), successors (), id (id), id_string ("")
{
  // TODO DUPLICATE CODE IN CONSTRUCTORS DELEGATE TO FUNCTION
  ostringstream stream;
  stream << id;
  id_string = stream.str ();
}

/**
 * Constructor.
 *
 * Allocates a new context.
 * \see{ContextTree::ContextTree(cfglib::Cfg*)}
 * \see{Context::getCalleeContext()}
 *
 * \param[in]     id          Context id.
 * \param[in]     predecessor Caller context, if any.
 * \param[in]     caller      Last call node, if any.
 * \param[in]     function    Current context function.
 */
Context::Context (context_id id, Context * predecessor, Node * caller, Cfg * function):
current_function (function), caller (caller), predecessor (predecessor), successors (), id (id), id_string ("")
{
  assert (!caller || current_function == caller->GetCallee ());

  ostringstream stream;
  stream << id;
  id_string = stream.str ();
}

/**
 * Destructor.
 */
Context::~Context ()
{
}

/**
 * Clone context.
 *
 * Create a clone of the context, in case of cfg elements duplication.
 * This way we point to the nodes and cfg clones.
 *
 * \return a clone of the current context.
 */
Context *
Context::clone (CloneHandle & handle)
{
  Context *clone = new Context (this->id);
  handle.RegisterClone (this, clone);

  handle.ResolveClone (this->current_function, (void **) &(clone->current_function));
  handle.ResolveClone (this->caller, (void **) &(clone->caller));
  handle.ResolveClone (this->predecessor, (void **) &(clone->predecessor));

  return clone;
}

/**
 * Get context if.
 * \return Context id.
 */
context_id
Context::getId () const
{
  return this->id;
}

/**
 * Get context id as a string.
 * \return Context id in the string format.
 */
string
Context::getStringId () const
{
  return this->id_string;
}

/**
 * Return the last called function in the context.
 */
Cfg *
Context::getCurrentFunction () const
{
  return this->current_function;
}

/**
 * Return the last call met in the context.
 */
Node *
Context::getCallerNode () const
{
  return this->caller;
}

/**
 * Return the caller context.
 */
Context *
Context::getCallerContext () const
{
  return this->predecessor;
}

/** 
 * Get the callee context when calling the \a call node.
 *
 * Search for the context attained by calling node \a call in the current
 * context.  The local callee context dictionnary, Context::successors, is
 * investigated. 
 */
Context *
Context::getCalleeContext (Node * call) const
{
  assert (call->GetCfg () == this->current_function);

  //Search for the edge in the successors map cache.
  successors_map::const_iterator position = successors.find (call);
  if (position != successors.end ())
    {
      return position->second;
    }

  assert (false);
  return NULL;
}

/**
 * Print the context.
 *
 * \param[in,out] os The output stream where the context should be print.
 */
void
Context::print (ostream & os) const
{
  string caller_name = "none";
  if (getCallerNode ())
    // caller_name = getCallerNode ()->GetCfg ()->GetName ();
    caller_name = getCallerNode ()->GetCfg ()->getStringName ();

  string caller_id = "none";
  if (getCallerContext ())
    caller_id = getCallerContext ()->getStringId ();

  os << "---- Context ------------------" << endl
    << endl
    << "  Id        : " << getId () << endl
     << "  Function  : " << getCurrentFunction ()->getStringName /*GetName*/ () << endl
    << "  Caller    : " << caller_name << endl << "  Caller Id : " << caller_id << endl << "-------------------------------" << endl;
}

/**
 * Serialise attribute to xml.
 *
 * Output the minimum required information. As Context::successors
 * is a mere cache for the callee contexts it is not serialised.
 * The owner context tree is responsible for keeping this information, 
 * see ContextTree::WriteXml.
 *
 * Xml:
 *	<CONTEXT 
 *    id="<Context id>" 
 *    current_function="<Current function>" 
 *  	predecessor_id="<Predecessor context id>" #If any
 *		call="<Last call node id>"                #If any
 *  />
 *
 * \param[in,out] os     Xml output stream.
 * \param[in,out] handle Serialisable objects id collection.
 */
ostream & Context::WriteXml (ostream & os, Handle & handle)
{
  os << "\t\t"
    << "<CONTEXT " << "id=\"" << handle.identify (this) << "\" " << "current_function=\"" << handle.identify (this->current_function) << "\" " << "";
  if (this->predecessor || this->caller)
    {
      assert (this->predecessor && this->caller);
      os << "predecessor_id=\"" << handle.identify (this->predecessor) << "\" " << "call=\"" << handle.identify (this->caller) << "\" " << "";
    }

  os << "/>" << endl;
  return os;
}

/**
 * Fetch attributes value from xml. 
 *
 * Note Context::successors, as a cache, is not * unserialised here. It is the
 * responsibility of the ContextTree.
 *
 * Xml:
 *	<CONTEXT 
 *    id="<Context id>" 
 *    current_function="<Current function>" 
 *  	predecessor_id="<Predecessor context id>" #If any
 *		call="<Last call node id>"                #If any
 *  />
 *
 * \param[in]     tag    Xml tag to parse for values.
 * \param[in,out] handle Serialisable objects id collection.
 */
void
Context::ReadXml (const XmlTag * tag, Handle & handle)
{
  //Fetch current context properties
  string id = tag->getAttributeString ("id");
  string function_id = tag->getAttributeString ("current_function");
  string predecessor_id = tag->getAttributeString ("predecessor_id");
  string call_node_id = tag->getAttributeString ("call");

  assert (id != "");
  assert (function_id != "");
  handle.addID_serialisable (id, this);

  handle.addID_handle (function_id, (Serialisable **) & (this->current_function));
  if (call_node_id != "")
    {
      handle.addID_handle (predecessor_id, (Serialisable **) & (this->predecessor));
      handle.addID_handle (call_node_id, (Serialisable **) & (this->caller));
    }
}

/**
 * Base constructor.
 *
 * Initialise an empty context list.
 */
ContextList::ContextList ():
contexts (new container_type ())
{
}

/**
 * Copy constructor.
 */
ContextList::ContextList (const ContextList & src):
contexts (new container_type (*(src.contexts)))
{
}

/**
 * Destructor.
 * Free the context list container.
 */
ContextList::~ContextList ()
{
  delete contexts;
}

/**
 * Create a new empty context list.
 */
ContextList *
ContextList::create ()
{
  return new ContextList ();
}

/**
 * Clone a context list.
 *
 * Create a clone of the list, in case of attribute duplication.
 * The pointer to the ContextList::container of the current context is given
 * to the clone and a new container is allocated for the current context.
 * This is required in case of attribute deserialisation, the contexts may
 * have not been deserialised yet and the resolution of their pointers has been 
 * delayed. However, only the current context container has been registered as
 * requiring pointers to the unserialised contexts. As cloning occurs on
 * attributes affectation, we must make sure that the pointers of attached
 * attributes will be resolved.
 *	
 */
ContextList *
ContextList::clone ()
{
  ContextList *clone = new ContextList ();

  swap (clone->contexts, this->contexts);
  *(this->contexts) = (*clone->contexts);

  return clone;
}

/**
 * Clone a context list.
 *
 * Create a clone of the list, in case of cfg elements duplications.
 * The contexts have to be registered for resolution once the contexts
 * will have been cloned.
 */
ContextList *
ContextList::clone (CloneHandle & handle)
{
  ContextList *clone = new ContextList ();
  clone->contexts->resize (this->contexts->size ());

  for (size_t c = 0; c < this->contexts->size (); ++c)
    {
      handle.ResolveClone (this->contexts->at (c), (void **) &(clone->contexts->at (c)));
    }

  return clone;
}

/**
 * Serialisable attribute to xml.
 * 
 * Xml:
 * <ATTR type="<List Attribute type Name>" name="<Attribute Name>">
 *	<CONTEXT id="<Context id>" />
 *	...
 * </ATTR>
 *
 * \param[in,out] os     Xml output stream.
 * \param[in,out] handle Serialisable objects id collection.
 */
ostream & ContextList::WriteXml (ostream & os, Handle & handle)
{
  os << "<ATTR " << "type=\"" << ContextListAttributeName << "\" " << "name=\"" << this->name << "\" " << ">" << endl;

  for (size_t c = 0; c < contexts->size (); ++c)
    {
      os << "<CONTEXT " << "id=\"" << handle.identify (contexts->at (c)) << "\" " << "/>" << endl;
    }

  os << "</ATTR>" << endl;

  return os;
}

/**
 * Fetch attributes value from xml.
 *
 * Xml:
 * <ATTR type="<List Attribute type Name>" name="<Attribute Name>">
 *	<CONTEXT id="<Context id>" />
 *	...
 * </ATTR>
 * * \param[in]     tag    Xml tag to parse for values.
 * \param[in,out] handle Serialisable objects id collection.
 */
void
ContextList::ReadXml (const XmlTag * tag, Handle & handle)
{

  //Fetch attribute name.
  string name = tag->getAttributeString ("name");
  this->name = name;
  assert (name != "");

  //Remove any stored data
  this->contexts->clear ();

  //Retrieve contexts of the list.
  ListXmlTag children = tag->getAllChildren ();
  this->contexts->resize (children.size ());
  for (size_t c = 0; c < this->contexts->size (); ++c)
    {
      XmlTag context_tag = children[c];
      assert (context_tag.getName () == "CONTEXT");

      string context_id = context_tag.getAttributeString ("id");
      assert (context_id != "");

      handle.addID_handle (context_id, (Serialisable **) & (this->contexts->at (c)));
    }

}

/**
 * Print context list on the \param os output stream.
 *
 * Format:
 * [ContextList <Attribute Name> - <Context Id>, <Context Id>...]
 */
void
ContextList::Print (ostream & os)
{
  os << "[ContextList " << this->name << " - ";
  for (size_t c = 0; c < this->contexts->size (); ++c)
    {
      if (c != 0)
	{
	  os << ", ";
	}
      os << this->contexts->at (c)->getId ();
    }
  os << "]" << endl;
}

/**
 * Get an iterator to the first context in the list.
 */
ContextList::const_iterator ContextList::begin () const
{
  return this->contexts->begin ();
}

ContextList::iterator ContextList::begin ()
{
  return this->contexts->begin ();
}

/**
 * Get an iterator to the past-end-element in the list.
 */
ContextList::const_iterator ContextList::end () const
{
  return this->contexts->end ();
}

ContextList::iterator ContextList::end ()
{
  return this->contexts->end ();
}

/**
 * Add a context at the end of the list, after its last element.
 * \param context Pointer to the context to be inserted.
 */
void
ContextList::push_back (Context * context)
{
  this->contexts->push_back (context);
}

/**
 * Get the number of contexts in the list. 
 */
size_t
ContextList::size () const
{
  return this->contexts->size ();
}

/**
 * Get the element at position \a index in the list.
 * \param[in] index Required element position.
 */
Context *
ContextList::operator[] (size_t index)
     const
     {
       return
	 this->
	 contexts->
       at (index);
     }

/**
 * Assignment operator.
 */
ContextList &
ContextList::operator= (const ContextList & rhs)
{
  if (this != &rhs)
    {
      delete this->contexts;
      this->contexts = new container_type (*(rhs.contexts));
    }

  return *this;
}
