#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef locale;
  CFTypeRef obj;
  
  locale = CFLocaleCreate (NULL,
    CFSTR("pt_BR@calendar=gregorian;collation=traditional;currency=BRL"));
  obj = CFLocaleGetValue (locale, kCFLocaleMeasurementSystem);
  PASS_CFEQ(obj, CFSTR("Metric"), "'pt_BR' measurement system is 'Metric'");
  obj = CFLocaleGetValue (locale, kCFLocaleDecimalSeparator);
  PASS_CFEQ(obj, CFSTR(","), "Decimal separtor is ','");
  obj = CFLocaleGetValue (locale, kCFLocaleGroupingSeparator);
  PASS_CFEQ(obj, CFSTR("."), "Grouping separator is '.'");
  obj = CFLocaleGetValue (locale, kCFLocaleCurrencySymbol);
  PASS_CFEQ(obj, CFSTR("R$"), "Currency symbol is 'R$'");
  obj = CFLocaleGetValue (locale, kCFLocaleCurrencyCode);
  PASS_CFEQ(obj, CFSTR("BRL"), "Currency code is 'BRL'");
  obj = CFLocaleGetValue (locale, kCFLocaleIdentifier);
  PASS_CFEQ(obj,
    CFSTR("pt_BR@calendar=gregorian;collation=traditional;currency=BRL"),
    "Locale identifier is correct");
  obj = CFLocaleGetValue (locale, kCFLocaleLanguageCode);
  PASS_CFEQ(obj, CFSTR("pt"), "Language code is 'pt'");
  obj = CFLocaleGetValue (locale, kCFLocaleCountryCode);
  PASS_CFEQ(obj, CFSTR("BR"), "Country code is 'BR'");
  obj = CFLocaleGetValue (locale, kCFLocaleScriptCode);
  PASS_CFEQ(obj, kCFNull, "No script code for locale");
  obj = CFLocaleGetValue (locale, kCFLocaleVariantCode);
  PASS_CFEQ(obj, kCFNull, "No variant code for locale");
  obj = CFLocaleGetValue (locale, kCFLocaleExemplarCharacterSet);
  // There isn't a good way to test this so I'm just testing it isn't NULL.
  PASS_CFNEQ(obj, kCFNull, "There is a character set");
  obj = CFLocaleGetValue (locale, kCFLocaleCalendarIdentifier);
  PASS_CFEQ(obj, CFSTR("gregorian"), "Calendar is 'gregorian'");
  obj = CFLocaleGetValue (locale, kCFLocaleCalendar);
  PASS_CFNEQ(obj, kCFNull, "Returns valid CFCalendar"); // FIXME: required CFCalendar
  obj = CFLocaleGetValue (locale, kCFLocaleCollationIdentifier);
  PASS_CFEQ(obj, CFSTR("traditional"), "Collation identifier is 'traditional'");
  obj = CFLocaleGetValue (locale, kCFLocaleUsesMetricSystem);
  PASS_CFEQ(obj, kCFNull, "Uses metric system"); // FIXME: needs CFBoolean
  obj = CFLocaleGetValue (locale, kCFLocaleCollatorIdentifier);
  PASS_CFEQ(obj, kCFNull, "Collator identifier is NULL");
  
  // These next 4 tests fail because CFSTR() does not support
  // characters and whines about non-ASCII characters in the string.
  obj = CFLocaleGetValue (locale, kCFLocaleQuotationBeginDelimiterKey);
  PASS_CFEQ(obj, CFSTR("“"), "Quotation begin delimiter is correct");
  obj = CFLocaleGetValue (locale, kCFLocaleQuotationEndDelimiterKey);
  PASS_CFEQ(obj, CFSTR("”"), "Quotation end delimiter is correct");
  obj = CFLocaleGetValue (locale, kCFLocaleAlternateQuotationBeginDelimiterKey);
  PASS_CFEQ(obj, CFSTR("‘"), "Alternate quotation begin delimiter is correct");
  obj = CFLocaleGetValue (locale, kCFLocaleAlternateQuotationEndDelimiterKey);
  PASS_CFEQ(obj, CFSTR("’"), "Alternate quotation end delimiter is correct");
  CFRelease (locale);
  
  return 0;
}
