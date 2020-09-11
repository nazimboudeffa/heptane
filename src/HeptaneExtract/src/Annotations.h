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

#ifndef _IRISA_ANNOT_H
#define _IRISA_ANNOT_H

#include <CfgLib.h>
#include "ConfigExtract.h"

typedef struct
{
  t_address address;
  long type;
  vector < long >values;
  cfglib::Node * node;
} t_annotation;

// Annotations
// ANNOT_START_MARK : start of annotation, the end of line is specific
// to the type of annotation (loop bounds or annotations of basic blocks)
// Loop bounds or basic block annotations should be integers
#define ANNOT_START_MARK ".lflags"

// Max string size in objdump files
#define STRMAX 80

// Annotation types
#define LOOP_MAXITER 1

// Name of annotation section. Same name than in annot.h
#define ANNOT_SECTION_NAME ".wcet_annot"

// Annotation used for the switch statement (begin of the switch). Same name than in annot.h
#define ANNOT_SWITCH_BEGIN ".switch_begin"

/** 
    Generate annotation XML from CFG
*/
extern void GenerateAnnotationXMLFile (cfglib::Program & p, string annotfilename, bool verbose);

/**
    Add annotations in a CFG from an annotation file
    in XML format
    In case the annotation file is not complete, fill-in annotations
    found in the file and go on
 */
extern void AttachAnnotationsFromXML (cfglib::Program & cfglib_program, string annotfilename, bool verbose);

/**
    Get annotations from the binary
    Parameters:
    - Program
    - File containing the dump of the annotation section, obtained previously
    using objdump -j
    NB: This function might be sensitive to the format of objdump output
 */
extern void AttachAnnotationFromBinary (cfglib::Program & cfglib_program, string annot_section_dump_file, bool verbose);

/**
   @return the list of the addresses (decimal) of the beginning "switch" blocks 
*/
extern vector < t_address > getSwitchInfos(string switch_section_dump_file);


#endif
