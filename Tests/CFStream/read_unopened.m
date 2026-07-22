#include <CoreFoundation/CFStream.h>

#include "../CFTesting.h"

int main (void)
{
  const char *data = "ABCDEFGHIJ";
  CFReadStreamRef r;

  r = CFReadStreamCreateWithBytesNoCopy (NULL, (const UInt8 *)data, 10,
                                         kCFAllocatorNull);
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusNotOpen,
          "Status is NotOpen before opening.");
  PASS_CF(CFReadStreamHasBytesAvailable (r) == false,
          "No bytes are available before the stream is opened.");
  CFRelease (r);

  return 0;
}
