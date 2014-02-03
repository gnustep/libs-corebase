/* NSCFOutputStream.m
   
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

@interface NSCFOutputStream : NSOutputStream
NSCFTYPE_VARS
@end

@interface NSOutputStream (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;

@end

@implementation NSCFOutputStream
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (NSInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length
{
  return CFWriteStreamWrite((CFWriteStreamRef) self, (const UInt8*) buffer,
                            length);
}

- (BOOL)hasSpaceAvailable
{
  return CFWriteStreamCanAcceptBytes((CFWriteStreamRef) self);
}
@end

@implementation NSOutputStream (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFWriteStreamGetTypeID();
}
@end
