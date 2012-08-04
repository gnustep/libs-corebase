#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "../CFTesting.h"

int main (void)
{
  CFNumberFormatterRef fmt;
  CFLocaleRef locale;
  CFStringRef str;
  CFStringRef exp;
  
  locale = CFLocaleCreate (NULL, CFSTR("en_GB"));
  fmt = CFNumberFormatterCreate (NULL, locale, kCFNumberFormatterNoStyle);
  PASS_CF(fmt != NULL, "CFNumberFormatterCreate() return non-nil");
  str = CFNumberFormatterCopyProperty (fmt,
    kCFNumberFormatterDefaultFormat);
  PASS_CFEQ(str, CFSTR("#"), "Default no-style format same as cocoa");
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, locale,
    kCFNumberFormatterDecimalStyle);
  str = CFNumberFormatterCopyProperty (fmt,
    kCFNumberFormatterDefaultFormat);
  PASS_CFEQ(str, CFSTR("#,##0.###"),
    "Default decimal-style format same as cocoa");
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, locale,
    kCFNumberFormatterCurrencyStyle);
  str = CFNumberFormatterCopyProperty (fmt,
    kCFNumberFormatterDefaultFormat);
  exp = CFStringCreateWithCString (NULL, "¤#,##0.00", kCFStringEncodingUTF8);
  PASS_CFEQ(str, exp, "Default currency-style format same as cocoa");
  CFRelease (exp);
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, locale,
    kCFNumberFormatterPercentStyle);
  str = CFNumberFormatterCopyProperty (fmt,
    kCFNumberFormatterDefaultFormat);
  PASS_CFEQ(str, CFSTR("#,##0%"),
    "Default percent-style format same as cocoa");
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, locale,
    kCFNumberFormatterScientificStyle);
  str = CFNumberFormatterCopyProperty (fmt,
    kCFNumberFormatterDefaultFormat);
  PASS_CFEQ(str, CFSTR("#E0"),
    "Default scientific-style format same as cocoa");
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterSpellOutStyle);
  PASS_CF(fmt != NULL,
    "CFNumberFormatterCreate() return non-nil for system locale");
  
  CFRelease ((CFTypeRef)locale);
  CFRelease (fmt);
  
  return 0;
}
