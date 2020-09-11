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

 usage: utl::public_method
 
 Kind of message:
 - info:        print on cout (print function)
 - warning:     print on cerr (print function)
 - error:       print on cerr (print function)
 - fatal:       print on cerr (addFatal function)

 Basic usage during an analysis to add a message:
    utl::addInfo("mesg");
    utl::addWarning("mesg");
    utl::addError("mesg");
    utl::addFatal("mesg");
 
*********************************************/

#ifndef UTL_H
#define UTL_H

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

using namespace std;

// End of String
#define EOS std::string::npos

class Utl
{
private:
    Utl ();
   ~Utl ();
public:
   /** removing spaces at the beginig andthe end of the operand string str*/
   static void rmSpaces(string &str);

   /** @return "true" if the operand string str is an hexadecimal value with 0x as prefix, "false" otherwise.*/
   static bool isHexNumber(const string& str);

   /** @return "true" if the operand string str is an address (hexadecimal value without 0x as prefix), "false" otherwise.*/
   static bool isAddr(const string& str);
   
   /** @return "true" if the operand string str is a decimal value, "false" otherwise.*/
   static bool isDecNumber(const string& str);

   /** @return the int value conversion of the operand string vpattern */
   static int  string2int(string vpattern);

   /** @return the long int value conversion of the operand string vpattern */
   static long string2long(string vpattern);

   /** @return the max(i,j) */
   static int  imax(int i, int j);

   /** It replaces the first occurrence of c1 by c2 in line */
   static bool replace(string &line, char c1, char c2);

   /** It replaces all the occurrences of c1 by c2 in line */
   static bool replaceAll(string &line, char c1, char c2);

   /** @return the number of element in line with sep as separator. */
   static int  count(string &line, char sep);

   /** @return the complement of the operand string v */
   static string complement(string v);

   /** @return the string value conversion of the operand integer v*/
   static string int2cstring(long v);

   /** @return "true" if the expression (operand1 codop operand2) is evaluated to v, "false" otherwise.
       (restriction: codop is in {+,-,*}) */
   static bool eval(string &operand1, string &codop, string &operand2, long *v);

   /** @return "true" if the expression exp is such that (operand1 {+|-|*} operand2) is evaluated to val, "false" otherwise. */
   static bool evalexpr(string &exp, long *val);
   static string extractStringValue(string &arg);

   /** @return true if a tmp file (res) is created, "false" otherwise.
       The name of the created file is prefix+S, where S a string choiced by the system.
    */
   static bool mktmpfile(string prefix, string &res);
   static void extractRegVal( string mem_pattern, string &reg, string &val);

   static bool getintvalue(string &valstr, int *n);
   static vector<string> split(string str, string seps);
   static bool getIndexFirstValue(vector<string> &vectOperand, int *i);
};
#endif
