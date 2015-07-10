/* CFCalendar.h
   
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

#ifndef __COREFOUNDATION_CFCALENDAR__
#define __COREFOUNDATION_CFCALENDAR__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFLocale.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

CF_EXTERN_C_BEGIN
/** \defgroup CFCalendarRef CFCalendar Reference
    \{
 */
/** CFCalendar is "toll-free bridged" to NSCalendar.
 */
typedef struct __CFCalendar *CFCalendarRef;

typedef enum
{
  kCFCalendarUnitEra = (1UL << 1),
  kCFCalendarUnitYear = (1UL << 2),
  kCFCalendarUnitMonth = (1UL << 3),
  kCFCalendarUnitDay = (1UL << 4),
  kCFCalendarUnitHour = (1UL << 5),
  kCFCalendarUnitMinute = (1UL << 6),
  kCFCalendarUnitSecond = (1UL << 7),
  kCFCalendarUnitWeek = (1UL << 8),
  kCFCalendarUnitWeekday = (1UL << 9),
  kCFCalendarUnitWeekdayOrdinal = (1UL << 10),
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
  kCFCalendarUnitQuarter = (1UL << 11),
#endif
} CFCalendarUnit;

enum
{
  kCFCalendarComponentsWrap = (1UL << 0)
};

/** \name Creating a Calendar
    \{
 */
CF_EXPORT CFCalendarRef CFCalendarCopyCurrent (void);

CF_EXPORT CFCalendarRef
CFCalendarCreateWithIdentifier (CFAllocatorRef allocator, CFStringRef ident);
/** \} */

/** \name Calendrical Calculations
    \{
 */
CF_EXPORT Boolean
CFCalendarAddComponents (CFCalendarRef cal, CFAbsoluteTime * at,
                         CFOptionFlags options, const char *componentDesc, ...);

CF_EXPORT Boolean
CFCalendarComposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime * at,
                               const char *componentDesc, ...);

CF_EXPORT Boolean
CFCalendarDecomposeAbsoluteTime (CFCalendarRef cal, CFAbsoluteTime at,
                                 const char *componentDesc, ...);

CF_EXPORT Boolean
CFCalendarGetComponentDifference (CFCalendarRef cal, CFAbsoluteTime startinAT,
                                  CFAbsoluteTime resultAT,
                                  CFOptionFlags options,
                                  const char *componentDesc, ...);
/** \} */

/** \name Getting Ranges of Units
    \{
 */
CF_EXPORT CFRange
CFCalendarGetRangeOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
                          CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CF_EXPORT CFIndex
CFCalendarGetOrdinalityOfUnit (CFCalendarRef cal, CFCalendarUnit smallerUnit,
                               CFCalendarUnit biggerUnit, CFAbsoluteTime at);

CF_EXPORT CFRange
CFCalendarGetMaximumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit);

CF_EXPORT CFRange
CFCalendarGetMinimumRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit);

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
CF_EXPORT Boolean
CFCalendarGetTimeRangeOfUnit (CFCalendarRef cal, CFCalendarUnit unit,
                              CFAbsoluteTime at, CFAbsoluteTime * startp,
                              CFTimeInterval * tip);
#endif
/** \} */

/** \name Getting and Setting the Time Zone
    \{
 */
CF_EXPORT CFTimeZoneRef CFCalendarCopyTimeZone (CFCalendarRef cal);

CF_EXPORT void CFCalendarSetTimeZone (CFCalendarRef cal, CFTimeZoneRef tz);
/** \} */

/** \name Getting the Identifier
    \{
 */
CF_EXPORT CFStringRef CFCalendarGetIdentifier (CFCalendarRef cal);
/** \} */

/** \name Getting and Setting the Locale
    \{
 */
CF_EXPORT CFLocaleRef CFCalendarCopyLocale (CFCalendarRef cal);

CF_EXPORT void CFCalendarSetLocale (CFCalendarRef cal, CFLocaleRef locale);
/** \} */

/** \name Getting and Setting Day Information
    \{
 */
CF_EXPORT CFIndex CFCalendarGetFirstWeekday (CFCalendarRef cal);

CF_EXPORT void CFCalendarSetFirstWeekday (CFCalendarRef cal, CFIndex wkdy);

CF_EXPORT CFIndex CFCalendarGetMinimumDaysInFirstWeek (CFCalendarRef cal);

CF_EXPORT void
CFCalendarSetMinimumDaysInFirstWeek (CFCalendarRef cal, CFIndex mwd);
/** \} */

/** \name Getting the Type ID
    \{
 */
CF_EXPORT CFTypeID CFCalendarGetTypeID (void);
/** \} */
/** \} */

CF_EXTERN_C_END
#endif /* MAC_OS_X_VERSION_10_4 */
#endif /* __COREFOUNDATION_CFCALENDAR__ */
