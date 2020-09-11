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

/* 
 * File:   cow_ptr.h
 * Author: blesage
 *
 * Created on 29 november 2010, 15:03
 *
 * Define a copy-on-write pointer.
 * A cow_ptr<T> holds a pointer to an instance of an object of type T. When copied, a cow_ptr shares the 
 * instance it points to with its copy. Upon write access to the object kept by a cow_ptr, if this 
 * instance is shared among multiple cow_ptr, a copy of the object is first performed and used for the 
 * write. Finally, when destroyed a cow_ptr destroys the object it points to if it is the last reference
 * to said object.
 * This means auto-deallocation of pointers and transparent ownership sharing to reduce memory footprint, 
 * when appliable.
 */

#ifndef COW_PTR_H
#define COW_PTR_H

//#include <boost/shared_ptr.hpp>

template < class T > class heptane_shared_ptr
{
public:
  typedef T type_name;

  explicit heptane_shared_ptr ():ref_cnt_ (NULL), p_ (NULL)
  {
  };
  explicit heptane_shared_ptr (T * p):ref_cnt_ (new size_t ()), p_ (p)
  {
    *ref_cnt_ = 1;
  };
  explicit heptane_shared_ptr (const heptane_shared_ptr < T > &rhs):ref_cnt_ (NULL), p_ (NULL)
  {
    if (!rhs.empty ())
      {
	ref_cnt_ = rhs.ref_cnt_;
	p_ = rhs.p_;

	++(*ref_cnt_);
      }
  };

  ~heptane_shared_ptr ()
  {
    release ();
  }

  inline bool unique () const
  {
    return (!empty () && *ref_cnt_ == 1);
  }

  inline const T *get () const
  {
    return p_;
  }

  inline T *get ()
  {
    return p_;
  }

  inline void reset (T * p)
  {
    release ();

    ref_cnt_ = new size_t (1);

    p_ = p;
  }

  inline bool operator== (const heptane_shared_ptr < T > &rhs) const
  {
    return get () == rhs.get ();
  };
  inline bool operator!= (const heptane_shared_ptr < T > &rhs) const
  {
    return !((*this) == rhs);
  };

  inline const T & operator* () const
  {
    return *get ();
  };
  inline const T *operator-> () const
  {
    return get ();
  };

  inline T & operator* ()
  {
    return *get ();
  };
  inline T *operator-> ()
  {
    return get ();
  };

  inline heptane_shared_ptr < T > &operator= (const heptane_shared_ptr < T > &rhs)
  {
    if (this != &rhs && get () != rhs.get ())
      {
	release ();
	ref_cnt_ = rhs.ref_cnt_;
	p_ = rhs.p_;

	++(*ref_cnt_);
      }
    return *this;
  };

private:
  size_t * ref_cnt_;
  T *p_;

  void release ()
  {
    if (unique ())
      {
	delete p_;
	delete ref_cnt_;
      }
    else if (!empty ())
      {
	--(*ref_cnt_);
      }

    ref_cnt_ = NULL;
    p_ = NULL;
  }

  inline bool empty () const
  {
    return (ref_cnt_ == NULL);
  }

};

template < class T > class cow_ptr
{
public:
  typedef T type_name;

  explicit cow_ptr ():p_ ()
  {
  };
  explicit cow_ptr (T * p):p_ (p)
  {
  };
  explicit cow_ptr (const T & rhs):p_ (new T (rhs))
  {
  };
  cow_ptr (const cow_ptr < T > &rhs)
  {
    this->p_ = rhs.p_;
  };

  cow_ptr < T > &operator= (const cow_ptr < T > &rhs)
  {
    this->p_ = rhs.p_;
    return *this;
  };
  cow_ptr < T > &operator= (const T & rhs)
  {
    this->p_.reset (new T (rhs));
    return *this;
  };

  inline const T & operator* () const
  {
    return *p_;
  };
  inline const T *operator-> () const
  {
    return p_.get ();
  };

  inline T & operator* ()
  {
    copy ();
    return *p_;
  };
  inline T *operator-> ()
  {
    copy ();
    return p_.get ();
  };

  bool operator== (const cow_ptr < T > &rhs) const
  {
    return rhs.p_ == p_ || *p_ == *(rhs.p_);
  };

private:
  heptane_shared_ptr < T > p_;

  void copy ()
  {
    if (!(p_.unique () || p_.get () == 0))
      {
	T *t = new T (*p_);
	this->p_.reset (t);
      }
  };
};

#endif
