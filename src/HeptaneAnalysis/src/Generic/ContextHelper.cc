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

#include "Generic/ContextHelper.h"


/** Empty constructor. */
ContextualNode::ContextualNode ():
context (NULL), node (NULL)
{
}


/** Basic constructor. */
ContextualNode::ContextualNode (Context * context, cfglib::Node * node):
context (context), node (node)
{
}

bool AreElemOfSameCfg(const ContextualNode &n1, const ContextualNode &n2)
{
  return (n1.node->GetCfg () ==  n2.node->GetCfg ());
}


int getIndexElemSameCfg(const std::vector < ContextualNode > lnodes, const ContextualNode &aNode)
{
  for (size_t i = 0; i < lnodes.size (); i++)
    if (AreElemOfSameCfg (lnodes[i], aNode)) return i;
  return 0; // default 
}


/** Get the predecessors and their contexts of a contextual node. */
std::vector < ContextualNode > GetContextualPredecessors (const ContextualNode & position)
{
  Cfg *function = position.node->GetCfg ();
  std::vector < ContextualNode > contextual_predecessors;

  // Get the node predecessors from its function.
  const vector < Node * >&predecessors = function->GetPredecessors (position.node);
  for (size_t p = 0; p < predecessors.size (); ++p)
    {
      if (predecessors[p]->IsCall ())
	{
	  Context *callee_context = position.context->getCalleeContext (predecessors[p]);
	  const vector < Node * >&callee_ends = predecessors[p]->GetCallee ()->GetEndNodes ();
	  for (size_t e = 0; e < callee_ends.size (); ++e)
	    {
	      contextual_predecessors.push_back (ContextualNode (callee_context, callee_ends[e]));
	    }
	}
      else
	{
	  contextual_predecessors.push_back (ContextualNode (position.context, predecessors[p]));
	}
    }

  // If the node is a function entry, add the caller node.
  Context *caller_context = position.context->getCallerContext ();
  if (function->GetStartNode () == position.node && caller_context != NULL)
    {
      contextual_predecessors.push_back (ContextualNode (caller_context, position.context->getCallerNode ()));
    }

  return contextual_predecessors;
}

/** If position is a function entry, return the caller node, otherwise NULL.*/
Node * getContextualNodeCallerNode (const ContextualNode & position)
{
  Cfg *function = position.node->GetCfg ();
  Context *caller_context = position.context->getCallerContext ();
  if (function->GetStartNode () == position.node && caller_context != NULL)
    return position.context->getCallerNode ();

return NULL;
}


/** Get the successors and their contexts of a contextual node. */
std::vector < ContextualNode > GetContextualSuccessors (const ContextualNode & position)
{
  Cfg *function = position.node->GetCfg ();
  std::vector < ContextualNode > contextual_successors;
  // If the node is a call, its successor is its callee entry point.
  if (position.node->IsCall ())
    {
      Context *callee_context = position.context->getCalleeContext (position.node);
      Node *callee_node = position.node->GetCallee ()->GetStartNode ();
      contextual_successors.push_back (ContextualNode (callee_context, callee_node));
    }
  else if (position.node->IsReturn ())
    {
      // If the node is a function end, add its caller successors.

      // A return node should have no successors in the cfg. This may happen on
      // predicated return instructions and is not supported yet. -BL
      assert (function->GetSuccessors (position.node).size () == 0);

      if (position.context->getCallerNode () != NULL)
	{
	  Node *caller_node = position.context->getCallerNode ();
	  Cfg *caller_function = caller_node->GetCfg ();
	  Context *caller_context = position.context->getCallerContext ();
	  const vector < Node * >&caller_successors = caller_function->GetSuccessors (caller_node);

	  for (size_t s = 0; s < caller_successors.size (); ++s)
	    {
	      contextual_successors.push_back (ContextualNode (caller_context, caller_successors[s]));
	    }
	}
    }
  else
    {
      // Get the node successors from its function.
      const vector < Node * >&successors = function->GetSuccessors (position.node);
      for (size_t s = 0; s < successors.size (); ++s)
	{
	  contextual_successors.push_back (ContextualNode (position.context, successors[s]));
	}
    }
  return contextual_successors;
}

