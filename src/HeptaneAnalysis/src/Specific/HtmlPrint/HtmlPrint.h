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

#ifndef HTMLPRINT_H
#define HTMLPRINT_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Analysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "CallGraph.h"

using namespace std;

/* This class holds frequencies associated to each 
   program line of a CFG.
*/
class LineFrequency
{
private:
  // map key: the program line / element: vector of frequencies 
  map < long, vector < long > >lineFreq_map;

  // CFG's source file
  string sourceFile;

  // associate a frequency to a line
  void addFreq (long line, long freq);

public:
    LineFrequency ();
   ~LineFrequency ();

  // associate frequency to a line for a whole CFG,
  // and associate the CFG with the corresponding source file
  void associateLineFreq (Cfg * cfg);

  string getSourceFile (void);
    vector < long >*getFrequency (long line);
};



/** Print source code with IPET frequency for each line in a html file.
 *  The location of the output file is the current directory 
 *  The colorize parameter is used to underline code line
 *  with a non nul IPET frequency.
 */
class HtmlPrint:public Analysis
{
private:
  string HtmlFileName;
  bool color;

  // set of LineFrequency (one per CFG)
  vector < LineFrequency > lf;

  static bool CheckInstrHaveCodeLine (Cfg * c, Node * n, void *param);
  static bool CheckNodeHaveFrequency (Cfg * c, Node * n, void *param);

  // structure used for dead code detection
  CallGraph *cg;

  void analyseProgram (void);

  // return all the source files concerned with this program
  vector < string > getFiles (void);

  // return the frequency list associated with a line
  vector < long >*getFrequencyFromFile (string file, long line);

public:
    HtmlPrint (Program * p, string f, bool colorize);
   ~HtmlPrint (void);

   /** Checks if all required attributes are in the CFG
       Returns true if successful, false otherwise*/
  bool CheckInputAttributes ();

  /** Performs the analysis
      Returns true if successful, false otherwise*/
  bool PerformAnalysis ();

  /** Remove all private attributes */
  void RemovePrivateAttributes ();

};


#endif
