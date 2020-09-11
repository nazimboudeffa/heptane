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

/* #includes and forward declarations. */
#include "Attributes.h"
#include <string>
#include <cassert>
#include <iostream>
#include <sstream>
#include "SerialisableAttributes.h"
#include "Handle.h"

/*! this namespace is the global namespace */
namespace cfglib 
{
  /*! Virtual constructor */
  SerialisableIntegerAttribute* SerialisableIntegerAttribute::clone() 
  {   
    return new SerialisableIntegerAttribute(*this) ; 
  }  

  SerialisableIntegerAttribute* SerialisableIntegerAttribute::create()
  {	return new SerialisableIntegerAttribute() ; }

  void SerialisableIntegerAttribute::Print(std::ostream&s) {
    s << "(type=SerialisableInteger,name="<<name<<",value="<<value<<")";
  }

  /*! Serialisation function */
  std::ostream& SerialisableIntegerAttribute::WriteXml( std::ostream& os, Handle& hand) 
  {   
    os  << "<ATTR " 
	<<     "type=\"integer\" " 
	<<     "name=\"" << this->name << "\" "  
	<<     "value=\"" << this->value << "\" "  
	<< "/>" << std::endl ;
    return os ;
  }
  
  void SerialisableIntegerAttribute::ReadXml(XmlTag const* tag, cfglib::Handle&hand) {  
    string attr_name = tag->getAttributeString(std::string("name"));
    assert(attr_name != "");
    this->name = attr_name;
    string val = tag->getAttributeString(std::string("value"));
    this->value = atoi(val.c_str());	
  }

  /*! Virtual constructor */
  SerialisableFloatAttribute* SerialisableFloatAttribute::clone() 
  {   
    return new SerialisableFloatAttribute(*this) ; 
  }  

  SerialisableFloatAttribute* SerialisableFloatAttribute::create()
  {	return new SerialisableFloatAttribute() ; }

  void SerialisableFloatAttribute::Print(std::ostream&s) {
    s << "(type=SerialisableFloat,name="<<name<<",value="<<value<<")";
  }

  /*! Serialisation function */
  std::ostream& SerialisableFloatAttribute::WriteXml( std::ostream& os, Handle& hand) 
  {   
    os  << "<ATTR " 
	<<     "type=\"float\" " 
	<<     "name=\"" << this->name << "\" "  
	<<     "value=\"" << this->value << "\" "  
	<< "/>" << std::endl ;
    return os ;
  }
  
  void SerialisableFloatAttribute::ReadXml(XmlTag const* tag, cfglib::Handle&hand) {  
    string attr_name = tag->getAttributeString(std::string("name"));
    assert(attr_name != "");
    this->name = attr_name;
    string val = tag->getAttributeString(std::string("value"));
    this->value = atof(val.c_str());	
  }

  /*! virtual constructor */
  SerialisableUnsignedLongAttribute* SerialisableUnsignedLongAttribute::clone() 
  {   return new SerialisableUnsignedLongAttribute(*this) ; }

  SerialisableUnsignedLongAttribute* SerialisableUnsignedLongAttribute::create()
  {	return new SerialisableUnsignedLongAttribute() ; }

  /*! Attribute printing function (debug only) */
  void SerialisableUnsignedLongAttribute::Print(std::ostream&s) {
    s << "(type=SerialisableUnsignedLong,name="<<name<<",value="<<value<<")";
  }

  /*! Serialisation function */
  std::ostream& SerialisableUnsignedLongAttribute::WriteXml( std::ostream& os, Handle& hand) 
  {   os  << "<ATTR " 
	  <<     "type=\"unsignedlong\" " 
	  <<     "name=\"" << this->name << "\" " 
	  <<     "value=\"" << this->value << "\" "  
	  << "/>" << std::endl ;
    return os ;
  }

  void SerialisableUnsignedLongAttribute::ReadXml(XmlTag const *tag, cfglib::Handle&hand)
  { 
    string attr_name = tag->getAttributeString(std::string("name"));
    assert(attr_name != "");
    this->name = attr_name;
    string val = tag->getAttributeString(std::string("value"));
    this->value = atol(val.c_str());
  }

  /*! virtual constructor */
  SerialisableStringAttribute* SerialisableStringAttribute::clone() 
  {   return new SerialisableStringAttribute(*this) ; }
 
  SerialisableStringAttribute* SerialisableStringAttribute::create() 
  { return new SerialisableStringAttribute() ;}

