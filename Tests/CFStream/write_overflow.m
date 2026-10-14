#include <CoreFoundation/CFStream.h>

#include "../CFTesting.h"
#include <string.h>
#include <errno.h>

int main (void)
{
  UInt8 fb[8];
  CFWriteStreamRef w;
  CFStreamError e;

  w = CFWriteStreamCreateWithBuffer (NULL, fb, 8);
  CFWriteStreamOpen (w);
  PASS_CF(CFWriteStreamWrite (w, (const UInt8 *)"ABCDEFGH", 8) == 8,
          "Filling the fixed buffer exactly succeeds.");
  PASS_CF(CFWriteStreamWrite (w, (const UInt8 *)"X", 1) == -1,
          "Writing past the fixed buffer returns -1.");
  PASS_CF(CFWriteStreamGetStatus (w) == kCFStreamStatusError,
          "Status is Error after overflowing the fixed buffer.");
  e = CFWriteStreamGetError (w);
  PASS_CF(e.domain == kCFStreamErrorDomainPOSIX && e.error == ENOMEM,
          "Overflowing the fixed buffer reports POSIX ENOMEM.");
  CFRelease (w);

  return 0;
}
