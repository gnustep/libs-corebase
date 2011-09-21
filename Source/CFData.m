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

#import <Foundation/NSData.h>

#include "CoreFoundation/CFData.h"

//
// CFData
//
CFTypeID
CFDataGetTypeID (void)
{
  return (CFTypeID)[NSData class];
}

CFDataRef
CFDataCreate (CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length)
{
  return (CFDataRef)[[NSData alloc] initWithBytes: bytes
                                           length: length];
}

CFDataRef
CFDataCreateCopy (CFAllocatorRef allocator, CFDataRef theData)
{
  return (CFDataRef)[(NSData*)theData copy];
}

CFDataRef
CFDataCreateWithBytesNoCopy (CFAllocatorRef allocator, const UInt8 *bytes,
                             CFIndex length, CFAllocatorRef bytesDeallocator)
{
  BOOL freeWhenDone = (bytesDeallocator == kCFAllocatorNull);
  
  return (CFDataRef)[[NSData alloc] initWithBytesNoCopy: (void*)bytes
                                                 length: length
                                           freeWhenDone: freeWhenDone];
}

const UInt8 *
CFDataGetBytePtr (CFDataRef theData)
{
  return [(NSData*)theData bytes];
}

void
CFDataGetBytes (CFDataRef theData, CFRange range, UInt8 *buffer)
{
  [(NSData*)theData getBytes: buffer 
                       range: NSMakeRange(range.location, range.length)];
}

CFIndex
CFDataGetLength (CFDataRef theData)
{
  return [(NSData*)theData length];
}



//
// CFMutableData
//
void
CFDataAppendBytes (CFMutableDataRef theData, const UInt8 *bytes, CFIndex length)
{
  [(NSMutableData*)theData appendBytes: bytes length: length];
}

CFMutableDataRef
CFDataCreateMutable (CFAllocatorRef allocator, CFIndex capacity)
{
  return (CFMutableDataRef)[[NSMutableData alloc] initWithCapacity: capacity];
}

CFMutableDataRef
CFDataCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                         CFDataRef theData)
{
  return (CFMutableDataRef)[[(NSMutableData*)theData mutableCopy] 
                             setCapacity: capacity];
}

void
CFDataDeleteBytes (CFMutableDataRef theData, CFRange range)
{
  CFDataReplaceBytes(theData, range, NULL, 0);
}

UInt8 *
CFDataGetMutableBytePtr (CFMutableDataRef theData)
{
  return (UInt8 *)[(NSMutableData*)theData bytes];
}

void
CFDataIncreaseLength (CFMutableDataRef theData, CFIndex extraLength)
{
  [(NSMutableData*)theData increaseLengthBy: extraLength];
}

void
CFDataReplaceBytes (CFMutableDataRef theData, CFRange range,
                    const UInt8 *newBytes, CFIndex newLength)
{
  [(NSMutableData*)theData replaceBytesInRange: NSMakeRange(range.location, range.length)
                                     withBytes: newBytes
                                        length: newLength];
}

void
CFDataSetLength (CFMutableDataRef theData, CFIndex length)
{
  [(NSMutableData*)theData setLength: length];
}
