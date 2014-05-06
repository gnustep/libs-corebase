/* CFDate.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
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

#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"

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

static CFTypeRef
CFDateCreateCopy (CFAllocatorRef alloc, CFTypeRef cf)
{
  CFDateRef date = (CFDateRef)cf;
  return CFDateCreate (alloc, date->_absTime);
}

static Boolean
CFDateEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return CFDateCompare ((CFDateRef)cf1, (CFDateRef)cf2, NULL)
    == kCFCompareEqualTo;
}

static CFHashCode
CFDateHash (CFTypeRef cf)
{
  return (CFHashCode)((CFDateRef)cf)->_absTime;
}

static const CFRuntimeClass CFDateClass =
{
  0,
  "CFDate",
  NULL,
  CFDateCreateCopy,
  NULL,
  CFDateEqual,
  CFDateHash,
  NULL,
  NULL
};

void CFDateInitialize (void)
{
  _kCFDateTypeID = _CFRuntimeRegisterClass(&CFDateClass);
}



/* isleap() defined in tzfile.h */
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

CFComparisonResult
CFDateCompare (CFDateRef theDate, CFDateRef otherDate, void *context)
{
/* context is unused! */
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
  CF_OBJC_FUNCDISPATCHV(_kCFDateTypeID, CFAbsoluteTime, theDate,
    "timeIntervalSinceReferenceDate");
  
  return theDate->_absTime;
}

CFTimeInterval
CFDateGetTimeIntervalSinceDate (CFDateRef theDate, CFDateRef otherDate)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDateTypeID, CFTimeInterval, theDate,
    "timeIntervalSinceDate:", otherDate);
  
  return CFDateGetAbsoluteTime (theDate) - CFDateGetAbsoluteTime (otherDate);
}

CFTypeID
CFDateGetTypeID (void)
{
  return _kCFDateTypeID;
}



CFAbsoluteTime
CFAbsoluteTimeGetCurrent (void)
{
  return UDATE_TO_ABSOLUTETIME(ucal_getNow());
}

static const uint16_t _daysBeforeMonth[] =
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

/* This function does most of the work for the GregorianDate functions.
   month, day, weekOfYear, dayOfWeek and dayOfYear are optional. */
static double
CFAbsoluteTimeToFields (CFAbsoluteTime at, SInt32 *year, SInt8 *month,
  SInt8 *day, SInt32 *weekOfYear, SInt32 *dayOfWeek, SInt32 *dayOfYear)
{
  Boolean isLeap;
  SInt32 y400, y100, y4, y1, M, d;
  double days, ret = modf (at / 86400.0, &days) * 86400.0;
  
  d = days;
  y400 = d / 146097; /* 400 years */
  d %= 146097;
  y100 = d / 36524; /* 100 years */
  d %= 36524;
  y4 = d / 1461; /* 4 years */
  d %= 1461;
  y1 = d / 365; /* 1 year */
  d %= 365;
  *year = y400 * 400 + y100 * 100 + y4 * 4 + y1 + 2001;
  isLeap = isleap (*year);
  
  if (ret < 0.0)
    {
      d += isLeap ? 365 : 364;
      *year -= 1;
      ret += 86400.0;
    }
  
  if (weekOfYear)
    *weekOfYear = d / 7 % 52; /* FIXME: I don't think this is correct. */
  if (dayOfWeek)
    {
      *dayOfWeek = (int)days % 7;
      /* 2001-01-01 was a Monday (day of week == 1), so that's our base. */
      *dayOfWeek += *dayOfWeek < 1 ? 7 : 1;
    }
  if (dayOfYear)
    *dayOfYear = d;
  
  M = isLeap ? 11 : 13;
  while (d < _daysBeforeMonth[M])
    ++M;
  if (month)
    *month = ++M;
  if (day)
    *day = d - _daysBeforeMonth[M] + 1;
  
  return ret;
}

static double
CFFieldsToAbsoluteTime (SInt32 year, SInt32 month, SInt32 day)
{
  double days;
  SInt32 y400, y100, y4, d;
  
  year = year - 2001;
  y400 = year / 400;
  year %= 400;
  y100 = year / 100;
  year %= 100;
  y4 = year / 4;
  year %= 4;
  d = y400 * 146097 + y100 * 36524 + y4 * 1461 + year * 365;
  d += _daysBeforeMonth[month - 1];
  d += day - (d >= 0 ? 1 : 0);
  
  days = (double)d;
  
  return days;
}

