#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef locale;
  CFTypeRef obj;
  CFTypeRef exp;
  
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
  PASS_CF(obj == NULL, "No script code for locale");
  obj = CFLocaleGetValue (locale, kCFLocaleVariantCode);
  PASS_CF(obj == NULL, "No variant code for locale");
  obj = CFLocaleGetValue (locale, kCFLocaleCalendarIdentifier);
  PASS_CFEQ(obj, CFSTR("gregorian"), "Calendar is 'gregorian'");
  obj = CFLocaleGetValue (locale, kCFLocaleCalendar);
  PASS_CFNEQ(obj, kCFNull, "Returns valid CFCalendar"); // FIXME: required CFCalendar
  obj = CFLocaleGetValue (locale, kCFLocaleCollationIdentifier);
  PASS_CFEQ(obj, CFSTR("traditional"), "Collation identifier is 'traditional'");
  obj = CFLocaleGetValue (locale, kCFLocaleUsesMetricSystem);
  PASS_CFEQ(obj, kCFBooleanTrue, "Uses metric system"); // FIXME: needs CFBoolean
  obj = CFLocaleGetValue (locale, kCFLocaleCollatorIdentifier);
  PASS_CFEQ(obj, NULL, "Collator identifier is NULL");
  
  exp = CFStringCreateWithCString (NULL, "“", kCFStringEncodingUTF8);
  obj = CFLocaleGetValue (locale, kCFLocaleQuotationBeginDelimiterKey);
  PASS_CFEQ(obj, exp, "Quotation begin delimiter is correct");
  CFRelease (exp);
  
  exp = CFStringCreateWithCString (NULL, "”", kCFStringEncodingUTF8);
  obj = CFLocaleGetValue (locale, kCFLocaleQuotationEndDelimiterKey);
  PASS_CFEQ(obj, exp, "Quotation end delimiter is correct");
  CFRelease (exp);
  
  exp = CFStringCreateWithCString (NULL, "‘", kCFStringEncodingUTF8);
  obj = CFLocaleGetValue (locale, kCFLocaleAlternateQuotationBeginDelimiterKey);
  PASS_CFEQ(obj, exp, "Alternate quotation begin delimiter is correct");
  obj = CFLocaleGetValue (locale, kCFLocaleAlternateQuotationEndDelimiterKey);
  CFRelease (exp);
  
  exp = CFStringCreateWithCString (NULL, "’", kCFStringEncodingUTF8);
  PASS_CFEQ(obj, exp, "Alternate quotation end delimiter is correct");
  CFRelease (exp);
  
  CFRelease ((CFTypeRef)locale);
  
  return 0;
}

