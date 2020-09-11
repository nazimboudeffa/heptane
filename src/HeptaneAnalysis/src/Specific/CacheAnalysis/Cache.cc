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

/**
 *
 * AbstractCacheSet implementation for the MAY, MUST, and PS
 *
 */


#include <iostream>
#include <map>
#include <utility>
#include <cassert>

#include "Cache.h"

using namespace std;

/**************************************************
 *
 *  MUST implementation
 *
 *************************************************/

/** Constructor */
MUST::MUST (unsigned int nbways, unsigned int nbways_removed)
{
  nb_ways = nbways;
  nb_ways_removed = nbways_removed;
  contents.resize (nb_ways);
}

/** returns true if the cache line containing addr is absent from the abstract cache set and false otherwise */
bool
MUST::Absent (t_address addr) const
{
  return GetAge (addr) >= nb_ways;
}

/** returns the age in the abstract cache of the cache line containing addr
    between [0..nb_ways-1] if present
    nb_ways+nb_ways_removed otherwise
*/
unsigned int
MUST::GetAge (t_address addr) const
{
  for (unsigned int i = 0; i < nb_ways; i++)
    {
      if (contents[i].find (addr) != contents[i].end ()) { return i; }
    }
  return nb_ways + nb_ways_removed;
}

/** Print the Abstract Cache Set for debugging purpose */
void
MUST::Print () const
{
  cout << hex;

  for (unsigned int i = 0; i < nb_ways; i++)
    {
      cout << "{";
      unsigned int cpt = 0;
      for (set < t_address >::const_iterator iter = contents[i].begin (); iter != contents[i].end (); iter++)
	{
	  cout << *iter;
	  if (cpt + 1 != contents[i].size ()) { cout << ","; }
	  cpt++;
	}
      cout << "}";
      if (i + 1 < nb_ways) { cout << " | "; }
    }
  cout << endl << dec;
}

/** Update function when only one address is accessed */
void
MUST::Update (t_address addr)
{
  assert (nb_ways > 0);
  assert (contents[0].size () <= 1);

  if (contents[0].find (addr) != contents[0].end ()) { return; } //nothing change in the set addr is alone in the first way

  bool found = false;
  int pos = nb_ways;
  for (unsigned int i = 1; i < nb_ways && !found; i++)
    {
      if (contents[i].erase (addr) == 1)
	{
	  found = true;
	  pos = i;
	}
    }

  //The contents of way pos is merged with pos-1
  if (found) { contents[pos].insert (contents[pos - 1].begin (), contents[pos - 1].end ()); }

  //update the age
  for (int i = pos - 1; i > 0; i--) 
    { 
      contents[i] = contents[i - 1]; 
    }

  //insert addr as MRU
  contents[0].clear ();
  contents[0].insert (addr);
}

/** Update function when a set of addresses is accessed
    used by the data cache analysis
*/
void
MUST::Update (const set < t_address > &addrs)
{
  assert (nb_ways > 0);
  assert (contents[0].size () <= 1);

  unsigned int max_age = 0;
  // Find max_age: the age of the oldest accessed block.
  for (set < t_address >::const_iterator it = addrs.begin (); it != addrs.end () && max_age < nb_ways; it++)
    {
      max_age = max (max_age, GetAge (*it));
    }

  // only one line in addrs at the MRU position (the cache set is unchanged)
  if (max_age == 0) { return; } 

  if (max_age < nb_ways) { contents[max_age].insert (contents[max_age - 1].begin (), contents[max_age - 1].end ()); }

  //update the age
  for (unsigned int i = min (max_age, nb_ways) - 1; i > 0; i--)
    {
      contents[i] = contents[i - 1];
    }

  //clear the MRU position
  contents[0].clear ();
}

/** Join function */
void
MUST::Join (const MUST & c)
{
  assert (nb_ways == c.nb_ways);

  set < t_address > intersect;

  //build the resulting ACS
  vector < set < t_address > >result;
  result.resize (nb_ways);

  //compute the addr present in both ACS and keep there maximal age
  for (unsigned int i = 0; i < nb_ways; i++)
    {
      for (set < t_address >::const_iterator iter = contents[i].begin (); iter != contents[i].end (); iter++)
	{
	  set < t_address >::iterator search = intersect.find (*iter);

	  if (search != intersect.end ())	//already in the other cache, thus i is the maximal age
	    {
	      result[i].insert (*iter);
	      intersect.erase (search);
	    }
	  else
	    {
	      intersect.insert (*iter);
	    }
	}

      for (set < t_address >::const_iterator iter = c.contents[i].begin (); iter != c.contents[i].end (); iter++)
	{
	  set < t_address >::iterator search = intersect.find (*iter);

	  if (search != intersect.end ())	//already in the other cache, thus i is the maximal age
	    {
	      result[i].insert (*iter);
	      intersect.erase (search);
	    }
	  else
	    {
	      intersect.insert (*iter);
	    }
	}
    }

  contents.swap (result);
}

