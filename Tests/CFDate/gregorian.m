#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

int main (void)
{
  CFGregorianDate gd;
  CFGregorianDate g2;
  CFGregorianDate bad;
  CFGregorianUnits u;
  CFAbsoluteTime t;

  PASS_CF(CFDateGetTypeID () != 0, "CFDateGetTypeID is not zero.");

  gd.year = 2001;
  gd.month = 1;
  gd.day = 1;
  gd.hour = 0;
  gd.minute = 0;
  gd.second = 0.0;
  t = CFGregorianDateGetAbsoluteTime (gd, NULL);
  PASS_CF(t == 0.0, "2001-01-01 00:00 UTC is absolute time 0.");

  g2 = CFAbsoluteTimeGetGregorianDate (0.0, NULL);
  PASS_CF(g2.year == 2001 && g2.month == 1 && g2.day == 1,
    "Absolute time 0 is 2001-01-01.");

  PASS_CF(CFGregorianDateIsValid (gd,
      kCFGregorianUnitsYears | kCFGregorianUnitsMonths | kCFGregorianUnitsDays),
    "A well-formed Gregorian date is valid.");
  bad = gd;
  bad.month = 13;
  PASS_CF(!CFGregorianDateIsValid (bad, kCFGregorianUnitsMonths),
    "Month 13 is not valid.");

  (void)u;
  (void)t;

  return 0;
}
