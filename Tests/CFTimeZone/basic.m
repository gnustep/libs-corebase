#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

#include <string.h>

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

  /* A TZif whose header declares far more transitions than the data holds
     must be rejected rather than read out of bounds. */
  {
    UInt8 hdr[44 + 16];
    CFDataRef data;
    CFStringRef name;

    memset (hdr, 0, sizeof hdr);
    memcpy (hdr, "TZif", 4);
    hdr[34] = 0x03; hdr[35] = 0xE8;  /* tzh_timecnt = 1000 (big-endian) */
    hdr[36] = 0; hdr[37] = 0; hdr[38] = 0; hdr[39] = 1; /* typecnt = 1 */
    hdr[40] = 0; hdr[41] = 0; hdr[42] = 0; hdr[43] = 4; /* charcnt = 4 */
    data = CFDataCreate (NULL, hdr, sizeof hdr);
    name = CFSTR ("Test/Truncated");
    tz = CFTimeZoneCreate (NULL, name, data);
    PASS_CF(tz == NULL,
      "A truncated TZif is rejected without reading out of bounds.");
    if (tz != NULL)
      CFRelease (tz);
    CFRelease (data);
  }

  return 0;
}
