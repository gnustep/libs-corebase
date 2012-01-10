#include "CoreFoundation/CFRuntime.h"
#include <stdlib.h>

struct __CFString
{
  CFRuntimeBase  _parent;
  void          *_contents;
  CFIndex        _count;
  CFHashCode     _hash;
  CFAllocatorRef _deallocator;
};

#if defined (_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define CONST_STRING_DECL(var, str) \
  static struct __CFString __ ## var ## __ = \
  { {0, 0, {1, 0, 0}}, (void*)str, sizeof(str) - 1, 0, NULL }; \
 DLL_EXPORT const CFStringRef var = (CFStringRef) & __ ## var ## __;

//
// CFError identifiers
//
CONST_STRING_DECL(kCFErrorDomainPOSIX,
  "kCFErrorDomainPOSIX");
CONST_STRING_DECL(kCFErrorDomainOSStatus,
  "kCFErrorDomainOSStatus");
CONST_STRING_DECL(kCFErrorDomainMach,
  "kCFErrorDomainMach");
CONST_STRING_DECL(kCFErrorDomainCocoa,
  "kCFErrorDomainCocoa");
CONST_STRING_DECL(kCFErrorLocalizedDescriptionKey,
  "kCFErrorLocalizedDescriptionKey");
CONST_STRING_DECL(kCFErrorLocalizedFailureReasonKey,
  "kCFErrorLocalizedFailureReasonKey");
CONST_STRING_DECL(kCFErrorLocalizedRecoverySuggestionKey,
  "kCFErrorLocalizedRecoverySuggestionKey");
CONST_STRING_DECL(kCFErrorDescriptionKey,
  "kCFErrorDescriptionKey");
CONST_STRING_DECL(kCFErrorUnderlyingErrorKey,
  "kCFErrorUnderlyingErrorKey");

// CFString Transformation identifiers
CONST_STRING_DECL(kCFStringTransformStripCombiningMarks,
  "NFD; [:nonspacing mark:]Remove; NFC");
CONST_STRING_DECL(kCFStringTransformToLatin,
  "Any-Latin");
CONST_STRING_DECL(kCFStringTransformFullwidthHalfwidth,
  "Fullwidth-Halfwidth");
CONST_STRING_DECL(kCFStringTransformLatinKatakana,
  "Latin-Katakana");
CONST_STRING_DECL(kCFStringTransformLatinHiragana,
  "Latin-Hiragana");
CONST_STRING_DECL(kCFStringTransformHiraganaKatakana ,
  "Hiragana-Katakana");
CONST_STRING_DECL(kCFStringTransformMandarinLatin,
  "Mandarin-Latin");
CONST_STRING_DECL(kCFStringTransformLatinHangul,
  "Latin-Hangul");
CONST_STRING_DECL(kCFStringTransformLatinArabic,
  "Latin-Arabic");
CONST_STRING_DECL(kCFStringTransformLatinHebrew,
  "Latin-Hebrew");
CONST_STRING_DECL(kCFStringTransformLatinThai,
  "Latin-Thai");
CONST_STRING_DECL(kCFStringTransformLatinCyrillic,
  "Latin-Cyrillic");
CONST_STRING_DECL(kCFStringTransformLatinGreek,
  "Latin-Greek");
CONST_STRING_DECL(kCFStringTransformToXMLHex,
  "Any-Hex/XML");
CONST_STRING_DECL(kCFStringTransformToUnicodeName,
  "Any-Name");
CONST_STRING_DECL(kCFStringTransformStripDiacritics,
  "NFD; [:nonspacing marks:]Remove; NFC");

//
// CFLocale keys
//
CONST_STRING_DECL(kCFLocaleMeasurementSystem,
  "kCFLocaleMeasurementSystem");
CONST_STRING_DECL(kCFLocaleDecimalSeparator,
  "kCFLocaleDecimalSeparator");
CONST_STRING_DECL(kCFLocaleGroupingSeparator,
  "kCFLocaleGroupingSeparator");
