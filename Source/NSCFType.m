/* NSCFType.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of GNUstep CoreBase library.
   
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

#import <Foundation/NSObject.h>

#include "CoreFoundation/CoreFoundation.h"
#include "NSCFType.h"

extern void CFInitialize (void);
extern UInt32 __CFRuntimeClassTableSize;
extern UInt32 __CFRuntimeClassTableCount;
extern Class NSCFTypeClass;

const CFStringRef *constant_cfstrings[] =
{
  // CFError Keys
  &kCFErrorDomainPOSIX,
  &kCFErrorDomainOSStatus,
  &kCFErrorDomainMach,
  &kCFErrorDomainCocoa,
  &kCFErrorLocalizedDescriptionKey,
  &kCFErrorLocalizedFailureReasonKey,
  &kCFErrorLocalizedRecoverySuggestionKey,
  &kCFErrorDescriptionKey,
  &kCFErrorUnderlyingErrorKey,

  // CFString Transformation identifiers
  &kCFStringTransformStripCombiningMarks,
  &kCFStringTransformToLatin,
  &kCFStringTransformFullwidthHalfwidth,
  &kCFStringTransformLatinKatakana,
  &kCFStringTransformLatinHiragana,
  &kCFStringTransformHiraganaKatakana,
  &kCFStringTransformMandarinLatin,
  &kCFStringTransformLatinHangul,
  &kCFStringTransformLatinArabic,
  &kCFStringTransformLatinHebrew,
  &kCFStringTransformLatinThai,
  &kCFStringTransformLatinCyrillic,
  &kCFStringTransformLatinGreek,
  &kCFStringTransformToXMLHex,
  &kCFStringTransformToUnicodeName,
  &kCFStringTransformStripDiacritics,

  // CFLocale keys
  &kCFLocaleMeasurementSystem,
  &kCFLocaleDecimalSeparator,
  &kCFLocaleGroupingSeparator,
  &kCFLocaleCurrencySymbol,
  &kCFLocaleCurrencyCode,
  &kCFLocaleIdentifier,
  &kCFLocaleLanguageCode,
  &kCFLocaleCountryCode,
  &kCFLocaleScriptCode,
  &kCFLocaleVariantCode,
  &kCFLocaleExemplarCharacterSet,
  &kCFLocaleCalendarIdentifier,
  &kCFLocaleCalendar,
  &kCFLocaleCollationIdentifier,
  &kCFLocaleUsesMetricSystem,
  &kCFLocaleCollatorIdentifier,
  &kCFLocaleQuotationBeginDelimiterKey,
  &kCFLocaleQuotationEndDelimiterKey,
  &kCFLocaleAlternateQuotationBeginDelimiterKey,
  &kCFLocaleAlternateQuotationEndDelimiterKey,

  // CFCalendar identifiers
  &kCFGregorianCalendar,
  &kCFBuddhistCalendar,
  &kCFChineseCalendar,
  &kCFHebrewCalendar,
  &kCFIslamicCalendar,
  &kCFIslamicCivilCalendar,
  &kCFJapaneseCalendar,
  &kCFRepublicOfChinaCalendar,
  &kCFPersianCalendar,
  &kCFIndianCalendar,
  &kCFISO8601Calendar,

  // CFDateFormatter keys
  &kCFDateFormatterIsLenient,
  &kCFDateFormatterTimeZone,
  &kCFDateFormatterCalendarName,
  &kCFDateFormatterDefaultFormat,
  &kCFDateFormatterTwoDigitStartDate,
  &kCFDateFormatterDefaultDate,
  &kCFDateFormatterCalendar,
  &kCFDateFormatterEraSymbols,
  &kCFDateFormatterMonthSymbols,
  &kCFDateFormatterShortMonthSymbols,
  &kCFDateFormatterWeekdaySymbols,
  &kCFDateFormatterShortWeekdaySymbols,
  &kCFDateFormatterAMSymbol,
  &kCFDateFormatterPMSymbol,
  &kCFDateFormatterLongEraSymbols,
  &kCFDateFormatterVeryShortMonthSymbols,
  &kCFDateFormatterStandaloneMonthSymbols,
  &kCFDateFormatterShortStandaloneMonthSymbols,
  &kCFDateFormatterVeryShortStandaloneMonthSymbols,
  &kCFDateFormatterVeryShortWeekdaySymbols,
  &kCFDateFormatterStandaloneWeekdaySymbols,
  &kCFDateFormatterShortStandaloneWeekdaySymbols,
  &kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
  &kCFDateFormatterQuarterSymbols,
  &kCFDateFormatterShortQuarterSymbols,
  &kCFDateFormatterStandaloneQuarterSymbols,
  &kCFDateFormatterShortStandaloneQuarterSymbols,
  &kCFDateFormatterGregorianStartDate,

  // CFNumberFormatter keys
  &kCFNumberFormatterCurrencyCode,
  &kCFNumberFormatterDecimalSeparator,
  &kCFNumberFormatterCurrencyDecimalSeparator,
  &kCFNumberFormatterAlwaysShowDecimalSeparator,
  &kCFNumberFormatterGroupingSeparator,
  &kCFNumberFormatterUseGroupingSeparator,
  &kCFNumberFormatterPercentSymbol,
  &kCFNumberFormatterZeroSymbol,
  &kCFNumberFormatterNaNSymbol,
  &kCFNumberFormatterInfinitySymbol,
  &kCFNumberFormatterMinusSign,
  &kCFNumberFormatterPlusSign,
  &kCFNumberFormatterCurrencySymbol,
  &kCFNumberFormatterExponentSymbol,
  &kCFNumberFormatterMinIntegerDigits,
  &kCFNumberFormatterMaxIntegerDigits,
  &kCFNumberFormatterMinFractionDigits,
  &kCFNumberFormatterMaxFractionDigits,
  &kCFNumberFormatterGroupingSize,
  &kCFNumberFormatterSecondaryGroupingSize,
  &kCFNumberFormatterRoundingMode,
  &kCFNumberFormatterRoundingIncrement,
  &kCFNumberFormatterFormatWidth,
  &kCFNumberFormatterPaddingPosition,
  &kCFNumberFormatterPaddingCharacter,
  &kCFNumberFormatterDefaultFormat,
  &kCFNumberFormatterMultiplier,
  &kCFNumberFormatterPositivePrefix,
  &kCFNumberFormatterPositiveSuffix,
  &kCFNumberFormatterNegativePrefix,
  &kCFNumberFormatterNegativeSuffix,
  &kCFNumberFormatterPerMillSymbol,
  &kCFNumberFormatterInternationalCurrencySymbol,
  &kCFNumberFormatterCurrencyGroupingSeparator,
  &kCFNumberFormatterIsLenient,
  &kCFNumberFormatterUseSignificantDigits,
  &kCFNumberFormatterMinSignificantDigits,
  &kCFNumberFormatterMaxSignificantDigits,

  // CFXMLTree
  &kCFXMLTreeErrorDescription,
  &kCFXMLTreeErrorLineNumber,
  &kCFXMLTreeErrorLocation,
  &kCFXMLTreeErrorStatusCode,
  
  NULL
};

void NSCFInitialize (void)
{
  static int requiredClasses = 5;
  --requiredClasses;
  
  if (requiredClasses == 0)
    {
      CFIndex i = 0;
      
      __CFRuntimeObjCClassTable = (Class *) calloc (__CFRuntimeClassTableSize,
                                    sizeof(Class));
      NSCFTypeClass = [NSCFType class];
      CFInitialize ();
      while (i < __CFRuntimeClassTableCount)
        __CFRuntimeObjCClassTable[i++] = NSCFTypeClass;
      
      /* This would need to be done in NSNull, but will be here for now. */
      CFRuntimeBridgeClass (CFNullGetTypeID(), "NSNull");
      CFRuntimeSetInstanceISA (kCFNull, objc_getClass("NSNull"));
      CFRuntimeBridgeClass (CFArrayGetTypeID(), "NSCFArray");
      CFRuntimeBridgeClass (CFDataGetTypeID(), "NSCFData");
      CFRuntimeBridgeClass (CFErrorGetTypeID(), "NSCFError");
      CFRuntimeBridgeClass (CFStringGetTypeID(), "NSCFString");
      
      // Init Constants Strings
      for (i = 0 ; constant_cfstrings[i] != NULL ; ++i)
        _CFRuntimeInitStaticInstance(*(constant_cfstrings[i]), CFStringGetTypeID());
      
      // Init Allocators
      CFRuntimeSetInstanceISA((void*)kCFAllocatorSystemDefault,
        NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFAllocatorMalloc,
        NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFAllocatorMallocZone,
        NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFAllocatorNull,
        NSCFTypeClass);
      
      // Init Numbers
      CFRuntimeSetInstanceISA((void*)kCFBooleanTrue, NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFBooleanFalse, NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFNumberNaN, NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFNumberNegativeInfinity,
        NSCFTypeClass);
      CFRuntimeSetInstanceISA((void*)kCFNumberPositiveInfinity,
        NSCFTypeClass);
    }
}

@interface NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return _kCFRuntimeNotATypeID;
}
@end

@implementation NSCFType

+ (void) load
{
  NSCFTypeClass = self;
  NSCFInitialize ();
}

- (id) retain
{
  return (id)CFRetain(self);
}

- (oneway void) release
{
  CFRelease(self);
}

- (NSUInteger) hash
{
  return (NSUInteger)CFHash (self);
}

- (BOOL) isEqual: (id) anObject
{
  return (BOOL)CFEqual (self, (CFTypeRef)anObject);
}

- (CFTypeID) _cfTypeID
{
  /* This is an undocumented method.
     See: http://www.cocoadev.com/index.pl?HowToCreateTollFreeBridgedClass for
     more info.
  */
  return (CFTypeID)_typeID;
}

@end
