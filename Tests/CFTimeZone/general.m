#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  CFStringRef str;
  CFLocaleRef loc;
  CFAbsoluteTime at;
  CFTimeInterval ti;
  
  tz = CFTimeZoneCreateWithName (NULL, CFSTR("CST"), false);
  PASS_CF(tz == NULL,
       "Time zone named 'CST' is not found if abbreviations are not searched.");
  tz = CFTimeZoneCreateWithName (NULL, CFSTR("CST"), true);
  PASS_CF(tz != NULL,
       "Time zone named 'CST' was found when abbreviations were searched");
  str = CFTimeZoneGetName (tz);
  PASS_CFEQ(str, CFSTR("America/Chicago"), "Time zone name is 'US/Central'");
  CFRelease (str);
  CFRelease (tz);
  
  tz = CFTimeZoneCreateWithName (NULL, CFSTR("Europe/Rome"), false);
  str = CFTimeZoneGetName (tz);
  PASS_CFEQ(str, CFSTR("Europe/Rome"), "'Europe/Rome' time zone created.");
  CFRelease (str);
  
  at = CFTimeZoneGetSecondsFromGMT (tz, 1000000.0);
  PASS_CF(at == 3600.0,
       "Offset from GMT at 1000000 seconds from absolute epoch is '%f'", at);
  PASS_CF(CFTimeZoneIsDaylightSavingTime (tz, 1000000.0) == false,
       "On daylight saving time at 1000000 seconds from absolute epoch.");
  
  loc = CFLocaleCreate (NULL, CFSTR("en_GB"));
  
  str = CFTimeZoneCopyLocalizedName (tz, kCFTimeZoneNameStyleShortStandard, loc);
  PASS_CFEQ(str, CFSTR("CET"), "Short standard localized name is correct.");
  CFRelease (str);
  CFRelease (loc);
  
  ti = CFTimeZoneGetDaylightSavingTimeOffset (tz, 0.0);
  PASS_CF(ti == 0.0,
       "Daylight Saving time offset at 0 second from absolute epoch is '%f'.", ti);
  
  at = CFTimeZoneGetNextDaylightSavingTimeTransition (tz, 1000000.0);
  PASS_CF(at == 7174800.0, "Next daylight saving transition is at '%f'.", at);
  
  CFRelease (tz);
  
  return 0;
}
