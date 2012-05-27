#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFTimeZone.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  CFTimeZoneRef tz;
  CFAbsoluteTime at = 0.0;
  CFTimeInterval ti= 0.0;
  int year, month, hour, minute, second;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, -3600.0);
  CFCalendarSetTimeZone (cal, tz);
  CFRelease (tz);
  
  PASS(CFCalendarComposeAbsoluteTime(cal, &at, "yMd", 2011, 7, 8),
    "Absolute time composed.");
  testHopeful = true;
  PASS(at == 331779600.0, "Absolute time composed correctly (%f).", at);
  testHopeful = false;
  
  PASS(CFCalendarDecomposeAbsoluteTime(cal, at, "yMH", &year, &month, &hour),
    "Absolute time decomposed.");
  PASS(year == 2011, "Year decomposed correctly.");
  PASS(month == 7, "Month decomosed correctly.");
  PASS(hour == 0, "Hour decomposed correctly.");
  
  PASS(CFCalendarGetComponentDifference(cal, at, at + 3600.0, 0, "s", &second),
    "Got component difference.");
  PASS(second == 3600, "Seconds difference is %d", second);
  
  PASS(CFCalendarGetComponentDifference(cal, at, at + 3663.0,
    kCFCalendarComponentsWrap, "Hms", &hour, &minute, &second),
    "Got wrapped components difference.");
  PASS(hour == 1 && minute == 1 && second == 3,
       "Component difference is %d hour(s), %d minute(s) and %d second(s)",
       hour, minute, second);
  
  PASS(CFCalendarGetTimeRangeOfUnit (cal, kCFCalendarUnitWeekday, 331365601.0, &at, &ti),
    "Got time range of kCFCalendarUnitWeekday.");
  PASS(at == 331347600.0, "Got start of weekday (%f).", at);
  PASS(ti == 86400.0, "Time interval is %d day(s) long (%f).",
       (int)(ti/86400.0), ti);
  
  CFRelease (cal);
  
  return 0;
}
