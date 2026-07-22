#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFCalendar.h"
#include "../CFTesting.h"

/* CFTimeZoneCreateWithName resolves GMT and UTC offset names, so a calendar
   whose time zone is a GMT offset can be copied back. */

int main (void)
{
  CFTimeZoneRef z0 = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  CFTimeZoneRef tz;
  CFCalendarRef cal;

  tz = CFTimeZoneCreateWithName (NULL, CFSTR ("GMT"), true);
  PASS_CF (tz != NULL && CFTimeZoneGetSecondsFromGMT (tz, 0.0) == 0,
    "GMT resolves to a zero offset.");

  tz = CFTimeZoneCreateWithName (NULL, CFSTR ("UTC"), true);
  PASS_CF (tz != NULL && CFTimeZoneGetSecondsFromGMT (tz, 0.0) == 0,
    "UTC resolves to a zero offset.");

  tz = CFTimeZoneCreateWithName (NULL, CFSTR ("GMT-05:00"), true);
  PASS_CF (tz != NULL && CFTimeZoneGetSecondsFromGMT (tz, 0.0) == -18000,
    "GMT-05:00 resolves to minus 18000 seconds.");

  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  CFCalendarSetTimeZone (cal, z0);
  tz = CFCalendarCopyTimeZone (cal);
  PASS_CF (tz != NULL && CFTimeZoneGetSecondsFromGMT (tz, 0.0) == 0,
    "A calendar's GMT time zone can be copied back.");

  CFRelease (cal);
  CFRelease (z0);
  return 0;
}
