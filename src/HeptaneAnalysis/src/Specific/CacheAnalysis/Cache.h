/* -------------------------------------------------------------------------------------

   Copyright IRISA, 2003-2017

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

   ------------------------------------------------------------------------------------ */

/**
 *
 * AbstractCache implementation and definition of the abstract cache set interface for the MAY, MUST, and PS analysis
 *
 */

#ifndef CACHE_H
#define CACHE_H

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>


#include "Generic/cow_ptr.h"

#include "Analysis.h"		//useful for t_address type

using namespace std;

/**************************************************
 *
 * AbstractCache
 *
 * generic type T stands for MUST, MAY or PS abstract cache set
 *
 *************************************************/

template < typename T > class AbstractCache
{
 private:
  /** internal structure */
  vector < cow_ptr < T > >contents;
  unsigned int nb_sets;
  unsigned int nb_ways;
  unsigned int cacheline_size;

  /** Computes the address corresponding to the beginning of the cache line where addr is */
  t_address computeStartLine (t_address addr) const
  {
    return addr - (addr % cacheline_size);
  }
  /** Computes the set where addr is mapped */
  unsigned int computeSet (t_address addr) const
  {
    return (computeStartLine (addr) / cacheline_size) % nb_sets;
  }

 public:

  /** default constructor */
  AbstractCache ()
    {
      nb_sets = nb_ways = cacheline_size = 0;
    }

  /** Constructor for a MAY Abstract cache only */
  AbstractCache (unsigned int nbsets, unsigned int nbways, unsigned int cachelinesize)
    {
      nb_sets = nbsets;
      nb_ways = nbways;
      cacheline_size = cachelinesize;
      cow_ptr < T > tmp (new T (nb_ways));
      contents.resize (nb_sets, tmp);
    }

  /** Constructor for the MUST and PS Abstract cache only
      nbways_removed is used to return a correct age to the function getAge when non LRU policy is used
  */
  AbstractCache (unsigned int nbsets, unsigned int nbways, unsigned int nbways_removed, unsigned int cachelinesize)
    {
      nb_sets = nbsets;
      nb_ways = nbways;
      cacheline_size = cachelinesize;
      cow_ptr < T > tmp (new T (nb_ways, nbways_removed));
      contents.resize (nb_sets, tmp);
    }

  /** Print the Abstract Cache for debugging purpose */
  void Print () const
  {
    cout << "---------------------------" << endl;
    for (unsigned int s = 0; s < nb_sets; s++)
      {
	contents[s]->Print ();
      }
  }

  /** returns true if this is equal to c and false otherwise */
  bool Equals (const AbstractCache < T > &c) const
  {
    if (c.nb_sets != nb_sets || c.nb_ways != nb_ways || c.cacheline_size != cacheline_size)
      {
	return false;
      }

    for (unsigned int s = 0; s < nb_sets; s++)
      {
	if (c.contents[s]->Equals (*(contents[s])) == false)
	  {
	    return false;
	  }
      }
    return true;
  }

  /** returns the age in the abstract cache of the cache line containing addr
      between [0..nb_ways-1] if present
      nb_ways otherwise
  */
  unsigned int GetAge (t_address addr) const
  {
    addr = computeStartLine (addr);
    int s = computeSet (addr);
    return contents[s]->GetAge (addr);
  }

  /** returns true if the cache line containing addr is absent from the abstract cache and false otherwise */
  bool Absent (t_address addr) const
  {
    addr = computeStartLine (addr);
    int s = computeSet (addr);
    return contents[s]->Absent (addr);
  }

  /** returns true if the cache line containing addr is present in the abstract cache and false otherwise */
  bool Present (t_address addr) const
  {
    return !Absent (addr);
  }

  /** returns the maximal age of all the blocks
      used by the data cache analysis
  */
  unsigned int GetMaxAge (const set < t_address > &blocks) const
  {
    unsigned int max_age = 0;

    for (set < t_address >::const_iterator it = blocks.begin (); it != blocks.end () && max_age < nb_ways; it++)
      {
	max_age = max (max_age, GetAge (*it));
      }
    return max_age;
  }

  /** returns true if all cache blocks are present and false otherwise
      used by the data cache analysis
  */
  bool AllPresent (const set < t_address > &blocks) const
  {
    bool present = true;
    for (set < t_address >::const_iterator it = blocks.begin (); it != blocks.end () && present; it++)
      {
	present = !Absent (*it);
      }
    return present;
  }

  /** returns true if at least one cache block of blocks is present and false otherwise
      used by the data cache analysis
  */
  bool OnePresent (const set < t_address > &blocks) const
  {
    for (set < t_address >::const_iterator it = blocks.begin (); it != blocks.end (); it++)
      {
	if (Present (*it)) { return true; }
      }
    return false;
  }

  /** Join function */
  void Join (const AbstractCache < T > &c)
  {
    assert (c.nb_sets == nb_sets && c.nb_ways == nb_ways && c.cacheline_size == cacheline_size);
    for (unsigned int s = 0; s < nb_sets; s++)
      {
	contents[s]->Join (*(c.contents[s]));
      }
  }

  /** Update function when only one address is accessed */
  void Update (t_address addr, string cac)
  {
    if (cac == "N") { return; }

    if (nb_sets > 0 && nb_ways > 0)
      {
	addr = computeStartLine (addr);
	int s = computeSet (addr);
	if (cac == "A")
	  {
	    contents[s]->Update (addr);
	  }
	else			//cac=="U" || cac="UN"
	  {
	    T before = *(contents[s]);
	    contents[s]->Update (addr);
	    contents[s]->Join (before);
	  }
      }
  }

  /** Update function when a set of addresses is accessed 
      used by the data cache analysis
  */
  void Update (const set < t_address > &addrs, string cac)
  {
    assert (nb_sets > 0 && nb_ways > 0);

    if (cac == "N")
      {
	return;
      }

    // Get the sets where addresses should be stored
    map < int, set < t_address > >inserted;
    for (set < t_address >::const_iterator it = addrs.begin (); it != addrs.end (); it++)
      {
	inserted[computeSet (*it)].insert (computeStartLine (*it));
      }

    //If the range of accessed addresses fits in a unique cache line,
    if (inserted.size () == 1)
      {
	map < int, set < t_address > >::iterator it = inserted.begin ();
	if (it->second.size () == 1)
	  {
	    Update (*(it->second.begin ()), cac);
	  }
	else
	  {
	    contents[it->first]->Update (it->second);
	  }
      }
    else			//Otherwise, we have to use the update function for unpredictable accesses
      {
	for (map < int, set < t_address > >::iterator it = inserted.begin (); it != inserted.end (); it++)
	  {
	    contents[it->first]->Update (it->second);	//safe for UNCERTAIN AND ALWAYS based on the semantic of unpredictable accesses
	  }
      }
  }
};


