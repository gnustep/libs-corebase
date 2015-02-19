/* NSCFError.m
   
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

#import <Foundation/NSObject.h>
#import <Foundation/NSError.h>
#import <Foundation/NSString.h>

#include "NSCFType.h"
#include "CoreFoundation/CFError.h"

@interface NSCFError : NSError
NSCFTYPE_VARS
@end

@interface NSError (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSCFError
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithDomain: (NSString*) domain
                 code: (NSInteger) code
             userInfo: (NSDictionary*) userInfo
{
  RELEASE(self);
  
  return (NSCFError*)CFErrorCreate (NULL, (CFStringRef)domain, (CFIndex)code,
                                    (CFDictionaryRef)userInfo);
}

- (NSString *) localizedDescription
{
  return AUTORELEASE(CFErrorCopyDescription (self));
}

- (NSString *) localizedFailureReason
{
  return AUTORELEASE(CFErrorCopyFailureReason (self));
}

- (NSArray *) localizedRecoveryOptions
{
  return nil; // FIXME
}

- (NSString *) localizedRecoverySuggestion
{
  return AUTORELEASE(CFErrorCopyRecoverySuggestion (self));
}

- (id) recoveryAttempter
{
  return nil; // FIXME
}

- (NSInteger) code
{
  return (NSInteger)CFErrorGetCode (self);
}

- (NSString*) domain
{
  return CFErrorGetDomain (self);
}

- (NSDictionary*) userInfo
{
  return (NSDictionary*)CFErrorCopyUserInfo ((CFErrorRef)self);
}

@end

@implementation NSError (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFErrorGetTypeID();
}
@end

