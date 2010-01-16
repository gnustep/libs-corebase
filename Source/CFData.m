/* CFData.m
   
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

#include "CoreFoundation/CFData.h"

//
// CFData
//
CFTypeID CFDataGetTypeID (void)
{
  return (CFTypeID)[NSData class];
}

CFDataRef CFDataCreate (CFAllocatorRef allocator, const UInt8 *bytes,
  CFIndex length)
{
  return [[NSData allocWithZone: allocator] initWithBytes: bytes
                                                   length: length];
}

CFDataRef CFDataCreateCopy (CFAllocatorRef allocator, CFDataRef theData)
{
  return [theData copyWithZone: allocator];
}

CFDataRef CFDataCreateWithBytesNoCopy (CFAllocatorRef allocator,
  const UInt8 *bytes, CFIndex length, CFAllocatorRef bytesDeallocator)
{
  BOOL freeWhenDone = (bytesDeallocator == kCFAllocatorNull);
  
  return [[NSData allocWithZone: allocator]
    initWithBytesNoCopy: (const char *)bytes
                 length: length
           freeWhenDone: freeWhenDone];
}

const UInt8 * CFDataGetBytePtr (CFDataRef theData)
{
  return [theData bytes];
}

void CFDataGetBytes (CFDataRef theData, CFRange range, UInt8 *buffer)
{
  [theData getBytes: buffer range: range];
}

CFIndex CFDataGetLength (CFDataRef theData)
{
  return [theData length];
}



//
// CFMutableData
//
void CFDataAppendBytes (CFMutableDataRef theData, const UInt8 *bytes,
  CFIndex length)
{
  [theData appendBytes: bytes length: length];
}

CFMutableDataRef CFDataCreateMutable (CFAllocatorRef allocator,
  CFIndex capacity)
{
  return [[NSMutableData allocWithZone: allocator] initWithCapacity: capacity];
}

CFMutableDataRef CFDataCreateMutableCopy (CFAllocatorRef allocator,
  CFIndex capacity, CFDataRef theData)
{
  return [[theData mutableCopyWithZone: allocator] setCapacity: capacity];
}

void CFDataDeleteBytes (CFMutableDataRef theData, CFRange range)
{
  /* FIXME: is there a single NSMutableData method to use? */
}

UInt8 *CFDataGetMutableBytePtr (CFMutableDataRef theData)
{
  return (UInt8 *)[theData bytes];
}

void CFDataIncreaseLength (CFMutableDataRef theData, CFIndex extraLength)
{
  [theData increaseLengthBy: extraLength];
}

void CFDataReplaceBytes (CFMutableDataRef theData, CFRange range,
  const UInt8 *newBytes, CFIndex newLength)
{
  [theData replaceBytesInRange: range
                     withBytes: newBytes
                        length: newLength];
}

void CFDataSetLength (CFMutableDataRef theData, CFIndex length)
{
  [theData setLength: length];
}
