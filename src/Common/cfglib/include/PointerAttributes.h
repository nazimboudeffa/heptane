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

#ifndef _IRISA_CFGLIB_PTR_ATTRIBUTES_H
#define _IRISA_CFGLIB_PTR_ATTRIBUTES_H

#include <cassert>

#include "Attributes.h"
#include "CloneHandle.h"
#include "Handle.h"
#include "SerialisableAttributes.h"
#include "XmlExtra.h"

#define REGISTER_POINTER_ATTRIBUTE(TYPE, ATTR_NAME) SetAttributeType(#TYPE, new ATTR_NAME())

#define DECLARE_POINTER_ATTRIBUTE(TYPE, ATTR_NAME)	\
  class TYPE;						\
  class ATTR_NAME : public SerialisableAttribute {	\
  private:						\
    TYPE** ptr;						\
  public:						\
    ATTR_NAME();					\
    ATTR_NAME(TYPE* ptr);				\
    ~ATTR_NAME();					\
							\
    ATTR_NAME* clone();					\
    ATTR_NAME* clone(CloneHandle&);			\
							\
    void Print(std::ostream&);				\
							\
    std::ostream& WriteXml(std::ostream&,Handle&);	\
							\
    void ReadXml(XmlTag const*, Handle&);		\
							\
    TYPE* GetValue() const;				\
							\
    void SetValue(TYPE* ptr);				\
							\
    ATTR_NAME* create();				\
  }

#define IMPLEMENT_POINTER_ATTRIBUTE(TYPE, ATTR_NAME)			\
  namespace cfglib{							\
    ATTR_NAME::ATTR_NAME() : ptr (new TYPE*(0)) {}			\
    ATTR_NAME::ATTR_NAME(TYPE* ptr): ptr (new TYPE*(ptr)) {}		\
    ATTR_NAME::~ATTR_NAME() {delete ptr;}				\
    ATTR_NAME* ATTR_NAME::clone() {					\
      ATTR_NAME* clone =						\
	new ATTR_NAME(*(this->ptr));					\
      swap (clone->ptr, this->ptr);					\
      return clone;							\
    }									\
    ATTR_NAME* ATTR_NAME::clone(CloneHandle& handle) {			\
      ATTR_NAME* clone =						\
	new ATTR_NAME();						\
      handle.ResolveClone (this->GetValue(), (void**) clone->ptr);	\
      return clone;							\
    }									\
    void ATTR_NAME::Print(std::ostream& os) {				\
      os << "(" << #ATTR_NAME << ", " << *ptr << ")" << std::endl;	\
    }									\
    std::ostream& ATTR_NAME::WriteXml(std::ostream& os,			\
				      Handle& handle) 	\
      {									\
	assert (*ptr);							\
	os << "<ATTR type=\"" << #TYPE << "\" name=\"" << this->name << "\" "; \
	os << "ptr_id=\"" << handle.identify ((Serialisable*)*(ptr)) << "\" />" << std::endl; \
	return os;							\
      }									\
    void ATTR_NAME::ReadXml(XmlTag const* tag, Handle& handle) {	\
      assert(ptr);							\
      string name = tag->getAttributeString(std::string("name"));	\
      this->name = name;						\
									\
      string nid = tag->getAttributeString(std::string("ptr_id"));	\
      handle.addID_handle (nid, (Serialisable**)ptr);			\
    }									\
    TYPE* ATTR_NAME::GetValue() const					\
      {									\
	assert (ptr);							\
	if (*ptr) return *ptr;						\
	else return NULL;						\
      }									\
    void ATTR_NAME::SetValue(TYPE* ptr)					\
    {									\
      *(this->ptr) = ptr;						\
    }									\
    ATTR_NAME* ATTR_NAME::create()					\
      {									\
	return new ATTR_NAME();						\
      }									\
  }

namespace cfglib
{

  DECLARE_POINTER_ATTRIBUTE(Cfg, SerialisableCfgAttribute);
  DECLARE_POINTER_ATTRIBUTE(Node, SerialisableNodeAttribute);
  DECLARE_POINTER_ATTRIBUTE(Instruction, SerialisableInstructionAttribute);
  DECLARE_POINTER_ATTRIBUTE(Loop, SerialisableLoopAttribute);

}

#endif