  /*! Attribute printing function (debug only) */
  void SerialisableStringAttribute::Print(std::ostream&s) {
    s << "(type=SerialisableString,name="<<name<<",value="<<value<<")";
  }
    
  /*! Serialisation function */
  std::ostream& SerialisableStringAttribute::WriteXml( std::ostream& os, Handle& hand) 
  {   os  << "<ATTR " 
	  <<     "type=\"string\" " 
	  <<     "name=\"" << this->name << "\" " 
	  <<     "value=\"" << this->value << "\" "  
	  << "/>" << std::endl ;
    return os ;
  }

  void SerialisableStringAttribute::ReadXml(XmlTag const* tag, cfglib::Handle&hand)
  {
    string attr_name = tag->getAttributeString(std::string("name"));
    assert(attr_name != "");
    this->name = attr_name;
    this->value = tag->getAttributeString(std::string("value"));
  }

///-----------------------
/*! Virtual constructor */
  SerialisableListAttribute* SerialisableListAttribute::clone() 
  {   
  
    list<SerialisableAttribute*> cloneContents = list<SerialisableAttribute*>();
	for( list<SerialisableAttribute*>::iterator iter=value.begin();iter!=value.end();iter++){
	  SerialisableAttribute* current  = *iter;
	  cloneContents.push_back( (SerialisableAttribute*)current->clone());
	}
    return new SerialisableListAttribute(cloneContents) ; 
  }  

  SerialisableListAttribute* SerialisableListAttribute::create()
  {	return new SerialisableListAttribute() ; }

  void SerialisableListAttribute::Print(std::ostream&s) {
    s << "(type=SerialisableList,name="<<name<<",value:"<< endl;
	for(list<SerialisableAttribute*>::iterator iter=value.begin();iter!=value.end();iter++){
	  SerialisableAttribute* current = *iter;
	  current->Print(s);
	}
	s << endl << ")" << endl;
  }


SerialisableListAttribute* 
SerialisableListAttribute::clone(CloneHandle& handle) { 
	SerialisableListAttribute* clone = new SerialisableListAttribute();

	// Clone each element of the list and insert it into the list clone.
	list<SerialisableAttribute*>::const_iterator element_it;
	for(element_it  = this->value.begin(); 
			element_it != this->value.end();
		  ++element_it) 
	{
		SerialisableAttribute* element = *element_it;
		clone->value.push_back((SerialisableAttribute*)element->clone(handle));
	}

	return clone;
}

  /*! Serialisation function */
  std::ostream& SerialisableListAttribute::WriteXml( std::ostream& os, Handle& hand) 
  {   
    os  << "<ATTR " 
	<<     "type=\"list\" " 
	<<     "name=\"" << this->name << "\" "  << ">" << endl;
	
	list<SerialisableAttribute*>::const_iterator iter;
	for(iter=value.begin();iter!=value.end();iter++){
	  os << "\t";
	  SerialisableAttribute* current = *iter;
	  current->WriteXml(os,hand);
	}
	
	os << "</ATTR>" << std::endl ;
    return os ;
  }
  
  void SerialisableListAttribute::ReadXml(XmlTag const* tag, cfglib::Handle&hand) {  
    string attr_name = tag->getAttributeString(std::string("name"));
    assert(attr_name != "");
    this->name = attr_name;
	
	assert("list" == tag->getAttributeString(std::string("type")));
	
	ListXmlTag children = tag->getAllChildren();
	
	for (unsigned int c=0;c<children.size();c++) {
		XmlTag child = children[c];
		
		assert(child.getName() == std::string("ATTR"));
		
		SerialisableAttribute* current;
		string attr_type = child.getAttributeString(std::string("type"));
		current = AttributesFactory::GetInstance()->CreateNewAttribute(attr_type);

		assert (current != 0);

		if (current) {
			current->ReadXml(&child, hand);
			current->SetName(child.getAttributeString(std::string("name")));
			value.push_back(current);
		}
	}
  }
	
	SerialisableListAttribute::SerialisableListAttribute(SerialisableListAttribute &v){
	  
	  this->name=v.name;
	  this->value=list<SerialisableAttribute*>();
	  for( list<SerialisableAttribute*>::iterator iter=v.value.begin();iter!=v.value.end();iter++){
	    SerialisableAttribute* current  = *iter;
	    this->value.push_back( (SerialisableAttribute*)current->clone());
	  }
	}



///-----------------------

} // cfglib::
