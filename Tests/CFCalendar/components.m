#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* CFCalendar accessors and component composition.  Times are built directly
   as offsets from the reference date. */

int main (void)
{
  double day = 86400.0;
  CFTimeZoneRef gmt = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  CFCalendarRef cal = CFCalendarCreateWithIdentifier (NULL,
    kCFGregorianCalendar);
  CFAbsoluteTime at;
  CFAbsoluteTime start;
  CFTimeInterval ti;
  int y, mo, d;

  CFCalendarSetTimeZone (cal, gmt);

  PASS_CF (CFCalendarGetTypeID () != 0, "CFCalendarGetTypeID is registered.");
  PASS_CF (CFGetTypeID (cal) == CFCalendarGetTypeID (),
    "A calendar carries the calendar type ID.");
  PASS_CFEQ (CFCalendarGetIdentifier (cal), kCFGregorianCalendar,
    "The identifier is the Gregorian calendar.");

  CFCalendarSetFirstWeekday (cal, 2);
  PASS_CF (CFCalendarGetFirstWeekday (cal) == 2,
    "The first weekday round-trips.");
  CFCalendarSetMinimumDaysInFirstWeek (cal, 4);
  PASS_CF (CFCalendarGetMinimumDaysInFirstWeek (cal) == 4,
    "The minimum days in the first week round-trips.");

  at = 0.0;
  CFCalendarComposeAbsoluteTime (cal, &at, "yMd", 2003, 3, 15);
  PASS_CF (at == 803 * day, "Composing 2003-03-15 gives the right time.");

  y = mo = d = 0;
  CFCalendarDecomposeAbsoluteTime (cal, 803 * day, "yMd", &y, &mo, &d);
  PASS_CF (y == 2003 && mo == 3 && d == 15,
    "Decomposing gives year 2003, month 3, day 15.");

  at = 45 * day;
  CFCalendarAddComponents (cal, &at, 0, "M", 1);
  PASS_CF (at == 73 * day, "Adding one month to 2001-02-15 gives 2001-03-15.");

  CFCalendarGetTimeRangeOfUnit (cal, kCFCalendarUnitMonth, 45 * day,
    &start, &ti);
  PASS_CF (start == 31 * day && ti == 28 * day,
    "The month containing 2001-02-15 starts on the 1st and lasts 28 days.");

  CFRelease (cal);
  CFRelease (gmt);
  return 0;
}