/** returns true if this is equal to c and false otherwise */
bool
MUST::Equals (const MUST & c) const
{
  assert (nb_ways == c.nb_ways);
  return this->contents == c.contents;
}

/**************************************************
 *
 *  MAY implementation
 *
 *************************************************/

/** Constructor */
MAY::MAY (unsigned int nbways)
{
  nb_ways = nbways;
  contents.resize (nb_ways);
}

/** returns true if the cache line containing addr is absent from the abstract cache set and false otherwise */
bool
MAY::Absent (t_address addr) const
{
  for (unsigned int i = 0; i < nb_ways; i++)
    {
      if (contents[i].find (addr) != contents[i].end ()) { return false; }
    }
  return true;
}

/** Print the Abstract Cache Set for debugging purpose */
void
MAY::Print () const
{
  cout << hex;

  for (unsigned int i = 0; i < nb_ways; i++)
    {
      cout << "{";
      unsigned int cpt = 0;
      for (set < t_address >::iterator iter = contents[i].begin (); iter != contents[i].end (); iter++)
	{
	  cout << *iter;
	  if (cpt + 1 != contents[i].size ()) { cout << ","; }
	  cpt++;
	}
      cout << "}";
      if (i + 1 < nb_ways) { cout << " | "; }
    }
  cout << endl << dec;
}

/** Update function when only one address is accessed */
void
MAY::Update (t_address addr)
{
  bool found = false;
  int pos = nb_ways;

  // Find the position of addr in the cache and remove it.
  for (unsigned int i = 0; i < nb_ways && !found; i++)
    {
      if (contents[i].erase (addr) == 1)
	{
	  found = true;
	  pos = i;
	}
    }

  // Shift lines from the MRU to the position of addr in cache, or the LRU.
  if (pos == 0)			//addr was found
    {
      if (nb_ways > 1 && contents[0].size () != 0)
	{
	  contents[1].insert (contents[0].begin (), contents[0].end ());
	}
    }
  else
    {
      if (found)
	{
	  contents[pos].insert (contents[pos - 1].begin (), contents[pos - 1].end ());
	}

      for (int i = pos - 1; i > 0; --i)
	{
	  contents[i] = contents[i - 1];
	}
    }

  // Set addr as the most recently used block.
  contents[0].clear ();
  contents[0].insert (addr);
}

/** Update function when a set of addresses is accessed
    used by the data cache analysis
*/
void
MAY::Update (const set < t_address > &addrs)
{
  //remove addrs from the abstractCacheSet
  for (set < t_address >::const_iterator it = addrs.begin (); it != addrs.end (); it++)
    {
      for (unsigned int i = 0; i < nb_ways; i++)
	{
	  if (contents[i].erase (*it) == 1)
	    {
	      break;
	    }
	}
    }

  //add addrs at age 0
  contents[0].insert (addrs.begin (), addrs.end ());
}

/** Join function */
void
MAY::Join (const MAY & c)
{
  assert (nb_ways == c.nb_ways);

  set < t_address > intersect;

  //build the resulting ACS
  vector < set < t_address > >result;
  result.resize (nb_ways);

  //compute the addr present in both ACS and keep there minimal age
  for (unsigned int i = 0; i < nb_ways; i++)
    {
      for (set < t_address >::const_iterator iter = contents[i].begin (); iter != contents[i].end (); iter++)
	{
	  set < t_address >::iterator search = intersect.find (*iter);
	  if (search == intersect.end ())	//not present already thus i is the minimal age
	    {
	      result[i].insert (*iter);
	      intersect.insert (*iter);
	    }
	  else
	    {
	      intersect.erase (search);
	    }
	}

      for (set < t_address >::const_iterator iter = c.contents[i].begin (); iter != c.contents[i].end (); iter++)
	{
	  set < t_address >::iterator search = intersect.find (*iter);
	  if (search == intersect.end ())	//not present already thus i is the minimal age
	    {
	      result[i].insert (*iter);
	      intersect.insert (*iter);
	    }
	  else
	    {
	      intersect.erase (search);
	    }
	}
    }

  contents.swap (result);
}

/** returns true if this is equal to c and false otherwise */
bool
MAY::Equals (const MAY & c) const
{
  assert (nb_ways == c.nb_ways);
  return this->contents == c.contents;
}


/**************************************************
 *
 *  PS implementation
 *
 *************************************************/

/** Constructor */
PS::PS (unsigned int nbways, unsigned int nbways_removed)
{
  nb_ways = nbways;
  nb_ways_removed = nbways_removed;
}

