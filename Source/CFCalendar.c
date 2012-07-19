/* CFCalendar.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
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

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"
#include "GSPrivate.h"

#include <unicode/ucal.h>

struct __CFCalendar
{
  CFRuntimeBase _parent;
  UCalendar    *_ucal;
  CFStringRef   _ident;
  CFStringRef   _localeIdent;
  CFStringRef   _tzIdent;
};

static CFTypeID _kCFCalendarTypeID = 0;
static CFCalendarRef _kCFCalendarCurrent = NULL;
static GSMutex _kCFCalendarLock;

#define BUFFER_SIZE 64

static void
CFCalendarOpenUCalendar (CFCalendarRef cal)
{
  if (cal->_ucal == NULL)
    {
      char localeIdent[ULOC_FULLNAME_CAPACITY];
      char calIdent[ULOC_KEYWORDS_CAPACITY];
      UniChar tzIdent[BUFFER_SIZE];
      CFIndex tzLen;
      UCalendar *ucal;
      UErrorCode err = U_ZERO_ERROR;
      
      CFStringGetCString (cal->_localeIdent, localeIdent,
                          ULOC_FULLNAME_CAPACITY, kCFStringEncodingASCII);
      CFStringGetCString (cal->_ident, calIdent, ULOC_KEYWORDS_CAPACITY,
                          kCFStringEncodingASCII);
      uloc_setKeywordValue ("calendar", (const char*)calIdent,
        localeIdent, ULOC_FULLNAME_CAPACITY, &err);
      
      tzLen = CFStringGetLength (cal->_tzIdent);
      if (tzLen > BUFFER_SIZE)
        tzLen = BUFFER_SIZE;
      CFStringGetCharacters (cal->_tzIdent, CFRangeMake(0, tzLen), tzIdent);
      
      ucal = ucal_open (tzIdent, tzLen, localeIdent, UCAL_TRADITIONAL, &err);
      
      cal->_ucal = ucal;
    }
  else
    {
      ucal_clear (cal->_ucal);
    }
}

CF_INLINE void
CFCalendarCloseUCalendar (CFCalendarRef cal)
{
  if (cal->_ucal)
    {
      ucal_close (cal->_ucal);
      cal->_ucal = NULL;
    }
}

static void CFCalendarDealloc (CFTypeRef cf)
{
  CFCalendarRef cal = (CFCalendarRef)cf;
  
  CFCalendarCloseUCalendar (cal);
  CFRelease (cal->_ident);
  CFRelease (cal->_localeIdent);
  CFRelease (cal->_tzIdent);
}

static Boolean CFCalendarEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFCalendarRef o1 = (CFCalendarRef)cf1;
  CFCalendarRef o2 = (CFCalendarRef)cf2;
  
  return (Boolean)(CFEqual(o1->_ident, o2->_ident)
                   && CFEqual(o1->_localeIdent, o2->_localeIdent)
                   && CFEqual(o1->_tzIdent, o2->_tzIdent));
}

static CFHashCode CFCalendarHash (CFTypeRef cf)
{
  return CFHash (((CFCalendarRef)cf)->_ident);
}

static CFStringRef CFCalendarCopyFormattingDesc (CFTypeRef cf,
  CFDictionaryRef formatOptions)
{
  return CFRetain (((CFCalendarRef)cf)->_ident);
}

static const CFRuntimeClass CFCalendarClass =
{
  0,
  "CFCalendar",
  NULL,
  NULL,
  CFCalendarDealloc,
  CFCalendarEqual,
  CFCalendarHash,
  CFCalendarCopyFormattingDesc,
  NULL
};

void CFCalendarInitialize (void)
{
  _kCFCalendarTypeID = _CFRuntimeRegisterClass (&CFCalendarClass);
  GSMutexInitialize (&_kCFCalendarLock);
}

CF_INLINE UCalendarDateFields
CFCalendarUnitToUCalendarDateFields (CFCalendarUnit unit)
{
  UCalendarDateFields ret;
  switch (unit)
    {
      case kCFCalendarUnitEra:
        ret = UCAL_ERA;
        break;
      case kCFCalendarUnitYear:
        ret = UCAL_YEAR;
        break;
      case kCFCalendarUnitMonth:
        ret = UCAL_MONTH;
        break;
      case kCFCalendarUnitDay:
        ret = UCAL_DATE;
        break;
      case kCFCalendarUnitHour:
        ret = UCAL_HOUR_OF_DAY;
        break;
      case kCFCalendarUnitMinute:
        ret = UCAL_MINUTE;
        break;
      case kCFCalendarUnitSecond:
        ret = UCAL_SECOND;
        break;
      case kCFCalendarUnitWeek:
        ret = UCAL_WEEK_OF_YEAR;
        break;
      case kCFCalendarUnitWeekday:
        ret = UCAL_DAY_OF_WEEK;
        break;
      case kCFCalendarUnitWeekdayOrdinal:
        ret = UCAL_DAY_OF_WEEK_IN_MONTH;
        break;
      case kCFCalendarUnitQuarter:
        ret = UCAL_MONTH; /* FIXME */
        break;
      default:
        ret = -1;
    }
  return ret;
}

