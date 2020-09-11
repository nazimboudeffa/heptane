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

#include "ConfigExtract.h"

void ConfigExtract::execute_cmd(string command)
{
  dbg_extract(cout << "** Executing : " << command << endl; );
  if (system(command.c_str()) == -1)
    Logger::addFatal("ConfigExtract error when executing command: " + command);
}

void ConfigExtract::copy( string ipath, string opath)
{ 
  execute_cmd("/bin/cp -f " + ipath + " " + opath);
}

/**
   Does all the dirty stuff for CFG construction:
   - Preprocess all files
   - Compile to asm
   - Assemble
   - Link to obtain a binary file
   - Use objdump to have instruction addresses and annotations, stored in a specific section in binary
   - Output intermediate files if asked for in the configuration file
*/
void ConfigExtract::CompileAll(ListXmlTag & lt)
{
  vector < string > objects;
  bool binary_file;

  string prefix_tmpfile = tmp_dir + "/" + program_name;

  cout << "Heptane extract::Generating files (" << program_name << ")" << endl;
  binary_file = (lt.size() == 1 && lt[0].getAttributeString("TYPE") == "BINARY");

  // If sources are in C and asm, generate the binary file
  if (binary_file == false)
    {
      for (unsigned int i = 0; i < lt.size(); i++)
	{
	  XmlTag ft = lt[i];
	  string file_name = ft.getAttributeString("NAME");
	  string file_type = ft.getAttributeString("TYPE");

	  assert(file_type == "C" || file_type == "ASM");
	  // Get file basename
	  string basename = cfglib::helper::extractBase(cfglib::helper::getBaseName(file_name));
	  string asmfile;
	  string objectfile;

	  // Management of C programs
	  if (file_type == "C")
	    {

	      // Preprocessing
	      if (pre_processor == "")
		Logger::addFatal("ConfigExtract error: PREPROCESSOR should be specified");
	      string ccpcommand = pre_processor + " " + pre_processor_args + " " + file_name + ">" + tmp_dir + "/" + basename + ".c";
	      execute_cmd(ccpcommand);

	      // Compilation to asm
	      if (compiler == "")
		Logger::addFatal("ConfigExtract error: COMPILER should be specified");
	      asmfile = tmp_dir + "/" + basename + ".s";
	      string cccommand = compiler + " " + compiler_args + assemblyOptions + " " + tmp_dir + "/" + basename + ".c" + " -o " + asmfile;
	      execute_cmd(cccommand);

	      // Assemble
	      if (assembler == "")
		Logger::addFatal("ConfigExtract error: ASSEMBLER should be specified");
	      objectfile = tmp_dir + "/" + basename + ".o";
	      string ascommand = assembler + " " + assembler_args + " " + asmfile + " -o " + objectfile;
	      execute_cmd(ascommand);
	      objects.insert(objects.end(), objectfile);
	    }
	  else
	    {
	      string asmfile = file_name;
	      string objectfile = tmp_dir + "/" + basename + ".o";
	      string ascommand = assembler + " " + assembler_args + " " + asmfile + " -o " + objectfile;
	      execute_cmd(ascommand);
	      objects.insert(objects.end(), objectfile);
	    }
	}

      // Link all object files together
      if (linker == "")
	Logger::addFatal("ConfigExtract error: LINKER should be specified");

      string linkcmd = linker + " " + linker_args + " -e " + entry_point_name + " -o " + prefix_tmpfile;
      unsigned int nbobjs = objects.size();
      for (unsigned int i = 0; i < nbobjs; i++)
	linkcmd += " " + objects[i];
      if (linker_LIBS != "")
	linkcmd += linker_LIBS;

      if (linker_script != "")
	linkcmd += " -T " + linker_script;
      execute_cmd(linkcmd);

      // Output binary if required
      if (output_exe)
	copy( prefix_tmpfile,  result_dir + "/" + program_name + ".exe ");

      // Output asm file if required
      if (output_asm) 
	copy( prefix_tmpfile + ".s", result_dir + "/" + program_name + ".s");
    }
  else
    {
      // Input is binary file
      copy(lt[0].getAttributeString("NAME"), prefix_tmpfile);
    }

  // Apply readelf if required, and generate the output file
  if (1 /*output_readelf */ )	//FIXME: readelf is now always needed for the extraction
    //TODO: find a better way to force it
    {
      if (readelf == "")
	Logger::addFatal("ConfigExtract error: READELF should be specified");
      string readelfcmd = readelf + " " + readelf_args + " -S " + prefix_tmpfile + ">" + result_dir + "/" + program_name + ".readelf";
      execute_cmd(readelfcmd);
    }

  // Disassemble to obtain addresses
  if (objdump == "")
    Logger::addFatal("ConfigExtract error: OBJDUMP should be specified");

  string objdumpcmd;
  objdumpcmd = objdump + " " + objdump_args + " -t -d -z  " + prefix_tmpfile + " > " + prefix_tmpfile + ".objdump";
  execute_cmd(objdumpcmd);
  dbg_extract(cout << "Objdump done (address extraction)" << endl);

  // Output objdump file if required
  if (output_objdump)
    copy(prefix_tmpfile + ".objdump", result_dir + "/" + program_name + ".objdump");
  
  // Use objdump to dump the annotation section
  objdumpcmd = objdump + " " + objdump_args + " -s -z --section=.wcet_annot " + prefix_tmpfile + " >  " + prefix_tmpfile + ".annot";
  execute_cmd(objdumpcmd);

  // Use objdump to dump the "switch" annotation section
  objdumpcmd = objdump + " " + objdump_args + " -s -z --section=.switch_begin " + prefix_tmpfile + " > " + prefix_tmpfile + ".switch";
  execute_cmd(objdumpcmd);

  // Create the Cfg once all files are generated
  cout << "Heptane extract::Creating Cfg" << endl << endl;

}

