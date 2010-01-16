/* CFArray.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
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

#include <Foundation/Foundation.h>

#include "CoreFoundation/CFArray.h"

//
// CFArray
//
void CFArrayApplyFunction (CFArrayRef theArray, CFRange range,
  CFArrayApplierFunction applier, void *context)
{
}

CFIndex CFArrayBSearchValues (CFArrayRef theArray, CFRange range,
  const void *value, CFComparatorFunction comparator, void *context)
{
  return 0;
}

Boolean CFArrayContainsValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  return ([theArray indexOfObject: (id)value
    inRange: range] != NSNotFound);
}

CFArrayRef CFArrayCreate (CFAllocatorRef allocator, const void **values,
  CFIndex numValues, const CFArrayCallBacks *callBacks)
{
  return [[NSArray allocWithZone: allocator] initWithObjects: (id *)values
                                                       count: numValues];
}

CFArrayRef CFArrayCreateCopy (CFAllocatorRef allocator, CFArrayRef theArray)
{
  return [theArray copyWithZone: allocator];
}

CFIndex CFArrayGetCount (CFArrayRef theArray)
{
  return [theArray count];
}

CFIndex CFArrayGetCountOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  return [theArray indexOfObject: (id)value inRange: range];
}

CFIndex CFArrayGetFirstIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
}

CFIndex CFArrayGetLastIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
}

CFTypeID CFArrayGetTypeID (void)
{
  return (CFTypeID)[NSArray class];
}

const void * CFArrayGetValueAtIndex (CFArrayRef theArray, CFIndex idx)
{
  return [theArray objectAtIndex: idx];
}

void CFArrayGetValues (CFArrayRef theArray, CFRange range, const void **values)
{
  [theArray getObjects: (id *)values range: range];
}



//
// CFMutableArray
//
void CFArrayAppendArray (CFMutableArrayRef theArray, CFArrayRef otherArray,
  CFRange otherRange)
{
}

void CFArrayAppendValue (CFMutableArrayRef theArray, const void *value)
{
  [theArray addObject: (id)value];
}

CFMutableArrayRef CFArrayCreateMutable (CFAllocatorRef allocator,
  CFIndex capacity, const CFArrayCallBacks *callBacks)
{
  return [[NSMutableArray allocWithZone: allocator]
    initWithCapacity: capacity];
}

CFMutableArrayRef CFArrayCreateMutableCopy (CFAllocatorRef allocator,
  CFIndex capacity, CFArrayRef theArray)
{
  return [theArray mutableCopyWithZone: allocator];
}

void CFArrayExchangeValuesAtIndices (CFMutableArrayRef theArray,
  CFIndex idx1, CFIndex idx2)
{
  [theArray exchangeObjectAtIndex: idx1 withObjectAtIndex: idx2];
}

void CFArrayInsertValueAtIndex (CFMutableArrayRef theArray,
  CFIndex idx, const void *value)
{
  [theArray insertObject: (id)value atIndex: idx];
}

void CFArrayRemoveAllValues (CFMutableArrayRef theArray)
{
  [theArray removeAllObjects];
}

void CFArrayRemoveValueAtIndex (CFMutableArrayRef theArray, CFIndex idx)
{
  [theArray removeObjectAtIndex: idx];
}

void CFArrayReplaceValues (CFMutableArrayRef theArray, CFRange range,
  const void **newValues, CFIndex newCount)
{
}

void CFArraySetValueAtIndex (CFMutableArrayRef theArray, CFIndex idx,
  const void *value)
{
  [theArray insertObject: (id)value atIndex: idx];
}

void CFArraySortValues (CFMutableArrayRef theArray, CFRange range,
  CFComparatorFunction comparator, void *context)
{
  [theArray sortUsingFunction: comparator context: context];
}
