/* CFDateFormatter.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFDateFormatter.h"
#include "GSPrivate.h"

#include <unicode/udat.h>
#include <unicode/udatpg.h>

#define BUFFER_SIZE 256

CONST_STRING_DECL(kCFDateFormatterIsLenient,
  "kCFDateFormatterIsLenient");
CONST_STRING_DECL(kCFDateFormatterTimeZone,
  "kCFDateFormatterTimeZone");
CONST_STRING_DECL(kCFDateFormatterCalendarName,
  "kCFDateFormatterCalendarName");
CONST_STRING_DECL(kCFDateFormatterDefaultFormat,
  "kCFDateFormatterDefaultFormat");
CONST_STRING_DECL(kCFDateFormatterTwoDigitStartDate,
  "kCFDateFormatterTwoDigitStartDate");
CONST_STRING_DECL(kCFDateFormatterDefaultDate,
  "kCFDateFormatterDefaultDate");
CONST_STRING_DECL(kCFDateFormatterCalendar,
  "kCFDateFormatterCalendar");
CONST_STRING_DECL(kCFDateFormatterEraSymbols,
  "kCFDateFormatterEraSymbols");
CONST_STRING_DECL(kCFDateFormatterMonthSymbols,
  "kCFDateFormatterMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterShortMonthSymbols,
  "kCFDateFormatterShortMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterWeekdaySymbols,
  "kCFDateFormatterWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterShortWeekdaySymbols,
  "kCFDateFormatterShortWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterAMSymbol,
  "kCFDateFormatterAMSymbol");
CONST_STRING_DECL(kCFDateFormatterPMSymbol,
  "kCFDateFormatterPMSymbol");
CONST_STRING_DECL(kCFDateFormatterLongEraSymbols,
  "kCFDateFormatterLongEraSymbols");
CONST_STRING_DECL(kCFDateFormatterVeryShortMonthSymbols,
  "kCFDateFormatterVeryShortMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterStandaloneMonthSymbols,
  "kCFDateFormatterStandaloneMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneMonthSymbols,
  "kCFDateFormatterShortStandaloneMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterVeryShortStandaloneMonthSymbols,
  "kCFDateFormatterVeryShortStandaloneMonthSymbols");
CONST_STRING_DECL(kCFDateFormatterVeryShortWeekdaySymbols,
  "kCFDateFormatterVeryShortWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterStandaloneWeekdaySymbols,
  "kCFDateFormatterStandaloneWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneWeekdaySymbols,
  "kCFDateFormatterShortStandaloneWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
  "kCFDateFormatterVeryShortStandaloneWeekdaySymbols");
CONST_STRING_DECL(kCFDateFormatterQuarterSymbols,
  "kCFDateFormatterQuarterSymbols");
CONST_STRING_DECL(kCFDateFormatterShortQuarterSymbols,
  "kCFDateFormatterShortQuarterSymbols");
CONST_STRING_DECL(kCFDateFormatterStandaloneQuarterSymbols,
  "kCFDateFormatterStandaloneQuarterSymbols");
CONST_STRING_DECL(kCFDateFormatterShortStandaloneQuarterSymbols,
  "kCFDateFormatterShortStandaloneQuarterSymbols");
CONST_STRING_DECL(kCFDateFormatterGregorianStartDate,
  "kCFDateFormatterGregorianStartDate");

struct __CFDateFormatter
{
  CFRuntimeBase _parent;
  UDateFormat  *_fmt;
  CFLocaleRef   _locale;
  CFTimeZoneRef _tz;
  CFDateFormatterStyle _dateStyle;
  CFDateFormatterStyle _timeStyle;
  CFStringRef   _defaultFormat;
  CFStringRef   _format;
};

static CFTypeID _kCFDateFormatterTypeID = 0;

static const CFRuntimeClass CFDateFormatterClass =
{
  0,
  "CFDateFormatter",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFDateFormatterInitialize (void)
{
  _kCFDateFormatterTypeID = _CFRuntimeRegisterClass(&CFDateFormatterClass);
}



CF_INLINE UDateFormatStyle
CFDataFormatterStyleToUDateFormatStyle (CFDateFormatterStyle style)
{
  UDateFormatStyle us = 0;
  switch (style)
    {
      case kCFDateFormatterNoStyle:
        us = UDAT_NONE;
        break;
      case kCFDateFormatterShortStyle:
        us = UDAT_SHORT;
        break;
      case kCFDateFormatterMediumStyle:
        us = UDAT_MEDIUM;
        break;
      case kCFDateFormatterLongStyle:
        us = UDAT_LONG;
        break;
      case kCFDateFormatterFullStyle:
        us = UDAT_FULL;
        break;
    }
  return us;
}

static void
CFDateFormatterSetup (CFDateFormatterRef dfmt)
{
  UDateFormatStyle utStyle;
  UDateFormatStyle udStyle;
  const char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  UniChar uTzID[BUFFER_SIZE];
  CFIndex uTzIDLength;
  UErrorCode err = U_ZERO_ERROR;
  
  if (dfmt->_fmt)
    udat_close (dfmt->_fmt);
  
  utStyle = CFDataFormatterStyleToUDateFormatStyle (dfmt->_timeStyle);
  udStyle = CFDataFormatterStyleToUDateFormatStyle (dfmt->_dateStyle);
  
  cLocale = CFLocaleGetCStringIdentifier (dfmt->_locale, buffer,
                                          ULOC_FULLNAME_CAPACITY);
  uTzIDLength = CFStringGetLength (CFTimeZoneGetName(dfmt->_tz));
  
  if (uTzIDLength > BUFFER_SIZE)
    uTzIDLength = BUFFER_SIZE;
  CFStringGetCharacters (CFTimeZoneGetName(dfmt->_tz),
    CFRangeMake(0, uTzIDLength), uTzID);
  
  dfmt->_fmt =
    udat_open (utStyle, udStyle, cLocale, uTzID, uTzIDLength, NULL, 0, &err);
  if (U_FAILURE(err))
    dfmt->_fmt = NULL;
}

static void
CFDateFormatterSetLenient (CFDateFormatterRef fmt, int prop, CFTypeRef value)
{
  udat_setLenient (fmt->_fmt, value == kCFBooleanTrue ? true : false);
}

static CFTypeRef
CFDateFormatterCopyLenient (CFDateFormatterRef fmt, int prop)
{
  return CFRetain(udat_isLenient(fmt->_fmt) ? kCFBooleanTrue : kCFBooleanFalse);
}

static void
CFDateFormatterSetTimeZone (CFDateFormatterRef fmt, int prop, CFTypeRef value)
{
  if (!CFEqual(fmt->_tz, value))
    CFRelease (fmt->_tz);
  fmt->_tz = CFRetain (value);
}

static CFTypeRef
CFDateFormatterCopyTimeZone (CFDateFormatterRef fmt, int prop)
{
  return CFRetain (fmt->_tz);
}

static void
CFDateFormatterSetCalendarName (CFDateFormatterRef fmt, int prop,
  CFTypeRef value)
{
  CFDictionaryRef dict;
  CFMutableDictionaryRef mDict;
  CFStringRef locale;
  
  dict = CFLocaleCreateComponentsFromLocaleIdentifier (NULL,
    CFLocaleGetIdentifier (fmt->_locale));
  mDict = CFDictionaryCreateMutableCopy (NULL, 0, dict);
  CFRelease (dict);
  CFDictionarySetValue (mDict, kCFLocaleCalendarIdentifier, value);
  
  locale = CFLocaleCreateLocaleIdentifierFromComponents (NULL, mDict);
  CFRelease (fmt->_locale);
  fmt->_locale = CFLocaleCreate (NULL, locale);
  CFRelease (locale);
  CFRelease (mDict);
  
  CFDateFormatterSetup (fmt);
}

static CFTypeRef
CFDateFormatterCopyCalendarName (CFDateFormatterRef fmt, int prop)
{
  CFStringRef calendarName;
  
  calendarName = CFLocaleGetValue (fmt->_locale, kCFLocaleCalendarIdentifier);
  
  return CFRetain(calendarName);
}

static void
CFDateFormatterSetDefaultFormat (CFDateFormatterRef fmt, int prop,
  CFTypeRef value)
{
  /* Do nothing, this property can't be changed */
}

