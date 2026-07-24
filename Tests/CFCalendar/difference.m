#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* CFCalendarGetComponentDifference reduces the difference across the
   requested components.  Times are built directly as offsets from the
   reference date. */

int main (void)
{
  double day = 86400.0;
  CFTimeZoneRef gmt = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  CFCalendarRef cal = CFCalendarCreateWithIdentifier (NULL,
    kCFGregorianCalendar);
  int y, mo, d, h, mi, s;

  CFCalendarSetTimeZone (cal, gmt);

  y = mo = d = 0;
  CFCalendarGetComponentDifference (cal, 0.0, 803 * day, 0, "yMd",
    &y, &mo, &d);
  PASS_CF (y == 2 && mo == 2 && d == 14,
    "The forward difference is 2 years 2 months 14 days.");

  y = mo = d = 0;
  CFCalendarGetComponentDifference (cal, 803 * day, 0.0, 0, "yMd",
    &y, &mo, &d);
  PASS_CF (y == -2 && mo == -2 && d == -14,
    "The backward difference is negated.");

  y = mo = d = h = mi = s = 0;
  CFCalendarGetComponentDifference (cal, 9 * day + 4 * 3600 + 5,
    45 * day + 6 * 3600 + 30 * 60 + 20, 0, "yMdHms",
    &y, &mo, &d, &h, &mi, &s);
  PASS_CF (y == 0 && mo == 1 && d == 5 && h == 2 && mi == 30 && s == 15,
    "A full breakdown is reduced across all components.");

  y = mo = d = 0;
  CFCalendarGetComponentDifference (cal, 323 * day, 369 * day, 0, "yMd",
    &y, &mo, &d);
  PASS_CF (y == 0 && mo == 1 && d == 16,
    "A month and day borrow yields 1 month 16 days.");

  y = mo = d = 0;
  CFCalendarGetComponentDifference (cal, 50 * day, 63 * day, 0, "yMd",
    &y, &mo, &d);
  PASS_CF (y == 0 && mo == 0 && d == 13,
    "A day-only difference is 13 days.");

  CFRelease (cal);
  CFRelease (gmt);
  return 0;
}
