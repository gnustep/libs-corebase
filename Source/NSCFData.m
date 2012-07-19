/* NSCFData.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#include <Foundation/NSObject.h>
#include <Foundation/NSData.h>

#include "NSCFType.h"
#include "CoreFoundation/CFData.h"

/* NSCFData inherits from NSMutableData and doesn't have any ivars
   because it is only an ObjC wrapper around CFData. */
@interface NSCFData : NSMutableData
@end

@implementation NSCFData
+ (void) load
{
  NSCFInitialize ();
}

- (const void *) bytes
{
  return CFDataGetBytePtr (self);
}

- (NSString *) description
{
  return AUTORELEASE(CFCopyDescription(self));
}

- (void) getBytes: (void *) buffer range: (NSRange) range
{
  CFDataGetBytes (self, CFRangeMake(range.location, range.length), buffer);
}

- (BOOL) isEqualToData: (NSData *) other
{
  return (BOOL)CFEqual (self, other);
}

- (NSUInteger) length
{
  return (NSUInteger)CFDataGetLength(self);
}

- (void) increaseLengthBy: (NSUInteger) extraLength
{
  CFDataIncreaseLength (self, (CFIndex)extraLength);
}

- (void) setLength: (NSUInteger) length
{
  CFDataSetLength (self, (CFIndex)length);
}

- (void *) mutableBytes
{
  return CFDataGetMutableBytePtr (self);
}

- (void) appendBytes: (const void *) buffer length: (NSUInteger) size
{
  CFDataAppendBytes (self, buffer, size);
}

- (void) replaceBytesInRange: (NSRange) range
                   withBytes: (const void *) bytes
                      length: (NSUInteger) length
{
  CFDataReplaceBytes (self, CFRangeMake(range.location, range.length), bytes,
    (CFIndex)length);
}

- (CFTypeID) _cfTypeID
{
  return CFDataGetTypeID ();
}
@end

