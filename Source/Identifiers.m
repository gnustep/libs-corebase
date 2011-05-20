#import <Foundation/NSString.h>

//
// CFString Transformation identifiers
//
const NSString *kCFStringTransformStripCombiningMarks =
  @"kCFStringTransformStripCombiningMarks";
const NSString *kCFStringTransformToLatin =
  @"kCFStringTransformToLatin";
const NSString *kCFStringTransformFullwidthHalfwidth =
  @"kCFStringTransformFullwidthHalfwidth";
const NSString *kCFStringTransformLatinKatakana =
  @"kCFStringTransformLatinKatakana";
const NSString *kCFStringTransformLatinHiragana =
  @"kCFStringTransformLatinHiragana";
const NSString *kCFStringTransformHiraganaKatakana  =
  @"kCFStringTransformHiraganaKatakana";
const NSString *kCFStringTransformMandarinLatin =
  @"kCFStringTransformMandarinLatin";
const NSString *kCFStringTransformLatinHangul =
  @"kCFStringTransformLatinHangul";
const NSString *kCFStringTransformLatinArabic =
  @"kCFStringTransformLatinArabic";
const NSString *kCFStringTransformLatinHebrew =
  @"kCFStringTransformLatinHebrew";
const NSString *kCFStringTransformLatinThai =
  @"kCFStringTransformLatinThai";
const NSString *kCFStringTransformLatinCyrillic =
  @"kCFStringTransformLatinCyrillic";
const NSString *kCFStringTransformLatinGreek =
  @"kCFStringTransformLatinGreek";
const NSString *kCFStringTransformToXMLHex =
  @"kCFStringTransformToXMLHex";
const NSString *kCFStringTransformToUnicodeName =
  @"kCFStringTransformToUnicodeName";
const NSString *kCFStringTransformStripDiacritics =
  @"kCFStringTransformStripDiacritics";

//
// CFLocale keys
//
const NSString *kCFLocaleMeasurementSystem =
  @"kCFLocaleMeasurementSystem";
const NSString *kCFLocaleDecimalSeparator =
  @"kCFLocaleDecimalSeparator";
const NSString *kCFLocaleGroupingSeparator =
  @"kCFLocaleGroupingSeparator";
const NSString *kCFLocaleCurrencySymbol =
  @"kCFLocaleCurrencySymbol";
const NSString *kCFLocaleCurrencyCode =
  @"currency";
const NSString *kCFLocaleIdentifier =
  @"kCFLocaleIdentifier";
const NSString *kCFLocaleLanguageCode =
  @"kCFLocaleLanguageCode";
const NSString *kCFLocaleCountryCode =
  @"kCFLocaleCountryCode";
const NSString *kCFLocaleScriptCode =
  @"kCFLocaleScriptCode";
const NSString *kCFLocaleVariantCode =
  @"kCFLocaleVariantCode";
const NSString *kCFLocaleExemplarCharacterSet =
  @"kCFLocaleExemplarCharacterSet";
const NSString *kCFLocaleCalendarIdentifier =
  @"kCFLocaleCalendarIdentifier";
const NSString *kCFLocaleCalendar =
  @"calendar";
const NSString *kCFLocaleCollationIdentifier =
  @"collation";
const NSString *kCFLocaleUsesMetricSystem =
  @"kCFLocaleUsesMetricSystem";
const NSString *kCFLocaleCollatorIdentifier =
  @"kCFLocaleCollatorIdentifier";
const NSString *kCFLocaleQuotationBeginDelimiterKey =
  @"kCFLocaleQuotationBeginDelimiterKey";
const NSString *kCFLocaleQuotationEndDelimiterKey =
  @"kCFLocaleQuotationEndDelimiterKey";
const NSString *kCFLocaleAlternateQuotationBeginDelimiterKey =
  @"kCFLocaleAlternateQuotationBeginDelimiterKey";
const NSString *kCFLocaleAlternateQuotationEndDelimiterKey =
  @"kCFLocaleAlternateQuotationEndDelimiterKey";



//
// CFCalendar identifiers
//
const NSString *kCFGregorianCalendar = @"gregorian";
const NSString *kCFBuddhistCalendar = @"buddhist";
const NSString *kCFChineseCalendar = @"chinese";
const NSString *kCFHebrewCalendar = @"hebrew";
const NSString *kCFIslamicCalendar = @"islamic";
const NSString *kCFIslamicCivilCalendar = @"islamic-civil";
const NSString *kCFJapaneseCalendar = @"japanese";
const NSString *kCFRepublicOfChinaCalendar = @"roc";
const NSString *kCFPersianCalendar = @"persian";
const NSString *kCFIndianCalendar = @"indian";
const NSString *kCFISO8601Calendar = @"";