/**************************************************
 *
 *  AbstractCacheSet MUST
 *
 *************************************************/

class MUST
{
 private:
  unsigned int nb_ways;		//the value corresponds to the number of ways to consider during the analysis
  //(cf. cache abstract factory in ICacheAnalysis.cc or DCacheAnalysis.cc)
  unsigned int nb_ways_removed;	//used in function GetAge and useful when a non LRU policy is used
  vector < set < t_address > >contents;	//abstract cache sets

 public:

  /** Constructor */
  explicit MUST (unsigned int nbways, unsigned int nbways_removed);

  /** @return the age in the abstract cache of the cache line containing addr between [0..nb_ways-1] if present,  nb_ways+nb_ways_removed otherwise
   */
  unsigned int GetAge (t_address addr) const;

  /** @return true if the cache line containing addr is absent from the abstract cache set, false otherwise */
  bool Absent (t_address) const;

  /** Print the Abstract Cache Set for debugging purpose */
  void Print () const;

  /** Update function when only one address is accessed */
  void Update (t_address);

  /** Update function when a set of addresses is accessed
      used by the data cache analysis
  */
  void Update (const set < t_address > &);

  /** Join function */
  void Join (const MUST &);

  /** returns true if this is equal to c and false otherwise */
  bool Equals (const MUST &) const;

};

/**************************************************
 *
 *  AbstractCacheSet MAY
 *
 *************************************************/

class MAY
{
 private:
  unsigned int nb_ways;		//the value corresponds to the number of ways to consider during the analysis
  //(cf. cache abstract factory in ICacheAnalysis.cc or DCacheAnalysis.cc)
  vector < set < t_address > >contents;	//abstract cache sets

 public:

  /** Constructor */
  explicit MAY (unsigned int nbways);

  //unsigned int GetAge(t_address addr) const; //Not implemented for MAY analysis: semantic issue with different replacement policies

  /** returns true if the cache line containing addr is absent from the abstract cache set and false otherwise */
  bool Absent (t_address) const;

  /** Print the Abstract Cache Set for debugging purpose */
  void Print () const;

  /** Update function when only one address is accessed */
  void Update (t_address);

  /** Update function when a set of addresses is accessed
      used by the data cache analysis
  */
  void Update (const set < t_address > &);

  /** Join function */
  void Join (const MAY &);

  /** returns true if this is equal to c and false otherwise */
  bool Equals (const MAY &) const;

};

/**************************************************
 *
 *  AbstractCacheSet PS
 *
 *************************************************/

class PS
{
 private:
  unsigned int nb_ways;		//the value corresponds to the number of ways to consider during the analysis
  //(cf. cache abstract factory in ICacheAnalysis.cc or DCacheAnalysis.cc)
  unsigned int nb_ways_removed;	//used in function GetAge and useful when a non LRU policy is used
  map < t_address, set < t_address > >contents;	//abstract cache sets
  set < t_address > evicted;	//used to reduce the size of the map (contents). No need to maintain the conflicts of evicted cache lines

 public:

  /** Constructor */
  explicit PS (unsigned int nbways, unsigned int nbways_removed);

  /** returns the age in the abstract cache of the cache line containing addr
      between [0..nb_ways-1] if present
      nb_ways+nb_ways_removed otherwise
  */
  unsigned int GetAge (t_address addr) const;

  /** returns true if the cache line containing addr is absent from the abstract cache set and false otherwise */
  bool Absent (t_address) const;

  /** Print the Abstract Cache Set for debugging purpose */
  void Print () const;

  /** Update function when only one address is accessed */
  void Update (t_address);

  /** Update function when a set of addresses is accessed
      used by the data cache analysis
  */
  void Update (const set < t_address > &);

  /** Join function */
  void Join (const PS &);

  /** returns true if this is equal to c and false otherwise */
  bool Equals (const PS &) const;

};

#endif
