#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

/* CFAbsoluteTimeGetDifferenceAsGregorianUnits.  Times are built directly as
   offsets from the reference date. */

int main (void)
{
  double day = 86400.0;
  CFOptionFlags ymd = kCFGregorianUnitsYears | kCFGregorianUnitsMonths
    | kCFGregorianUnitsDays;
  CFOptionFlags all = ymd | kCFGregorianUnitsHours | kCFGregorianUnitsMinutes
    | kCFGregorianUnitsSeconds;
  CFGregorianUnits u;

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (803 * day, 0.0, NULL, ymd);
  PASS_CF (u.years == 2 && u.months == 2 && u.days == 14,
    "2003-03-15 minus 2001-01-01 is 2 years 2 months 14 days.");

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (0.0, 803 * day, NULL, ymd);
  PASS_CF (u.years == -2 && u.months == -2 && u.days == -14,
    "The reverse difference is negated.");

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (
    45 * day + 6 * 3600 + 30 * 60 + 20, 9 * day + 4 * 3600 + 5, NULL, all);
  PASS_CF (u.years == 0 && u.months == 1 && u.days == 5 && u.hours == 2
    && u.minutes == 30 && u.seconds == 15.0,
    "A full component difference is broken down correctly.");

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (63 * day, 50 * day, NULL,
    all);
  PASS_CF (u.years == 0 && u.months == 0 && u.days == 13,
    "A day borrow across February yields 13 days.");

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (
    151 * day + 15 * 60 + 10, 150 * day + 23 * 3600 + 45 * 60 + 20, NULL, all);
  PASS_CF (u.years == 0 && u.months == 0 && u.days == 0 && u.hours == 0
    && u.minutes == 29 && u.seconds == 50.0,
    "A borrow across midnight yields 29 minutes 50 seconds.");

  u = CFAbsoluteTimeGetDifferenceAsGregorianUnits (369 * day, 323 * day, NULL,
    all);
  PASS_CF (u.years == 0 && u.months == 1 && u.days == 16,
    "A year and month and day borrow yields 1 month 16 days.");

  return 0;
}
