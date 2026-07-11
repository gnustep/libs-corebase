#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"
#include <string.h>

/* CFDataReplaceBytes and CFDataDeleteBytes preserve the tail after the edited range. */

const UInt8 src[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
const UInt8 ins[] = { 0xAA, 0xBB, 0xCC, 0xDD };
/* Replace (2,3) with the 4 insert bytes: length grows 10-3+4 = 11. */
const UInt8 grown[] = { 0, 1, 0xAA, 0xBB, 0xCC, 0xDD, 5, 6, 7, 8, 9 };
/* Delete (2,3): length shrinks to 7 and the tail shifts down. */
const UInt8 deleted[] = { 0, 1, 5, 6, 7, 8, 9 };

int main (void)
{
  CFMutableDataRef d;
  const UInt8 *p;

  d = CFDataCreateMutable (NULL, 0);
  CFDataAppendBytes (d, src, sizeof(src));
  CFDataReplaceBytes (d, CFRangeMake(2, 3), ins, sizeof(ins));
  p = CFDataGetBytePtr(d);
  PASS_CF(CFDataGetLength(d) == (CFIndex)sizeof(grown)
    && memcmp(p, grown, sizeof(grown)) == 0,
    "Grow replace keeps the prefix and tail around the inserted bytes.");
  CFRelease (d);

  d = CFDataCreateMutable (NULL, 0);
  CFDataAppendBytes (d, src, sizeof(src));
  CFDataDeleteBytes (d, CFRangeMake(2, 3));
  p = CFDataGetBytePtr(d);
  PASS_CF(CFDataGetLength(d) == (CFIndex)sizeof(deleted)
    && memcmp(p, deleted, sizeof(deleted)) == 0,
    "Delete removes the range and shifts the tail down.");
  CFRelease (d);

  return 0;
}