/** returns true if the cache line containing addr is absent from the abstract cache set and false otherwise */
bool
PS::Absent (t_address addr) const
{
  map < t_address, set < t_address > >::const_iterator it_this = contents.find (addr);
  if (it_this == contents.end ())
    {
      return true;
    }
  assert (it_this->second.size () < nb_ways);	//check for evicted
  return false;
}

/** returns the age in the abstract cache of the cache line containing addr
    between [0..nb_ways-1] if present
    nb_ways+nb_ways_removed otherwise
*/
unsigned int
PS::GetAge (t_address addr) const
{
  map < t_address, set < t_address > >::const_iterator it_this = contents.find (addr);
  if (it_this == contents.end ())
    {
      return nb_ways;
    }
  assert (it_this->second.size () < nb_ways);	//check for evicted
  return it_this->second.size () + nb_ways_removed;
}

/** Print the Abstract Cache Set for debugging purpose */
void
PS::Print () const
{
  for (map < t_address, set < t_address > >::const_iterator it = contents.begin (); it != contents.end (); it++)
    {
      if (it != contents.begin ())
	{
	  cout << "|";
	}
      cout << "{" << hex << it->first << dec << ";" << it->second.size () << "}";	//{addr;#conflict}
    }

  cout << " | [";
  for (set < t_address >::const_iterator it = evicted.begin (); it != evicted.end (); it++)
    {
      if (it != evicted.begin ())
	{
	  cout << ",";
	}
      cout << *it;
    }

  cout << "]" << endl;
}

/** Update function when only one address is accessed */
void
PS::Update (t_address addr)
{
  set < t_address > to_evict;

  for (map < t_address, set < t_address > >::iterator it_this = contents.begin (); it_this != contents.end (); it_this++)
    {
      it_this->second.insert (addr);
      if (it_this->second.size () == nb_ways)
	{
	  to_evict.insert (it_this->first);
	}
    }

  for (set < t_address >::const_iterator it = to_evict.begin (); it != to_evict.end (); it++)
    {
      contents.erase (*it);
      evicted.insert (*it);
    }

  contents[addr].clear ();
  evicted.erase (addr);
}

/** Update function when a set of addresses is accessed
    used by the data cache analysis
*/
void
PS::Update (const set < t_address > &addrs)
{
  set < t_address > absent = addrs;	//used to determine the addrs not already present in the map
  set < t_address > to_evict;	//use to determine the addrs evicted by this access 

  //add the conflicts to all the addresses present in the cache before the access 
  for (map < t_address, set < t_address > >::iterator it_this = contents.begin (); it_this != contents.end (); it_this++)
    {
      it_this->second.insert (addrs.begin (), addrs.end ());

      if (addrs.find (it_this->first) != addrs.end ())
	{
	  it_this->second.erase (it_this->first);
	  absent.erase (it_this->first);
	}

      if (it_this->second.size () >= nb_ways)
	{
	  to_evict.insert (it_this->first);
	}
    }

  //remove from absent addrs which are in the evicted set
  for (set < t_address >::const_iterator it = evicted.begin (); it != evicted.end () && absent.size () > 0; it++)
    {
      absent.erase (*it);
    }

  //insert addresses of addrs not present in the cache before this access
  for (set < t_address >::iterator it = absent.begin (); it != absent.end (); it++)
    {
      contents[*it].insert (addrs.begin (), addrs.end ());
      contents[*it].erase (*it);
      if (contents[*it].size () >= nb_ways)
	{
	  to_evict.insert (*it);
	}
    }

  //manage eviction
  evicted.insert (to_evict.begin (), to_evict.end ());
  for (set < t_address >::const_iterator it = to_evict.begin (); it != to_evict.end (); it++)
    {
      contents.erase (*it);
    }
}

/** Join function */
void
PS::Join (const PS & c)
{
  assert (nb_ways == c.nb_ways);

  //merge contents
  for (map < t_address, set < t_address > >::const_iterator it_c = c.contents.begin (); it_c != c.contents.end (); it_c++)
    {
      if (evicted.find (it_c->first) != evicted.end ())	{ continue; }			//if in this->evicted no need to insert it
      contents[it_c->first].insert (it_c->second.begin (), it_c->second.end ());
      map < t_address, set < t_address > >::iterator it_this = contents.find (it_c->first);
      if (it_this->second.size () >= nb_ways)
	{
	  evicted.insert (it_this->first);
	  contents.erase (it_this);
	}
    }

  //erase all c.evicted from this->contents
  for (set < t_address >::const_iterator it = c.evicted.begin (); it != c.evicted.end (); it++)
    {
      contents.erase (*it);
    }

  //merge evicted sets
  evicted.insert (c.evicted.begin (), c.evicted.end ());
}

/** returns true if this is equal to c and false otherwise */
bool
PS::Equals (const PS & c) const
{
  assert (nb_ways == c.nb_ways);
  return this->contents == c.contents && this->evicted == c.evicted;
}