static CFTypeRef
CFDateFormatterCopyDefaultFormat (CFDateFormatterRef fmt, int prop)
{
  return CFRetain (fmt->_defaultFormat);
}

static void
CFDateFormatterSetTwoDigitYearStart (CFDateFormatterRef fmt, int prop,
  CFTypeRef value)
{
  UDate udate = ABSOLUTETIME_TO_UDATE(CFDateGetAbsoluteTime(value));
  UErrorCode err = U_ZERO_ERROR;
  udat_set2DigitYearStart (fmt->_fmt, udate, &err);
}

static CFTypeRef
CFDateFormatterCopyTwoDigitYearStart (CFDateFormatterRef fmt, int prop)
{
  UErrorCode err = U_ZERO_ERROR;
  CFAbsoluteTime at =
    UDATE_TO_ABSOLUTETIME(udat_get2DigitYearStart (fmt->_fmt, &err));
  return CFDateCreate (NULL, at);
}

static void
CFDateFormatterSetDefaultDate (CFDateFormatterRef fmt, int prop,
  CFTypeRef value)
{
  
}

static CFTypeRef
CFDateFormatterCopyDefaultDate (CFDateFormatterRef fmt, int prop)
{
  return NULL;
}


static void
CFDateFormatterSetCalendar (CFDateFormatterRef fmt, int prop, CFTypeRef value)
{
  
}

