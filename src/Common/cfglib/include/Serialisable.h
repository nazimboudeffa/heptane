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

#ifndef _IRISA_CFGLIB_SERIALISABLE_H
#define _IRISA_CFGLIB_SERIALISABLE_H
/* #includes and forward declarations */
#include <string>
#include "XmlExtra.h"

/*! this namespace is the global namespace */
namespace cfglib 
{
  /* #includes and forward declarations */
  class Handle; 

  class Serialisable {
  private:
  public:
    /*! Serialisation function. */
    virtual std::ostream& WriteXml(std::ostream& os, Handle& hand) = 0;

    /*! Deserialisation function.
     *
     * The deserialisation is in two phase : first the
     * creation of the object (in several ways depending
     * on the object) and second call to O->ReadXml()
     * which initialize the object with correct values. */
    virtual void ReadXml( XmlTag const* tag, Handle& hand)       = 0;
    /*! destructor */
    virtual ~Serialisable(){};
  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_SERIALISABLE_H
