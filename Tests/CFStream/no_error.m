#include <CoreFoundation/CFStream.h>

#include "../CFTesting.h"

int main (void)
{
  const char *data = "ABC";
  CFReadStreamRef r;
  CFWriteStreamRef w;
  CFStreamError e;

  r = CFReadStreamCreateWithBytesNoCopy (NULL, (const UInt8 *)data, 3,
                                         kCFAllocatorNull);
  CFReadStreamOpen (r);
  e = CFReadStreamGetError (r);
  PASS_CF(e.domain == 0 && e.error == 0,
          "A read stream with no error reports domain 0.");
  CFReadStreamClose (r);
  CFRelease (r);

  w = CFWriteStreamCreateWithAllocatedBuffers (NULL, NULL);
  CFWriteStreamOpen (w);
  e = CFWriteStreamGetError (w);
  PASS_CF(e.domain == 0 && e.error == 0,
          "A write stream with no error reports domain 0.");
  CFWriteStreamClose (w);
  CFRelease (w);

  return 0;
}
