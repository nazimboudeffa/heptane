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

/*********************************************

 This class is a logger to store messages during an analysis. 
 The logger is implemented as a singleton.

 usage: Logger::public_method
 
 Kind of message:
 - info:        print on cout (print function)
 - warning:     print on cerr (print function)
 - error:       print on cerr (print function)
 - fatal:       print on cerr (addFatal function)

 Basic usage during an analysis to add a message:
    Logger::addInfo("mesg");
    Logger::addWarning("mesg");
    Logger::addError("mesg");
    Logger::addFatal("mesg");
 
*********************************************/

#ifndef LOGGER_H
#define LOGGER_H

#define TRACE(S) 

/* 3.0 : Address analysis revisited: stack managing 
   3.1 : Array index managing */
#define HEPTANE_VERSION "3.1"

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>

using namespace std;

class Logger
{
private:
  static Logger *instance;
  Logger ();
  ~Logger ();
  bool error_state;
  bool TRACE_MODE;
  vector < string > infos;
  vector < string > warnings;
  vector < string > errors;
 public:
  /** delete the singleton */
  static void kill ();
  /** remove all messages and set error_state to false */
  static void clean ();
  /** add an info message */
  static void addInfo (const string &);
  /** add a warning message */
  static void addWarning (const string &);
  /** add an error message and set the error_state to true */
  static void addError (const string &);
  /** print all stored messages, the fatal message and stop the program execution */
  static void addFatal (const string &);
  /** print all the warnings and errors */
  static void print ();
  /** get the error state */
  static bool getErrorState ();
  /** get the debug mode. 
     Set to true if a shell variable called HEPTANE_DEBUG exists. */
  static bool isDebugMode ();
  /** Print mess in debug mode. */
  static void printDebug (const string & mess);
  /** Print (always) mess and immediatly. */
  static void print (const string & mess);
  /** Print the version of  Heptane in debug mode */
  static void printVersion();
  static void setOptionTrace(bool b);
};

#endif
