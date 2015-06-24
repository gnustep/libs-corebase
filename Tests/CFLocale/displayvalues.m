#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef locale;
  CFStringRef str;
  
  locale = CFLocaleCreate (NULL, CFSTR("nb"));
  str = CFLocaleCopyDisplayNameForPropertyValue (locale, kCFLocaleLanguageCode,
                                                 CFSTR("pt"));
  PASS_CFEQ (str, CFSTR("portugisisk"), "Display language is correct");
  CFRelease (str);
  
  str = CFLocaleCopyDisplayNameForPropertyValue (locale, kCFLocaleCountryCode,
                                                 CFSTR("BR"));
  PASS_CFEQ (str, CFSTR("Brasil"), "Display country is correct");
  CFRelease (str);
  
  str = CFLocaleCopyDisplayNameForPropertyValue (locale,
                                                 kCFLocaleCalendarIdentifier,
                                                 CFSTR("gregorian"));
  PASS_CFEQ (str, CFSTR("gregoriansk kalender"),
             "Display calendar identifier is correct");
  CFRelease (str);
  
  str = CFLocaleCopyDisplayNameForPropertyValue (locale,
                                                 kCFLocaleCollationIdentifier,
                                                 CFSTR("traditional"));
  PASS_CFEQ (str, CFSTR("tradisjonell sortering"),
             "Display collation identifier is correct");
  CFRelease (str);
  
  return 0;
}

