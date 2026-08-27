#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CF(CFAbsoluteTimeGetDayOfWeek (0.0, NULL) == 1,
    "2001-01-01 is day of week 1 (Monday).");
  PASS_CF(CFAbsoluteTimeGetDayOfYear (0.0, NULL) == 1,
    "2001-01-01 is day of year 1.");

  PASS_CF(CFAbsoluteTimeGetDayOfWeek (86400.0, NULL) == 2,
    "2001-01-02 is day of week 2 (Tuesday).");
  PASS_CF(CFAbsoluteTimeGetDayOfYear (86400.0, NULL) == 2,
    "2001-01-02 is day of year 2.");

  PASS_CF(CFAbsoluteTimeGetDayOfYear (31.0 * 86400.0, NULL) == 32,
    "2001-02-01 is day of year 32.");

  return 0;
}
