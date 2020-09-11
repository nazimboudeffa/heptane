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

#include "XmlExtra.h"
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

void initXML()
{
	xmlKeepBlanksDefault(1);
	xmlXPathInit();
}

/******************************************************************
XmlDocument is a front-end to the XmlDocPtr of the libxml2 library
Auto alloc/dealloc.
*******************************************************************/

XmlDocument::XmlDocument(string fn)
{
	document=NULL;
	openDocument(fn);
}

int XmlDocument::openDocument(string fn)
{
	fileName= fn;
	document = xmlParseFile(fileName.c_str());
	if (!document) throw string("ERROR WHILE OPENING XML DOCUMENT : "+fn);
	return true;
}

XmlDocument::~XmlDocument()
{
	if (document) xmlFreeDoc(document);
	document=NULL;
}

ListXmlTag XmlDocument::searchChildren(string name)
{
	XmlTag root(document);
	return root.searchChildren(name);
}

XmlTag XmlDocument::getRootTag()
{
	XmlTag root(document);
	return root;
}

/******************************************************************
XmlTag is a front-end to the NodePtr of the libxml2 library
Auto alloc/dealloc.
*******************************************************************/

XmlTag::XmlTag(xmlDocPtr doc,xmlNodePtr ptr) : xmlTag(ptr)
{
  document = doc;
}

XmlTag::XmlTag(xmlDocPtr doc) : document(doc)       
{
  xmlTag=xmlDocGetRootElement(document);
}

XmlTag::~XmlTag()
{
}

// Taking an attribute's int
int XmlTag::getAttributeInt(string AttributeName) const
{
  xmlChar* prop=(xmlChar*)AttributeName.c_str();
  if (xmlTag)
    if (xmlHasProp(xmlTag,prop)) {
      float ret;
      char* buff=(char*)xmlGetProp(xmlTag, prop);
      sscanf(buff,"%f",&ret);
      free(buff);
      return int(ret);
    }
  return 0;
}

// Taking an attribute's double
double XmlTag::getAttributeDouble(string AttributeName) const
{
	xmlChar* prop=(xmlChar*)AttributeName.c_str();
	if (xmlTag)
		if (xmlHasProp(xmlTag,prop)) {
			double ret;
			char*
				buff=(char*)xmlGetProp(xmlTag,
						prop);
			sscanf(buff,"%lf",&ret);
			free(buff);
			return
				ret;
		}
	return
		0;
}



// Taking an attribute's hexa
int XmlTag::getAttributeHexa(string AttributeName) const
{
	xmlChar* prop=(xmlChar*)AttributeName.c_str();
  if (xmlTag)
    if (xmlHasProp(xmlTag,prop)) {
      char* hexa=(char*)xmlGetProp(xmlTag, prop);
      if (!hexa) return 0;
      unsigned int ret;
      sscanf(hexa,"%x",&ret);
      free(hexa);
      return ret;
    }
  return 0;
}

// Taking an attribute's string
string XmlTag::getAttributeString(string AttributeName) const
{
  xmlChar* prop=(xmlChar*)AttributeName.c_str();
  if (xmlTag)
    if (xmlHasProp(xmlTag,prop)) {
      char *str = (char*)xmlGetProp(xmlTag, prop );
      string res(str);
      free(str);
      return res;
    }
  return string("");
}

// Return the contents of a node
string XmlTag::getContent() const
{
  if (xmlTag) {
    char *content = (char*)xmlNodeGetContent(xmlTag);
    string res(content);
    free(content);
    return res;
  }
  return string("");
}

// Get XML tag's name
string XmlTag::getName() const
{
	return string((const char*)xmlTag->name);
}

// searching a list of children Tags from current node, with a specific name.
// This is a basic method to be called by searchChildren
ListXmlTag XmlTag::subSearchChildren(string childrenName) const
{
  xmlNodePtr child=xmlTag;
  ListXmlTag lxt;
  child=child->children;
  while (child!=NULL) {
    if (!xmlStrcmp(child->name,(xmlChar*)childrenName.c_str())) {
      lxt.insert(lxt.end(),XmlTag(document,child));
    }
    child=child->next;
  }
  return lxt;
}

ListXmlTag XmlTag::getAllChildren() const
{
  xmlNodePtr child=xmlTag;
  ListXmlTag lxt;
  child=child->children;
  while (child!=NULL) {
    if (xmlStrcmp(child->name,(xmlChar*)"text")) 
      lxt.insert(lxt.end(),XmlTag(document,child));
    child=child->next;
  }
  return lxt;
}

void XmlTag::print() const
{
  xmlNodePtr child=xmlTag;
  cout << "Name " << this->getName() << endl;
  child=child->children;
  while (child!=NULL) {
    if (xmlStrcmp(child->name,(xmlChar*)"text")) {
      cout << "   Name of child " << child->name << endl;
    }
    child=child->next;
  }
}

ListXmlTag XmlTag::getAllChildrenRecursive()
{
  xmlNodePtr child=xmlTag;
  ListXmlTag lxt;
  child=child->children;
  while (child!=NULL) {
    // We found a child
    if (xmlStrcmp(child->name,(xmlChar*)"text")) {
      // Insert it into the list
      lxt.insert(lxt.end(),XmlTag(document,child));
      // Recursive call
      XmlTag t(document,child);
      ListXmlTag ltx2=t.getAllChildrenRecursive();
      for (unsigned int i=0;i<ltx2.size();i++) 
	lxt.insert(lxt.end(),ltx2[i]);
    }
    child=child->next;
  }
  return lxt;
}


// searching a list of children Tags from current node, with a specific name,
// in the XPath style (/*/*//child/popof.
ListXmlTag XmlTag::searchChildren(string childrenName) const
{
  xmlNodePtr child=xmlTag;
  ListXmlTag lxt;
  lxt.insert(lxt.begin(),(*this));
  unsigned int position=0;
  while (position<childrenName.size()) {
    string localString;
    while ((position<childrenName.size()) &&
	   (childrenName[position]!='/'))
      localString+=childrenName[position++];
    position++;
    //cout << "LocalString --" << localString << "--" << endl;
    if (localString=="") { // Chaine commence par un /
      if (childrenName.size()>=2 && childrenName[1]=='/') { 
	// The name begins by a //
        // Get all children recursively (current node included)
	XmlTag xt(document,child);
	lxt=xt.getAllChildrenRecursive();
	lxt.insert(lxt.begin(),(*this));
      }
      else { 
	// The name begins by a / = get all children of the current node
	XmlTag xt(document,child);
	//cout << "getAllChildren" << endl;
	lxt=xt.getAllChildren();
      }
    }
    else {
      ListXmlTag lxt2,lxt3;
      for (unsigned int i=0;i!=lxt.size();i++) {
	if (localString=="*") {
	  lxt2=lxt[i].getAllChildren();
	}
	else {
	  lxt2=lxt[i].subSearchChildren(localString);
	}
	for (unsigned int j=0;j!=lxt2.size();j++) lxt3.insert(lxt3.end(),lxt2[j]);
      }
      lxt=lxt3;
    }
  }
  return lxt;
}

string indentXML(int i)
{
  string res="";
  for (int n=0;n<i;n++) res += " ";
  return res;
}

string int2string(int i)
{
	char buffer[1000]="";
	sprintf(buffer,"%i",i);
  string ret=string(buffer);
	return ret;
}

string hexa2string(int i)
{
	char buffer[1000]="";
	sprintf(buffer,"%X",i);
	string ret=string(buffer);
  return ret;
}