CFGregorianUnits
CFAbsoluteTimeGetDifferenceAsGregorianUnits (CFAbsoluteTime at1,
  CFAbsoluteTime at2, CFTimeZoneRef tz, CFOptionFlags unitFlags)
{
  /* FIXME: This is wrong but I'll fix it later... */
  int year, month, day, hour, minute, second;
  CFCalendarRef cal;
  CFGregorianUnits gunits = { 0 };
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  CFCalendarSetTimeZone (cal, tz);
  
  CFCalendarGetComponentDifference (cal, at1, at2, 0, "yMdHms",
    &year, &month, &day, &hour, &minute, &second);
  if (unitFlags & kCFGregorianUnitsYears)
    gunits.years = year;
  if (unitFlags & kCFGregorianUnitsMonths)
    gunits.months = month;
  if (unitFlags & kCFGregorianUnitsDays)
    gunits.days = day;
  if (unitFlags & kCFGregorianUnitsHours)
    gunits.hours = hour;
  if (unitFlags & kCFGregorianUnitsMinutes)
    gunits.minutes = minute;
  if (unitFlags & kCFGregorianUnitsSeconds)
    {
      gunits.seconds = (double)second;
      gunits.seconds += modf (at1 - at2, NULL);
    }
  
  CFRelease (cal);
  
  return gunits;
}

CFGregorianDate
CFAbsoluteTimeGetGregorianDate (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  double seconds;
  CFGregorianDate gdate;
  
  if (tz != NULL)
    at += CFTimeZoneGetSecondsFromGMT (tz, at);
  
  seconds = CFAbsoluteTimeToFields (at, &gdate.year, &gdate.month, &gdate.day,
    NULL, NULL, NULL);
  
  gdate.hour = (SInt8)floor (seconds / 3600.0) % 24;
  gdate.minute = (SInt8)floor (seconds / 60.0) % 60;
  gdate.second = seconds - (floor(seconds / 60.0) * 60.0);
  
  return gdate;
}

CFAbsoluteTime
CFGregorianDateGetAbsoluteTime (CFGregorianDate gdate, CFTimeZoneRef tz)
{
  double seconds;
  CFAbsoluteTime at;
  
  at = CFFieldsToAbsoluteTime (gdate.year, gdate.month, gdate.day);
  
  seconds = (gdate.hour * 3600 + gdate.minute * 60) + gdate.second;
  if (at < 0.0)
    seconds = -seconds;
  at += seconds;
  
  if (tz != NULL)
    at += CFTimeZoneGetSecondsFromGMT (tz, at);
  
  return at;
}

SInt32
CFAbsoluteTimeGetDayOfWeek (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  SInt32 year, dayOfWeek;
  
  if (tz != NULL)
    at += CFTimeZoneGetSecondsFromGMT (tz, at);
  
  CFAbsoluteTimeToFields (at, &year, NULL, NULL, NULL, &dayOfWeek, NULL);
  
  return dayOfWeek;
}

SInt32
CFAbsoluteTimeGetDayOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  SInt32 year, dayOfYear;
  
  if (tz != NULL)
    at += CFTimeZoneGetSecondsFromGMT (tz, at);
  
  CFAbsoluteTimeToFields (at, &year, NULL, NULL, NULL, NULL, &dayOfYear);
  
  return dayOfYear;
}

SInt32
CFAbsoluteTimeGetWeekOfYear (CFAbsoluteTime at, CFTimeZoneRef tz)
{
  SInt32 year, weekOfYear;
  
  if (tz != NULL)
    at += CFTimeZoneGetSecondsFromGMT (tz, at);
  
  CFAbsoluteTimeToFields (at, &year, NULL, NULL, &weekOfYear, NULL, NULL);
  
  return weekOfYear;
}

CFAbsoluteTime
CFAbsoluteTimeAddGregorianUnits (CFAbsoluteTime at, CFTimeZoneRef tz,
  CFGregorianUnits units)
{
  return at;
} 

Boolean
CFGregorianDateIsValid (CFGregorianDate gdate, CFOptionFlags unitFlags)
{
  /* unitFlags is unused, must be ignored */
  Boolean isValid = FALSE;
  
  if (unitFlags | kCFGregorianUnitsYears)
    isValid = TRUE; /* FIXME: What's the test here? */
  if (unitFlags | kCFGregorianUnitsMonths)
    isValid = ((gdate.month >= 1) && (gdate.month <= 12));
  if (unitFlags | kCFGregorianUnitsDays)
    isValid = TRUE; /* FIXME */
  if (unitFlags | kCFGregorianUnitsHours)
    isValid = ((gdate.hour >= 0) && (gdate.hour < 24));
  if (unitFlags | kCFGregorianUnitsMinutes)
    isValid = ((gdate.minute >= 0) && (gdate.minute < 60));
  if (unitFlags | kCFGregorianUnitsSeconds)
    isValid = ((gdate.second >= 0.0) && (gdate.second < 60.0));
  
  return isValid;
}

