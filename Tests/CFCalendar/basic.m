#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFCalendar.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  CFAbsoluteTime at = 0.0;
  CFTimeInterval ti= 0.0;
  int year, month, hour, minute, second;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  // FIXME: Need to specify a CFTimeZone but it is currently unsupported.
  PASS(CFCalendarComposeAbsoluteTime(cal, &at, "yMd", 2011, 7, 8),
    "Absolute time composed.");
  PASS(at == 331794000.0, "Absolute time composed correctly.");
  
  PASS(CFCalendarDecomposeAbsoluteTime(cal, at, "yMH", &year, &month, &hour),
    "Absolute time decomposed.");
  PASS(year == 2011, "Year decomposed correctly.");
  PASS(month == 7, "Month decomosed correctly.");
  PASS(hour == 0, "Hour decomposed correctly.");
  
  PASS(CFCalendarGetComponentDifference(cal, at, at + 3600.0, 0, "s", &second),
    "Got component difference.");
  PASS(second == 3600, "Got seconds component difference correctly.");
  
  PASS(CFCalendarGetComponentDifference(cal, at, at + 3663.0,
    kCFCalendarComponentsWrap, "Hms", &hour, &minute, &second),
    "Got wrapped components difference.");
  PASS(hour == 1 && minute == 1 && second == 3,
    "Got component difference correctly.");
  
  PASS(CFCalendarGetTimeRangeOfUnit (cal, kCFCalendarUnitWeekday, 331365600.0, &at, &ti),
    "Got time range of kCFCalendarUnitWeekday.");
  PASS(at == 331365600.0, "Got start of week.");
  PASS(ti == 604800.0, "Time interval is 7 days long.");
  
  CFRelease (cal);
  
  return 0;
}
