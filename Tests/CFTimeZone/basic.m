#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  CFStringRef str;
  CFTimeInterval ti;
  CFAbsoluteTime at;
  
  tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  PASS_CF(tz != NULL, "CFTimeZone create successfully.");
  PASS_CFEQ(CFTimeZoneGetName(tz), CFSTR("GMT+00:00"),
    "CFTimeZone has correct name.");
  
  str = CFTimeZoneCopyAbbreviation (tz, 0.0);
  PASS_CFEQ(str, CFSTR("GMT+00:00"), "Time zone abbreviations are equal.");
  
  ti = CFTimeZoneGetSecondsFromGMT (tz, 0.0);
  PASS_CF(ti == 0.0, "GMT+00:00 offset from GMT is %g", ti);
  
  at = CFTimeZoneGetNextDaylightSavingTimeTransition (tz, 0.0);
  PASS_CF(at == 0.0, "Next transition for GMT+00:00 is %g", at);
  
  CFRelease (str);
  CFRelease (tz);
  
  return 0;
}
