/* CFConstantString.c
   
   Copyright (C) 2016 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2016
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is disibuted in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "GSObjCRuntime.h"

/* Need the next few includes so we can initialize the string constants */
#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDateFormatter.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFStream.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURLAccess.h"
#include "CoreFoundation/CFXMLParser.h"

/* WARNING!!!
   This initialization function must be called after CFStringInitialize(), or
   else CFStringGetTypeID() will not return a valid type id.
 */
void CFConstantStringInitialize (void)
{
  CFTypeID _kCFStringTypeID;

  _kCFStringTypeID = CFStringGetTypeID ();

  /* Constants defined in CFError.h */
  GSRuntimeConstantInit (kCFErrorDomainPOSIX, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainOSStatus, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainMach, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDomainCocoa, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedDescriptionKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedFailureReasonKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorLocalizedRecoverySuggestionKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorDescriptionKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFErrorUnderlyingErrorKey, _kCFStringTypeID);

  /* Constants defined in CFString.h */
  GSRuntimeConstantInit (kCFStringTransformStripCombiningMarks,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToLatin, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformFullwidthHalfwidth,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinKatakana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHiragana, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformHiraganaKatakana,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformMandarinLatin, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHangul, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinArabic, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinHebrew, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinThai, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinCyrillic, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformLatinGreek, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToXMLHex, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformToUnicodeName, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStringTransformStripDiacritics, _kCFStringTypeID);

  /* Constants defined in CFLocale.h */
  GSRuntimeConstantInit (kCFLocaleMeasurementSystem, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleDecimalSeparator, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleGroupingSeparator, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCurrencySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCurrencyCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleLanguageCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCountryCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleScriptCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleVariantCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleExemplarCharacterSet, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCalendarIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCollationIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleUsesMetricSystem, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleCollatorIdentifier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleQuotationBeginDelimiterKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleQuotationEndDelimiterKey, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleAlternateQuotationBeginDelimiterKey,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFLocaleAlternateQuotationEndDelimiterKey,
                         _kCFStringTypeID);

  /* Constants defined in CFCalendar.h */
  GSRuntimeConstantInit (kCFGregorianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFBuddhistCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFChineseCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFHebrewCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIslamicCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIslamicCivilCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFJapaneseCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFRepublicOfChinaCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFPersianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFIndianCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFISO8601Calendar, _kCFStringTypeID);

  /* Cosntants defined in CFDataFormatter.h */
  GSRuntimeConstantInit (kCFDateFormatterIsLenient, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterTimeZone, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterCalendarName, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterDefaultFormat, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterTwoDigitStartDate, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterDefaultDate, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterCalendar, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterEraSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterMonthSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortMonthSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterWeekdaySymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterAMSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterPMSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterLongEraSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortStandaloneMonthSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterVeryShortStandaloneWeekdaySymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterQuarterSymbols, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterStandaloneQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterShortStandaloneQuarterSymbols,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFDateFormatterGregorianStartDate,
                         _kCFStringTypeID);

  /* Constants defined in CFNumberFormatter.h */
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterAlwaysShowDecimalSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterUseGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPercentSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterZeroSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNaNSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterInfinitySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinusSign, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPlusSign, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencySymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterExponentSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinIntegerDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxIntegerDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinFractionDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxFractionDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterGroupingSize, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterSecondaryGroupingSize,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingMode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterRoundingIncrement,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterFormatWidth, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingPosition, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPaddingCharacter,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterDefaultFormat, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMultiplier, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPositivePrefix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPositiveSuffix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNegativePrefix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterNegativeSuffix, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterPerMillSymbol, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterInternationalCurrencySymbol,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterCurrencyGroupingSeparator,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterIsLenient, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterUseSignificantDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMinSignificantDigits,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFNumberFormatterMaxSignificantDigits,
                         _kCFStringTypeID);

  /* Constants defined in CFRunLoop.h */
  GSRuntimeConstantInit (kCFRunLoopCommonModes, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFRunLoopDefaultMode, _kCFStringTypeID);

  /* Constants defined in CFStream.h */
  GSRuntimeConstantInit (kCFStreamPropertyDataWritten, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketNativeHandle,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketRemoteHostName,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertySocketRemotePortNumber,
                         _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertyAppendToFile, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFStreamPropertyFileCurrentOffset,
                         _kCFStringTypeID);

  /* Constants definedin CFXMLParser.h */
  GSRuntimeConstantInit (kCFXMLTreeErrorDescription, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorLineNumber, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorLocation, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFXMLTreeErrorStatusCode, _kCFStringTypeID);

  /* Constants defined in CFURLAccess.h */
  GSRuntimeConstantInit (kCFURLFileExists, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileDirectoryContents, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileLength, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileLastModificationTime, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFilePOSIXMode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLFileOwnerID, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLHTTPStatusCode, _kCFStringTypeID);
  GSRuntimeConstantInit (kCFURLHTTPStatusLine, _kCFStringTypeID);
}
