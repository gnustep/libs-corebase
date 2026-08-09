#include <CoreFoundation/CFStream.h>

#include "../CFTesting.h"

int main (void)
{
  const char *data = "ABCDEFGHIJ";
  UInt8 buf[64];
  CFReadStreamRef r;

  r = CFReadStreamCreateWithBytesNoCopy (NULL, (const UInt8 *)data, 10,
                                         kCFAllocatorNull);
  CFReadStreamOpen (r);
  CFReadStreamRead (r, buf, 10);
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusAtEnd,
          "Status is AtEnd once all bytes are consumed.");
  PASS_CF(CFReadStreamHasBytesAvailable (r) == false,
          "No bytes are available at the end.");
  PASS_CF(CFReadStreamRead (r, buf, 64) == 0,
          "Reading past the end returns zero.");
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusAtEnd,
          "Status remains AtEnd past the end.");
  CFReadStreamClose (r);
  CFRelease (r);

  return 0;
}
