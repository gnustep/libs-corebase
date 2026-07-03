#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

/* Building a CFMutableData by many single-byte appends drives the capacity
   reallocation path repeatedly; check that the length and bytes stay intact
   across the reallocations. */

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
