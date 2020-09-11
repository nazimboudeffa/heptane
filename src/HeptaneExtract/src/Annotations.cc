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

#include <assert.h>
#include <stdint.h>
#include <fstream>
#include "Annotations.h"
#include "GlobalAttributes.h"
#include "LoopTree.h"

using namespace std;
using namespace cfglib;

static bool opt_verbose= false;

/**
   Change endianness of a 32-bits word.
*/
static uint32_t
ChangeEndianness (uint32_t word)
{
  register uint32_t result;
  result = (word >> 24) & 0x000000ff;
  result |= (word >> 8) & 0x0000ff00;
  result |= (word << 8) & 0x00ff0000;
  result |= (word << 24) & 0xff000000;
  return result;
}

/**
   Utility function to get first dans last code addresses in Cfg nodes.
   NB: Addresses should have been attached to instructions before.
*/
static void
GetFirstLastAddresses (cfglib::Node * n, t_address * first, t_address * last)
{
  vector < cfglib::Instruction * >vi = n->GetAsm ();
  assert (vi[0]->HasAttribute (AddressAttributeNameExtract));
  assert (vi[vi.size () - 1]->HasAttribute (AddressAttributeNameExtract));
  AddressAttribute attr_first = (AddressAttribute &) vi[0]->GetAttribute (AddressAttributeNameExtract);
  *first = attr_first.getCodeAddress ();
  AddressAttribute attr_last = (AddressAttribute &) vi[vi.size () - 1]->GetAttribute (AddressAttributeNameExtract);
  *last = attr_last.getCodeAddress ();
}


/**
   Add annotations in a CFG from an annotation file, for a specific loop
   If the loop already has an annotation, emit a warning and go on
*/
static void
addAnnotationsFromFileToLoop (LoopTreeNode * tree_node_cfg, XmlTag & tree_node_xml)
{

  // Check the loop does not already has a maxiter attached
  cfglib::Loop * loop = tree_node_cfg->getLoop ();
  if (loop->HasAttribute (MaxiterAttributeName))
    {
      // cerr << "*** WARNING: loop in CFG " << tree_node_cfg->getCfg ()->GetName () << " already has a maxiter attached" << endl; replaced by (LBesnard)
      cerr << "*** WARNING: loop in CFG " << tree_node_cfg->getCfg ()->getStringName () << " already has a maxiter attached" << endl;
      cerr << "Going on ..." << endl;
    }

  // Get the maxiter from the XML file and attach it to the loop
  int bound = tree_node_xml.getAttributeInt (MaxiterAttributeName);
  if (bound == 0)
    {
      Logger::addWarning ("addAnnotationsFromFileToLoop: strange maxiter (0)\n Going on ...\n");
    }
  else
    {
      cout << "Annotation found " << bound << endl;
    }

  cfglib::SerialisableIntegerAttribute ii (bound);
  loop->SetAttribute (MaxiterAttributeName, ii);

  // Recursive call on subloops
  unsigned nb_subloops_cfg = tree_node_cfg->getNbSubloops ();
  ListXmlTag subloops_xml = tree_node_xml.searchChildren ("LOOP");
  if (nb_subloops_cfg != subloops_xml.size ())
    {
      Logger::addFatal ("addAnnotationsFromFileToLoop: mismatch between number of loops in CFG and XML annotation in cfg ");
    }
  for (unsigned int l = 0; l < nb_subloops_cfg; l++)
    {
      addAnnotationsFromFileToLoop (tree_node_cfg->getSubloop (l), subloops_xml[l]);
    }
}


static void
GenerateAnnotationXMLForLoop (LoopTreeNode * n, ofstream & os, int indent)
{

  // Generate the maxiter information for the loop itself
  cfglib::Loop * l = n->getLoop ();
  if (l->HasAttribute (MaxiterAttributeName))
    {
      cfglib::SerialisableIntegerAttribute ii = (cfglib::SerialisableIntegerAttribute &) l->GetAttribute (MaxiterAttributeName);
      for (int i = 0; i < indent; i++)
	os << " ";
      os << "<LOOP maxiter=\"" << ii.GetValue () << "\">" << endl;
      // Generate the information for subloops
      unsigned nl = n->getNbSubloops ();
      for (unsigned int i = 0; i < nl; i++)
	{
	  GenerateAnnotationXMLForLoop (n->getSubloop (i), os, indent + 2);
	}
      for (int i = 0; i < indent; i++)
	os << " ";
      os << "</LOOP>" << endl;
    }
}

