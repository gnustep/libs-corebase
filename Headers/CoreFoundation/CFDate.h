/* CFDate.h
   
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

#include <CoreFoundation/CFBase.h>

#ifndef __COREFOUNDATION_CFDATE_H__
#define __COREFOUNDATION_CFDATE_H__

#ifdef __OBJC__
@class NSDate;
@class NSTimeZone;
typedef NSDate* CFDateRef;
typedef NSTimeZone* CFTimeZoneRef;
#else
typedef struct CFDate *CFDateRef;
typedef struct CFTimeZone *CFTimeZoneRef;
#endif

typedef double CFTimeInterval;
typedef CFTimeInterval CFAbsoluteTime;

typedef struct CFGregorianDate CFGregorianDate;
struct CFGregorianDate
{
  SInt32 year;
  SInt8 month;
  SInt8 day;
  SInt8 hour;
  SInt8 minute;
  double second;
};

typedef struct CFGregorianUnits CFGregorianUnits;
struct CFGregorianUnits
{
  SInt32 years;
  SInt32 months;
  SInt32 days;
  SInt32 hours;
  SInt32 minutes;
  double seconds;
};

typedef enum CFGregorianUnitFlags CFGregorianUnitFlags;
enum CFGregorianUnitFlags
{
  kCFGregorianUnitsYears = (1 << 0),
  kCFGregorianUnitsMonths = (1 << 1),
  kCFGregorianUnitsDays = (1 << 2),
  kCFGregorianUnitsHours = (1 << 3),
  kCFGregorianUnitsMinutes = (1 << 4),
  kCFGregorianUnitsSeconds = (1 << 5),
  kCFGregorianAllUnits = 0x00FFFFFF
};

const CFTimeInterval kCFAbsoluteTimeIntervalSince1970;
const CFTimeInterval kCFAbsoluteTimeIntervalSince1904;



//
// Time Utilities
//
CFAbsoluteTime
CFAbsoluteTimeAddGregorianUnits (CFAbsoluteTime at, CFTimeZoneRef tz,
  CFGregorianUnits units);

CFAbsoluteTime
CFAbsoluteTimeGetCurrent (void);

SInt32
CFAbsoluteTimeGetDayOfWeek (CFAbsoluteTime at, CFTimeZoneRef tz);

SInt32
CFAbsoluteTimeGetDayOfYear (CFAbsoluteTime at, CFTimeZoneRef tz);

CFGregorianUnits
CFAbsoluteTimeGetDifferenceAsGregorianUnits (CFAbsoluteTime at1,
  CFAbsoluteTime at2, CFTimeZoneRef tz, CFOptionFlags unitFlags);

CFGregorianDate
CFAbsoluteTimeGetGregorianDate (CFAbsoluteTime at, CFTimeZoneRef tz);

SInt32
CFAbsoluteTimeGetWeekOfYear (CFAbsoluteTime at, CFTimeZoneRef tz);

CFAbsoluteTime
CFGregorianDateGetAbsoluteTime (CFGregorianDate gdate, CFTimeZoneRef tz);

Boolean
CFGregorianDateIsValid (CFGregorianDate gdate, CFOptionFlags unitFlags);

//
// CFDate Functions
//
CFComparisonResult
CFDateCompare (CFDateRef theDate, CFDateRef otherDate, void *context);

CFDateRef
CFDateCreate (CFAllocatorRef allocator, CFAbsoluteTime at);

CFAbsoluteTime
CFDateGetAbsoluteTime (CFDateRef theDate);

CFTimeInterval
CFDateGetTimeIntervalSinceDate (CFDateRef theDate, CFDateRef otherDate);

CFTypeID
CFDateGetTypeID (void);

#endif /* __COREFOUNDATION_CFDATE_H__ */
