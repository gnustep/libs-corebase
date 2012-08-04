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
  
  return 0;
}