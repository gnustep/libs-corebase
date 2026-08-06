#include "CoreFoundation/CFDate.h"
#include "../CFTesting.h"

int main (void)
{
  CFDateRef date;
  CFDateRef date2;
  
  date = CFDateCreate (NULL, 128.0);
  PASS_CF(CFDateGetAbsoluteTime(date) == 128.0, "Date created correctly.");
  
  date2 = CFDateCreate (NULL, 127.0);
  PASS_CF(CFDateCompare(date, date2, NULL) == kCFCompareGreaterThan,
    "First date is greater than second date.");
  PASS_CF(CFDateGetTimeIntervalSinceDate(date, date2) == 1.0,
    "First date is one second ahead of second date.");
  
  CFRelease (date);
  CFRelease (date2);

  /* CFAbsoluteTimeToFields walked _daysBeforeMonth past its end when
     locating the month; check both halves of the table (non-leap and
     leap years). */
  {
    CFGregorianDate g;

    g = CFAbsoluteTimeGetGregorianDate (0.0, NULL);
    PASS_CF(g.year == 2001 && g.month == 1 && g.day == 1,
      "GregorianDate for absolute time 0 is 2001-01-01 (got %d-%02d-%02d).",
      (int)g.year, (int)g.month, (int)g.day);

    /* 2004 is a leap year; 1154 days after 2001-01-01 is the leap day. */
    g = CFAbsoluteTimeGetGregorianDate (86400.0 * 1154, NULL);
    PASS_CF(g.year == 2004 && g.month == 2 && g.day == 29,
      "GregorianDate for the leap day is 2004-02-29 (got %d-%02d-%02d).",
      (int)g.year, (int)g.month, (int)g.day);
  }

  return 0;
}