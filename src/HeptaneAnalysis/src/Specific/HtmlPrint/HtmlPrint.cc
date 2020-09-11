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

#include "HtmlPrint.h"


HtmlPrint::HtmlPrint (Program * p, string f, bool colorize):Analysis (p)
{
  HtmlFileName = f;
  color = colorize;
  cg = new CallGraph (p);
}

HtmlPrint::~HtmlPrint (void)
{
  delete cg;
}


// -------------------------------------------------
// Checks every instruction in the node
// contains CodeLine attribute
// -------------------------------------------------
bool
HtmlPrint::CheckInstrHaveCodeLine (Cfg * c, Node * n, void *param)
{
  vector < Instruction * >vi = n->GetAsm ();
  for (unsigned int i = 0; i < vi.size (); i++)
    {
      if (vi[i]->HasAttribute (CodeLineAttributeName) == false)
	return false;
    }
  return true;
}


// -------------------------------------------------
// Checks that every node
// contains Frequency attribute
// -------------------------------------------------
bool
HtmlPrint::CheckNodeHaveFrequency (Cfg * c, Node * n, void *param)
{
  // The previous code 
  /* // Check only for context 0 (minimim context available in both context-dependent and context-independent IPET)
     string AttName = AnalysisHelper::getContextAttrFrequencyName(0);
     return n->HasAttribute (AttName);
  */
  // replaced by the code of CheckFrequencyAttribute of the CacheStatistics analysis.
  string contextName;

  const ContextList & contexts = (ContextList &) c->GetAttribute (ContextListAttributeName);
  unsigned int nc = contexts.size ();
  unsigned int nb_ctx_found = 0;
  for (unsigned int ic = 0; ic < nc; ic++)
    {
      contextName = contexts[ic]->getStringId();
      if (n->HasAttribute (AnalysisHelper::getContextAttrFrequencyName(contextName)))
	nb_ctx_found++;
    }
  if (nb_ctx_found != 1 && nb_ctx_found != nc) return false;
  return true;
}


// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
HtmlPrint::CheckInputAttributes ()
{
  vector < Cfg * >lcfg = p->GetAllCfgs ();

  // Check instructions have CodeLine attribute
  for (unsigned int i = 0; i < lcfg.size (); i++)
    {
      if (AnalysisHelper::applyToAllCfgNodes (lcfg[i], CheckInstrHaveCodeLine, NULL) == false)
	{
	  stringstream errorstr;
	  errorstr << "HtmlPrint: Instruction should have the " << CodeLineAttributeName << " attribute set, sorry ... ";
	  Logger::addFatal (errorstr.str ());
	  return false;
	}

      if (cg->isDeadCode (lcfg[i]) == false)
	{
	  if (AnalysisHelper::applyToAllCfgNodes (lcfg[i], CheckNodeHaveFrequency, NULL) == false)
	    {
	      stringstream errorstr;
	      errorstr << "HtmlPrint: Nodes should have the " << FrequencyAttributeName << " attribute set for all call contexts, sorry ... ";
	      Logger::addFatal (errorstr.str ());
	      return false;
	    }
	}
    }

  return true;
}


vector < string > HtmlPrint::getFiles (void)
{
  vector < string > fileList;
  string tmp;
  bool present;

  for (unsigned int i = 0; i < lf.size (); i++)
    {
      tmp = lf[i].getSourceFile ();
      present = false;
      for (unsigned int j = 0; j < fileList.size (); j++)
	{
	  if (tmp == fileList[j])
	    present = true;
	}
      if (!present)
	{
	  fileList.push_back (tmp);
	}
    }

  return fileList;
}



vector < long >*
HtmlPrint::getFrequencyFromFile (string file, long line)
{
  vector < long >*tmp;

  for (unsigned int i = 0; i < lf.size (); i++)
    {
      if (file == lf[i].getSourceFile ())
	{
	  tmp = lf[i].getFrequency (line);
	  if (tmp != NULL)
	    {
	      return tmp;
	    }
	}
    }

  // No frequency associated with this line
  return NULL;
}


//associate a frequency to each code line for every CFG of the program
void
HtmlPrint::analyseProgram (void)
{
  vector < Cfg * >lcfg = p->GetAllCfgs ();

  for (unsigned int i = 0; i < lcfg.size (); i++)
    {
      if (cg->isDeadCode (lcfg[i]) == false)
	{
	  LineFrequency cur;
	  cur.associateLineFreq (lcfg[i]);
	  lf.push_back (cur);
	}
    }
}


