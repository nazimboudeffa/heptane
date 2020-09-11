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

/* -----------------------------------------------------
   Main file to call the different steps of WCET estimation
   -------------------------------------------------------- */

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <sys/time.h>

#include "Logger.h"
#include "Generic/Config.h"
#include "Generic/Analysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Specific/CacheAnalysis/ICacheAnalysis.h"
#include "Specific/CacheAnalysis/CacheStatistics.h"
#include "Specific/CacheAnalysis/DCacheAnalysis.h"
#include "Specific/SimplePrint/SimplePrint.h"
#include "Specific/DotPrint/DotPrint.h"
#include "Specific/IPETAnalysis/IPETAnalysis.h"
#include "Specific/DataAddressAnalysis/AddressAnalysis.h"
#include "Specific/HtmlPrint/HtmlPrint.h"
#include "Specific/PipelineAnalysis/PipelineAnalysis.h"
#include "Generic/Timer.h"

// --------------------------------------------
//
// Print a duration from 2 timeval structures
//
// --------------------------------------------
/*
//DH removed to avoid a warning " defined but not used"
static void
PrintDuration (struct timeval *t1, struct timeval *t2)
{
  // check t2 later than t1 or emit a warning
  if (t2->tv_sec < t1->tv_sec)
      Logger::addFatal ("PrintDuration: warning, negative length time interval");
  else
    {
      double secs = (double) (t2->tv_sec - t1->tv_sec) + (double) (t2->tv_usec - t1->tv_usec) * 1.0e-6;
      cout << secs;
    }
}
 */

// --------------------------------------------
//
// Example of WCET computation that does not use the xml configuration file
// (don't know if it is useful, but had to test if it was not too painful
//
// Here, analysis of icache only, + IPET
// ---------------------------------------------
/*
//DH removed to avoid a warning " defined but not used"
static string
WCETcomputation (string xml_input_file)
{
  // UNUSED --UNUSED -- UNUSED
  string WCET;

  // For the timing of the several operations
  struct timeval t1, t2;

  cout << endl << "***** WCETcomputation: " << xml_input_file << endl << endl;

  Program *p = Program::unserialise_program_file (xml_input_file);

  if (!p)
    {
      string error_msg = "Impossible to open file " + xml_input_file;
      Logger::addFatal (error_msg);
    }

  // Check that program is WCET analyzable
  AnalysisHelper::ProgramCheck (p);

  // Cache analysis
  ICacheAnalysis ca (p, 32,	// nbsets
		     2,		// nbways
		     8,		// cachelinesize
		     LRU,	// replacement_policy
		     1,		// level
		     true,	// apply_must
		     true,	// apply_persistence
		     true,	// apply_may
		     true);	// keep_age
  Logger::clean ();
  gettimeofday (&t1, NULL);
  if ( ! ca.CheckPerformCleanup ()) Logger::addFatal ("Analysis failed");
  gettimeofday (&t2, NULL);
  cout << "Cache analysis OK" << endl;
  cout << "Duration of cache analysis: ";
  PrintDuration (&t1, &t2);
  cout << endl;
  Logger::print ();


  // IPET analysis
  IPETAnalysis ia (p, METHOD_NOPIPELINE_ICACHE_DCACHE,	// ps->method,
		   LP_SOLVE,	// ps->solver,
		   true,	// ps->attach_WCET_info,
		   true,      //  ps->generate_node_freq,
		   // configuration parameters
		   config->getNbICacheLevels(),  
		   config->getNbDCacheLevels(),
		   config->getCacheLatencies(),
		   1,
		   0
		   );
  Logger::clean ();
  gettimeofday (&t1, NULL);
  if (! ia.CheckPerformCleanup ()) Logger::addFatal ("Analysis failed");
  gettimeofday (&t2, NULL);
  cout << "Cache analysis OK" << endl;
  cout << "Duration of IPET analysis: ";
  PrintDuration (&t1, &t2);
  cout << endl;
  Logger::print ();

  Cfg *c = p->GetEntryPoint ();
  assert (c->HasAttribute (WCETAttributeName));
  SerialisableStringAttribute ba = (SerialisableStringAttribute &) c->GetAttribute (WCETAttributeName);
  WCET = ba.GetValue ();

  // Store result in file (comment if not needed)
  // Here, put in the same xml than originally
  p->serialise_program (xml_input_file);

  // Cleanup and return
  delete p;
  return WCET;
}
*/

/**
   Fill configuration object by program then calls WCETcomputation().
   >>>>> Currently not executed.
 */
static void Analysis_by_program()
{
  return; 

  //  config = new Config(15 /* load_latency */,  1 /* store latency*/ , MIPS_ARCHITECTURE); should be replaced by 
  //  config->setXXX(15 /* load_latency */,  1 /* store latency*/ , MIPS_ARCHITECTURE);
  /* 
     config->AddCacheLevel(
     32, // nbsets
     2, // nbways,
     8, // cachelinesize,
     1, //  level,
     LRU, // replacement_policy,
     ICACHE, //  type,
     1); // latency
  */

  // WCET computation
  // string WCET = WCETcomputation("simple/simple.xml");
  // cout << "WCET: "  << WCET << endl;
}


/*! 
 * Entry point of the WCET analyser 
 * Here, automatic execution of analyses from a configuration file
 */
int
main (int argc, char **argv)
{
  string configFile;
  bool printTime = true;

  if (argc == 3)
    {
      if ( string(argv[1]) == "-t")
	{
	  Logger::setOptionTrace(false); 
	  printTime = false;
	}
      else
	cout << "Unknown option " << argv[1] << "...ignored " << endl;
    }
  else
    if (argc != 2)
      {
	cerr <<  "Usage " << string (argv[0]) << " <configfilename.xml>" << endl;
	exit (-1);
      }
  
  configFile = string (argv[argc-1]);
  
  // Initialisation code (do not remove, useful to create serialisation code
  // for attribute types not supported by cfglib
  AttributesFactory *af = AttributesFactory::GetInstance ();
  af->SetAttributeType (AddressAttributeName, new AddressAttribute ());
  af->SetAttributeType (SymbolTableAttributeName, new SymbolTableAttribute ());
  af->SetAttributeType (ARMWordsAttributeName, new ARMWordsAttribute ());
  af->SetAttributeType (StackInfoAttributeName, new StackInfoAttribute ());
  af->SetAttributeType (CodeLineAttributeName, new CodeLineAttribute ());
  af->SetAttributeType (ContextListAttributeName, new ContextList ());
  af->SetAttributeType (ContextTreeAttributeName, new ContextTree ());
  af->SetAttributeType (MetaInstructionAttributeName, new MetaInstructionAttribute ());

  Timer timer_AllAnalysis;
  float time = 0.0;
  timer_AllAnalysis.initTimer();


  // Main analysis code from configuration file
  // -------------------------------------------
  Logger::printVersion();
  Logger::printDebug ("Reading configuration file");
  config->FillArchitectureFromXml (configFile);
  Logger::printDebug ("Executing from configuration file");
  config->ExecuteFromXml (configFile, printTime);

  // Analysis code by program (to be modified for specific purposes)
  Analysis_by_program();


  if (printTime)
    {
      timer_AllAnalysis.addTimer(time);
      stringstream infostr;
      infostr << " =======> Total Time = "  << time;
      Logger::addInfo(infostr.str());
      Logger::print();
    }

  // Cleanup code
  Logger::kill ();
  delete config;

 
  return 0;
}