//
// CFNumberFormatter keys
//
const NSString *kCFNumberFormatterCurrencyCode = 
  @"kCFNumberFormatterCurrencyCode";
const NSString *kCFNumberFormatterDecimalSeparator = 
  @"kCFNumberFormatterDecimalSeparator";
const NSString *kCFNumberFormatterCurrencyDecimalSeparator = 
  @"kCFNumberFormatterCurrencyDecimalSeparator";
const NSString *kCFNumberFormatterAlwaysShowDecimalSeparator = 
  @"kCFNumberFormatterAlwaysShowDecimalSeparator";
const NSString *kCFNumberFormatterGroupingSeparator = 
  @"kCFNumberFormatterGroupingSeparator";
const NSString *kCFNumberFormatterUseGroupingSeparator = 
  @"kCFNumberFormatterUseGroupingSeparator";
const NSString *kCFNumberFormatterPercentSymbol = 
  @"kCFNumberFormatterPercentSymbol";
const NSString *kCFNumberFormatterZeroSymbol = 
  @"kCFNumberFormatterZeroSymbol";
const NSString *kCFNumberFormatterNaNSymbol = 
  @"kCFNumberFormatterNaNSymbol";
const NSString *kCFNumberFormatterInfinitySymbol = 
  @"kCFNumberFormatterInfinitySymbol";
const NSString *kCFNumberFormatterMinusSign = 
  @"kCFNumberFormatterMinusSign";
const NSString *kCFNumberFormatterPlusSign = 
  @"kCFNumberFormatterPlusSign";
const NSString *kCFNumberFormatterCurrencySymbol = 
  @"kCFNumberFormatterCurrencySymbol";
const NSString *kCFNumberFormatterExponentSymbol = 
  @"kCFNumberFormatterExponentSymbol";
const NSString *kCFNumberFormatterMinIntegerDigits = 
  @"kCFNumberFormatterMinIntegerDigits";
const NSString *kCFNumberFormatterMaxIntegerDigits = 
  @"kCFNumberFormatterMaxIntegerDigits";
const NSString *kCFNumberFormatterMinFractionDigits = 
  @"kCFNumberFormatterMinFractionDigits";
const NSString *kCFNumberFormatterMaxFractionDigits = 
  @"kCFNumberFormatterMaxFractionDigits";
const NSString *kCFNumberFormatterGroupingSize = 
  @"kCFNumberFormatterGroupingSize";
const NSString *kCFNumberFormatterSecondaryGroupingSize = 
  @"kCFNumberFormatterSecondaryGroupingSize";
const NSString *kCFNumberFormatterRoundingMode = 
  @"kCFNumberFormatterRoundingMode";
const NSString *kCFNumberFormatterRoundingIncrement = 
  @"kCFNumberFormatterRoundingIncrement";
const NSString *kCFNumberFormatterFormatWidth = 
  @"kCFNumberFormatterFormatWidth";
const NSString *kCFNumberFormatterPaddingPosition = 
  @"kCFNumberFormatterPaddingPosition";
const NSString *kCFNumberFormatterPaddingCharacter = 
  @"kCFNumberFormatterPaddingCharacter";
const NSString *kCFNumberFormatterDefaultFormat = 
  @"kCFNumberFormatterDefaultFormat";
const NSString *kCFNumberFormatterMultiplier = 
  @"kCFNumberFormatterMultiplier";
const NSString *kCFNumberFormatterPositivePrefix = 
  @"kCFNumberFormatterPositivePrefix";
const NSString *kCFNumberFormatterPositiveSuffix = 
  @"kCFNumberFormatterPositiveSuffix";
const NSString *kCFNumberFormatterNegativePrefix = 
  @"kCFNumberFormatterNegativePrefix";
const NSString *kCFNumberFormatterNegativeSuffix = 
  @"kCFNumberFormatterNegativeSuffix";
const NSString *kCFNumberFormatterPerMillSymbol = 
  @"kCFNumberFormatterPerMillSymbol";
const NSString *kCFNumberFormatterInternationalCurrencySymbol = 
  @"kCFNumberFormatterInternationalCurrencySymbol"; 
const NSString *kCFNumberFormatterCurrencyGroupingSeparator = 
  @"kCFNumberFormatterCurrencyGroupingSeparator";
const NSString *kCFNumberFormatterIsLenient = 
  @"kCFNumberFormatterIsLenient";
const NSString *kCFNumberFormatterUseSignificantDigits = 
  @"kCFNumberFormatterUseSignificantDigits";
const NSString *kCFNumberFormatterMinSignificantDigits = 
  @"kCFNumberFormatterMinSignificantDigits";
const NSString *kCFNumberFormatterMaxSignificantDigits =
  @"kCFNumberFormatterMaxSignificantDigits";
