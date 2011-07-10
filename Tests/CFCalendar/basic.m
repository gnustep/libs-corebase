#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFCalendar.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  CFAbsoluteTime at = 0.0;
  int year, month, hour;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  PASS(CFCalendarComposeAbsoluteTime(cal, &at, "yMd", 2011, 7, 8),
    "Absolute time composed.");
  PASS(at == 334476000.0, "Absolute time composed correctly.");
  
  PASS(CFCalendarDecomposeAbsoluteTime(cal, at, "yMh", &year, &month, &hour),
    "Absolute time decomposed.");
  PASS(year == 2011, "Year decomposed correctly.");
  PASS(month == 7, "Month decomosed correctly.");
  PASS(hour == 7, "Hour decomposed correctly.");
  
  CFRelease (cal);
  
  return 0;
}
