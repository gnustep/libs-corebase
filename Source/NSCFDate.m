/* NSCFError.m
   
   Copyright (C) 2014 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: May, 2014
   
   This file is part of the GNUstep CoreBase Library.
   
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

#import <Foundation/NSObject.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSString.h>

#include "NSCFType.h"
#include "CoreFoundation/CFDate.h"

@interface NSCFDate : NSDate
NSCFTYPE_VARS
@end

@interface NSDate (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSCFDate
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id)initWithTimeIntervalSinceReferenceDate:(NSTimeInterval)seconds
{
  RELEASE(self);

  return (NSCFDate*) CFDateCreate(NULL, seconds);
}

- (NSTimeInterval)timeIntervalSinceReferenceDate
{
  return CFDateGetAbsoluteTime((CFDateRef) self);
}
@end

@implementation NSDate (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFDateGetTypeID();
}
@end
