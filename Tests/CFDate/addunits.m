#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

static CFAbsoluteTime
mk (int y, int mo, int d)
{
  CFGregorianDate g;
  g.year = y;
  g.month = mo;
  g.day = d;
  g.hour = 0;
  g.minute = 0;
  g.second = 0.0;
  return CFGregorianDateGetAbsoluteTime (g, NULL);
}

static CFGregorianUnits
un (int y, int mo, int d, int h, int mi, double s)
{
  CFGregorianUnits u;
  u.years = y;
  u.months = mo;
  u.days = d;
  u.hours = h;
  u.minutes = mi;
  u.seconds = s;
  return u;
}

int main (void)
{
  CFGregorianDate g;

  PASS_CF(mk (2002, 1, 1) == 365.0 * 86400.0,
    "CFGregorianDateGetAbsoluteTime converts the day count to seconds.");

  g = CFAbsoluteTimeGetGregorianDate (
    CFAbsoluteTimeAddGregorianUnits (mk (2001, 1, 1), NULL,
      un (1, 0, 0, 0, 0, 0)), NULL);
  PASS_CF(g.year == 2002 && g.month == 1 && g.day == 1,
    "Adding one year to 2001-01-01 gives 2002-01-01.");

  g = CFAbsoluteTimeGetGregorianDate (
    CFAbsoluteTimeAddGregorianUnits (mk (2001, 1, 15), NULL,
      un (0, 1, 0, 0, 0, 0)), NULL);
  PASS_CF(g.year == 2001 && g.month == 2 && g.day == 15,
    "Adding one month to 2001-01-15 gives 2001-02-15.");

  g = CFAbsoluteTimeGetGregorianDate (
    CFAbsoluteTimeAddGregorianUnits (mk (2001, 1, 1), NULL,
      un (0, 0, 10, 0, 0, 0)), NULL);
  PASS_CF(g.year == 2001 && g.month == 1 && g.day == 11,
    "Adding ten days to 2001-01-01 gives 2001-01-11.");

  g = CFAbsoluteTimeGetGregorianDate (
    CFAbsoluteTimeAddGregorianUnits (mk (2001, 1, 1), NULL,
      un (0, 13, 0, 0, 0, 0)), NULL);
  PASS_CF(g.year == 2002 && g.month == 2 && g.day == 1,
    "Adding thirteen months to 2001-01-01 normalizes to 2002-02-01.");

  return 0;
}