/*
  Check all loops have a maxiter.
  In the case a loop is not annotated, just emit a warning and go on.
*/
static void loopVerifications(vector < cfglib::Cfg * > &lc)
{
  for (unsigned int c = 0; c < lc.size (); c++)
    {
      vector < cfglib::Loop * >vl = lc[c]->GetAllLoops ();
      // cout << " loopVerifications for CFG : " << lc[c]->getStringName () << " nbloops = " << vl.size () << endl;
      for (unsigned int l = 0; l < vl.size (); l++)
	{
	  if (! vl[l]->HasAttribute (MaxiterAttributeName))
	    {
	      cout << "Warning: a loop does not have the " << MaxiterAttributeName << " attribute set in CFG " << lc[c]->getStringName () << endl;
	    }
	}
    }
}

void
AttachAnnotationsFromXML (cfglib::Program & cfglib_program, string annotfilename, bool verbose)
{
  opt_verbose = verbose;
  if (opt_verbose) cout << "AttachAnnotationsFromXML" << endl;

  // Open and parse the annotation file
  ifstream f;
  f.open (annotfilename.c_str ());
  if (f.fail ())
    {
      Logger::addFatal ("AttachAnnotationsFromXML: unable to open annotation file " + annotfilename);
    }
  f.close ();

  XmlDocument xmldoc (annotfilename);

  // cout << "File name " << annotfilename << endl;

  // Find all CFGs in XML and program
  ListXmlTag lt = xmldoc.searchChildren ("CFG");

  // Scan loop trees from the program structure
  LoopTree looptree (cfglib_program);
  unsigned int nbtrees = looptree.getNbTrees ();
  for (unsigned int t = 0; t < nbtrees; t++)
    {
      LoopTreeNode *n = looptree.getLoopTree (t);
      // string cfg_name =  n->getCfg ()->GetName ();  replaced by (LBesnard) , to be completed
      ListOfString lnames = n->getCfg ()->GetName (); 
      string cfg_name = lnames.front();
      if (cfg_name == "_unknown_") continue; // ignore indirect function calls

      // Find annotations regarding cfg of name cfg_name in the XML annotation file
      bool found = false;
      unsigned int tag;
      for (tag = 0; tag < lt.size (); tag++)
	{
	  string cfg_name_xml = lt[tag].getAttributeString ("name");
	  if (cfg_name_xml == cfg_name)
	    {
	      found = true;
	      break;
	    }
	}
      if (!found)
	{
	  Logger::addWarning ("AttachAnnotationsFromXML: no annotation found for cfg " + cfg_name + " in file " + annotfilename);
	  continue;
	}

      // Match subloops in annotations XML with subloops in CFG
      unsigned nb_subloops_cfg = n->getNbSubloops ();
      ListXmlTag subloops_xml = lt[tag].searchChildren ("LOOP");
      if (nb_subloops_cfg != subloops_xml.size ())
	{
	  Logger::addFatal ("AttachAnnotationsFromXML: mismatch between number of loops in CFG and XML annotation file in cfg " + cfg_name);
	}
      for (unsigned int l = 0; l < nb_subloops_cfg; l++)
	{
	  addAnnotationsFromFileToLoop (n->getSubloop (l), subloops_xml[l]);
	}
    }
  vector < cfglib::Cfg * >lc = cfglib_program.GetAllCfgs ();
  loopVerifications(lc);
}


/*
  Generate annotation XML from CFG
*/
void
GenerateAnnotationXMLFile (cfglib::Program & p, string annotfilename, bool verbose)
{
  opt_verbose = verbose;

  ofstream os;
  os.open (annotfilename.c_str ());
  if (os.fail ())
    {
      cerr << "*** Error: unable to open annotation file " << annotfilename << endl;
    }
  LoopTree lt (p);
  unsigned int nbtrees = lt.getNbTrees ();

  os << "<CONFIGURATION> " << endl;
  for (unsigned int t = 0; t < nbtrees; t++)
    {
      LoopTreeNode *n = lt.getLoopTree (t);
      if (n->getCfg ()->IsExternal () == false)
	{			// For all CFGs except external ones

	  os << "<CFG name=\"" << n->getCfg ()->getStringName() << "\">" << std::endl;

	  // Generate annotations for loops (if any)
	  unsigned nl = n->getNbSubloops ();
	  for (unsigned int l = 0; l < nl; l++)
	    {
	      GenerateAnnotationXMLForLoop (n->getSubloop (l), os, 2);
	    }
	  os << "</CFG>" << std::endl;
	}
    }
  os << "</CONFIGURATION> " << endl;
  os.close ();
}

