#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* CFCalendarGetMaximumRangeOfUnit is the widest a unit can be, and
   CFCalendarGetMinimumRangeOfUnit is the narrowest. */

int main (void)
{
  CFTimeZoneRef gmt = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  CFCalendarRef cal = CFCalendarCreateWithIdentifier (NULL,
    kCFGregorianCalendar);
  CFRange r;

  CFCalendarSetTimeZone (cal, gmt);

  r = CFCalendarGetMaximumRangeOfUnit (cal, kCFCalendarUnitDay);
  PASS_CF (r.location == 1 && r.length == 31,
    "The maximum day-of-month range is 1 to 31.");

  r = CFCalendarGetMinimumRangeOfUnit (cal, kCFCalendarUnitDay);
  PASS_CF (r.location == 1 && r.length == 28,
    "The minimum day-of-month range is 1 to 28.");

  CFRelease (cal);
  CFRelease (gmt);
  return 0;
}
