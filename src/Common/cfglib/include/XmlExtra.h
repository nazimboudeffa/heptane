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

#ifndef XML__H
#define XML__H

#include <iostream>
#include <libxml/tree.h>
#include <list>
#include <string>
#include <vector>

using namespace std;

/* --------------------------------------------------

	Useful routines to manipulate XML documents

  --------------------------------------------------- */

/* -------------------------------------------
ParseXmlFile :

   parse a file from its file name 
   return the xmlDocNode if there's no error
   exit if an error occurs
---------------------------------------------- */
xmlDocPtr ParseXmlFile(const string &xml_file);

/* --------------------------------------------------------
GoodNode : return true if the node does exist and if
           its name is value 
---------------------------------------------------------- */
int GoodNode(const xmlNodePtr &node ,const string &value);

/*  ----------------------------------------------------
   GetXmlParam:
   return the char * value of the parameter or exit if it
   doesn't exist 
------------------------------------------------------- */
string GetXmlParam(const xmlNodePtr &node, const string &param_name);

/* ------------------------------------------------------
getNextXmlElt: get the next sibling element 
------------------------------------------------------- */
xmlNodePtr getNextXmlElt(const xmlNodePtr &node);

/* ------------------------------------------------------ */
/* getFirtsChildXmlElt: get the first child element */
/* ------------------------------------------------------ */
xmlNodePtr getFirstChildXmlElt(const xmlNodePtr &node);

/* ------------------------------------------------------ */
/* getChild by name */
/* ------------------------------------------------------ */
xmlNodePtr getChildNodeByName(xmlNodePtr parent, const char * name);

typedef vector<string> ListString;

class XmlTag;

typedef vector<XmlTag> ListXmlTag;

typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

/** This is a generic pointer to an XML tag. provides an encapsulation for 
all libxml2 xlmTag methods */
/* ------------------------------------------------------------------------ */
class XmlTag
{
  /** pointer */
  xmlNodePtr xmlTag;
  /** XPath used to access it */
  string currentPath;
  /** A pointer to the libxml2 document */
  xmlDocPtr document;

  /** Get all children whose name match the XPath-like query */
  ListXmlTag subSearchChildren(string childrenName) const;

  /* Get all children tags of this tag, recursively */
  ListXmlTag getAllChildrenRecursive();

public:

  /** XPath-style initialization */
  XmlTag(xmlDocPtr,xmlNodePtr ptr);
  /** Init at root document */
  XmlTag(xmlDocPtr);
  /** Desallocate the good arg */
  ~XmlTag();

  /** Taking an attribute's of type int, acording to its name as parameter */
  int getAttributeInt(string AttributeName) const;

	/** Taking an attribute's of type double, acording to its name as parameter * */
	double getAttributeDouble(string AttributeName) const;

  /** Taking an attribute's of type hexadecimal, acording to its name as parameter */
  int getAttributeHexa(string AttributeName) const;

  /** Taking an attribute's of type string, acording to its name as parameter */
  string getAttributeString(string AttributeName) const;

  /** Return the XmlTag contents */
  string getContent() const;

  /** searching a list of Tags from current node, with a specific name (very simple XPath -like syntax). */
  ListXmlTag searchChildren(string childrenName) const;

  /** Get all children tags of this tag */
  ListXmlTag getAllChildren() const;

  /** Get back the XML tag's name
  (i.e, what is it exactly in the tag : <TAG parm="dd"/> will return "TAG" */
  string getName() const;
  
  /** Debug only */
  void print() const;
};

/** An XML document to handle: opening, closing, get the first children tags */
class XmlDocument
{
  /** Pointer to real libxml2 document */
  xmlDocPtr document;
  /** The name of the file used to open it*/
  string fileName;
  
  public:

  /** opening a file, whose name is given */
  XmlDocument(string);
  /** Real opening method */
  int openDocument(string);
  /** Close document. */
  ~XmlDocument();

  /** search a list of tag, whose name is given (very simple XPath -like syntax) */
  ListXmlTag searchChildren(string childrenName);
  
  /** get root tag */
  XmlTag getRootTag();
};

/** Initialization of libxml2. You must call this before any XmlDocument request */
void initXML();
/** Returns a string with "i" spaces */
string indentXML(int i);
/** Tranformst an int into a string */
string int2string(int i);
/** transform an int into a string, formatted as an hexadecimal */
string hexa2string(int i);

#endif
