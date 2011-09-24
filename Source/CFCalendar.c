/* CFCalendar.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFRuntime.h"

#include <pthread.h>
#include <unicode/ucal.h>

struct __CFCalendar
{
  CFRuntimeBase _parent;
  UCalendar    *_ucal;
  CFStringRef   _ident;
  CFStringRef   _localeIdent;
  CFStringRef   _tzIdent;
};

static CFTypeID _kCFCalendarTypeID;
static CFCalendarRef _kCFCalendarCurrent = NULL;
static pthread_mutex_t _kCFCalendarLock = PTHREAD_MUTEX_INITIALIZER;

#define UDATE_TO_ABSOLUTETIME(d) \
  (((d) / 1000.0) - kCFAbsoluteTimeIntervalSince1970)
#define ABSOLUTETIME_TO_UDATE(at) \
  (((at) + kCFAbsoluteTimeIntervalSince1970) * 1000.0)

static Boolean
CFCalendarSetupUCalendar (CFCalendarRef cal)
{
  char localeIdent[ULOC_FULLNAME_CAPACITY];
  UniChar *tzIdent = NULL; // FIXME
  UCalendar *ucal;
  UErrorCode err = U_ZERO_ERROR;
  
  if (cal->_localeIdent != NULL)
    {
      char calIdent[ULOC_KEYWORDS_CAPACITY];
      CFStringGetCString (cal->_localeIdent, localeIdent,
        ULOC_FULLNAME_CAPACITY, kCFStringEncodingASCII);
      CFStringGetCString (cal->_ident, calIdent, ULOC_KEYWORDS_CAPACITY,
        kCFStringEncodingASCII);
      uloc_setKeywordValue ("calendar", (const char*)calIdent, localeIdent,
        ULOC_FULLNAME_CAPACITY, &err);
    }
  
  ucal = ucal_open (tzIdent, 0, localeIdent, UCAL_TRADITIONAL, &err);
  if (U_FAILURE(err))
    {
      ucal = NULL;
      return false;
    }
  
  cal->_ucal = ucal;
  return true;
}

static inline Boolean
CFCalendarOpenUCalendar (CFCalendarRef cal)
{
  if (cal->_ucal == NULL)
    return CFCalendarSetupUCalendar (cal);
  
  ucal_clear (cal->_ucal);
  return true; // Already open
}

static inline void
CFCalendarCloseUCalendar (CFCalendarRef cal)
{
  ucal_close (cal->_ucal);
  cal->_ucal = NULL;
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
  "CFLocale",
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
}

static inline UCalendarDateFields
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
        ret = UCAL_MONTH; // FIXME
        break;
      default:
        ret = -1;
    }
  return ret;
}

/* This function returns true is there is still characters to be consumed. */
static inline Boolean
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
      pthread_mutex_lock (&_kCFCalendarLock);
      if (_kCFCalendarCurrent == NULL)
        {
          CFLocaleRef locale;
          CFStringRef calIdent;
          CFCalendarRef cal;
          
          locale = CFLocaleCopyCurrent ();
          calIdent = CFLocaleGetValue (locale, kCFLocaleCalendarIdentifier);
          cal = CFCalendarCreateWithIdentifier (NULL, calIdent);
          CFCalendarSetLocale (cal, locale);
          
          CFRelease (calIdent);
          CFRelease (locale);
          _kCFCalendarCurrent = cal;
        }
      pthread_mutex_unlock (&_kCFCalendarLock);
    }
  
  return _kCFCalendarCurrent;
}

CFCalendarRef
CFCalendarCreateWithIdentifier (CFAllocatorRef allocator, CFStringRef ident)
{
  CFLocaleRef locale;
  CFCalendarRef new;
  
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
  
  new->_tzIdent = NULL; // FIXME
  
  return new;
}

CFTimeZoneRef
CFCalendarCopyTimeZone (CFCalendarRef cal)
{
  return NULL; // FIXME
}

void
CFCalendarSetTimeZone (CFCalendarRef cal, CFTimeZoneRef tz)
{
  // FIXME
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
  if (CFCalendarOpenUCalendar (cal))
    ucal_setAttribute (cal->_ucal, UCAL_FIRST_DAY_OF_WEEK, wkdy);
}

CFIndex
CFCalendarGetMinimumDaysInFirstWeek (CFCalendarRef cal)
{
  if (CFCalendarOpenUCalendar (cal))
    return ucal_getAttribute (cal->_ucal, UCAL_MINIMAL_DAYS_IN_FIRST_WEEK);
  return 0;
}

