#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* Regression test for the time-zone name cache use-after-free. */

int main (void)
{
  CFTimeZoneRef tz1;
  CFTimeZoneRef tz2;
  CFStringRef name = CFSTR("America/New_York");

  tz1 = CFTimeZoneCreateWithName (NULL, name, false);
  if (tz1 == NULL)
    {
      /* zoneinfo for this name is not installed; nothing to exercise */
      PASS_CF(1, "zoneinfo for America/New_York unavailable, skipping");
      return 0;
    }

  /* Drop our own reference; the cache must still hold a live one. */
  CFRelease (tz1);

  /* Look the same name up again: the cache must own a live reference. */
  tz2 = CFTimeZoneCreateWithName (NULL, name, false);
  PASS_CF(tz2 != NULL,
    "re-creating a released cached time zone returns a valid object");
  PASS_CFEQ(CFTimeZoneGetName (tz2), name,
    "the re-created time zone has the expected name");

  CFRelease (tz2);

  return 0;
}
