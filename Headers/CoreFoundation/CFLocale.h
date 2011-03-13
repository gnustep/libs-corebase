/* CFLocale.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
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

#ifndef __COREFOUNDATION_CFLOCALE__
#define __COREFOUNDATION_CFLOCALE__ 1

#include <CoreFoundation/CFBase.h>

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)

/** @brief CFLocale provides basic functionality for language and/or region
 *  specific operations.
 *  
 *  Locale-sensitive operations, such as collation, calendars and
 *  capitalization, may use CFLocale objects to provide language and/or region
 *  specific functionality.
 *
 *  CFLocale is "toll-free bridged" to NSLocale.
 */
typedef const struct __CFLocale *CFLocaleRef;

/** 
 */
typedef CFIndex CFLocaleLanguageDirection;
enum
{
  kCFLocaleLanguageDirectionUnknown     = 0,
  kCFLocaleLanguageDirectionLeftToRight = 1,
  kCFLocaleLanguageDirectionRightToLeft = 2,
  kCFLocaleLanguageDirectionTopToBottom = 3,
  kCFLocaleLanguageDirectionBottomToTop = 4
};

/** @defgroup LocaleKeys Locale property keys used to get corresponding values.
 *  @{
 */
const CFStringRef kCFLocaleMeasurementSystem;
const CFStringRef kCFLocaleDecimalSeparator;
const CFStringRef kCFLocaleGroupingSeparator;
const CFStringRef kCFLocaleCurrencySymbol;
const CFStringRef kCFLocaleCurrencyCode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
const CFStringRef kCFLocaleIdentifier;
const CFStringRef kCFLocaleLanguageCode;
const CFStringRef kCFLocaleCountryCode;
const CFStringRef kCFLocaleScriptCode;
const CFStringRef kCFLocaleVariantCode;
const CFStringRef kCFLocaleExemplarCharacterSet;
const CFStringRef kCFLocaleCalendarIdentifier;
const CFStringRef kCFLocaleCalendar;
const CFStringRef kCFLocaleCollationIdentifier;
const CFStringRef kCFLocaleUsesMetricSystem;
#endif
/** @} */

/** @defgroup CalendarIdentifiers CFCalendar identifiers
 *  @{
 */
const CFStringRef kCFGregorianCalendar;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
const CFStringRef kCFBuddhistCalendar;
const CFStringRef kCFChineseCalendar;
const CFStringRef kCFHebrewCalendar;
const CFStringRef kCFIslamicCalendar;
const CFStringRef kCFIslamicCivilCalendar;
const CFStringRef kCFJapaneseCalendar;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
const CFStringRef kCFRepublicOfChinaCalendar;
const CFStringRef kCFPersianCalendar;
const CFStringRef kCFIndianCalendar;
const CFStringRef kCFISO8601Calendar;
#endif
/** @} */

/** Locale Change Notification
 */
const CFStringRef kCFLocaleCurrentLocaleDidChangeNotification;



//
// Creating a Locale
//
CFLocaleRef
CFLocaleCopyCurrent (void);

CFLocaleRef
CFLocaleCreate (CFAllocatorRef allocator,
                CFStringRef localeIdent);

CFLocaleRef
CFLocaleCreateCopy (CFAllocatorRef allocator,
                    CFLocaleRef locale);

CFLocaleRef
CFLocaleGetSystem (void);

//
// Getting System Locale Information
//
/** @return 
 */
 #if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
CFArrayRef
CFLocaleCopyAvailableLocaleIdentifiers (void);
#endif

//
// Getting ISO Information
//
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
CFArrayRef
CFLocaleCopyISOCountryCodes (void);

CFArrayRef
CFLocaleCopyISOLanguageCodes (void);

CFArrayRef
CFLocaleCopyISOCurrencyCodes (void);
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
CFArrayRef
CFLocaleCopyCommonISOCurrencyCodes (void);
#endif

//
// Language Preferences
//
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
CFArrayRef
CFLocaleCopyPreferredLanguages (void);
#endif

//
// Getting Information About a Locale
//
CFStringRef
CFLocaleCopyDisplayNameForPropertyValue (CFLocaleRef displayLocale,
                                         CFStringRef key,
                                         CFStringRef value);

CFTypeRef
CFLocaleGetValue (CFLocaleRef locale,
                  CFStringRef key);

CFStringRef
CFLocaleGetIdentifier (CFLocaleRef locale);

//
// Getting and Creating Locale Identifiers
//
CFStringRef
CFLocaleCreateCanonicalLocaleIdentifierFromString (CFAllocatorRef allocator,
                                                   CFStringRef localeIdent);

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
CFStringRef
CFLocaleCreateCanonicalLanguageIdentifierFromString (CFAllocatorRef allocator,
                                                     CFStringRef localeIdent);

CFDictionaryRef
CFLocaleCreateComponentsFromLocaleIdentifier (CFAllocatorRef allocator,
                                              CFStringRef localeIdent);

CFStringRef
CFLocaleCreateLocaleIdentifierFromComponents (CFAllocatorRef allocator,
                                              CFDictionaryRef dictionary);
#endif

//
// Getting the CFLocale Type ID
//
CFTypeID
CFLocaleGetTypeID (void);

//
// New Functions
//
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
CFStringRef
CFLocaleCreateLocaleIdentifierFromWindowsLocaleCode (CFAllocatorRef allocator,
                                                     uint32_t lcid);

CFLocaleLanguageDirection
CFLocaleGetLanguageCharacterDirection (CFStringRef isoLangCode);

CFLocaleLanguageDirection
CFLocaleGetLanguageLineDirection (CFStringRef isoLangCode);

uint32_t
CFLocaleGetWindowsLocaleCodeFromLocaleIdentifier (CFStringRef localeIdent);
#endif

#endif /* OS_API_VERSION >= MAC_OS_X_VERSION_10_3 */

#endif /* __COREFOUNDATION_CFLOCALE__ */
