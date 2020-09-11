/* ---------------------------------------------------------------------
   Copyright IRISA, 2003-2017
   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

/* #includes and forward declarations : */
#include "Program.h"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include "Serialisable.h"
#include "Cfg.h"
#include "Handle.h"
#include "CloneHandle.h"
#include "Factory.h"

/* this namespace is the global namespace */
namespace cfglib 
{

  /* constructors. */
  Program::Program(string pgm_name) : entry_point(0), name(pgm_name)
  {}

  Program::Program() : entry_point(0), name("")
  {}

  Program* Program::Clone(){
    CloneHandle handle;
    Program *P = new Program(name);

    handle.RegisterClone (this, P);
    handle.ResolveClone (this->entry_point, (void**)&(P->entry_point));
    //clone cfgs
    for(listOfCfg::iterator it = this->cfgs_list.begin(); it != this->cfgs_list.end(); it ++)
      {
	Cfg* cfgTmp = (*it)->Clone(handle);
	P->cfgs_list.push_back(cfgTmp);
      }
    
    //handle program attributes
    this->CloneAttributesFor (P, handle);
    
    return P;
  }

  /* Destructor for Program. A Program is basically composed by
     Cfgs (stored in cfgs_list). When you destroy a Program you
     just want to destroy the included Cfgs too. There is no
     cross-pointers between Programs. We do not touch to
     entry_point because it is a pointer on a Cfg from the list. */
  Program::~Program()
  {
    for (listOfCfg::iterator it = this->cfgs_list.begin();
	 it != this->cfgs_list.end();
	 it ++)
      {
	delete (*it) ;
      }
  }

  /* Get program name */
  string Program::GetName() const {
    return name;
  }

  /* Set program name */
  void Program::SetName(string pgm_name) {
    name = pgm_name;
  }

  /* Add Cfg (functions) to Program.  The value returned is
   * valid only as long as this Program exists. It must not be
   * used in two different Programs. The first created Cfg is the
   * entry point. */
  Cfg* Program::CreateNewCfg(ListOfString const& name)
  {
    Cfg* cfg = new Cfg(this, name);
    if (NULL == this->entry_point) { this->entry_point = cfg ; }
    this->cfgs_list.push_back(cfg);
    return cfg;
  }

  /* The cgf with entry_point_name as name, becomes the entry point.
      @return true if the entry point exsits, false otherwise.
  */
  bool Program::SetEntryPoint(string entry_point_name)
  {
    this->entry_point = GetCfgByName(entry_point_name);
    return (entry_point != NULL);
  }


  /* Set main() of program. */
  void Program::SetEntryPoint(Cfg* function)
  {
    this->entry_point = function;
  }
  
  /* Get entry point of the program. Can return NULL if the program
   * have no defined entry point. */
  Cfg* Program::GetEntryPoint()
  {
    return this->entry_point;
  }
  
  /* return the list of Cfgs */
  listOfCfg Program::GetAllCfgs() 
  { return this->cfgs_list ; }
  
  /* return the Cfg with a specified name, 0 if it does not exists */
  Cfg* Program::GetCfgByName(std::string name)
  {
    ListOfString lnames;
    for ( listOfCfg::iterator it (this->cfgs_list.begin()) ; it != this->cfgs_list.end() ; ++it )
      {
	// if ((*it)->GetName() == name) { return *it ; }
	lnames = (*it)->GetName();
	ListOfString::iterator itnames;
	for (itnames = lnames.begin(); itnames != lnames.end(); itnames++)
	  {	
	    if ((*itnames) == name) { return *it ; }
	  }
      }
    return 0 ;
  }

  /* Return the Cfg with a specific number in the list of Cfgs
    (starting by index 0) or 0 if the index does not exist */
  Cfg* Program::GetCfgByNum(int index) {
    if ((unsigned int)index>=this->cfgs_list.size()) return 0;
    else return this->cfgs_list[index];
  }

  /* Serialisation function */
  std::ostream& Program::WriteXml( std::ostream& os, Handle& hand)
  {
    os << "<!DOCTYPE PROGRAM SYSTEM 'cfglib.dtd'>" << std::endl;
    os << "<PROGRAM "
       << "  id =\"" << hand.identify(this) << "\" "
       << "  name=\"" << this->name << "\" "
       << "  entry=\"" << hand.identify(this->entry_point) << "\" "
       << ">" << std::endl ;
    int i = 0 ;
    for (listOfCfg::const_iterator it = this->cfgs_list.begin();
	 it != this->cfgs_list.end();
	 it++)
      {
	std::ostringstream id ;
	id << hand.identify(*it) ;
	(*it)->WriteXml(os, hand);
	i++;
      }
    
    this->WriteXmlAttributes(os, hand) ;
    
    os << "</PROGRAM>" << std::endl ;
    return os ;
  }
  
  /* Deserialisation function. */
  void Program::ReadXml(XmlTag const* tag, Handle& h)
  {
    assert(tag->getName()==string("PROGRAM"));
    this->name = tag->getAttributeString("name");
    assert(this->name!="");
    string entry_point_number = tag->getAttributeString("entry");
    this->entry_point = NULL;
    hand.addID_handle(entry_point_number,(Serialisable **)&(this->entry_point));
    ListXmlTag children = tag->getAllChildren();
    
    for (unsigned int c=0;c<children.size();c++) {
      XmlTag child = children[c];
      if (child.getName()==string("ATTRS_LIST")) continue;
      assert(child.getName()==string("CFG"));
      string name = child.getAttributeString("name");
      assert(name!="");
      istringstream lnames(name);
      string aname;
     ListOfString names;
     while (!lnames.eof())
	{
	  getline(lnames, aname,' ');
	  names.push_front(aname);
	}
      
     Cfg* cfg = this->CreateNewCfg(names);
     cfg->ReadXml(&child,h);
    }
    this->ReadXmlAttributes(tag, h) ;
    h.resolveHandles();
  }

  Program *Program::unserialise_program_file(std::string const& file_name) {
    Program *prog_deserialise = new Program();
    XmlDocument docu(file_name);
    XmlTag root = docu.getRootTag();
    prog_deserialise->ReadXml(&root,prog_deserialise->hand);
    return prog_deserialise;
  }
  
  std::ostream& Program::serialise_program(std::ostream& os) 
  {
    this->WriteXml(os,this->hand);
    return os ;
  }
  
  void Program::serialise_program(std::string& file_name) 
  {
    std::ofstream output_file_stream ;
    output_file_stream.open(file_name.c_str()) ;
    this->serialise_program(output_file_stream) ;
  }

} // cfglib::