static CFTypeRef
CFDateFormatterCopyCalendar (CFDateFormatterRef fmt, int prop)
{
  return NULL;
}

static void
CFDateFromatterSetSymbolAtIndex (UDateFormat fmt, UDateFormatSymbolType type,
  CFStringRef value, CFIndex idx)
{
  CFIndex textLength;
  UniChar text[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  textLength = CFStringGetLength (value);
  if (textLength > BUFFER_SIZE)
    textLength = BUFFER_SIZE;
  CFStringGetCharacters (value, CFRangeMake(0, textLength), text);
  
  udat_setSymbols (fmt, type, idx, text, textLength, &err);
}

static CFStringRef
CFDateFormatterCopySymbolAtIndex (UDateFormat fmt, UDateFormatSymbolType type,
  CFIndex idx)
{
  CFIndex textLength;
  UniChar text[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  textLength = udat_getSymbols (fmt, type, idx, text, BUFFER_SIZE, &err);
  if (textLength > BUFFER_SIZE)
    textLength = BUFFER_SIZE;
  return CFStringCreateWithCharacters (NULL, text, textLength);
}

static void
CFDateFormatterSetSymbols (CFDateFormatterRef fmt, int prop, CFTypeRef value)
{
  CFIndex idx;
  CFIndex count;
  CFArrayRef array = (CFArrayRef)value;
  
  count = CFArrayGetCount (array);
  /* Something's wrong if count != # of symbols in ICU. */
  if (count != udat_countSymbols(fmt->_fmt, prop))
    return;
  
  for (idx = 0 ; idx < count ; ++ idx)
    {
      CFDateFromatterSetSymbolAtIndex (fmt->_fmt, prop,
        CFArrayGetValueAtIndex (array, idx), idx);
    }
}

static CFTypeRef
CFDateFormatterCopySymbols (CFDateFormatterRef fmt, int prop)
{
  CFIndex idx;
  CFIndex count;
  CFStringRef str;
  CFMutableArrayRef mArray;
  CFArrayRef ret = NULL;
  
  count = udat_countSymbols(fmt->_fmt, prop);
  mArray = CFArrayCreateMutable (NULL, count, &kCFTypeArrayCallBacks);
  
  for (idx = 0 ; idx < count ; ++ idx)
    {
      str = CFDateFormatterCopySymbolAtIndex (fmt->_fmt, prop, idx);
      CFArrayAppendValue (mArray, str);
      CFRelease (str);
    }
  
  ret = CFArrayCreateCopy (NULL, mArray);
  CFRelease (mArray);
  
  return ret;
}

static void
CFDateFormatterSetAMPMSymbol (CFDateFormatterRef fmt, int idx,
  CFTypeRef value)
{
  CFDateFromatterSetSymbolAtIndex (fmt->_fmt, UDAT_AM_PMS, value, idx);
}

static CFTypeRef
CFDateFormatterCopyAMPMSymbol (CFDateFormatterRef fmt, int idx)
{
  return CFDateFormatterCopySymbolAtIndex (fmt->_fmt, UDAT_AM_PMS, idx);
}

static void
CFDateFormatterSetGregorianStartDate (CFDateFormatterRef fmt, int prop,
  CFTypeRef value)
{
  CFAbsoluteTime at;
  UErrorCode err = U_ZERO_ERROR;
  UCalendar *ucal = (UCalendar*)udat_getCalendar (fmt->_fmt);
  
  at = CFDateGetAbsoluteTime (value);
  ucal_setGregorianChange (ucal, ABSOLUTETIME_TO_UDATE(at), &err);
}

static CFTypeRef
CFDateFormatterCopyGregorianStartDate (CFDateFormatterRef fmt, int prop)
{
  UDate udate;
  UErrorCode err = U_ZERO_ERROR;
  UCalendar *ucal = (UCalendar*)udat_getCalendar (fmt->_fmt);
  
  udate = ucal_getGregorianChange (ucal, &err);
  return CFDateCreate (NULL, UDATE_TO_ABSOLUTETIME(udate));
}

static struct _kCFDateFormatterProperties
{
  const CFStringRef *prop;
  int icuProp;
  void (*set)(CFDateFormatterRef fmt, int prop, CFTypeRef value);
  CFTypeRef (*copy)(CFDateFormatterRef fmt, int prop);
} _kCFDateFormatterProperties[] =
{
  { &kCFDateFormatterIsLenient, 0,
      CFDateFormatterSetLenient, CFDateFormatterCopyLenient },
  { &kCFDateFormatterTimeZone, 0,
      CFDateFormatterSetTimeZone, CFDateFormatterCopyTimeZone },
  { &kCFDateFormatterCalendarName, 0,
      CFDateFormatterSetCalendarName, CFDateFormatterCopyCalendarName },
  { &kCFDateFormatterDefaultFormat, 0,
      CFDateFormatterSetDefaultFormat, CFDateFormatterCopyDefaultFormat },
  { &kCFDateFormatterTwoDigitStartDate, 0,
      CFDateFormatterSetTwoDigitYearStart,
      CFDateFormatterCopyTwoDigitYearStart },
  { &kCFDateFormatterDefaultDate, 0,
      CFDateFormatterSetDefaultDate, CFDateFormatterCopyDefaultDate },
  { &kCFDateFormatterCalendar, 0,
      CFDateFormatterSetCalendar, CFDateFormatterCopyCalendar },
  { &kCFDateFormatterEraSymbols, UDAT_ERAS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterMonthSymbols, UDAT_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortMonthSymbols, UDAT_SHORT_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterWeekdaySymbols, UDAT_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortWeekdaySymbols, UDAT_SHORT_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterAMSymbol, 0,
      CFDateFormatterSetAMPMSymbol, CFDateFormatterCopyAMPMSymbol },
  { &kCFDateFormatterPMSymbol, 1,
      CFDateFormatterSetAMPMSymbol, CFDateFormatterCopyAMPMSymbol },
  { &kCFDateFormatterLongEraSymbols, UDAT_ERA_NAMES,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterVeryShortMonthSymbols, UDAT_NARROW_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterStandaloneMonthSymbols, UDAT_STANDALONE_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortStandaloneMonthSymbols,
      UDAT_STANDALONE_SHORT_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterVeryShortStandaloneMonthSymbols,
      UDAT_STANDALONE_NARROW_MONTHS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterVeryShortWeekdaySymbols, UDAT_NARROW_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterStandaloneWeekdaySymbols, UDAT_STANDALONE_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortStandaloneWeekdaySymbols,
      UDAT_STANDALONE_SHORT_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
      UDAT_STANDALONE_NARROW_WEEKDAYS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterQuarterSymbols, UDAT_QUARTERS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortQuarterSymbols, UDAT_SHORT_QUARTERS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterStandaloneQuarterSymbols, UDAT_STANDALONE_QUARTERS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterShortStandaloneQuarterSymbols,
      UDAT_STANDALONE_SHORT_QUARTERS,
      CFDateFormatterSetSymbols, CFDateFormatterCopySymbols },
  { &kCFDateFormatterGregorianStartDate, 0,
      CFDateFormatterSetGregorianStartDate,
      CFDateFormatterCopyGregorianStartDate }
};
static const CFIndex
_kCFDateFormatterPropertiesCount = sizeof(_kCFDateFormatterProperties) /
  sizeof(struct _kCFDateFormatterProperties);



#define CFDATEFORMATTER_SIZE \
  sizeof(struct __CFDateFormatter) - sizeof(CFRuntimeBase)

CFDateFormatterRef
CFDateFormatterCreate (CFAllocatorRef alloc, CFLocaleRef locale,
  CFDateFormatterStyle dateStyle, CFDateFormatterStyle timeStyle)
{
  struct __CFDateFormatter *new;
  
  new = (struct __CFDateFormatter*)_CFRuntimeCreateInstance (alloc,
    _kCFDateFormatterTypeID, CFDATEFORMATTER_SIZE, 0);
  if (new)
    {
      CFIndex formatLength;
      UniChar format[BUFFER_SIZE];
      UErrorCode err = U_ZERO_ERROR;
      
      new->_locale = locale ? CFRetain(locale) : CFLocaleCopyCurrent ();
      new->_tz = CFTimeZoneCopyDefault ();
      new->_dateStyle = dateStyle;
      new->_timeStyle = timeStyle;
      
      CFDateFormatterSetup (new);
      
      formatLength =
        udat_toPattern (new->_fmt, false, format, BUFFER_SIZE, &err);
      if (formatLength > BUFFER_SIZE)
        formatLength = BUFFER_SIZE;
      new->_defaultFormat =
        CFStringCreateWithCharacters (alloc, format, formatLength);
      new->_format = CFRetain (new->_defaultFormat);
    }
  
  return new;
}

void
CFDateFormatterSetFormat (CFDateFormatterRef fmt, CFStringRef formatString)
{
  if (formatString != fmt->_format)
    CFRelease (fmt->_format);
  fmt->_format = CFRetain (formatString);
}

void
CFDateFormatterSetProperty (CFDateFormatterRef fmt, CFStringRef key,
  CFTypeRef value)
{
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesCount ; ++idx)
    {
      if (key == *(_kCFDateFormatterProperties[idx].prop))
        {
          (_kCFDateFormatterProperties[idx].set)(fmt,
            _kCFDateFormatterProperties[idx].icuProp, value);
          return;
        }
    }
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesCount ; ++idx)
    {
      if (CFEqual(key, *(_kCFDateFormatterProperties[idx].prop)))
        {
          (_kCFDateFormatterProperties[idx].set)(fmt,
            _kCFDateFormatterProperties[idx].icuProp, value);
          return;
        }
    }
}

