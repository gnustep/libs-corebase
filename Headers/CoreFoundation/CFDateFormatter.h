/* CFDateFormatter.h

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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef __COREFOUNDATION_CFDATEFORMATTER_H__
#define __COREFOUNDATION_CFDATEFORMATTER_H__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFLocale.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

CF_EXTERN_C_BEGIN
/** \defgroup CFDateFormatterRef CFDateFormatter Reference
    \{
 */
typedef struct __CFDateFormatter *CFDateFormatterRef;

typedef enum _CFDateFormatterStyle
{
  kCFDateFormatterNoStyle = 0,
  kCFDateFormatterShortStyle = 1,
  kCFDateFormatterMediumStyle = 2,
  kCFDateFormatterLongStyle = 3,
  kCFDateFormatterFullStyle = 4
} CFDateFormatterStyle;

/** \name Constants
    \{
  */
CF_EXPORT const CFStringRef kCFDateFormatterIsLenient;  /* CFBoolean */
CF_EXPORT const CFStringRef kCFDateFormatterTimeZone;   /* CFTimeZone */
CF_EXPORT const CFStringRef kCFDateFormatterCalendarName;       /* CFString */
CF_EXPORT const CFStringRef kCFDateFormatterDefaultFormat;      /* CFString */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
CF_EXPORT const CFStringRef kCFDateFormatterTwoDigitStartDate;  /* CFDate */
CF_EXPORT const CFStringRef kCFDateFormatterDefaultDate;        /* CFDate */
CF_EXPORT const CFStringRef kCFDateFormatterCalendar;   /* CFCalendar */
CF_EXPORT const CFStringRef kCFDateFormatterEraSymbols; /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterMonthSymbols;       /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortMonthSymbols;  /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterWeekdaySymbols;     /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortWeekdaySymbols;        /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterAMSymbol;   /* CFString */
CF_EXPORT const CFStringRef kCFDateFormatterPMSymbol;   /* CFString */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
CF_EXPORT const CFStringRef kCFDateFormatterLongEraSymbols;     /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterVeryShortMonthSymbols;      /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterStandaloneMonthSymbols;     /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortStandaloneMonthSymbols;        /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterVeryShortStandaloneMonthSymbols;    /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterVeryShortWeekdaySymbols;    /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterStandaloneWeekdaySymbols;   /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortStandaloneWeekdaySymbols;      /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterVeryShortStandaloneWeekdaySymbols;  /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterQuarterSymbols;     /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortQuarterSymbols;        /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterStandaloneQuarterSymbols;   /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterShortStandaloneQuarterSymbols;      /* CFArray */
CF_EXPORT const CFStringRef kCFDateFormatterGregorianStartDate; /* CFDate */
#endif
/** \} */

/** \name Creating a Date Formatter
 */
CF_EXPORT CFDateFormatterRef
CFDateFormatterCreate (CFAllocatorRef alloc, CFLocaleRef locale,
                       CFDateFormatterStyle dateStyle,
                       CFDateFormatterStyle timeStyle);
/** \} */

/** \name Configuring a Date Formatter
    \{
 */
CF_EXPORT void
CFDateFormatterSetFormat (CFDateFormatterRef fmt, CFStringRef formatString);

CF_EXPORT void
CFDateFormatterSetProperty (CFDateFormatterRef fmt, CFStringRef key,
                            CFTypeRef value);
/** \} */

/** \name Parsing Strings
    \{
 */
CF_EXPORT CFDateRef
CFDateFormatterCreateDateFromString (CFAllocatorRef alloc,
                                     CFDateFormatterRef fmt, CFStringRef string,
                                     CFRange * rangep);

CF_EXPORT Boolean
CFDateFormatterGetAbsoluteTimeFromString (CFDateFormatterRef fmt,
                                          CFStringRef string, CFRange * rangep,
                                          CFAbsoluteTime * atp);
/** \} */

/** \name Creating Strings From Data
    \{
 */
CF_EXPORT CFStringRef
CFDateFormatterCreateStringWithAbsoluteTime (CFAllocatorRef alloc,
                                             CFDateFormatterRef fmt,
                                             CFAbsoluteTime at);

CF_EXPORT CFStringRef
CFDateFormatterCreateStringWithDate (CFAllocatorRef alloc,
                                     CFDateFormatterRef fmt, CFDateRef date);

CF_EXPORT CFStringRef
CFDateFormatterCreateDateFormatFromTemplate (CFAllocatorRef alloc,
                                             CFStringRef templ,
                                             CFOptionFlags options,
                                             CFLocaleRef locale);
/** \} */

/** \name Getting Information About a Date Formatter
    \{
 */
CF_EXPORT CFTypeRef
CFDateFormatterCopyProperty (CFDateFormatterRef fmt, CFStringRef key);

CF_EXPORT CFDateFormatterStyle
CFDateFormatterGetDateStyle (CFDateFormatterRef fmt);

CF_EXPORT CFStringRef CFDateFormatterGetFormat (CFDateFormatterRef fmt);

CF_EXPORT CFLocaleRef CFDateFormatterGetLocale (CFDateFormatterRef fmt);

CF_EXPORT CFDateFormatterStyle
CFDateFormatterGetTimeStyle (CFDateFormatterRef fmt);
/** \} */

/** \name Getting the CFDateFormatter Type ID
    \{
 */
CF_EXPORT CFTypeID CFDateFormatterGetTypeID (void);
/** \} */
/** \} */

CF_EXTERN_C_END

#endif /* MAC_OS_X_VERSION_10_3 */

#endif /* __COREFOUNDATION_CFDATEFORMATTER_H__ */

