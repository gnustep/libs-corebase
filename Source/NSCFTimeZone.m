/* NSCFTimeZone.m
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: March, 2013
   
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

#import <Foundation/NSTimeZone.h>
#import <Foundation/NSString.h>

#include "NSCFType.h"
#include "CoreFoundation/CFTimeZone.h"


@interface NSCFTimeZone : NSTimeZone
NSCFTYPE_VARS
@end

@interface NSTimeZone (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;

- (NSString*) _cfCopyAbbreviation: (CFAbsoluteTime)at;
- (NSTimeInterval) _cfGetDaylightSavingTimeOffset: (CFAbsoluteTime)at;
- (Boolean) _cfIsDaylightSavingTime: (CFAbsoluteTime)at;
- (NSTimeInterval) _cfGetSecondsFromGMT: (CFAbsoluteTime)at;
- (CFAbsoluteTime) _cfGetNextDaylightSavingTimeTransition: (CFAbsoluteTime)at;
@end

//#if 0

@implementation NSCFTimeZone
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id)initWithName:(NSString *)tzName data:(NSData *)data
{
  RELEASE(self);
  
  if (data != NULL)
    {
      self = (NSCFTimeZone*) CFTimeZoneCreate(NULL,
        (CFStringRef) tzName, (CFDataRef) data);
    }
  else
    {
      self = (NSCFTimeZone*) CFTimeZoneCreateWithName(NULL,
        (CFStringRef) tzName, YES);
    }
  
  return self;
}

- (NSString*) name
{
  return (NSString*) CFTimeZoneGetName((CFTimeZoneRef) self);
}

- (NSData *)data
{
  return (NSData*) CFTimeZoneGetData((CFTimeZoneRef) self);
}

- (NSInteger) secondsFromGMTForDate: (NSDate*)aDate
{
  CFAbsoluteTime at = [aDate timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
  
  return CFTimeZoneGetSecondsFromGMT((CFTimeZoneRef) self, at);
}

- (NSString *)abbreviationForDate:(NSDate *)aDate
{
  NSString *abbr;
  CFAbsoluteTime at = [aDate timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
  
  abbr = (NSString*) CFTimeZoneCopyAbbreviation((CFTimeZoneRef) self, at);
  
  AUTORELEASE(abbr);
  
  return abbr;
}

- (NSString *)localizedName:(NSTimeZoneNameStyle)style
                     locale:(NSLocale *)locale
{
  NSString *str;
  
  str = (NSString*) CFTimeZoneCopyLocalizedName((CFTimeZoneRef) self,
    (CFTimeZoneNameStyle)style, (CFLocaleRef)locale);
  
  AUTORELEASE(str);
  return str;
}

- (NSDate *)nextDaylightSavingTimeTransitionAfterDate:(NSDate *)aDate
{
  CFAbsoluteTime at;
  
  at = [aDate timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
  
  at = CFTimeZoneGetNextDaylightSavingTimeTransition((CFTimeZoneRef) self,
    at);
  
  return [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
}

- (BOOL)isDaylightSavingTimeForDate:(NSDate *)aDate
{
  CFAbsoluteTime at;
  
  at = [aDate timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
  
  return CFTimeZoneIsDaylightSavingTime((CFTimeZoneRef) self, at);
}

- (NSTimeInterval)daylightSavingTimeOffsetForDate:(NSDate *)aDate
{
  CFAbsoluteTime at;
  
  at = [aDate timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
  
  return CFTimeZoneGetDaylightSavingTimeOffset((CFTimeZoneRef) self, at);
}
@end
//#endif

@implementation NSTimeZone (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFTimeZoneGetTypeID();
}

- (NSString*) _cfCopyAbbreviation: (CFAbsoluteTime)at
{
  NSString *abbr;
  NSDate *date;
  
  date = [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
  
  abbr = [self abbreviationForDate: date];
  RETAIN(abbr);
  
  return abbr;
}

- (NSTimeInterval) _cfGetDaylightSavingTimeOffset: (CFAbsoluteTime)at
{
  NSDate *date;
  
  date = [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
  
  return [self daylightSavingTimeOffsetForDate: date];
}

- (Boolean) _cfIsDaylightSavingTime: (CFAbsoluteTime)at
{
  NSDate *date;
  
  date = [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
  
  return [self isDaylightSavingTimeForDate: date];
}

- (NSTimeInterval) _cfGetSecondsFromGMT: (CFAbsoluteTime)at
{
  NSDate *date;
  
  date = [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
  
  return (NSTimeInterval)  [self secondsFromGMTForDate: date];
}

- (CFAbsoluteTime) _cfGetNextDaylightSavingTimeTransition: (CFAbsoluteTime)at
{
  NSDate *date;
  
  date = [NSDate dateWithTimeIntervalSince1970: at
    + kCFAbsoluteTimeIntervalSince1970];
  
  date = [self nextDaylightSavingTimeTransitionAfterDate: date];
  
  return [date timeIntervalSince1970]
    - kCFAbsoluteTimeIntervalSince1970;
}

- (BOOL)isEqualToTimeZone:(NSTimeZone *)aTimeZone
{
  return [[self name] isEqual: [aTimeZone name]];
}
@end