CFDateRef
CFDateFormatterCreateDateFromString (CFAllocatorRef alloc,
  CFDateFormatterRef fmt, CFStringRef string, CFRange *rangep)
{
  CFAbsoluteTime at;
  
  if (CFDateFormatterGetAbsoluteTimeFromString (fmt, string, rangep, &at))
    return CFDateCreate (alloc, at);
  return NULL;
}

Boolean
CFDateFormatterGetAbsoluteTimeFromString (CFDateFormatterRef fmt,
  CFStringRef string, CFRange *rangep, CFAbsoluteTime *atp)
{
  UDate udate;
  UniChar text[BUFFER_SIZE];
  CFRange range;
  UErrorCode err = U_ZERO_ERROR;
  int32_t pPos = 0;
  
  CFDateFormatterSetup (fmt);
  
  if (rangep)
    range = *rangep;
  else
    range = CFRangeMake (0, CFStringGetLength(string));
  CFStringGetCharacters (string, range, text);
  
  udate = udat_parse (fmt->_fmt, text, range.length, &pPos, &err);
  if (U_FAILURE(err))
    return false;
  
  if (rangep)
    rangep->length = pPos;
  if (atp)
    *atp = UDATE_TO_ABSOLUTETIME(udate);
  return true;
}

CFStringRef
CFDateFormatterCreateStringWithAbsoluteTime (CFAllocatorRef alloc,
  CFDateFormatterRef fmt, CFAbsoluteTime at)
{
  CFStringRef result;
  CFIndex length;
  UniChar string[BUFFER_SIZE];
  UDate udate = ABSOLUTETIME_TO_UDATE(at);
  UErrorCode err = U_ZERO_ERROR;
  
  CFDateFormatterSetup (fmt);
  
  length = udat_format (fmt->_fmt, udate, string, BUFFER_SIZE, NULL, &err);
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  if (U_FAILURE(err))
    return NULL;
  
  result = CFStringCreateWithCharacters (alloc, string, length);
  return result;
}

