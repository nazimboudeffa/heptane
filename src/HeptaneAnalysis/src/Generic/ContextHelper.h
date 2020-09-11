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
 * \brief Call context-based helper structures and operations.
 *
 * Operations based on the call context representation. Provide contextual
 * derivations of cfglib elements.
 */
#ifndef CONTEXT_HELPER_H
#define CONTEXT_HELPER_H

#include <functional>
#include <stack>

#include "Generic/Context.h"

/**
 * \class ContextualNode
 * \brief A cfg node in a given call context.
 */
class ContextualNode
{
public:
  Context * context;
  Node *node;

  /** Empty constructor. */
  ContextualNode ();
  /** Basic constructor. */
  ContextualNode (Context *, Node *);
  Node * getNode()
{
  return node;
}

Context * getContext()
{
  return context;
}

};

/** ContextualNode relational operators. */
inline bool operator== (const ContextualNode &, const ContextualNode &);
inline bool operator!= (const ContextualNode &, const ContextualNode &);
inline bool operator< (const ContextualNode &, const ContextualNode &);
inline bool operator> (const ContextualNode &, const ContextualNode &);
inline bool operator<= (const ContextualNode &, const ContextualNode &);
inline bool operator>= (const ContextualNode &, const ContextualNode &);

bool AreElemOfSameCfg(const ContextualNode &current, const ContextualNode &pred);
int getIndexElemSameCfg(const std::vector < ContextualNode > lnodes, const ContextualNode &aNode);

/** Get the predecessors and their contexts of a contextual node. */
std::vector < ContextualNode > GetContextualPredecessors (const ContextualNode &);

/** Get the successors and their contexts of a contextual node. */
std::vector < ContextualNode > GetContextualSuccessors (const ContextualNode &);

Node * getContextualNodeCallerNode (const ContextualNode & position);
/** Check if a context is an ancestor of another context. */
bool IsAncestorContextOf (const Context * ancestor, const Context * context);

/** Get the earliest common ancestor of two contexts. */
const Context *GetCommonAncestor (const Context *, const Context *);

/** Get the calls and their contexts traversed to reach a context. */
std::vector < ContextualNode > GetCallerStack (const Context *);

/** Get the call nodes traversed to reach a context from the program entry. */
std::vector < Node * >GetCallerNodeStack (const Context *);

/** Get the contexts traversed to reach a context from the program entry. */
std::vector < Context * >GetCallerContextStack (const Context *);

/** Get all the contexts that can be reached from a given context. */
std::vector < Context * >GetCalleeContexts (const Context *);

/** Get the string representation of the current context */
string getStringContextRepresentation (const Context * context);

/* Implementation */

/* ContextualNode relational operators implementation. */
bool
operator== (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return (lhs.context == rhs.context) && (lhs.node == rhs.node);
}

bool
operator!= (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return !operator== (lhs, rhs);
}

bool
operator< (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return std::less < Node * >()(lhs.node, rhs.node) || (lhs.node == rhs.node && std::less < Context * >()(lhs.context, rhs.context));
}

bool
operator> (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return operator< (rhs, lhs);
}

bool
operator<= (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return !operator> (lhs, rhs);
}

bool
operator>= (const ContextualNode & lhs, const ContextualNode & rhs)
{
  return !operator< (lhs, rhs);
}



#endif
