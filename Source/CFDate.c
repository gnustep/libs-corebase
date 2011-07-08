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

#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFRuntime.h"

#include <math.h>
#include <unicode/ucal.h>

static CFTypeID _kCFDateTypeID = 0;

struct __CFDate
{
  CFRuntimeBase _parent;
  CFAbsoluteTime _absTime;
};

const CFTimeInterval kCFAbsoluteTimeIntervalSince1970 = 978307200.0;
const CFTimeInterval kCFAbsoluteTimeIntervalSince1904 = 3061152000.0;

static const CFRuntimeClass CFDateClass =
{
  0,
  "CFDate",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFDateInitialize (void)
{
  _kCFDateTypeID = _CFRuntimeRegisterClass(&CFDateClass);
}



#define UDATE_TO_ABSOLUTETIME(d) \
  (((d) * 1000.0) + kCFAbsoluteTimeIntervalSince1970)
#define ABSOLUTETIME_TO_UDATE(at) \
  (((at) - kCFAbsoluteTimeIntervalSince1970) / 1000.0)

CFComparisonResult
CFDateCompare (CFDateRef theDate, CFDateRef otherDate, void *context)
{
// context is unused!
  CFAbsoluteTime diff = CFDateGetTimeIntervalSinceDate (theDate, otherDate);
  
  if (diff < 0.0)
    return kCFCompareLessThan;
  if (diff > 0.0)
    return kCFCompareGreaterThan;
  
  return kCFCompareEqualTo;
}

CFDateRef
CFDateCreate (CFAllocatorRef allocator, CFAbsoluteTime at)
{
  struct __CFDate *new;
  
  new = (struct __CFDate *)_CFRuntimeCreateInstance (allocator,
    _kCFDateTypeID,
    sizeof(struct __CFDate) - sizeof(CFRuntimeBase),
    NULL);
  new->_absTime = at;
  
  return (CFDateRef)new;
}

CFAbsoluteTime
CFDateGetAbsoluteTime (CFDateRef theDate)
{
  return theDate->_absTime;
}

CFTimeInterval
CFDateGetTimeIntervalSinceDate (CFDateRef theDate, CFDateRef otherDate)
{
  CFAbsoluteTime first = CFDateGetAbsoluteTime (theDate);
  CFAbsoluteTime second = CFDateGetAbsoluteTime (otherDate);
  
  return first - second;
}

CFTypeID
CFDateGetTypeID (void)
{
  return _kCFDateTypeID;
}



/* The following functions all use CFCalendar. */
CFAbsoluteTime
CFAbsoluteTimeAddGregorianUnits (CFAbsoluteTime at, CFTimeZoneRef tz,
  CFGregorianUnits units)
{
  return at;
}

CFAbsoluteTime
CFAbsoluteTimeGetCurrent (void)
{
  return UDATE_TO_ABSOLUTETIME(ucal_getNow());
}

SInt32
CFAbsoluteTimeGetDayOfWeek (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  return 0;
}

SInt32
CFAbsoluteTimeGetDayOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  return 0;
}

CFGregorianUnits
CFAbsoluteTimeGetDifferenceAsGregorianUnits (CFAbsoluteTime at1,
  CFAbsoluteTime at2, CFTimeZoneRef tz, CFOptionFlags unitFlags)
{
//  double d;
  CFGregorianUnits gunits = { 0 };
  
//  gunits.seconds += modf (at1 - at2, &d);
  
  return gunits;
}

CFGregorianDate
CFAbsoluteTimeGetGregorianDate (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  CFGregorianDate gdate = { 0 };
  return gdate;
}

SInt32
CFAbsoluteTimeGetWeekOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  return 0;
}

CFAbsoluteTime
CFGregorianDateGetAbsoluteTime (CFGregorianDate gdate, CFTimeZoneRef tz)
{
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
