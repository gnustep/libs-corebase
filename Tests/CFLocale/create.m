#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef locale;
  CFLocaleRef locale2;
  CFStringRef expectedIdentifier;
  
  locale = CFLocaleCopyCurrent ();
  PASS_CF(locale != NULL, "CFLocaleCopyCurrent() returns a value");
  CFRelease((CFTypeRef)locale);
  
  locale = CFLocaleGetSystem ();
  PASS_CF(locale != NULL, "CFLocaleCopyCurrent() returns a value");
  
  locale = CFLocaleCreate (NULL, CFSTR("es_ES_PREEURO"));
  PASS_CF(locale != NULL, "CFLocaleCreate() returns a value");
  expectedIdentifier = CFLocaleGetIdentifier(locale);
  PASS_CF(CFEqual(expectedIdentifier, CFSTR("es_ES_PREEURO"))
    || CFEqual(expectedIdentifier, CFSTR("es_ES@currency=ESP")),
    "Locale identifier matches one of the supported canonical PREEURO forms");
  locale2 = CFLocaleCreateCopy (NULL, locale);
  PASS_CF(locale2 != NULL, "CFLocaleCreateCopy() returns a value");
  CFRelease((CFTypeRef)locale);
  CFRelease((CFTypeRef)locale2);
  
  return 0;
}
