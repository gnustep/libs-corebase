#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFCalendar.h"
#include "../CFTesting.h"

int main (void)
{
  CFCalendarRef cal;
  CFLocaleRef locale1, locale2;
  
  cal = CFCalendarCreateWithIdentifier (NULL, kCFGregorianCalendar);
  PASS_CF(cal != NULL, "Calendar created.");
  PASS_CFEQ(CFCalendarGetIdentifier(cal), kCFGregorianCalendar,
    "Correct calendar identifier.");
  CFRelease (cal);
  
  cal = CFCalendarCopyCurrent ();
  PASS_CF(cal != NULL, "CFCalendarCopyCurrent returns a calendar");
  locale1 = CFCalendarCopyLocale (cal);
  locale2 = CFLocaleCopyCurrent ();
  PASS_CFEQ(locale1, locale2,
    "Current calendar locale matches current locale.");
  CFRelease (locale1);
  CFRelease (locale2);
  CFRelease (cal);
  
  return 0;
}