/* This function returns true is there is still characters to be consumed. */
CF_INLINE Boolean
CFCalendarGetCalendarUnitFromDescription (const char **description,
  CFCalendarUnit *unit)
{
  const char *current = *description;
  
  if (current == NULL || *current == '\0')
    return false;
  
  switch (*current++)
    {
      case 'y':
        *unit = kCFCalendarUnitYear;
        break;
      case 'M':
        *unit = kCFCalendarUnitMonth;
        break;
      case 'd':
        *unit = kCFCalendarUnitDay;
        break;
      case 'H':
        *unit = kCFCalendarUnitHour;
        break;
      case 'm':
        *unit = kCFCalendarUnitMinute;
        break;
      case 's':
        *unit = kCFCalendarUnitSecond;
        break;
    }
  
  *description = current;
  
  return true;
}



CFTypeID
CFCalendarGetTypeID (void)
{
  return _kCFCalendarTypeID;
}

CFCalendarRef
CFCalendarCopyCurrent (void)
{
  if (_kCFCalendarCurrent == NULL)
    {
      GSMutexLock (&_kCFCalendarLock);
      if (_kCFCalendarCurrent == NULL)
        {
          CFLocaleRef locale;
          CFStringRef calIdent;
          CFCalendarRef cal;
          
          locale = CFLocaleCopyCurrent ();
          calIdent = CFLocaleGetValue (locale, kCFLocaleCalendarIdentifier);
          cal = CFCalendarCreateWithIdentifier (NULL, calIdent);
          CFCalendarSetLocale (cal, locale);
          
          CFRelease (locale);
          _kCFCalendarCurrent = cal;
        }
      GSMutexUnlock (&_kCFCalendarLock);
    }
  
  return _kCFCalendarCurrent;
}