/*
    Get annotations from the binary
    Parameters:
    - Program
    - File containing the dump of the annotation section, obtained previously
    using objdump -j
    NB: This function might be sensitive to the format of objdump output
*/
void
AttachAnnotationFromBinary (cfglib::Program & cfglib_program, string annot_section_dump_file, bool verbose)
{
 opt_verbose = verbose;
 // Read the annotations from the objdump of the annotations section 
  // (raw format, only read list of long integers)
  // Assumes the following obdump output format:
  // Address data data data data ascii representation of contents
  vector < unsigned long >raw_annots;
  FILE *f = fopen (annot_section_dump_file.c_str (), "r");
  bool started = false;
  unsigned int nb_vals_total = 0;
  char mark[STRMAX];

  bool bigIndian = Arch::isBigEndian ();
  snprintf (mark, STRMAX, "%s:", ANNOT_SECTION_NAME);
  while (1)
    {
      char buf[STRMAX];
      int nb_vals;
      char address[STRMAX], v1[STRMAX], v2[STRMAX], v3[STRMAX], v4[STRMAX], ascii[STRMAX];
      if (fgets (buf, STRMAX, f) == NULL)
	break;
      nb_vals = sscanf (buf, "%s %s %s %s %s %s", address, v1, v2, v3, v4, ascii);
      if (nb_vals >= 4 && strcmp (v3, mark) == 0)
	{
	  started = true;
	  continue;
	}
      if (started)
	{
	  nb_vals_total += nb_vals - 2;
	  // lbesnard: strtol replaced by strtoll, for 64-bits archi.
	  if (!bigIndian)
	    {
	      raw_annots.push_back (ChangeEndianness (strtoll (v1, (char **) NULL, 16)));
	      raw_annots.push_back (ChangeEndianness (strtoll (v2, (char **) NULL, 16)));
	      raw_annots.push_back (ChangeEndianness (strtoll (v3, (char **) NULL, 16)));
	      raw_annots.push_back (ChangeEndianness (strtoll (v4, (char **) NULL, 16)));
	    }
	  else
	    {
	      raw_annots.push_back (strtoll (v1, (char **) NULL, 16));
	      raw_annots.push_back (strtoll (v2, (char **) NULL, 16));
	      raw_annots.push_back (strtoll (v3, (char **) NULL, 16));
	      raw_annots.push_back (strtoll (v4, (char **) NULL, 16));
	    }
	}
    }
  fclose (f);
  /* --- trace lbesnard
  for (size_t i = 0; i <  nb_vals_total;)
  { cout << "[ " << raw_annots[i++] << "," << raw_annots[i++] << "," << raw_annots[i++] << "]" << endl; }
  */

  // Fill-in the vector of annotations (vector of structures <address,type,values>)
  vector < t_annotation > annots;
  for (unsigned int i = 0; i < nb_vals_total;)
    {
      t_annotation a;
      a.address = raw_annots[i++];
      a.type = raw_annots[i++];
      switch (a.type)
	{
	case LOOP_MAXITER:
	  a.values.push_back (raw_annots[i++]);
	  // cout << "Maxiter found xxxxx = " << raw_annots[i-1] << endl;
	  break;
	default:
	  cerr << "Unknown annotation type found in binary " << a.type << endl;
	  cerr << "Skipping... " << endl;
	}
      a.node = NULL;
      annots.push_back (a);
    }
  unsigned int nb_annots = annots.size ();

  // Associate a CFG Node*
  // to every annotation (useful for attachement of annotations to loops/nodes)
  // Done by scanning the cfgs and then their BBs
  vector < cfglib::Cfg * >lc = cfglib_program.GetAllCfgs ();
  for (unsigned int c = 0; c < lc.size (); c++)
    {
      vector < cfglib::Node * >vn = lc[c]->GetAllNodes ();
      for (unsigned int nn = 0; nn < vn.size (); nn++)
	{
	  cfglib::Node * n = vn[nn];	// pointer to the node
	  t_address first_address;
	  t_address last_address;
	  GetFirstLastAddresses (n, &first_address, &last_address);
	  // Search if one of the annotation is in the address range of the BB
	  for (unsigned int i = 0; i < nb_annots; i++)
	    {
	      if (annots[i].address >= first_address && annots[i].address <= last_address)
		{
		  annots[i].node = n;
		}
	    }
	}
    }
  /*  for (unsigned int ii = 0; ii < nb_annots; ii++)
    {
      cout << "1 = " << annots[ii].address << endl;
      cout << "2 = " << annots[ii].type << endl;
      cout << "3 = " << annots[ii].node << endl;
      } */

  // Attach the annotations to the cfg nodes and loops depending on the type of annotation
  for (unsigned int i = 0; i < nb_annots; i++)
    {
      assert (annots[i].node != NULL);

      // Loop annotations (more tricky)
      if (annots[i].type == LOOP_MAXITER)
	{
	  cfglib::Node * n = annots[i].node;
	  cfglib::Cfg * c = n->GetCfg ();
	  // cout << "loop setting in CFG = " << c ->getStringName () << endl;
	  vector < cfglib::Loop * >ll = c->GetAllLoops ();
	  // cout << " nb_loops = " << ll.size () << endl;
	  for (unsigned int nl = 0; nl < ll.size (); nl++)
	    { // For all loops in the CFG
	      cfglib::Loop * myloop = ll[nl];
	      if (myloop->FindInLoop (n))
		{
		  cfglib::Loop * inner = myloop;
		  // unsigned int i_inner = nl; 
		  for (unsigned int ol = 0; ol < ll.size (); ol++)
		    {
		      cfglib::Loop * otherloop = ll[ol];
		      if (otherloop != myloop)
			{
			  if (otherloop->FindInLoop (n) && otherloop->IsNestedIn (inner))
			    {
			      inner = otherloop;
			      // i_inner = ol;
			    }
			}
		    }
		  if (! inner->HasAttribute (MaxiterAttributeName))
		    {
		      cfglib::SerialisableIntegerAttribute attr_bound (annots[i].values[0]);
		      inner->SetAttribute (MaxiterAttributeName, attr_bound);
		      // cout << "SetAttribute (MaxiterAttributeName, index loop= " << i_inner << " maxiter = " << annots[i].values[0] << ")" << endl;
		    }
		  else
		    {
		      cfglib::SerialisableIntegerAttribute ii = (cfglib::SerialisableIntegerAttribute &) inner->GetAttribute (MaxiterAttributeName);
		      if (ii.GetValue () != annots[i].values[0])
			{
			  cerr << "The loop already has a loop bound " << ii.GetValue () << endl;
			  cerr << "Erasing with the new loop bound " << annots[i].values[0] << endl;
			  cfglib::SerialisableIntegerAttribute attr_bound (annots[i].values[0]);
			  inner->SetAttribute (MaxiterAttributeName, attr_bound);
			}
		    }
		}
	    }
	}
    }
  loopVerifications(lc);
}

