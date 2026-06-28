#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

/* CFDataReplaceBytes and CFDataDeleteBytes preserve the tail after the edited range. */

const UInt8 src[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
const UInt8 ins[] = { 0xAA, 0xBB, 0xCC, 0xDD };

int main (void)
{
  CFMutableDataRef d;
  const UInt8 *p;

  /* Grow a middle range: replace (2,3) with 4 bytes -> length 10-3+4 = 11,
   * tail {5..9} preserved after the inserted bytes. */
  d = CFDataCreateMutable (NULL, 0);
  CFDataAppendBytes (d, src, sizeof(src));
  CFDataReplaceBytes (d, CFRangeMake(2, 3), ins, sizeof(ins));
  PASS_CF(CFDataGetLength(d) == 11, "Grow replace yields correct length.");
  p = CFDataGetBytePtr(d);
  PASS_CF(p[0] == 0 && p[1] == 1, "Prefix preserved.");
  PASS_CF(p[2] == 0xAA && p[5] == 0xDD, "Inserted bytes present.");
  PASS_CF(p[6] == 5 && p[10] == 9, "Tail preserved after grow.");
  CFRelease (d);

  /* Delete a middle range: delete (2,3) -> length 7, tail {5..9} shifts down. */
  d = CFDataCreateMutable (NULL, 0);
  CFDataAppendBytes (d, src, sizeof(src));
  CFDataDeleteBytes (d, CFRangeMake(2, 3));
  PASS_CF(CFDataGetLength(d) == 7, "Delete yields correct length.");
  p = CFDataGetBytePtr(d);
  PASS_CF(p[0] == 0 && p[1] == 1 && p[2] == 5 && p[6] == 9,
    "Tail preserved after delete.");
  CFRelease (d);

  return 0;
}
