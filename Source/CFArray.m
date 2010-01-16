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

#import <Foundation/NSArray.h>

#include "CoreFoundation/CFArray.h"


//
// CFArray
//
void CFArrayApplyFunction (CFArrayRef theArray, CFRange range,
  CFArrayApplierFunction applier, void *context)
{
  CFIndex i;

  for (i = range.location; i < range.location + range.length; i++)
    {
      applier(CFArrayGetValueAtIndex(theArray, i), context);
    }
}

CFIndex CFArrayBSearchValues (CFArrayRef theArray, CFRange range,
  const void *value, CFComparatorFunction comparator, void *context)
{
  // FIXME
  return 0;
}

Boolean CFArrayContainsValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  return (CFArrayGetFirstIndexOfValue(theArray, range, value) != -1);
}

CFArrayRef CFArrayCreate (CFAllocatorRef allocator, const void **values,
  CFIndex numValues, const CFArrayCallBacks *callBacks)
{
  // FIXME
  return (CFArrayRef)[[NSArray allocWithZone: allocator] 
                       initWithObjects: (id *)values
                                 count: numValues];
}

CFArrayRef CFArrayCreateCopy (CFAllocatorRef allocator, CFArrayRef theArray)
{
  return (CFArrayRef)[(NSArray*)theArray copyWithZone: allocator];
}

CFIndex CFArrayGetCount (CFArrayRef theArray)
{
  return [(NSArray*)theArray count];
}

CFIndex CFArrayGetCountOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  CFIndex count = 0;
  CFIndex i;

  while (( i = CFArrayGetFirstIndexOfValue(theArray, range, value)) != -1)
    {
      count++;
      range.length -= range.location - i - 1;
      range.location = i + 1;
    }
  return count;
}

CFIndex CFArrayGetFirstIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  return [(NSArray*)theArray indexOfObject: (id)value 
                                   inRange: NSMakeRange(range.location, range.length)];
}

CFIndex CFArrayGetLastIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value)
{
  // FIXME
}

CFTypeID CFArrayGetTypeID (void)
{
  return (CFTypeID)[NSArray class];
}

const void * CFArrayGetValueAtIndex (CFArrayRef theArray, CFIndex idx)
{
  return (void*)[(NSArray*)theArray objectAtIndex: idx];
}

void CFArrayGetValues (CFArrayRef theArray, CFRange range, const void **values)
{
  [(NSArray*)theArray getObjects: (id *)values 
                           range: NSMakeRange(range.location, range.length)];
}



//
// CFMutableArray
//
void CFArrayAppendArray (CFMutableArrayRef theArray, CFArrayRef otherArray,
                         CFRange otherRange)
{
  [(NSMutableArray*)theArray replaceObjectsInRange: 
                      NSMakeRange([(NSMutableArray*)theArray count], 0)
                              withObjectsFromArray: (NSMutableArray*)otherArray
                                             range: NSMakeRange(otherRange.location, otherRange.length)];
}

void CFArrayAppendValue (CFMutableArrayRef theArray, const void *value)
{
  [(NSMutableArray*)theArray addObject: (id)value];
}

CFMutableArrayRef CFArrayCreateMutable (CFAllocatorRef allocator,
                                        CFIndex capacity, const CFArrayCallBacks *callBacks)
{
  // FIXME
  return (CFMutableArrayRef)[[NSMutableArray allocWithZone: allocator]
                              initWithCapacity: capacity];
}

CFMutableArrayRef CFArrayCreateMutableCopy (CFAllocatorRef allocator,
                                            CFIndex capacity, CFArrayRef theArray)
{
  return (CFMutableArrayRef)[(NSArray*)theArray mutableCopyWithZone: allocator];
}

void CFArrayExchangeValuesAtIndices (CFMutableArrayRef theArray,
                                     CFIndex idx1, CFIndex idx2)
{
  [(NSMutableArray*)theArray exchangeObjectAtIndex: idx1 
                                 withObjectAtIndex: idx2];
}

void CFArrayInsertValueAtIndex (CFMutableArrayRef theArray,
                                CFIndex idx, const void *value)
{
  [(NSMutableArray*)theArray insertObject: (id)value atIndex: idx];
}

void CFArrayRemoveAllValues (CFMutableArrayRef theArray)
{
  [(NSMutableArray*)theArray removeAllObjects];
}

void CFArrayRemoveValueAtIndex (CFMutableArrayRef theArray, CFIndex idx)
{
  [(NSMutableArray*)theArray removeObjectAtIndex: idx];
}

void CFArrayReplaceValues (CFMutableArrayRef theArray, CFRange range,
                           const void **newValues, CFIndex newCount)
{
  // FIXME
}

void CFArraySetValueAtIndex (CFMutableArrayRef theArray, CFIndex idx,
                             const void *value)
{
  [(NSMutableArray*)theArray insertObject: (id)value atIndex: idx];
}

void CFArraySortValues (CFMutableArrayRef theArray, CFRange range,
                        CFComparatorFunction comparator, void *context)
{
  [(NSMutableArray*)theArray 
      sortUsingFunction: (NSComparisonResult(*)(id,id,void*))comparator
      context: context];
}