CONST_STRING_DECL(kCFLocaleCurrencySymbol,
  "kCFLocaleCurrencySymbol");
CONST_STRING_DECL(kCFLocaleCurrencyCode,
  "currency");
CONST_STRING_DECL(kCFLocaleIdentifier,
  "kCFLocaleIdentifier");
CONST_STRING_DECL(kCFLocaleLanguageCode,
  "kCFLocaleLanguageCode");
CONST_STRING_DECL(kCFLocaleCountryCode,
  "kCFLocaleCountryCode");
CONST_STRING_DECL(kCFLocaleScriptCode,
  "kCFLocaleScriptCode");
CONST_STRING_DECL(kCFLocaleVariantCode,
  "kCFLocaleVariantCode");
CONST_STRING_DECL(kCFLocaleExemplarCharacterSet,
  "kCFLocaleExemplarCharacterSet");
CONST_STRING_DECL(kCFLocaleCalendarIdentifier,
  "kCFLocaleCalendarIdentifier");
CONST_STRING_DECL(kCFLocaleCalendar,
  "calendar");
CONST_STRING_DECL(kCFLocaleCollationIdentifier,
  "collation");
CONST_STRING_DECL(kCFLocaleUsesMetricSystem,
  "kCFLocaleUsesMetricSystem");
CONST_STRING_DECL(kCFLocaleCollatorIdentifier,
  "kCFLocaleCollatorIdentifier");
CONST_STRING_DECL(kCFLocaleQuotationBeginDelimiterKey,
  "kCFLocaleQuotationBeginDelimiterKey");
CONST_STRING_DECL(kCFLocaleQuotationEndDelimiterKey,
  "kCFLocaleQuotationEndDelimiterKey");
CONST_STRING_DECL(kCFLocaleAlternateQuotationBeginDelimiterKey,
  "kCFLocaleAlternateQuotationBeginDelimiterKey");
CONST_STRING_DECL(kCFLocaleAlternateQuotationEndDelimiterKey,
  "kCFLocaleAlternateQuotationEndDelimiterKey");



//
// CFCalendar identifiers
//
CONST_STRING_DECL(kCFGregorianCalendar,
  "gregorian");
CONST_STRING_DECL(kCFBuddhistCalendar,
  "buddhist");
CONST_STRING_DECL(kCFChineseCalendar,
  "chinese");
CONST_STRING_DECL(kCFHebrewCalendar,
  "hebrew");
CONST_STRING_DECL(kCFIslamicCalendar,
  "islamic");
CONST_STRING_DECL(kCFIslamicCivilCalendar,
  "islamic-civil");
CONST_STRING_DECL(kCFJapaneseCalendar,
  "japanese");
CONST_STRING_DECL(kCFRepublicOfChinaCalendar,
  "roc");
CONST_STRING_DECL(kCFPersianCalendar,
  "persian");
CONST_STRING_DECL(kCFIndianCalendar,
  "indian");
CONST_STRING_DECL(kCFISO8601Calendar,
  "iso8601"); // Introduced on UTS #35 v2.0



//
// CFDateFormatter keys
//
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



//
// CFNumberFormatter keys
//
CONST_STRING_DECL(kCFNumberFormatterCurrencyCode,
  "kCFNumberFormatterCurrencyCode");
CONST_STRING_DECL(kCFNumberFormatterDecimalSeparator,
  "kCFNumberFormatterDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterCurrencyDecimalSeparator,
  "kCFNumberFormatterCurrencyDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterAlwaysShowDecimalSeparator,
  "kCFNumberFormatterAlwaysShowDecimalSeparator");
CONST_STRING_DECL(kCFNumberFormatterGroupingSeparator,
  "kCFNumberFormatterGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterUseGroupingSeparator,
  "kCFNumberFormatterUseGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterPercentSymbol,
  "kCFNumberFormatterPercentSymbol");
CONST_STRING_DECL(kCFNumberFormatterZeroSymbol,
  "kCFNumberFormatterZeroSymbol");
