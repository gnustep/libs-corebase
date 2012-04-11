#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFTimeZoneRef tz;
  CFStringRef str;
  CFTimeInterval ti;
  CFAbsoluteTime at;
  
  tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  PASS(tz != NULL, "CFTimeZone create successfully.");
  PASS_CFEQ(CFTimeZoneGetName(tz), CFSTR("GMT+0000"),
    "CFTimeZone has correct name.");
  
  str = CFTimeZoneCopyAbbreviation (tz, 0.0);
  PASS_CFEQ(str, CFSTR("GMT+0000"), "Time zone abbreviations are equal.");
  
  ti = CFTimeZoneGetSecondsFromGMT (tz, 0.0);
  PASS(ti == 0.0, "GMT+0000 offset from GMT is %g", ti);
  
  at = CFTimeZoneGetNextDaylightSavingTimeTransition (tz, 0.0);
  PASS(at == 0.0, "Next transition for GMT+0000 is %g", at);
  
  CFRelease (str);
  CFRelease (tz);
  
  return 0;
}
