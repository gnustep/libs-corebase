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
#include "CoreFoundation/CFRuntime.h"

#include <unicode/ucal.h>

struct __CFCalendar
{
  CFRuntimeBase _parent;
  UCalendar    *_cal;
  CFStringRef   _ident;
};

static CFTypeID _kCFCalendarTypeID;

static Boolean CFCalendarEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFCalendarRef o1 = (CFCalendarRef)cf1;
  CFCalendarRef o2 = (CFCalendarRef)cf2;
  
  return (Boolean)ucal_equivalentTo (o1->_cal, o2->_cal);
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
  NULL,
  CFCalendarEqual,
  NULL,
  CFCalendarCopyFormattingDesc,
  NULL
};

void CFCalendarInitialize (void)
{
  _kCFCalendarTypeID = _CFRuntimeRegisterClass (&CFCalendarClass);
}



CFCalendarRef
CFCalendarCopyCurrent (void)
{
  return NULL;
}

CFCalendarRef
CFCalendarCreateWithIdentifier (CFAllocatorRef allocator, CFStringRef ident)
{
  return NULL;
}

Boolean
CFCalendarAddComponents (CFCalendarRef cal, CFAbsoluteTime *at,
  CFOptionFlags options, const unsigned char *componentDesc, ...)
{
  return false;
}

Boolean
CFCalendarComposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime *at,
  const unsigned char *componentDesc, ...)
{
  return false;
}

Boolean
CFCalendarDecomposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime at,
  const unsigned char *componentDesc, ...)
{
  return false;
}

Boolean
CFCalendarGetComponentDifference (CFCalendarRef cal, CFAbsoluteTime startinAT,
  CFAbsoluteTime resultAT, CFOptionFlags options,
  const unsigned char *componentDesc, ...)
{
  return false;
}

CFRange
CFCalendarGetRangeOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
  CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CFIndex
CFCalendarGetOrdinalityOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
  CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CFRange
CFCalendarGetMaximumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit);

CFRange
CFCalendarGetMinimumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit);

CFTimeZoneRef
CFCalendarCopyTimeZone (CFCalendarRef cal)
{
  return NULL;
}

void
CFCalendarSetTimeZone (CFCalendarRef cal, CFTimeZoneRef tz)
{
}

CFStringRef
CFCalendarGetIdentifier (CFCalendarRef cal)
{
  return NULL;
}

CFLocaleRef
CFCalendarCopyLocale (CFCalendarRef cal)
{
  return NULL;
}

void
CFCalendarSetLocale (CFCalendarRef cal, CFLocaleRef locale)
{
}

CFIndex
CFCalendarGetFirstWeekday (CFCalendarRef cal)
{
  return 0;
}

void
CFCalendarSetFirstWeekday (CFCalendarRef cal, CFIndex wkdy)
{
}

CFIndex
CFCalendarGetMinimumDaysInFirstWeek (CFCalendarRef cal)
{
  return 0;
}

void
CFCalendarSetMinimumDaysInFirstWeek (CFCalendarRef cal, CFIndex mwd)
{
}

CFTypeID
CFCalendarGetTypeID (void)
{
  return _kCFCalendarTypeID;
}

Boolean
CFCalendarGetTimeRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit,
  CFAbsoluteTime at, CFAbsoluteTime *startp, CFTimeInterval *tip)
{
  return false;
}