CONST_STRING_DECL(kCFNumberFormatterNaNSymbol,
  "kCFNumberFormatterNaNSymbol");
CONST_STRING_DECL(kCFNumberFormatterInfinitySymbol,
  "kCFNumberFormatterInfinitySymbol");
CONST_STRING_DECL(kCFNumberFormatterMinusSign,
  "kCFNumberFormatterMinusSign");
CONST_STRING_DECL(kCFNumberFormatterPlusSign,
  "kCFNumberFormatterPlusSign");
CONST_STRING_DECL(kCFNumberFormatterCurrencySymbol,
  "kCFNumberFormatterCurrencySymbol");
CONST_STRING_DECL(kCFNumberFormatterExponentSymbol,
  "kCFNumberFormatterExponentSymbol");
CONST_STRING_DECL(kCFNumberFormatterMinIntegerDigits,
  "kCFNumberFormatterMinIntegerDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxIntegerDigits,
  "kCFNumberFormatterMaxIntegerDigits");
CONST_STRING_DECL(kCFNumberFormatterMinFractionDigits,
  "kCFNumberFormatterMinFractionDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxFractionDigits,
  "kCFNumberFormatterMaxFractionDigits");
CONST_STRING_DECL(kCFNumberFormatterGroupingSize,
  "kCFNumberFormatterGroupingSize");
CONST_STRING_DECL(kCFNumberFormatterSecondaryGroupingSize,
  "kCFNumberFormatterSecondaryGroupingSize");
CONST_STRING_DECL(kCFNumberFormatterRoundingMode,
  "kCFNumberFormatterRoundingMode");
CONST_STRING_DECL(kCFNumberFormatterRoundingIncrement,
  "kCFNumberFormatterRoundingIncrement");
CONST_STRING_DECL(kCFNumberFormatterFormatWidth,
  "kCFNumberFormatterFormatWidth");
CONST_STRING_DECL(kCFNumberFormatterPaddingPosition,
  "kCFNumberFormatterPaddingPosition");
CONST_STRING_DECL(kCFNumberFormatterPaddingCharacter,
  "kCFNumberFormatterPaddingCharacter");
CONST_STRING_DECL(kCFNumberFormatterDefaultFormat,
  "kCFNumberFormatterDefaultFormat");
CONST_STRING_DECL(kCFNumberFormatterMultiplier,
  "kCFNumberFormatterMultiplier");
CONST_STRING_DECL(kCFNumberFormatterPositivePrefix,
  "kCFNumberFormatterPositivePrefix");
CONST_STRING_DECL(kCFNumberFormatterPositiveSuffix,
  "kCFNumberFormatterPositiveSuffix");
CONST_STRING_DECL(kCFNumberFormatterNegativePrefix,
  "kCFNumberFormatterNegativePrefix");
CONST_STRING_DECL(kCFNumberFormatterNegativeSuffix,
  "kCFNumberFormatterNegativeSuffix");
CONST_STRING_DECL(kCFNumberFormatterPerMillSymbol,
  "kCFNumberFormatterPerMillSymbol");
CONST_STRING_DECL(kCFNumberFormatterInternationalCurrencySymbol,
  "kCFNumberFormatterInternationalCurrencySymbol"); 
CONST_STRING_DECL(kCFNumberFormatterCurrencyGroupingSeparator,
  "kCFNumberFormatterCurrencyGroupingSeparator");
CONST_STRING_DECL(kCFNumberFormatterIsLenient,
  "kCFNumberFormatterIsLenient");
CONST_STRING_DECL(kCFNumberFormatterUseSignificantDigits,
  "kCFNumberFormatterUseSignificantDigits");
CONST_STRING_DECL(kCFNumberFormatterMinSignificantDigits,
  "kCFNumberFormatterMinSignificantDigits");
CONST_STRING_DECL(kCFNumberFormatterMaxSignificantDigits,
  "kCFNumberFormatterMaxSignificantDigits");



