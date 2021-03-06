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

/*****************************************************************
                             AddressAttribute
	This is a sharedAttribute generated by the MIPS address analysis

	This attribute is present for all asm instruction to indicate
	the different with at least 1 access to the code instruction.
	It is a serialisable attribute

	It is composed by a class AddressInfo which group the
	information of 1 access to main memory for an instruction
	and
	a class AddressAttributeFactory needed for the serialization process


*****************************************************************/


#include <utility>
#include <string>
#include <vector>
#include "CfgLib.h"

#ifndef ADDRESSINFO
#define ADDRESSINFO

/*!
 * The class AddressInfo is a structure to group the informations
 * of 1 access to memory realized by an instruction
 *
 */
class AddressInfo{

  // a vector of all possible <begin address to memory area acceded ; size of the access in byte> of the access
  vector<pair<string,string> > adrsize;

  // the type of access from/to the memory: read or write
  string type;
  //the name of the variable for static data
  string name;
  //the segment of the access : code, stack, data...
  string segment;

  //true if the access is detected as precise during the Address analysis and false otherwise
  bool precision;
    
  // the real size acceded by the instruction: if the address analysis just
  // find an area (for instance: an array), it's the size acceded by the instruction
  //int sizeOfMemoryAcceded;

 public:

  AddressInfo();

  /*! accessors */
  void setName(string n);
  void setType(string t);
  //void setSizeOfMemoryAcceded(int);
  void setSegment(string seg);
  void setPrecision(bool p);

  vector<pair<string,string> > getAdrSize(); 
  string getType();
  string getName();
  string getSegment();
  bool getPrecision();
  //int getSizeOfMemoryAcceded();

  /*!
   *
   * This function is used to add a pair:
   * <begin address to memory area acceded ; size of the access in byte>
   *
   */
  void addAdrSize(string adr,string size);


  //debug
  void print();

};
#endif

#ifndef ADDRESSATTRIBUTE
#define ADDRESSATTRIBUTE



/*!
 * The Address attribute class
 *
 * This attribute is generate by the MIPSAddressAnalysis
 * and is attached to each asm instruction of a program
 *
 */
class AddressAttribute : public cfglib::SerialisableAttribute {

  /*! set of addressInfo: all the access from/to for an instruction */
  vector<AddressInfo> listInfo;

  /*!accessor only used in the clone function*/
  void setInfo(vector<AddressInfo>);

 public:

  /*!constructor*/
  AddressAttribute();

  /*!accessor*/
  vector<AddressInfo> getListInfo();



  //cloning function
  AddressAttribute *clone();

  // Serialisation function
  ostream& WriteXml(std::ostream&,cfglib::Handle&);
  void ReadXml(XmlTag const*xml_node,cfglib::Handle&) ;

  /*!
   * This function add an object AddressInfo to the listInfo vector of this
   */
  void addInfo(AddressInfo);


  /*!return the code begin address of the code associated to the instruction or -1 if it is not an asm instruction*/
  long getCodeAddress();

  SerialisableAttribute *create() ;


  //debug
  void Print(std::ostream&);

}; 
#endif
