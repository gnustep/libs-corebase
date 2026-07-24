#include <CoreFoundation/CFStream.h>

#include "../CFTesting.h"
#include <string.h>

int main (void)
{
  const char *data = "ABCDEFGHIJ";
  UInt8 buf[64];
  CFIndex avail = 0;
  const UInt8 *gb;
  CFIndex n;
  CFReadStreamRef r;

  r = CFReadStreamCreateWithBytesNoCopy (NULL, (const UInt8 *)data, 10,
                                         kCFAllocatorNull);
  CFReadStreamOpen (r);

  gb = CFReadStreamGetBuffer (r, 4, &avail);
  PASS_CF(gb != NULL, "CFReadStreamGetBuffer returns a pointer.");
  PASS_CF(avail == 4, "CFReadStreamGetBuffer reports the requested count.");
  PASS_CF(memcmp (gb, "ABCD", 4) == 0,
          "CFReadStreamGetBuffer points at the unread bytes.");

  n = CFReadStreamRead (r, buf, 4);
  PASS_CF(n == 4 && memcmp (buf, "EFGH", 4) == 0,
          "A read after GetBuffer continues past the returned bytes.");

  CFReadStreamClose (r);
  CFRelease (r);

  return 0;
}