//
// CFXMLTree Error Dictionary Keys
//
CONST_STRING_DECL(kCFXMLTreeErrorDescription,
  "kCFXMLTreeErrorDescription");
CONST_STRING_DECL(kCFXMLTreeErrorLineNumber,
  "kCFXMLTreeErrorLineNumber");
CONST_STRING_DECL(kCFXMLTreeErrorLocation,
  "kCFXMLTreeErrorLocation");
CONST_STRING_DECL(kCFXMLTreeErrorStatusCode,
  "kCFXMLTreeErrorStatusCode");

const CFStringRef keys_identifiers[] =
{
  // CFError Keys
  &__kCFErrorDomainPOSIX__,
  &__kCFErrorDomainOSStatus__,
  &__kCFErrorDomainMach__,
  &__kCFErrorDomainCocoa__,
  &__kCFErrorLocalizedDescriptionKey__,
  &__kCFErrorLocalizedFailureReasonKey__,
  &__kCFErrorLocalizedRecoverySuggestionKey__,
  &__kCFErrorDescriptionKey__,
  &__kCFErrorUnderlyingErrorKey__,

  // CFString Transformation identifiers
  &__kCFStringTransformStripCombiningMarks__,
  &__kCFStringTransformToLatin__,
  &__kCFStringTransformFullwidthHalfwidth__,
  &__kCFStringTransformLatinKatakana__,
  &__kCFStringTransformLatinHiragana__,
  &__kCFStringTransformHiraganaKatakana__,
  &__kCFStringTransformMandarinLatin__,
  &__kCFStringTransformLatinHangul__,
  &__kCFStringTransformLatinArabic__,
  &__kCFStringTransformLatinHebrew__,
  &__kCFStringTransformLatinThai__,
  &__kCFStringTransformLatinCyrillic__,
  &__kCFStringTransformLatinGreek__,
  &__kCFStringTransformToXMLHex__,
  &__kCFStringTransformToUnicodeName__,
  &__kCFStringTransformStripDiacritics__,

  // CFLocale keys
  &__kCFLocaleMeasurementSystem__,
  &__kCFLocaleDecimalSeparator__,
  &__kCFLocaleGroupingSeparator__,
  &__kCFLocaleCurrencySymbol__,
  &__kCFLocaleCurrencyCode__,
  &__kCFLocaleIdentifier__,
  &__kCFLocaleLanguageCode__,
  &__kCFLocaleCountryCode__,
  &__kCFLocaleScriptCode__,
  &__kCFLocaleVariantCode__,
  &__kCFLocaleExemplarCharacterSet__,
  &__kCFLocaleCalendarIdentifier__,
  &__kCFLocaleCalendar__,
  &__kCFLocaleCollationIdentifier__,
  &__kCFLocaleUsesMetricSystem__,
  &__kCFLocaleCollatorIdentifier__,
  &__kCFLocaleQuotationBeginDelimiterKey__,
  &__kCFLocaleQuotationEndDelimiterKey__,
  &__kCFLocaleAlternateQuotationBeginDelimiterKey__,
  &__kCFLocaleAlternateQuotationEndDelimiterKey__,

  // CFCalendar identifiers
  &__kCFGregorianCalendar__,
  &__kCFBuddhistCalendar__,
  &__kCFChineseCalendar__,
  &__kCFHebrewCalendar__,
  &__kCFIslamicCalendar__,
  &__kCFIslamicCivilCalendar__,
  &__kCFJapaneseCalendar__,
  &__kCFRepublicOfChinaCalendar__,
  &__kCFPersianCalendar__,
  &__kCFIndianCalendar__,
  &__kCFISO8601Calendar__,

  // CFDateFormatter keys
  &__kCFDateFormatterIsLenient__,
  &__kCFDateFormatterTimeZone__,
  &__kCFDateFormatterCalendarName__,
  &__kCFDateFormatterDefaultFormat__,
  &__kCFDateFormatterTwoDigitStartDate__,
  &__kCFDateFormatterDefaultDate__,
  &__kCFDateFormatterCalendar__,
  &__kCFDateFormatterEraSymbols__,
  &__kCFDateFormatterMonthSymbols__,
  &__kCFDateFormatterShortMonthSymbols__,
  &__kCFDateFormatterWeekdaySymbols__,
  &__kCFDateFormatterShortWeekdaySymbols__,
  &__kCFDateFormatterAMSymbol__,
  &__kCFDateFormatterPMSymbol__,
  &__kCFDateFormatterLongEraSymbols__,
  &__kCFDateFormatterVeryShortMonthSymbols__,
  &__kCFDateFormatterStandaloneMonthSymbols__,
  &__kCFDateFormatterShortStandaloneMonthSymbols__,
  &__kCFDateFormatterVeryShortStandaloneMonthSymbols__,
  &__kCFDateFormatterVeryShortWeekdaySymbols__,
  &__kCFDateFormatterStandaloneWeekdaySymbols__,
  &__kCFDateFormatterShortStandaloneWeekdaySymbols__,
  &__kCFDateFormatterVeryShortStandaloneWeekdaySymbols__,
  &__kCFDateFormatterQuarterSymbols__,
  &__kCFDateFormatterShortQuarterSymbols__,
  &__kCFDateFormatterStandaloneQuarterSymbols__,
  &__kCFDateFormatterShortStandaloneQuarterSymbols__,
  &__kCFDateFormatterGregorianStartDate__,

  // CFNumberFormatter keys
  &__kCFNumberFormatterCurrencyCode__,
  &__kCFNumberFormatterDecimalSeparator__,
  &__kCFNumberFormatterCurrencyDecimalSeparator__,
  &__kCFNumberFormatterAlwaysShowDecimalSeparator__,
  &__kCFNumberFormatterGroupingSeparator__,
  &__kCFNumberFormatterUseGroupingSeparator__,
  &__kCFNumberFormatterPercentSymbol__,
  &__kCFNumberFormatterZeroSymbol__,
  &__kCFNumberFormatterNaNSymbol__,
  &__kCFNumberFormatterInfinitySymbol__,
  &__kCFNumberFormatterMinusSign__,
  &__kCFNumberFormatterPlusSign__,
  &__kCFNumberFormatterCurrencySymbol__,
  &__kCFNumberFormatterExponentSymbol__,
  &__kCFNumberFormatterMinIntegerDigits__,
  &__kCFNumberFormatterMaxIntegerDigits__,
  &__kCFNumberFormatterMinFractionDigits__,
  &__kCFNumberFormatterMaxFractionDigits__,
  &__kCFNumberFormatterGroupingSize__,
  &__kCFNumberFormatterSecondaryGroupingSize__,
  &__kCFNumberFormatterRoundingMode__,
  &__kCFNumberFormatterRoundingIncrement__,
  &__kCFNumberFormatterFormatWidth__,
  &__kCFNumberFormatterPaddingPosition__,
  &__kCFNumberFormatterPaddingCharacter__,
  &__kCFNumberFormatterDefaultFormat__,
  &__kCFNumberFormatterMultiplier__,
  &__kCFNumberFormatterPositivePrefix__,
  &__kCFNumberFormatterPositiveSuffix__,
  &__kCFNumberFormatterNegativePrefix__,
  &__kCFNumberFormatterNegativeSuffix__,
  &__kCFNumberFormatterPerMillSymbol__,
  &__kCFNumberFormatterInternationalCurrencySymbol__,
  &__kCFNumberFormatterCurrencyGroupingSeparator__,
  &__kCFNumberFormatterIsLenient__,
  &__kCFNumberFormatterUseSignificantDigits__,
  &__kCFNumberFormatterMinSignificantDigits__,
  &__kCFNumberFormatterMaxSignificantDigits__,

  // CFXMLTree
  &__kCFXMLTreeErrorDescription__,
  &__kCFXMLTreeErrorLineNumber__,
  &__kCFXMLTreeErrorLocation__,
  &__kCFXMLTreeErrorStatusCode__,
  
  NULL
};