static void addAddress( char address[STRMAX], vector < t_address> &laddr, bool bigIndian)
{
  t_address addr = strtoll (address, (char **) NULL, 16);
  if (!bigIndian) addr = ChangeEndianness (addr);
  laddr.push_back (addr);
}


vector < t_address > getSwitchInfos(string switch_section_dump_file)
{
  vector < t_address> laddr;

  FILE *f = fopen (switch_section_dump_file.c_str (), "r");
  bool started = false;
  char mark[STRMAX];
  char buf[STRMAX];
  char address[STRMAX], v1[STRMAX], v2[STRMAX], v3[STRMAX], v4[STRMAX], ascii[STRMAX];
  int nb_vals;

  bool bigIndian = Arch::isBigEndian ();
  snprintf (mark, STRMAX, "%s:", ANNOT_SWITCH_BEGIN);
  while (1) 
    {

      if (fgets (buf, STRMAX, f) == NULL)
	break;
      nb_vals = sscanf (buf, "%s %s %s %s %s %s", address, v1, v2, v3, v4, ascii);
      if (nb_vals >= 4 && strcmp (v3, mark) == 0)
	{
	  started = true;
	  continue;
	}
      if (started)
	{
	  // first and last are not "switch address" values.
	  nb_vals = nb_vals - 2;
	  if (nb_vals >= 1) addAddress(v1, laddr, bigIndian);
	  if (nb_vals >= 2) addAddress(v2, laddr, bigIndian);
	  if (nb_vals >= 3) addAddress(v3, laddr, bigIndian);
	  if (nb_vals == 4) addAddress(v4, laddr, bigIndian);
	}
    }
  fclose (f);
  return laddr;
}



