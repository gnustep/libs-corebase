#include <stdio.h>

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFLocale.h>

#include "../Testing.h"

int main (void)
{
  CFLocaleRef locale;
  CFLocaleRef locale2;
  CFStringRef str;
  
  locale = CFLocaleCopyCurrent ();
  PASS_NEQ (locale, NULL, "CFLocaleCopyCurrent() returns a value");
  CFRelease((CFTypeRef)locale);
  
  locale = CFLocaleGetSystem ();
  PASS_NEQ (locale, NULL, "CFLocaleCopyCurrent() returns a value");
  
  locale = CFLocaleCreate (NULL, CFSTR("es_ES_PREEURO"));
  PASS_NEQ (locale, NULL, "CFLocaleCreate() returns a value");
  PASS_EQ (CFLocaleGetIdentifier(locale), CFSTR("es_ES@currency=ESP"),
    "en_ES_PREEURO is stored as es_ES@currency=ESP");
  locale2 = CFLocaleCreateCopy (NULL, locale);
  PASS_NEQ (locale2, NULL, "CFLocaleCreateCopy() returns a value");
  CFRelease((CFTypeRef)locale);
  CFRelease((CFTypeRef)locale2);
  
  return 0;
}
