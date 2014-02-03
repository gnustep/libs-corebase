/* NSCFInputStream.m
   
   Copyright (C) 2014 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: February, 2014
   
   This file is part of GNUstep CoreBase Library.
   
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

#import <Foundation/NSStream.h>

#include "NSCFType.h"
#include "CoreFoundation/CFStream.h"

@interface NSCFInputStream : NSInputStream
NSCFTYPE_VARS
@end

@interface NSInputStream (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;

@end

@implementation NSCFInputStream
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len
{
  return CFReadStreamRead((CFReadStreamRef) self, buffer, len);
}

- (BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)length
{
  const UInt8* buf;
  CFIndex out;

  buf = CFReadStreamGetBuffer((CFReadStreamRef) self, *length, &out);
  *length = out;

  *buffer = (uint8_t *) buf;
  return buf != NULL;
}

- (BOOL)hasBytesAvailable
{
  return CFReadStreamHasBytesAvailable((CFReadStreamRef) self);
}
@end

@implementation NSInputStream (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFReadStreamGetTypeID();
}
@end
