#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

/* CFAbsoluteTimeGetWeekOfYear returns the ISO 8601 week number.  Times are
   built directly as offsets from the reference date. */

int main (void)
{
  double day = 86400.0;

  PASS_CF (CFAbsoluteTimeGetWeekOfYear (0.0, NULL) == 1,
    "2001-01-01 is in week 1.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (6 * day, NULL) == 1,
    "2001-01-07 is in week 1.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (7 * day, NULL) == 2,
    "2001-01-08 is in week 2.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (14 * day, NULL) == 3,
    "2001-01-15 is in week 3.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (364 * day, NULL) == 1,
    "2001-12-31 belongs to week 1 of the following year.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (1460 * day, NULL) == 53,
    "2004-12-31 is in week 53.");
  PASS_CF (CFAbsoluteTimeGetWeekOfYear (1461 * day, NULL) == 53,
    "2005-01-01 belongs to week 53 of the previous year.");

  return 0;
}
