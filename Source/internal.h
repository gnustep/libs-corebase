/* internal.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: June, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFBase.h"
#include <pthread.h>

/* Should be fairly easy to use CriticalSections on Windows instead of
   pthread_mutex_t. */

#define _mutex_t pthread_mutex_t

CF_INLINE void
_mutex_init (_mutex_t *mutex)
{
  pthread_mutex_init (mutex, NULL);
}

CF_INLINE void
_mutex_lock (_mutex_t *mutex)
{
  pthread_mutex_lock (mutex);
}

CF_INLINE void
_mutex_unlock (_mutex_t *mutex)
{
  pthread_mutex_unlock (mutex);
}