CFCalendarRef
CFCalendarCreateWithIdentifier (CFAllocatorRef allocator, CFStringRef ident)
{
  CFLocaleRef locale;
  CFCalendarRef new;
  CFTimeZoneRef tz;
  
  if (!(ident == kCFGregorianCalendar
      || ident == kCFBuddhistCalendar
      || ident == kCFChineseCalendar
      || ident == kCFHebrewCalendar
      || ident == kCFIslamicCalendar
      || ident == kCFIslamicCivilCalendar
      || ident == kCFJapaneseCalendar
      || ident == kCFRepublicOfChinaCalendar
      || ident == kCFPersianCalendar
      || ident == kCFIndianCalendar
      || ident == kCFISO8601Calendar))
    {
      if (CFEqual(ident, kCFGregorianCalendar))
        ident = kCFGregorianCalendar;
      else if (CFEqual(ident, kCFBuddhistCalendar))
        ident = kCFBuddhistCalendar;
      else if (CFEqual(ident, kCFChineseCalendar))
        ident = kCFChineseCalendar;
      else if (CFEqual(ident, kCFHebrewCalendar))
        ident = kCFHebrewCalendar;
      else if (CFEqual(ident, kCFIslamicCalendar))
        ident = kCFIslamicCalendar;
      else if (CFEqual(ident, kCFIslamicCivilCalendar))
        ident = kCFIslamicCivilCalendar;
      else if (CFEqual(ident, kCFJapaneseCalendar))
        ident = kCFJapaneseCalendar;
      else if (CFEqual(ident, kCFRepublicOfChinaCalendar))
        ident = kCFRepublicOfChinaCalendar;
      else if (CFEqual(ident, kCFPersianCalendar))
        ident = kCFPersianCalendar;
      else if (CFEqual(ident, kCFIndianCalendar))
        ident = kCFIndianCalendar;
      else if (CFEqual(ident, kCFISO8601Calendar))
        ident = kCFISO8601Calendar;
      else
        return NULL;
    }
  
  new = (CFCalendarRef)_CFRuntimeCreateInstance (allocator, _kCFCalendarTypeID,
    sizeof(struct __CFCalendar) - sizeof(CFRuntimeBase), NULL);
  new->_ident = ident;
  
  locale = CFLocaleCopyCurrent ();
  new->_localeIdent = CFRetain (CFLocaleGetIdentifier(locale));
  CFRelease (locale);
  
  tz = CFTimeZoneCopyDefault ();
  new->_tzIdent = CFRetain (CFTimeZoneGetName (tz));
  CFRelease (tz);
  
  return new;
}

CFTimeZoneRef
CFCalendarCopyTimeZone (CFCalendarRef cal)
{
  return CFTimeZoneCreateWithName (CFGetAllocator(cal), cal->_tzIdent, true);
}

void
CFCalendarSetTimeZone (CFCalendarRef cal, CFTimeZoneRef tz)
{
  CFStringRef tzIdent;
  
  tzIdent = CFTimeZoneGetName(tz);
  if (CFStringCompare (cal->_tzIdent, tzIdent, 0) != 0)
    {
      CFIndex len;
      UniChar buffer[BUFFER_SIZE];
      UErrorCode err = U_ZERO_ERROR;
      
      CFCalendarOpenUCalendar (cal);
      
      len = CFStringGetLength (tzIdent);
      if (len > BUFFER_SIZE)
        len = BUFFER_SIZE;
      CFStringGetCharacters (tzIdent, CFRangeMake(0, len), buffer);
      
      ucal_setTimeZone (cal->_ucal, buffer, len, &err);
    }
}

CFStringRef
CFCalendarGetIdentifier (CFCalendarRef cal)
{
  return cal->_ident;
}

CFLocaleRef
CFCalendarCopyLocale (CFCalendarRef cal)
{
  return CFLocaleCreate (NULL, cal->_localeIdent);
}

void
CFCalendarSetLocale (CFCalendarRef cal, CFLocaleRef locale)
{
  if (cal->_localeIdent != NULL)
    CFRelease (cal->_localeIdent);
  cal->_localeIdent = CFRetain (CFLocaleGetIdentifier(locale));
  CFCalendarCloseUCalendar (cal);
}

CFIndex
CFCalendarGetFirstWeekday (CFCalendarRef cal)
{
  CFCalendarOpenUCalendar (cal);
  return ucal_getAttribute (cal->_ucal, UCAL_FIRST_DAY_OF_WEEK);
}

void
CFCalendarSetFirstWeekday (CFCalendarRef cal, CFIndex wkdy)
{
  CFCalendarOpenUCalendar (cal);
  ucal_setAttribute (cal->_ucal, UCAL_FIRST_DAY_OF_WEEK, wkdy);
}

CFIndex
CFCalendarGetMinimumDaysInFirstWeek (CFCalendarRef cal)
{
  CFCalendarOpenUCalendar (cal);
  return ucal_getAttribute (cal->_ucal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);
}

void
CFCalendarSetMinimumDaysInFirstWeek (CFCalendarRef cal, CFIndex mwd)
{
  CFCalendarOpenUCalendar (cal);
  ucal_setAttribute (cal->_ucal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK, mwd);
}