/** Check if a context is an ancestor of another context. */
bool
IsAncestorContextOf (const Context * ancestor, const Context * context)
{
  bool isAncestor = false;
  const Context *current = context;
  while (current && !isAncestor)
    {
      isAncestor = (current == ancestor);
      current = current->getCallerContext ();
    }
  return isAncestor;
}

/** Get the earliest common ancestor of two contexts.
 *
 * Return NULL if there is no such ancestor.
 */
const Context *
GetCommonAncestor (const Context * lhs, const Context * rhs)
{
  if (lhs == NULL || rhs == NULL)
    return NULL;
  if (lhs == rhs)
    return lhs;

  const Context *ancestor = NULL;
  const Context *lhs_ancestor = lhs;
  const Context *rhs_ancestor = rhs;
  while (lhs_ancestor && ancestor == NULL)
    {
      while (rhs_ancestor && ancestor == NULL)
	{
	  if (lhs_ancestor == rhs_ancestor)
	    {
	      ancestor = lhs_ancestor;
	    }
	  rhs_ancestor = rhs_ancestor->getCallerContext ();
	}
      lhs_ancestor = lhs_ancestor->getCallerContext ();
      rhs_ancestor = rhs;
    }
  return ancestor;
}

/** Get the calls and their contexts traversed to reach a context. */
std::vector < ContextualNode > GetCallerStack (const Context * context)
{
  std::vector < ContextualNode > callers;
  const Context *callee = context;
  Context *caller = context->getCallerContext ();
  while (caller)
    {
      callers.push_back (ContextualNode (caller, callee->getCallerNode ()));
      callee = caller;
      caller = caller->getCallerContext ();
    }
  return callers;
}

/** Get the call nodes traversed to reach a context from the program entry. */
std::vector < Node * >GetCallerNodeStack (const Context * context)
{
  std::vector < Node * >caller_nodes;
  const Context *callee = context;
  Context *caller = context->getCallerContext ();
  while (caller)
    {
      caller_nodes.push_back (callee->getCallerNode ());
      callee = caller;
      caller = caller->getCallerContext ();
    }
  return caller_nodes;
}

/** Get the contexts traversed to reach a context from the program entry. */
std::vector < Context * >GetCallerContextStack (const Context * context)
{
  std::vector < Context * >caller_contexts;
  Context *caller = context->getCallerContext ();
  while (caller)
    {
      caller_contexts.push_back (caller);
      caller = caller->getCallerContext ();
    }
  return caller_contexts;
}

/** Get all the contexts that can be reached from a given context. */
std::vector < Context * >GetCalleeContexts (const Context * context)
{
  std::vector < Context * >callees;

  {
    const std::vector < Node * >&call_nodes = context->getCurrentFunction ()->GetCallNodes ();
    for (size_t n = 0; n < call_nodes.size (); ++n)
      {
	callees.push_back (context->getCalleeContext (call_nodes[n]));
      }
  }


  for (size_t c = 0; c < callees.size (); ++c)
    {
      const std::vector < Node * >&call_nodes = callees[c]->getCurrentFunction ()->GetCallNodes ();
      for (size_t n = 0; n < call_nodes.size (); ++n)
	{
	  callees.push_back (callees[c]->getCalleeContext (call_nodes[n]));
	}
    }

  return callees;
}

/** Get the string representation of the current context */
string
getStringContextRepresentation (const Context * context)
{
  const vector < Context * >&caller_stack = GetCallerContextStack (context);
  ostringstream ctx_str;
  for (size_t i = caller_stack.size (); i > 0; --i)
    {
      ctx_str << caller_stack[i - 1]->getCurrentFunction ()->getStringName() << "#";
    }
  ctx_str << context->getCurrentFunction ()->getStringName /*GetName*/ ();
  return ctx_str.str ();
}
