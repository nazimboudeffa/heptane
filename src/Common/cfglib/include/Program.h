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

#ifndef _IRISA_CFGLIB_PROGRAM_H
#define _IRISA_CFGLIB_PROGRAM_H

/* #includes and forward declarations : */
#include <string>
#include <vector>
#include "Attributed.h"
#include "Factory.h"
#include "Handle.h"
#include "HeptaneStdTypes.h"

/** this namespace is the global namespace */
namespace cfglib 
{
  /* #includes and forward declarations : */
  class Cfg ; // cf. Cfg.h
  class AttributesFactory ; // cf. Factory.h
  typedef std::vector<Cfg*> listOfCfg;


  /** This class represents a program. A program is a set of
      functions each with a corresponding CFG. A program has one
      entry point. */
  class Program : public Attributed {
  private:
    listOfCfg cfgs_list ;
    Cfg* entry_point;
    string name;
  public:

    Handle hand; // Memory of id-pointer mapping for all objects of this program

   /** constructor */
    Program(string pgm_name);
    Program();

    /** clone the program */
    Program* Clone();

    /** destructor */
    ~Program();

    /** Get program name */
    string GetName() const;

    /** Set program name */
    void SetName(string pgm_name);

    /** Add Cfg (functions) to Program.  The value returned is
       valid only as long as this Program exists. It must not
       be used in two diferent Programs. The first created Cfg
       is the entry point.
    */
    Cfg* CreateNewCfg(ListOfString const &lnames);

    /** The cgf with entry_point_name as name, becomes the entry point of the program.
	@return true if the entry point exsits, false otherwise. */
    bool SetEntryPoint(string entry_point_name);

    /** Set the entry point of this program. The Cfg passed in argument must be a Cfg of this Program. */
    void SetEntryPoint(Cfg* function);

    /** Get entry point of the program */
    Cfg* GetEntryPoint();

    /** get the list of Cfgs of the program. */
    listOfCfg GetAllCfgs() ;

    /* Return the number of Cfgs */
    int GetNbCfgs() { return cfgs_list.size();}

    /** return the Cfg with a specified name, 0 if it does not exists */
    Cfg* GetCfgByName(std::string name) ;

    /** Return the Cfg with a specific number in the list of Cfgs
      (starting by index 0) or 0 if the index does not exist */
    Cfg* GetCfgByNum(int index);

    /** Serialisation function. */
    virtual std::ostream& WriteXml(std::ostream& os, Handle& hand);

    /** Internal seserialisation function. */
    virtual void ReadXml(XmlTag const* tag, Handle& hand);
     
    /** Deserialisation function. This function is the one
	really meant for user usage. ReadXml should not be
	used. cf. unserialise_program for precision on
	arguments. */
    static Program *unserialise_program_file(std::string const& file_name) ;
    
    /** Serialisation function. */
    std::ostream& serialise_program(std::ostream& os) ;
    
    /** Serialisation to file function */
    void serialise_program(std::string& file_name) ;

  } ;

} // cfglib::
#endif // _IRISA_CFGLIB_PROGRAM_H