void
CFCalendarSetMinimumDaysInFirstWeek (CFCalendarRef cal, CFIndex mwd)
{
  if (CFCalendarOpenUCalendar (cal))
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
  
  if (!CFCalendarOpenUCalendar(cal))
    return false;
  
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
            va_arg (arg, int); // Skip
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
  int year = 0, month = 0, date = 0, hour = 0, minute = 0, second = 0;
  CFCalendarUnit unit = 0;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFCalendarOpenUCalendar(cal))
    return false;
  
  va_start (arg, componentDesc);
  while (CFCalendarGetCalendarUnitFromDescription(&componentDesc, &unit))
    {
      switch (unit)
        {
          case kCFCalendarUnitYear:
            year = va_arg (arg, int);
            break;
          case kCFCalendarUnitMonth:
            month = va_arg (arg, int) - 1;
            break;
          case kCFCalendarUnitDay:
            date = va_arg (arg, int);
            break;
          case kCFCalendarUnitHour:
            hour = va_arg (arg, int);
            break;
          case kCFCalendarUnitMinute:
            minute = va_arg (arg, int);
            break;
          case kCFCalendarUnitSecond:
            second = va_arg (arg, int);
            break;
          default:
            va_arg (arg, int); // Skip
        }
    }
  va_end(arg);
  
  ucal_clear (cal->_ucal);
  ucal_setDateTime (cal->_ucal, year, month, date, hour, minute, second, &err);
  if (U_FAILURE(err))
    return false;
  
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
  
  if (!CFCalendarOpenUCalendar(cal))
    return false;
  
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
            va_arg (arg, int*); // Skip
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
  
  if (!CFCalendarOpenUCalendar(cal))
    return false;
  
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
            va_arg (arg, int*); // Skip
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
              max <<= 1; // multiply by 2...
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
  int32_t min, max;
  double start, end;
  UCalendar *ucal;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarDateFields field = CFCalendarUnitToUCalendarDateFields (unit);
  
  if (!CFCalendarOpenUCalendar (cal))
    return false;
  
  ucal = cal->_ucal;
  
  ucal_setMillis (cal->_ucal, ABSOLUTETIME_TO_UDATE(at), &err);
  if (U_FAILURE(err))
    return false;
  
  min = ucal_getLimit(ucal, field, UCAL_ACTUAL_MINIMUM, &err);
  max = ucal_getLimit(ucal, field, UCAL_ACTUAL_MAXIMUM, &err);
  
  /* Clear lower fields */
  switch (field)
    {
      case UCAL_ERA:
        ucal_clearField (ucal, UCAL_YEAR);
      case UCAL_YEAR:
        ucal_clearField (ucal, UCAL_MONTH);
      case UCAL_MONTH:
        ucal_clearField (ucal, UCAL_DATE);
      case UCAL_DATE:
      case UCAL_DAY_OF_YEAR:
      case UCAL_DAY_OF_WEEK:
      case UCAL_DAY_OF_WEEK_IN_MONTH:
        ucal_clearField (ucal, UCAL_HOUR_OF_DAY);
      case UCAL_HOUR_OF_DAY:
        ucal_clearField (ucal, UCAL_MINUTE);
      case UCAL_MINUTE:
        ucal_clearField (ucal, UCAL_SECOND);
      default:
        break;
    }
  
  ucal_set (ucal, field, min);
  start = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
  ucal_add (ucal, field, max, &err);
  end = UDATE_TO_ABSOLUTETIME(ucal_getMillis (cal->_ucal, &err));
  
  if (startp)
    *startp = start;
  if (tip)
    *tip = end - start;
  
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

CFRange
CFCalendarGetMaximumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit)
{
  CFRange range;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarDateFields field = CFCalendarUnitToUCalendarDateFields (unit);
  
  if (!CFCalendarOpenUCalendar (cal))
    return CFRangeMake (kCFNotFound, 0);
  
  range.location = ucal_getLimit (cal->_ucal, field, UCAL_GREATEST_MINIMUM,
    &err);
  range.length = ucal_getLimit (cal->_ucal, field, UCAL_LEAST_MAXIMUM, &err)
    - range.location + 1;
  if (unit == kCFCalendarUnitMonth)
    range.location += 1;
  
  return range;
}

CFRange
CFCalendarGetMinimumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit)
{
  CFRange range;
  UErrorCode err = U_ZERO_ERROR;
  UCalendarDateFields field = CFCalendarUnitToUCalendarDateFields (unit);
  
  if (!CFCalendarOpenUCalendar (cal))
    return CFRangeMake (kCFNotFound, 0);
  
  range.location = ucal_getLimit (cal->_ucal, field, UCAL_MINIMUM,
    &err);
  range.length = ucal_getLimit (cal->_ucal, field, UCAL_MAXIMUM, &err)
    - range.location + 1;
  if (unit == kCFCalendarUnitMonth)
    range.location += 1;
  
  return range;
}
