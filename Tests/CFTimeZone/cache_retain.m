#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* Regression test for the time-zone name cache use-after-free.

   CFTimeZoneCreate caches every created zone in the global name cache.
   When the cache stored the zone without retaining it, releasing the
   caller's only reference deallocated the object while the cache still
   held a dangling pointer to it; the next lookup of the same name then
   returned and retained freed memory.  AddressSanitizer flags this as a
   heap-use-after-free in CFRetain/CFGetTypeID on the second create. */

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

  /* Drop our only reference.  With the bug the cached entry is now a
     dangling pointer. */
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