// -------------------------------------------------------------------
//
// Initialization stuff: scan and check the validity of the
// cfg extractor configuration file
// Then, CFG extraction
//
// -------------------------------------------------------------------
ConfigExtract::ConfigExtract(string filename, bool verbose)
{
  overbose = verbose;
  // Init the parameters with their default value, to be possibly overwritten
  // in the configuration file
  assemblyOptions = " -c -S ";	// Assembler options

  binary_only = false;		// Only one binary file to handle
  output_asm = true;
  output_exe = true;
  output_objdump = true;
  output_annot = false;
  output_readelf = true;
  output_cfg = true;
  output_code_addresses = true;

  // Open the config file
  ifstream cf;
  cf.open(filename.c_str());
  if (cf.fail())
    Logger::addFatal("ConfigExtract error: unable to open configuration file" + filename);

  XmlDocument xmldoc(filename);

  // Get the architecture file from xml
  ListXmlTag lt = xmldoc.searchChildren("TARGET");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      string arch_endianness = lt[0].getAttributeString("ENDIANNESS");
      if (arch_endianness != "BIG" && arch_endianness != "LITTLE")
	Logger::addFatal("ConfigExtract error: Unsupported endianness (should be BIG or LITTLE): " + arch_endianness);
      string arch_name = lt[0].getAttributeString("NAME");
      if ((arch_name == "MIPS") || (arch_name == "ARM") /* removed LBesnard March 2016 || (arch_name == "PPC") */ )
	Arch::init(arch_name, arch_endianness == "BIG");
      else
	Logger::addFatal("ConfigExtract error: Unsupported target architecture: " + arch_name);
    }

  // Check there is a PROGRAM tag and fill-in program properties
  lt = xmldoc.searchChildren("PROGRAM");
  if (lt.size() != 1)
    Logger::addFatal("ConfigExtract error: there should be a single PROGRAM tag in your XML");

  XmlTag pt = lt[0];
  program_name = pt.getAttributeString("NAME");
  entry_point_name = pt.getAttributeString("ENTRYPOINT");

  // Fill in locations and options of compiler tools 
  lt = xmldoc.searchChildren("PREPROCESSOR");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      pre_processor = lt[0].getAttributeString("NAME");
      pre_processor_args = lt[0].getAttributeString("OPT");
    }

  lt = xmldoc.searchChildren("COMPILER");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      compiler = lt[0].getAttributeString("NAME");
      compiler_args = lt[0].getAttributeString("OPT");
    }

  lt = xmldoc.searchChildren("ASSEMBLER");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      assembler = lt[0].getAttributeString("NAME");
      assembler_args = lt[0].getAttributeString("OPT");
    }

  lt = xmldoc.searchChildren("LINKER");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      linker = lt[0].getAttributeString("NAME");
      linker_args = lt[0].getAttributeString("OPT");
      linker_script = lt[0].getAttributeString("SCRIPT");
      linker_LIBS = lt[0].getAttributeString("LIBS");
    }

  lt = xmldoc.searchChildren("OBJDUMP");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      objdump = lt[0].getAttributeString("NAME");
      objdump_args = lt[0].getAttributeString("OPT");
    }

  lt = xmldoc.searchChildren("READELF");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      readelf = lt[0].getAttributeString("NAME");
      readelf_args = lt[0].getAttributeString("OPT");
    }

  lt = xmldoc.searchChildren("TMPDIR");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      tmp_dir = lt[0].getAttributeString("NAME");
    }
  if (tmp_dir == "")
    {
      cerr << "Warning: TMPDIR is not set, setting it to /tmp" << endl;
      tmp_dir = "/tmp";
    }

  lt = xmldoc.searchChildren("RESULTDIR");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      result_dir = lt[0].getAttributeString("NAME");
    }
  if (result_dir == "")
    {
      cerr << "Warning: RESULTDIR is not set, setting it to ./" << endl;
      result_dir = "./";
    }

  lt = xmldoc.searchChildren("INPUTDIR");
  assert(lt.size() <= 1);
  if (lt.size() == 1)
    {
      input_dir = lt[0].getAttributeString("NAME");
    }
  if (input_dir == "")
    {
      cerr << "Warning: INPUTDIR is not set, setting it to ./" << endl;
      input_dir = "./";
    }


  // Verification of file types
  lt = pt.searchChildren("SOURCEFILE");
  int n = lt.size();
  if (n == 0)
    {
      Logger::addFatal("ConfigExtract error: no source file specified");
    }
  else
    {
      for (int i = 0; i < n; i++)
	{
	  XmlTag ft = lt[i];
	  string file_type = ft.getAttributeString("TYPE");

	  if (file_type != "BINARY" && file_type != "C" && file_type != "ASM" && file_type != "")
	    {
	      Logger::addFatal("ConfigExtract error: unknownn file type " + file_type);;
	    }
	  else
	    {
	      annotation_file = ft.getAttributeString("ANNOTATIONFILE");

	      if (((file_type == "BINARY") || (file_type == "ASM")) && (n != 1))
		{
		  Logger::addFatal("ConfigExtract error: only one BINARY/ASM file is supported");
		}
	      if (file_type == "C")
		{
		  if (annotation_file != "")
		    {
		      Logger::addWarning("ConfigExtract warning: ANNOTATIONFILE is ignored for C file");
		      annotation_file = "";
		    }
		}
	      else  // BINARY, ASM
		if (annotation_file == "")
		  {
		    Logger::addFatal("ConfigExtract error: ANNOTATIONFILE is required for BINARY/ASM file");
		  }
		else
		  {
		    ifstream an;
		    an.open(annotation_file.c_str());
		    if (an.fail())
		      {
			Logger::addFatal("ConfigExtract error: ANNOTATIONFILE not found");
		      }
		    binary_only = true;
		  }
	    }
	}
    }

  Logger::print();

  if (!Logger::getErrorState())
    {
      lt = xmldoc.searchChildren("PROGRAM");
      lt = lt[0].searchChildren("SOURCEFILE");
      // Pre-process, compile, assemble, link, objdump the binary + fix for Salto compliance
      CompileAll(lt);
    }
}