Boolean
CFCalendarAddComponents (CFCalendarRef cal, CFAbsoluteTime *at,
  CFOptionFlags options, const char *componentDesc, ...)
{
  va_list arg;
  int value;
  CFCalendarUnit unit = 0;
  UCalendarDateFields field;
  UErrorCode err = U_ZERO_ERROR;
  
  CFCalendarOpenUCalendar(cal);
  
  ucal_setMillis (cal->_ucal, ABSOLUTETIME_TO_UDATE(*at), &err);
  if (U_FAILURE(err))
    return false;
  
  va_start (arg, componentDesc);
  while (CFCalendarGetCalendarUnitFromDescription(&componentDesc, &unit))
    {
      switch (unit)
        {
          case kCFCalendarUnitYear:
            field = UCAL_YEAR;
            value = va_arg (arg, int);
            break;
          case kCFCalendarUnitMonth:
            field = UCAL_MONTH;
            value = va_arg (arg, int);
            break;
          case kCFCalendarUnitDay:
            field = UCAL_DAY_OF_MONTH;
            value = va_arg (arg, int);
            break;
          case kCFCalendarUnitHour:
            field = UCAL_HOUR_OF_DAY;
            value = va_arg (arg, int);
            break;
          case kCFCalendarUnitMinute:
            field = UCAL_MINUTE;
            value = va_arg (arg, int);
            break;
          case kCFCalendarUnitSecond:
            field = UCAL_SECOND;
            value = va_arg (arg, int) - 1;
            break;
          default:
            va_arg (arg, int); /* Skip */
            continue;
        }
      if (options & kCFCalendarComponentsWrap)
        ucal_roll (cal->_ucal, field, value, &err);
      else
        ucal_add (cal->_ucal, field, value, &err);
    }
  va_end(arg);
  
  *at = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
  
  if (U_FAILURE(err))
    return false;
  
  return true;
}

Boolean
CFCalendarComposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime *at,
  const char *componentDesc, ...)
{
  va_list arg;
  int value;
  UCalendarDateFields field;
  CFCalendarUnit unit = 0;
  UErrorCode err = U_ZERO_ERROR;
  
  CFCalendarOpenUCalendar(cal);
  
  va_start (arg, componentDesc);
  while (CFCalendarGetCalendarUnitFromDescription(&componentDesc, &unit))
    {
      switch (unit)
        {
          case kCFCalendarUnitYear:
            value = va_arg (arg, int);
            field = UCAL_YEAR;
            break;
          case kCFCalendarUnitMonth:
            value = va_arg (arg, int) - 1;
            field = UCAL_MONTH;
            break;
          case kCFCalendarUnitDay:
            value = va_arg (arg, int);
            field = UCAL_DATE;
            break;
          case kCFCalendarUnitHour:
            value = va_arg (arg, int);
            field = UCAL_HOUR_OF_DAY;
            break;
          case kCFCalendarUnitMinute:
            value = va_arg (arg, int);
            field = UCAL_MINUTE;
            break;
          case kCFCalendarUnitSecond:
            value = va_arg (arg, int);
            field = UCAL_SECOND;
            break;
          default:
            va_arg (arg, int); /* Skip */
            continue;
        }
      ucal_set (cal->_ucal, field, value);
    }
  va_end(arg);
  
  *at = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
  if (U_FAILURE(err))
    return false;
  
  return true;
}

