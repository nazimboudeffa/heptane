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

#ifndef _IRISA_CFGLIB_HELPER_H
#define _IRISA_CFGLIB_HELPER_H

/*! #includes and forward declarations */
#include <map>
#include <vector>
#include <string>
#include <sstream>

/*! this namespace is the global namespace */
namespace cfglib 
{
	/*! functions in this namespace are reserved for internal use
	 * only */
	namespace helper {
		template<class K, class T>
			inline std::vector<T> get_values_of_map(std::map<K, T> mymap)
			{
				std::vector<T> values ;
				for (typename std::map<K,T>::const_iterator it =
						mymap.begin() ;
						it != mymap.end() ;
						it++)
				{ values.push_back(it->second) ; }
				return values ;
			}

		template<typename K, typename T>
			inline std::vector<K> get_keys_of_map(std::map<K, T> mymap)
			{
				std::vector<K> keys ;
				for (typename std::map<K,T>::const_iterator it =
						mymap.begin() ;
						it != mymap.end() ;
						it++)
				{ keys.push_back(it->first) ; }
				return keys ;
			}

		inline std::string int_to_string(int const& i)
		{
			std::ostringstream str ;
			str << i ;
			return str.str() ;
		}

		inline void substitute_all_substrings(
			               std::string& cp,
			               std::string const& for_this,
			               std::string const& sub_this)
		{
			if (cp.empty())
			{
				//cp = sub_this;
				return;
			}
			size_t prec_loc_end=0 ;
			while (true)
			{
				size_t loc = cp.find(for_this,prec_loc_end);
				if (loc != std::string::npos)
				{	cp.replace(loc,for_this.length(),sub_this);
					prec_loc_end = loc + sub_this.length() ;
				}
				else 
				{	return; }
			}
		}

		inline std::string escape_xml(std::string const& str)
		{
			std::string escaped(str) ;
			substitute_all_substrings(escaped, "&", "&amp;") ;
			substitute_all_substrings(escaped, "\"", "&quot;") ;
			substitute_all_substrings(escaped, ">", "&gt;") ;
			substitute_all_substrings(escaped, "<", "&lt;") ;
			substitute_all_substrings(escaped, "'", "&apos;") ;
			return escaped ;
		}

		inline std::string unescape_xml(std::string const& str)
		{
			std::string escaped(str) ;
			substitute_all_substrings(escaped, "&quot;", "\"") ;
			substitute_all_substrings(escaped, "&gt;",   ">" ) ;
			substitute_all_substrings(escaped, "&lt;",   "<" ) ;
			substitute_all_substrings(escaped, "&apos;", "'" ) ;
			substitute_all_substrings(escaped, "&amp;",  "&" ) ;
			return escaped ;
		}

	  inline std::vector<std::string> split_string(
						       std::string const& input, 
						       std::string const& delimiter,
						       bool empty_fields=false) 
	  {
	    using std::vector ;
	    using std::string ;
	    
	    vector<string> result ; 
	    
	    if ((input.size() == 0 ) || (delimiter.size() == 0))
	      {	return result; }
	    
	    string::size_type size_delim = delimiter.size() ;
	    string::size_type prec_pos = 0;
	    string::size_type cur_pos = input.find(delimiter, 0);
	    while (cur_pos != string::npos)
	      {
		string cur(input.substr(prec_pos, cur_pos - prec_pos)) ;
		if ((0 != cur.size()) || empty_fields)  
		  {	result.push_back(cur) ; }
		prec_pos = cur_pos + size_delim ;
		cur_pos = input.find(delimiter, cur_pos + size_delim) ;
	      }
	    return result ;
	  }
	  /*! Get the base name of a file, including suffix (if any)
	   *  ex: getBaseName("/tmp/toto/x.c") -> "x.c"
	   *  ex: getBaseName("x.c") -> "x.c"
	   */
	  inline std::string getBaseName(std::string file)
	  {
	    unsigned int position=0;
	    bool hasdir = false;
	    for (unsigned int i=0;i<file.size();i++) 
	      if (file[i]=='/') {position=i; hasdir=true;}
	    std::string p;
	    unsigned int start;
	    if (hasdir) start=position+1; else start=position;
	    for (unsigned int i=start;i<file.size();i++) 
	      p+=file[i];
	    return p;
	  }
	  /*! Get the directory where a file is stored
	   * ex: getDirName("/tmp/toto/x.c") -> "/tmp/toto/"
	   * ex: getDirName("x.c") -> ""
	   */
	  inline std::string getDirName(std::string file)
	  {
	    unsigned int position=0;
	    for (unsigned int i=0;i<file.size();i++) 
	      if (file[i]=='/') position=i;
	    std::string p;
	    for (unsigned int i=0;i<position+1;i++) p+=file[i];
	    return p;
	  }
	  /*! Remove the suffix of a file name
	   *  ex: extractBase("/tmp/toto/x.c") -> "/tmp/toto/x"
	    * ex: extractBase("x.c") -> "x"
	    */
	  inline std::string extractBase(std::string file)
	  {
	    unsigned int position=0;
	    for (unsigned int i=0;i<file.size();i++) 
	      if (file[i]=='.') position=i;
	    std::string p;
	    for (unsigned int i=0;i<position;i++) p+=file[i];
	    return p;
	  }

	}


} // cfglib::
#endif // _IRISA_CFGLIB_HELPER_H
