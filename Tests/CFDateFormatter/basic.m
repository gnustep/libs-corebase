#include "CoreFoundation/CFDateFormatter.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

int main (void)
{
  CFAbsoluteTime at;
  CFLocaleRef loc;
  CFStringRef str;
  CFDateFormatterRef fmt;
  CFTimeZoneRef tz;
  
  loc = CFLocaleCreate (NULL, CFSTR("de_DE"));
  fmt = CFDateFormatterCreate (NULL, loc, kCFDateFormatterFullStyle,
    kCFDateFormatterShortStyle);
  tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 3600.0);
  CFDateFormatterSetProperty (fmt, kCFDateFormatterTimeZone, tz);
  
  str = CFDateFormatterGetFormat (fmt);
  PASS_CFEQ(str, CFSTR("EEEE, d. MMMM y HH:mm"),
    "Default format for de_DE locale is EEEE, d. MMMM y HH:mm");
  
  str = CFDateFormatterCreateStringWithAbsoluteTime (NULL, fmt, 65.0);
  PASS_CFEQ(str, CFSTR("Montag, 1. Januar 2001 01:01"),
    "Absolute time can be formatted using full date style.");
  CFRelease(str);
  
  PASS_CF(CFDateFormatterGetAbsoluteTimeFromString (fmt,
       CFSTR("Montag, 1. Januar 2011 23:00"), NULL, &at),
       "Absolute time gotten for 2/1/2003");
  PASS_CF(at == 315612000.0,
    "Absolute time for Montag, 1. Januar 2011 23:00 is %f", at);
  
  CFRelease(fmt);
  
  fmt = CFDateFormatterCreate (NULL, loc, kCFDateFormatterNoStyle,
    kCFDateFormatterNoStyle);
  str = CFDateFormatterCreateStringWithAbsoluteTime (NULL, fmt, 65.0);
  PASS_CFEQ(str, CFSTR("20010101 12:01 vorm."),
    "Absolute time can be formatted using no date style.");
  CFRelease(str);
  
  PASS_CF(CFDateFormatterGetAbsoluteTimeFromString (fmt,
                                                 CFSTR("20050403 02:01 vorm."),
                                                 NULL, &at),
    "Absolute time gotten for 20050403 02:01 vorm.");
  PASS_CF(at == 134186460.0,
    "Absolute time for 20050403 02:01 vorm. is %f", at);
  
  CFRelease(fmt);
  CFRelease(loc);
  
  return 0;
}