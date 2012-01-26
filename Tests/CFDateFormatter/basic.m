#include "CoreFoundation/CFDateFormatter.h"
#include "../CFTesting.h"

int main (void)
{
  CFLocaleRef loc;
  CFStringRef str;
  CFDateFormatterRef fmt;
  
  loc = CFLocaleCreate (NULL, CFSTR("de_DE"));
  fmt = CFDateFormatterCreate (NULL, loc, kCFDateFormatterFullStyle,
    kCFDateFormatterShortStyle);
  
  str = CFDateFormatterGetFormat (fmt);
  PASS_CFEQ(str, CFSTR("EEEE, d. MMMM y HH:mm"),
    "Default format for de_DE locale is EEEE, d. MMMM y HH:mm");
  
  str = CFDateFormatterCreateStringWithAbsoluteTime (NULL, fmt, 65.0);
  PASS_CFEQ(str, CFSTR("Montag, 1. Januar 2001 00:01"),
    "Absolute time can be formatted.");
  CFRelease(str);
  
  
  
  return 0;
}