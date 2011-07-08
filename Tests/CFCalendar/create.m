#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFCalendar.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  CFLocaleRef locale1, locale2;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  PASS(cal != NULL, "Calendar created.");
  PASS_EQUAL(CFCalendarGetIdentifier(cal), kCFGregorianCalendar,
    "Correct calendar identifier.");
  CFRelease (cal);
  
  cal = CFCalendarCopyCurrent ();
  PASS(cal != NULL, "CFCalendarCopyCurrent returns a calendar");
  locale1 = CFCalendarCopyLocale (cal);
  locale2 = CFLocaleCopyCurrent ();
  PASS_EQUAL(locale1, locale2,
    "Current calendar locale matches current locale.");
  CFRelease (locale1);
  CFRelease (locale2);
  CFRelease (cal);
  
  return 0;
}
