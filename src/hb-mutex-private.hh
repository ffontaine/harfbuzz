/*
 * Copyright © 2007  Chris Wilson
 * Copyright © 2009,2010  Red Hat, Inc.
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Contributor(s):
 *	Chris Wilson <chris@chris-wilson.co.uk>
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_MUTEX_PRIVATE_HH
#define HB_MUTEX_PRIVATE_HH

#include "hb-private.hh"

HB_BEGIN_DECLS


/* mutex */

/* We need external help for these */

#ifdef HAVE_GLIB

#include <glib.h>

typedef GStaticMutex hb_mutex_impl_t;
#define HB_MUTEX_IMPL_INIT	G_STATIC_MUTEX_INIT
#define hb_mutex_impl_init(M)	g_static_mutex_init (M)
#define hb_mutex_impl_lock(M)	g_static_mutex_lock (M)
#define hb_mutex_impl_unlock(M)	g_static_mutex_unlock (M)
#define hb_mutex_impl_free(M)	g_static_mutex_free (M)


#elif defined(_MSC_VER)

#include <Windows.h>

typedef CRITICAL_SECTION hb_mutex_impl_t;
#define HB_MUTEX_IMPL_INIT	{ NULL, 0, 0, NULL, NULL, 0 }
#define hb_mutex_impl_init(M)	InitializeCriticalSection (M)
#define hb_mutex_impl_lock(M)	EnterCriticalSection (M)
#define hb_mutex_impl_unlock(M)	LeaveCriticalSection (M)
#define hb_mutex_impl_free(M)	DeleteCriticalSection (M)


#else

#warning "Could not find any system to define platform macros, library will NOT be thread-safe"

typedef volatile int hb_mutex_impl_t;
#define HB_MUTEX_IMPL_INIT	0
#define hb_mutex_impl_init(M)	((void) (*(M) = 0))
#define hb_mutex_impl_lock(M)	((void) (*(M) = 1))
#define hb_mutex_impl_unlock(M)	((void) (*(M) = 0))
#define hb_mutex_impl_free(M)	((void) (*(M) = 2))


#endif


struct hb_mutex_t
{
  hb_mutex_impl_t m;

  inline void init   (void) { hb_mutex_impl_init   (&m); }
  inline void lock   (void) { hb_mutex_impl_lock   (&m); }
  inline void unlock (void) { hb_mutex_impl_unlock (&m); }
  inline void free   (void) { hb_mutex_impl_free   (&m); }
};

#define HB_MUTEX_INIT		{HB_MUTEX_IMPL_INIT}
#define hb_mutex_init(M)	(M)->init ()
#define hb_mutex_lock(M)	(M)->lock ()
#define hb_mutex_unlock(M)	(M)->unlock ()
#define hb_mutex_free(M)	(M)->free ()


struct hb_static_mutex_t : hb_mutex_t
{
  hb_static_mutex_t (void)  { this->init (); }
  ~hb_static_mutex_t (void) { this->free (); }

  private:
  NO_COPY (hb_static_mutex_t);
};


HB_END_DECLS


template <typename item_t>
struct hb_static_threadsafe_set_t
{
  hb_lockable_set_t <item_t, hb_static_mutex_t> set;
  hb_static_mutex_t lock;

  template <typename T>
  inline item_t *replace_or_insert (T v)
  {
    return set.replace_or_insert (v, lock);
  }

  template <typename T>
  inline void remove (T v)
  {
    set.remove (v, lock);
  }

  template <typename T>
  inline bool find (T v, item_t *i)
  {
    return set.find (v, i, lock);
  }

  template <typename T>
  inline item_t *find_or_insert (T v)
  {
    return set.find_or_insert (v, lock);
  }

  void finish (void)
  {
    set.finish (lock);
  }
};


HB_BEGIN_DECLS

HB_END_DECLS

#endif /* HB_MUTEX_PRIVATE_HH */
