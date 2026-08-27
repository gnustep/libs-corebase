#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* CFCalendarGetOrdinalityOfUnit and CFCalendarGetRangeOfUnit.  Times are
   built directly as offsets from the reference date. */

int main (void)
{
  double day = 86400.0;
  CFTimeZoneRef gmt = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0);
  CFCalendarRef cal = CFCalendarCreateWithIdentifier (NULL,
    kCFGregorianCalendar);
  CFRange r;

  CFCalendarSetTimeZone (cal, gmt);

  r = CFCalendarGetRangeOfUnit (cal, kCFCalendarUnitDay, kCFCalendarUnitMonth,
    45 * day);                   /* 2001-02-15 */
  PASS_CF (r.location == 1 && r.length == 28,
    "The days of February 2001 range from 1 to 28.");

  r = CFCalendarGetRangeOfUnit (cal, kCFCalendarUnitDay, kCFCalendarUnitMonth,
    63 * day);                   /* 2001-03-05 */
  PASS_CF (r.location == 1 && r.length == 31,
    "The days of March 2001 range from 1 to 31.");

  PASS_CF (CFCalendarGetOrdinalityOfUnit (cal, kCFCalendarUnitDay,
    kCFCalendarUnitMonth, 45 * day) == 15,
    "2001-02-15 is the 15th day of the month.");
  PASS_CF (CFCalendarGetOrdinalityOfUnit (cal, kCFCalendarUnitDay,
    kCFCalendarUnitYear, 45 * day) == 46,
    "2001-02-15 is the 46th day of the year.");
  PASS_CF (CFCalendarGetOrdinalityOfUnit (cal, kCFCalendarUnitMonth,
    kCFCalendarUnitYear, 45 * day) == 2,
    "2001-02-15 is in the 2nd month of the year.");

  CFRelease (cal);
  CFRelease (gmt);
  return 0;
}