// Performs the analysis
// Returns true if successful, false otherwise
bool
HtmlPrint::PerformAnalysis ()
{
  ofstream html (HtmlFileName.c_str ());
  vector < string > fileList;

  if (!html.is_open ())
    {
      stringstream errorstr;
      errorstr << "HtmlPrint: cannot create output html file " << string (HtmlFileName.c_str ());
      Logger::addFatal (errorstr.str ());
      return false;
    }

  analyseProgram ();
  fileList = getFiles ();

  html << "<html>" << endl << "<head>" << endl << "<title>" << "</title>" << endl << "</head>" << endl << "<body>" << endl;

  for (unsigned int i = 0; i < fileList.size (); i++)
    {

      string filename = fileList[i];
      ifstream sourceFile (filename.c_str ());

      if (sourceFile.is_open ())
	{
	  char line[512];
	  long lineNb = 0;

	  html << "<h1>" << fileList[i] << "</h1> <br>" << endl;
	  html << "<table border=0 cellpadding=0 cellspacing=0>" << endl << "<tr><td><h2>Frequency</h2></td><th><h2>Source</h2></th></tr>";

	  sourceFile.getline (line, 512);
	  lineNb++;
	  while (!sourceFile.fail ())
	    {
	      vector < long >*freqList = getFrequencyFromFile (fileList[i], lineNb);
	      if (freqList != NULL)
		{
		  if (color)
		    {
		      bool useColor = false;
		      for (unsigned int j = 0; j < freqList->size (); j++)
			{
			  if (freqList->at (j) != 0)
			    useColor = true;
			}
		      if (useColor)
			html << "<tr bgcolor=\"lightgreen\"> <td>";
		      else
			html << "<tr> <td>";
		    }
		  else
		    {
		      html << "<tr> <td>";
		    }

		  for (unsigned int j = 0; j < freqList->size (); j++)
		    html << freqList->at (j) << " ";
		}
	      else
		{		// no frequency associated with this line
		  html << "<tr><td> 0";
		}

	      html << "</td>" << endl << "<td><tt><pre>" << line << "</pre></tt></td>" << "</tr>" << endl;

	      sourceFile.getline (line, 512);
	      lineNb++;
	    }

	  html << "</table>" << endl;
	  sourceFile.close ();
	}
      else
	{
	  stringstream errorstr;
	  errorstr << "HtmlPrint: problem opening file:" << fileList[i] << endl;
	  Logger::addError (errorstr.str ());
	  if (fileList[i] == string ("??"))
	    {
	      Logger::addFatal ("HtmlPrint: problem opening file: May come from a program not compiled with the -ggdb option");
	    }
	  return false;
	}
    }

  html << "</body> </html>" << endl;

  html.close ();

  return true;
}

// Remove all private attributes
void
HtmlPrint::RemovePrivateAttributes ()
{
}

//------------------------------------------------------
// LineFrequency class
//------------------------------------------------------
LineFrequency::LineFrequency ()
{
}

LineFrequency::~LineFrequency ()
{
}

// add frequency to a line
void
LineFrequency::addFreq (long line, long freq)
{
  vector < long >*lf;
  bool present = false;

  lf = &lineFreq_map[line];
  for (unsigned int i = 0; i < lf->size (); i++)
    {
      if (lf->at (i) == freq)
	present = true;
    }

  if (!present)
    lf->push_back (freq);
}

void
LineFrequency::associateLineFreq (Cfg * cfg)
{
  vector < Node * >nodes = cfg->GetAllNodes ();

  // Extract frequency of each node of the CFG
  for (unsigned int i = 0; i < nodes.size (); i++)
    {
      Node *curNode = nodes[i];

      // Frequency = sum of frequencies for all execution contexts
      unsigned long frequency = 0;
      // 1. Get the number of execution contexts
      const ContextList & contexts = (ContextList &) cfg->GetAttribute (ContextListAttributeName);
      unsigned int nb_ctx = contexts.size ();
      // 2. Sum the frequencies
      for (unsigned int c = 0; c < nb_ctx; c++)
	{
	  string AttName = AnalysisHelper::getContextAttrFrequencyName(contexts[c]->getStringId());
	  if (curNode->HasAttribute (AttName))
	    {
	      SerialisableUnsignedLongAttribute freq_attr = (SerialisableUnsignedLongAttribute &) curNode->GetAttribute (AttName);
	      frequency += freq_attr.GetValue ();
	    }
	}

      // Get every asm instruction of a node and associate a frequency to each code line
      vector < Instruction * >insts = curNode->GetAsm ();
      for (unsigned int j = 0; j < insts.size (); j++)
	{
	  Instruction *curInst = insts[j];
	  CodeLineAttribute cl = (CodeLineAttribute &) curInst->GetAttribute (CodeLineAttributeName);
	  addFreq (cl.getLine (), frequency);

	  // associate source file to the CFG
	  if (sourceFile.empty ())
	    {
	      sourceFile = cl.getFile ();
	    }
	}
    }
}


string
LineFrequency::getSourceFile (void)
{
  return sourceFile;
}

vector < long >*
LineFrequency::getFrequency (long line)
{
  map < long, vector < long > >::iterator it;

  it = lineFreq_map.find (line);
  if (it == lineFreq_map.end ()) return NULL;
  return &(*it).second;
}
