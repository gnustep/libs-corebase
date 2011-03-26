#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "../CFTesting.h"

int main (void)
{
  CFNumberFormatterRef fmt;
  CFLocaleRef locale;
  CFStringRef str;
  
  locale = CFLocaleCreate (NULL, CFSTR("en_GB"));
  fmt = CFNumberFormatterCreate (NULL, locale, kCFNumberFormatterNoStyle);
  PASS(fmt != NULL, "CFNumberFormatterCreate() return non-nil");
  CFRelease ((CFTypeRef)locale);
  CFRelease (fmt);
  
  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterSpellOutStyle);
  PASS(fmt != NULL,
    "CFNumberFormatterCreate() return non-nil for system locale");
  
  return 0;
}
