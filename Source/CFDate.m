/* CFDate.m
   
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

#import <Foundation/NSDate.h>

#include "CoreFoundation/CFDate.h"

const CFTimeInterval kCFAbsoluteTimeIntervalSince1970 = 978307200.0;
const CFTimeInterval kCFAbsoluteTimeIntervalSince1904 = 3061152000.0;



//
// Time Utilities
//
CFAbsoluteTime
CFAbsoluteTimeAddGregorianUnits (CFAbsoluteTime at, CFTimeZoneRef tz,
  CFGregorianUnits units)
{
  // FIXME
  return 0.0;
}

CFAbsoluteTime
CFAbsoluteTimeGetCurrent (void)
{
  return [NSDate timeIntervalSinceReferenceDate];
}

SInt32
CFAbsoluteTimeGetDayOfWeek (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  // FIXME
  return 0;
}

SInt32
CFAbsoluteTimeGetDayOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  // FIXME
  return 0;
}

CFGregorianUnits
CFAbsoluteTimeGetDifferenceAsGregorianUnits (CFAbsoluteTime at1,
  CFAbsoluteTime at2, CFTimeZoneRef tz, CFOptionFlags unitFlags)
{
  // FIXME
  return (CFGregorianUnits){0, 0, 0, 0, 0, 0.0};
}

CFGregorianDate
CFAbsoluteTimeGetGregorianDate (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  // FIXME
  return (CFGregorianDate){0, 0, 0, 0, 0, 0.0};
}

SInt32
CFAbsoluteTimeGetWeekOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  // FIXME
  return 0;
}

CFAbsoluteTime
CFGregorianDateGetAbsoluteTime (CFGregorianDate gdate, CFTimeZoneRef tz)
{
  // FIXME
  return 0.0;
}

Boolean
CFGregorianDateIsValid (CFGregorianDate gdate, CFOptionFlags unitFlags)
{
  /* unitFlags is unused, must be ignored */
  Boolean isValid = FALSE;
  
  if (unitFlags | kCFGregorianUnitsYears)
    isValid = TRUE; // FIXME: What's the test here?
  if (unitFlags | kCFGregorianUnitsMonths)
    isValid = ((gdate.month >= 1) && (gdate.month <= 12));
  if (unitFlags | kCFGregorianUnitsDays)
    isValid = TRUE; // FIXME
  if (unitFlags | kCFGregorianUnitsHours)
    isValid = ((gdate.hour >= 0) && (gdate.hour < 24));
    // FIXME: I'm assuming this would be in 24 hour time (24 == 0 in this case)
  if (unitFlags | kCFGregorianUnitsMinutes)
    isValid = ((gdate.minute >= 0) && (gdate.minute < 60));
  if (unitFlags | kCFGregorianUnitsSeconds)
    isValid = ((gdate.second >= 0.0) && (gdate.second < 60.0));
  
  return isValid;
}

//
// CFDate Functions
//
CFComparisonResult
CFDateCompare (CFDateRef theDate, CFDateRef otherDate, void *context)
{
  return [(NSDate *)theDate compare: (NSDate *)otherDate];
}

CFDateRef
CFDateCreate (CFAllocatorRef allocator, CFAbsoluteTime at)
{
  return (CFDateRef)[[NSDate allocWithZone: allocator]
                      initWithTimeIntervalSinceReferenceDate: at];
}

CFAbsoluteTime
CFDateGetAbsoluteTime (CFDateRef theDate)
{
  return [(NSDate *)theDate timeIntervalSinceReferenceDate];
}

CFTimeInterval
CFDateGetTimeIntervalSinceDate (CFDateRef theDate, CFDateRef otherDate)
{
  return (CFTimeInterval)[(NSDate *)theDate
           timeIntervalSinceDate: (NSDate *)otherDate];
}

CFTypeID
CFDateGetTypeID (void)
{
  return (CFTypeID)[NSDate class];
}
