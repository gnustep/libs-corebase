/* CFBinaryHeap.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include "CoreFoundation/CFBinaryHeap.h"

static CFTypeID _kCFBinaryHeapTypeID = 0;

CFTypeID
CFBinaryHeapGetTypeID (void)
{
  return _kCFBinaryHeapTypeID;
}

CFBinaryHeapRef
CFBinaryHeapCreate (CFAllocatorRef allocator, CFIndex capacity,
  const CFBinaryHeapCallBacks *callBacks,
  const CFBinaryHeapCompareContext *compareContext)
{
  return NULL;
}

CFBinaryHeapRef
CFBinaryHeapCreateCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFBinaryHeapRef heap)
{
  return NULL;
}

void
CFBinaryHeapAddValue (CFBinaryHeapRef heap, const void *value)
{
  
}

void
CFBinaryHeapApplyFunction (CFBinaryHeapRef heap,
  CFBinaryHeapApplierFunction applier, void *context)
{
  
}

Boolean
CFBinaryHeapContainsValue (CFBinaryHeapRef heap, const void *value)
{
  return false;
}

CFIndex
CFBinaryHeapGetCount (CFBinaryHeapRef heap)
{
  return 0;
}

CFIndex
CFBinaryHeapGetCountOfValue (CFBinaryHeapRef heap, const void *value)
{
  return 0;
}

const void *
CFBinaryHeapGetMinimum (CFBinaryHeapRef heap)
{
  return NULL;
}

Boolean
CFBinaryHeapGetMinimumIfPresent (CFBinaryHeapRef heap, const void **value)
{
  return false;
}

void
CFBinaryHeapGetValues (CFBinaryHeapRef heap, const void **values)
{
  
}

void
CFBinaryHeapRemoveAllValues (CFBinaryHeapRef heap)
{
  
}

void
CFBinaryHeapRemoveMinimumValue (CFBinaryHeapRef heap)
{
  
}
