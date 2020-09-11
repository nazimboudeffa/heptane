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

#include "Logger.h"

//singleton declaration
Logger * Logger::instance = NULL;

Logger::Logger ()
{
  error_state = false;
  TRACE_MODE = true;
}

Logger::~Logger ()
{ }

void
Logger::kill ()
{
  if (instance)
    {
      delete instance;
      instance = NULL;
    }
}

void
Logger::clean ()
{
  if (instance)
    {
      instance->error_state = false, instance->infos.clear ();
      instance->warnings.clear ();
      instance->errors.clear ();
    }
}

bool Logger::getErrorState ()
{
  if (!instance)
    {
      return false;
    }
  return instance->error_state;
}

bool Logger::isDebugMode ()
{
  if (!instance) instance = new Logger ();
  return instance->TRACE_MODE;
}

void
Logger::addError (const string & s)
{
  if (!instance)
    {
      instance = new Logger ();
    }
  instance->error_state = true;
  instance->errors.push_back (s);
}

void
Logger::addWarning (const string & s)
{
  if (!instance)
    {
      instance = new Logger ();
    }
  instance->warnings.push_back (s);
}

void
Logger::addInfo (const string & s)
{
  if (!instance)
    {
      instance = new Logger ();
    }
  instance->infos.push_back (s);
}

void
Logger::addFatal (const string & s)
{
  if (instance)
    {
      instance->print ();
    }
  cerr << "[FATAL]\t" << s << endl;
  exit (-1);
}

void
Logger::print ()
{
  if (instance && isDebugMode ())
    {
      for (size_t i = 0; i < instance->infos.size (); i++)
	cout << "[INFO]\t" << instance->infos[i] << endl;
      for (size_t i = 0; i < instance->warnings.size (); i++)
	cerr << "[WARNING]\t" << instance->warnings[i] << endl;
      for (size_t i = 0; i < instance->errors.size (); i++)
	cerr << "[ERROR]\t" << instance->errors[i] << endl;
      instance->infos.clear(); 
      instance->warnings.clear();
      instance->errors.clear();
    }
}

void
Logger::printDebug (const string & mess)
{
  if (isDebugMode ())
    {
      cout << mess << endl;
    }
}


void
Logger::printVersion()
{
  if (isDebugMode ())
    cout << "Heptane Analysis, version " << HEPTANE_VERSION << endl;
}

void
Logger::print (const string & mess)
{
  cout << mess << endl;
}


void Logger::setOptionTrace(bool b)
{
 if (!instance) instance = new Logger ();
  instance->TRACE_MODE = b;
}
