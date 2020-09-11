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

#ifndef _CONFIG_EXTRACT_H
#define _CONFIG_EXTRACT_H

// Inclusions
#include <assert.h>
#include <fstream>
#include <string.h>
#include <stdint.h>		// for uint32_t
#include "CfgLib.h"
#include "arch.h"
#include "Logger.h"
#include "AddressAttribute.h"

// Namespaces
using namespace std;
using namespace cfglib;

// FIXME: remove during the integration
#define AddressAttributeNameExtract "address"

/* Debug of cfg extractor */
// Uncomment one of these two lines to enter/leave debug mode
#define dbg_extract(x) if (overbose) x
// #define dbg_extract(x)

/** Type of analyzed code */
#define C_TYPE 0
#define ASM_TYPE 1
#define BINARY_TYPE 2

/// Address type for mips. FIXME: to be moved elsewhere
typedef unsigned long t_address;

/** Class for configuration file. All members are public to avoid a multiplication of accessors */
class ConfigExtract
{

private:
  /** Compilation / link / objdump generation from the configuration paramaters*/
  void CompileAll (ListXmlTag & lt);
  void execute_cmd(string command);
  void copy( string ipath, string opath);

public:
  // Parameters of CFG extraction, read from xml file
  // --------------------------------------------------

  // The role of most of then directly comes from their name and need not be commented
  string program_name;
  string entry_point_name;
  string annotation_file;
  string tmp_dir;
  string result_dir;
  string input_dir;
  string pre_processor;
  string pre_processor_args;
  string compiler;
  string compiler_args;
  string assembler;
  string assembler_args;
  string linker;
  string linker_args;
  string linker_script;
  string linker_LIBS;
  string objdump;
  string objdump_args;
  string readelf;
  string readelf_args;

  // Default values for parameters
  string assemblyOptions;	// Assembler options
  bool binary_only;		// Only one binary file to handle
  bool output_asm;
  bool output_exe;
  bool output_objdump;
  bool output_annot;
  bool output_readelf;
  bool output_cfg;
  bool output_code_addresses;

  // option
  bool overbose;
  // Exported methods
  // -----------------

  /** Constructor: parsing of configuration file + compilation/link of everything */
  ConfigExtract (string filename, bool optverbose);

  /** Destructor. */
  ~ConfigExtract ()
    { };
};

#endif
