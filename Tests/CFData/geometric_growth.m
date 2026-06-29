#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

/* Exercises the CFMutableData growth path.  Building a CFMutableData by many
   single-byte appends drives CFDataCheckCapacityAndGrow repeatedly; this
   guards that the geometric-growth reallocation keeps the contents intact
   (length and bytes) across the reallocations.  (The geometric growth itself
   is a performance fix -- exact-size growth was O(n^2) -- and is validated by
   measurement; this test guards the correctness of the growth code.) */

int main (void)
{
  CFMutableDataRef d;
  const CFIndex n = 20000;
  CFIndex i;
  const UInt8 *p;
  Boolean intact = true;

  d = CFDataCreateMutable (NULL, 0);
  PASS_CF(d != NULL, "created an empty mutable data");

  for (i = 0; i < n; i++)
    {
      UInt8 b = (UInt8)(i & 0xff);
      CFDataAppendBytes (d, &b, 1);
    }

  PASS_CF(CFDataGetLength (d) == n,
    "length is correct after %ld single-byte appends", (long)n);

  p = CFDataGetBytePtr (d);
  for (i = 0; i < n; i++)
    {
      if (p[i] != (UInt8)(i & 0xff))
        {
          intact = false;
          break;
        }
    }
  PASS_CF(intact, "contents are intact across the growth reallocations");

  CFRelease (d);
  return 0;
}
