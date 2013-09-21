/* GSMemory.h
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: August, 2013
   
   This file is part of GNUstep CoreBase library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "config.h"

#ifndef GSMEMORY_H
#define GSMEMORY_H

#include "CoreFoundation/CFBase.h"
#if HOST_OS_WINDOWS
#include <windows.h>
#else
#include <string.h>
#endif

CF_INLINE void *
GSMemoryCopy (void *dst, const void *src, CFIndex len)
{
#if HOST_OS_WINDOWS
  CopyMemory (dst, src, len);
  return dst;
#else
  return memcpy (dst, src, len);
#endif
}

CF_INLINE void *
GSMemoryMove (void *dst, const void *src, CFIndex len)
{
#if HOST_OS_WINDOWS
  MoveMemory (dst, src, len);
  return dst;
#else
  return memmove (dst, src, len);
#endif
}

CF_INLINE void *
GSMemorySet (void *dst, int c, CFIndex len)
{
#if HOST_OS_WINDOWS
  FillMemory (dst, len, c);
  return dst;
#else
  return memset (dst, c, len);
#endif
}

CF_INLINE void *
GSMemoryZero (void *dst, CFIndex len)
{
#if HOST_OS_WINDOWS
  ZeroMemory (dst, len);
  return dst;
#else
  return memset (dst, 0, len);
#endif
}

#endif /* GSMEMORY_H */
