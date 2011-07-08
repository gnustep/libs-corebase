#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFCalendar.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  PASS(cal != NULL, "Calendar created.");
  CFRelease (cal);
  
  cal = CFCalendarCopyCurrent ();
  PASS(cal != NULL, "CFCalendarCopyCurrent returns a calendar");
  
  return 0;
}