Boolean
CFCalendarDecomposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime at,
  const char *componentDesc, ...)
{
  va_list arg;
  int *value;
  CFCalendarUnit unit = 0;
  UCalendarDateFields field;
  UErrorCode err = U_ZERO_ERROR;
  
  CFCalendarOpenUCalendar(cal);
  
  ucal_setMillis (cal->_ucal, ABSOLUTETIME_TO_UDATE(at), &err);
  if (U_FAILURE(err))
    return false;
  
  va_start (arg, componentDesc);
  while (CFCalendarGetCalendarUnitFromDescription(&componentDesc, &unit))
    {
      value = NULL;
      switch (unit)
        {
          case kCFCalendarUnitYear:
            field = UCAL_YEAR;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitMonth:
            field = UCAL_MONTH;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitDay:
            field = UCAL_DAY_OF_MONTH;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitHour:
            field = UCAL_HOUR_OF_DAY;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitMinute:
            field = UCAL_MINUTE;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitSecond:
            field = UCAL_SECOND;
            value = va_arg (arg, int*);
            break;
          default:
            va_arg (arg, int*); /* Skip */
        }
      if (value)
        {
          *value = ucal_get (cal->_ucal, field, &err);
          if (unit == kCFCalendarUnitMonth)
            *value += 1;
        }
    }
  va_end(arg);
  
  if (U_FAILURE(err))
    return false;
  
  return true;
}

Boolean
CFCalendarGetComponentDifference (CFCalendarRef cal, CFAbsoluteTime startAT,
  CFAbsoluteTime resultAT, CFOptionFlags options,
  const char *componentDesc, ...)
{
  /* FIXME: ICU 4.8 introduced ucal_getFieldDifference() which
     should make implementing this function very easy. */
  va_list arg;
  int *value;
  int32_t mult;
  CFCalendarUnit unit = 0;
  UDate start;
  UDate end;
  UCalendarDateFields field;
  UCalendar *ucal;
  UErrorCode err = U_ZERO_ERROR;
  
  CFCalendarOpenUCalendar(cal);
  
  if (startAT > resultAT)
    {
      start = ABSOLUTETIME_TO_UDATE(resultAT);
      end = ABSOLUTETIME_TO_UDATE(startAT);
      mult = -1;
    }
  else
    {
      start = ABSOLUTETIME_TO_UDATE(startAT);
      end = ABSOLUTETIME_TO_UDATE(resultAT);
      mult = 1;
    }
  
  ucal = cal->_ucal;
  ucal_setMillis (ucal, start, &err);
  if (U_FAILURE(err))
    return false;
  
  va_start (arg, componentDesc);
  while (CFCalendarGetCalendarUnitFromDescription(&componentDesc, &unit))
    {
      int32_t min = 0;
      int32_t max = 1;
      double millis;
      
      value = NULL;
      switch (unit)
        {
          case kCFCalendarUnitYear:
            field = UCAL_YEAR;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitMonth:
            field = UCAL_MONTH;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitDay:
            field = UCAL_DAY_OF_MONTH;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitHour:
            field = UCAL_HOUR_OF_DAY;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitMinute:
            field = UCAL_MINUTE;
            value = va_arg (arg, int*);
            break;
          case kCFCalendarUnitSecond:
            field = UCAL_SECOND;
            value = va_arg (arg, int*);
            break;
          default:
            va_arg (arg, int*); /* Skip */
            continue;
        }
      
      /* Get a range */
      do
        {
          ucal_setMillis (ucal, start, &err);
          if (options & kCFCalendarComponentsWrap)
            ucal_roll (cal->_ucal, field, max, &err);
          else
            ucal_add (cal->_ucal, field, max, &err);
          millis = ucal_getMillis (ucal, &err);
          
          if (millis < end)
            {
              min = max;
              max <<= 1; /* multiply by 2... */
              if (max < 0)
                return false;
            }
        } while (millis < end && U_SUCCESS(err));
      
      if (millis != end)
        {
          while ((max - min) > 1 && U_SUCCESS(err))
            {
              int32_t add = (max + min) / 2;
              ucal_setMillis (ucal, start, &err);
              if (options & kCFCalendarComponentsWrap)
                ucal_roll (cal->_ucal, field, add, &err);
              else
                ucal_add (cal->_ucal, field, add, &err);
              millis = ucal_getMillis (ucal, &err);
              
              if (millis == end)
                {
                  *value = add * mult;
                  break;
                }
              else if (millis < end)
                {
                  min = add;
                }
              else
                {
                  max = add;
                }
            }
          
          if (millis > end)
            *value = min * mult;
        }
      else
        {
          *value = max * mult;
        }
      
      ucal_setMillis (ucal, start, &err);
      if (options & kCFCalendarComponentsWrap)
        ucal_roll (cal->_ucal, field, min, &err);
      else
        ucal_add (cal->_ucal, field, min, &err);
      start = ucal_getMillis (ucal, &err);
      
      if (U_FAILURE(err))
        return false;
    }
  va_end(arg);
  
  return true;
}