CFStringRef
CFDateFormatterCreateStringWithDate (CFAllocatorRef alloc,
  CFDateFormatterRef fmt, CFDateRef date)
{
  CFAbsoluteTime at = CFDateGetAbsoluteTime (date);
  
  return CFDateFormatterCreateStringWithAbsoluteTime (alloc, fmt, at);
}

CFStringRef
CFDateFormatterCreateDateFormatFromTemplate (CFAllocatorRef alloc,
  CFStringRef templ, CFOptionFlags options, CFLocaleRef loc)
{
  const char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  UniChar pat[BUFFER_SIZE];
  UniChar skel[BUFFER_SIZE];
  CFIndex patLen;
  CFIndex skelLen;
  UDateTimePatternGenerator *datpg;
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier (loc, buffer, ULOC_FULLNAME_CAPACITY);
  datpg = udatpg_open (cLocale, &err);
  if (U_FAILURE(err))
    return NULL;
  
  if ((patLen = CFStringGetLength(templ)) > BUFFER_SIZE)
    patLen = BUFFER_SIZE;
  CFStringGetCharacters(templ, CFRangeMake(0, patLen), pat);
  
  skelLen = udatpg_getSkeleton (datpg, pat, patLen, skel, BUFFER_SIZE, &err);
  if (U_FAILURE(err))
    return NULL;
  
  patLen =
    udatpg_getBestPattern (datpg, skel, skelLen, pat, BUFFER_SIZE, &err);
  
  udatpg_close (datpg);
  
  return CFStringCreateWithCharacters (alloc, pat, patLen);
}

