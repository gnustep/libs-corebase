/* CFDateFormatter.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFDateFormatter.h"

#include <unicode/udat.h>

#define BUFFER_SIZE 512

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



static struct _kCFDateFormatterProperties
{
  const CFStringRef *prop;
  int icuProp;
  void (*set)(CFDateFormatterRef fmt, int attrib, CFTypeRef value);
  CFTypeRef (*copy)(CFDateFormatterRef fmt, int attrib);
} _kCFDateFormatterProperties[] =
{
  { &kCFDateFormatterIsLenient, 0,
      NULL, NULL },
  { &kCFDateFormatterTimeZone, 0,
      NULL, NULL },
  { &kCFDateFormatterCalendarName, 0,
      NULL, NULL },
  { &kCFDateFormatterDefaultFormat, 0,
      NULL, NULL },
  { &kCFDateFormatterTwoDigitStartDate, 0,
      NULL, NULL },
  { &kCFDateFormatterDefaultDate, 0,
      NULL, NULL },
  { &kCFDateFormatterCalendar, 0,
      NULL, NULL },
  { &kCFDateFormatterEraSymbols, UDAT_ERAS,
      NULL, NULL },
  { &kCFDateFormatterMonthSymbols, UDAT_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterShortMonthSymbols, UDAT_SHORT_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterWeekdaySymbols, UDAT_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterShortWeekdaySymbols, UDAT_SHORT_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterAMSymbol, UDAT_AM_PMS,
      NULL, NULL },
  { &kCFDateFormatterPMSymbol, UDAT_AM_PMS,
      NULL, NULL },
  { &kCFDateFormatterLongEraSymbols, UDAT_ERA_NAMES,
      NULL, NULL },
  { &kCFDateFormatterVeryShortMonthSymbols, UDAT_NARROW_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterStandaloneMonthSymbols, UDAT_STANDALONE_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterShortStandaloneMonthSymbols,
      UDAT_STANDALONE_SHORT_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterVeryShortStandaloneMonthSymbols,
      UDAT_STANDALONE_NARROW_MONTHS,
      NULL, NULL },
  { &kCFDateFormatterVeryShortWeekdaySymbols, UDAT_NARROW_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterStandaloneWeekdaySymbols, UDAT_STANDALONE_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterShortStandaloneWeekdaySymbols,
      UDAT_STANDALONE_SHORT_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
      UDAT_STANDALONE_NARROW_WEEKDAYS,
      NULL, NULL },
  { &kCFDateFormatterQuarterSymbols, UDAT_QUARTERS,
      NULL, NULL },
  { &kCFDateFormatterShortQuarterSymbols, UDAT_SHORT_QUARTERS,
      NULL, NULL },
  { &kCFDateFormatterStandaloneQuarterSymbols, UDAT_STANDALONE_QUARTERS,
      NULL, NULL },
  { &kCFDateFormatterShortStandaloneQuarterSymbols,
      UDAT_STANDALONE_SHORT_QUARTERS,
      NULL, NULL },
  { &kCFDateFormatterGregorianStartDate, 0,
      NULL, NULL }
};
static const CFIndex
_kCFDateFormatterPropertiesSize = sizeof(_kCFDateFormatterProperties) /
  sizeof(struct _kCFDateFormatterProperties);



CF_INLINE UDateFormatStyle
CFDataFormatterStyleToUDateFormatStyle (CFDateFormatterStyle style)
{
  UDateFormatStyle us;
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
      new->_locale = locale ? CFRetain(locale) : CFLocaleCopyCurrent ();
      new->_tz = CFTimeZoneCopyDefault ();
      new->_dateStyle = dateStyle;
      new->_timeStyle = timeStyle;
    }
  
  return new;
}

void
CFDateFormatterSetFormat (CFDateFormatterRef fmt, CFStringRef formatString)
{
  
}

void
CFDateFormatterSetProperty (CFDateFormatterRef fmt, CFStringRef key,
  CFTypeRef value)
{
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesSize ; ++idx)
    {
      if (key == *(_kCFDateFormatterProperties[idx].prop))
        {
          (_kCFDateFormatterProperties[idx].set)(fmt,
            _kCFDateFormatterProperties[idx].icuProp, value);
          return;
        }
    }
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesSize ; ++idx)
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
  return NULL;
}

Boolean
CFDateFormatterGetAbsoluteTimeFromString (CFDateFormatterRef fmt,
  CFStringRef string, CFRange *rangep, CFAbsoluteTime *atp)
{
  return false;
}

CFStringRef
CFDateFormatterCreateStringWithAbsoluteTime (CFAllocatorRef alloc,
  CFDateFormatterRef fmt, CFAbsoluteTime at)
{
  return NULL;
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
  CFStringRef templ, CFOptionFlags options, CFLocaleRef locale)
{
  return NULL;
}

CFTypeRef
CFDateFormatterCopyProperty (CFDateFormatterRef fmt, CFStringRef key)
{
  CFIndex idx;
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesSize ; ++idx)
    {
      if (key == *(_kCFDateFormatterProperties[idx].prop))
        return (_kCFDateFormatterProperties[idx].copy)(fmt,
          _kCFDateFormatterProperties[idx].icuProp);
    }
  
  for (idx = 0 ; idx < _kCFDateFormatterPropertiesSize ; ++idx)
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