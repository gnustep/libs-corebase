/* threading.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
   This file is part of GNUstep CoreBase library.
   
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

#ifndef __THREADING_H__
#define __THREADING_H__ 1

#if defined(_WIN32)

#include <windows.h>

#define CFMutex CRITICAL_SECTION
#define CFMutexInitialize(x) InitializeCriticalSection(x)
#define CFMutexLock(x) EnterCriticalSection(x)
#define CFMutexUnlock(x) LeaveCriticalSection(x)
#define CFMutexDestroy(x) DeleteCriticalSection(x)

#else /* _WIN32 */

#include <pthread.h>

#define CFMutex pthread_mutex_t
#define CFMutexInitialize(x) pthread_mutex_init(x, NULL)
#define CFMutexLock(x) pthread_mutex_lock(x)
#define CFMutexUnlock(x) pthread_mutex_unlock(x)
#define CFMutexDestroy(x) pthraed_mutex_destroy(x)

#endif

#endif /* __THREADING_H__ */