CFTypeRef
CFDateFormatterCopyProperty (CFDateFormatterRef fmt, CFStringRef key)
{
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesCount ; ++idx)
    {
      if (key == *(_kCFDateFormatterProperties[idx].prop))
        return (_kCFDateFormatterProperties[idx].copy)(fmt,
          _kCFDateFormatterProperties[idx].icuProp);
    }
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesCount ; ++idx)
    {
      if (CFEqual(key, *(_kCFDateFormatterProperties[idx].prop)))
        return (_kCFDateFormatterProperties[idx].copy)(fmt,
          _kCFDateFormatterProperties[idx].icuProp);
    }
  
  return NULL;
}

CFDateFormatterStyle
CFDateFormatterGetDateStyle (CFDateFormatterRef fmt)
{
  return fmt->_dateStyle;
}

CFStringRef
CFDateFormatterGetFormat (CFDateFormatterRef fmt)
{
  return fmt->_format;
}

CFLocaleRef
CFDateFormatterGetLocale (CFDateFormatterRef fmt)
{
  return fmt->_locale;
}

CFDateFormatterStyle
CFDateFormatterGetTimeStyle (CFDateFormatterRef fmt)
{
  return fmt->_timeStyle;
}

CFTypeID
CFDateFormatterGetTypeID (void)
{
  return _kCFDateFormatterTypeID;
}