Boolean
CFCalendarGetTimeRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit,
  CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip)
{
  double start;
  double end;
  UCalendar *ucal;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarDateFields field = CFCalendarUnitToUCalendarDateFields (unit);
  
  CFCalendarOpenUCalendar (cal);
  
  ucal = cal->_ucal;
  
  ucal_setMillis (ucal, ABSOLUTETIME_TO_UDATE(at), &err);
  if (U_FAILURE(err))
    return false;
  
  /* Clear lower fields */
  switch (field)
    {
      case UCAL_ERA:
        ucal_set (ucal, UCAL_YEAR,
                  ucal_getLimit(ucal, UCAL_YEAR, UCAL_ACTUAL_MINIMUM, &err));
      case UCAL_YEAR:
        ucal_set (ucal, UCAL_MONTH,
                  ucal_getLimit(ucal, UCAL_MONTH, UCAL_ACTUAL_MINIMUM, &err));
      case UCAL_MONTH:
        ucal_set (ucal, UCAL_DATE,
                  ucal_getLimit(ucal, UCAL_DATE, UCAL_ACTUAL_MINIMUM, &err));
      case UCAL_DATE:
      case UCAL_DAY_OF_YEAR:
      case UCAL_DAY_OF_WEEK:
      case UCAL_DAY_OF_WEEK_IN_MONTH:
        ucal_set (ucal, UCAL_HOUR_OF_DAY,
                  ucal_getLimit(ucal, UCAL_HOUR_OF_DAY, UCAL_ACTUAL_MINIMUM, &err));
      case UCAL_HOUR_OF_DAY:
        ucal_set (ucal, UCAL_MINUTE,
                  ucal_getLimit(ucal, UCAL_MINUTE, UCAL_ACTUAL_MINIMUM, &err));
      case UCAL_MINUTE:
        ucal_set (ucal, UCAL_SECOND,
                  ucal_getLimit(ucal, UCAL_SECOND, UCAL_ACTUAL_MINIMUM, &err));
      default:
        break;
    }
  
  start = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
  
  if (startp)
    *startp = start;
  if (tip)
    {
      ucal_add (ucal, field, 1, &err);
      end = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
      *tip = end - start;
    }
  
  if (U_FAILURE(err))
    return false;
  
  return true;
}

CFRange
CFCalendarGetRangeOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
  CFCalendarUnit biggerUnit, CFAbsoluteTime at)
{
  return CFRangeMake (kCFNotFound, kCFNotFound);
}

CFIndex
CFCalendarGetOrdinalityOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
  CFCalendarUnit biggerUnit, CFAbsoluteTime at)
{
  return kCFNotFound;
}

static CFRange
CFCalendarGetMinMaxRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit,
  UCalendarLimitType min, UCalendarLimitType max)
{
  CFRange range;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarDateFields field = CFCalendarUnitToUCalendarDateFields (unit);
  
  CFCalendarOpenUCalendar (cal);
  
  range.location = ucal_getLimit (cal->_ucal, field, min, &err);
  range.length = ucal_getLimit (cal->_ucal, field, max, &err)
    - range.location + 1;
  if (unit == kCFCalendarUnitMonth)
    range.location += 1;
  
  return range;
}

CFRange
CFCalendarGetMaximumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit)
{
  return CFCalendarGetMinMaxRangeOfUnit (cal, unit,
    UCAL_GREATEST_MINIMUM, UCAL_LEAST_MAXIMUM);
}

CFRange
CFCalendarGetMinimumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit)
{
  return CFCalendarGetMinMaxRangeOfUnit (cal, unit,
    UCAL_MINIMUM, UCAL_MAXIMUM);
}

