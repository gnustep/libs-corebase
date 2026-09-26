#include <CoreFoundation/CFStream.h>
#include <CoreFoundation/CFData.h>

#include "../CFTesting.h"
#include <string.h>

int main (void)
{
  const char *data = "ABCDEFGHIJ";
  UInt8 buf[64];
  CFIndex n;
  CFReadStreamRef r;
  CFWriteStreamRef wa;
  CFWriteStreamRef w;
  CFDataRef d;
  UInt8 fb[8];

  PASS_CF(CFReadStreamGetTypeID () != CFWriteStreamGetTypeID (),
          "Read and write streams have distinct type IDs.");

  r = CFReadStreamCreateWithBytesNoCopy (NULL, (const UInt8 *)data, 10,
                                         kCFAllocatorNull);
  PASS_CF(r != NULL, "Created a bytes read stream.");
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusNotOpen,
          "Read status is NotOpen before opening.");
  PASS_CF(CFReadStreamOpen (r), "Opening the read stream succeeds.");
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusOpen,
          "Read status is Open after opening.");
  n = CFReadStreamRead (r, buf, 4);
  PASS_CF(n == 4 && memcmp (buf, "ABCD", 4) == 0, "Read the first four bytes.");
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusOpen,
          "Read status is still Open with bytes remaining.");
  n = CFReadStreamRead (r, buf, 64);
  PASS_CF(n == 6 && memcmp (buf, "EFGHIJ", 6) == 0, "Read the remaining bytes.");
  n = CFReadStreamRead (r, buf, 64);
  PASS_CF(n == 0, "Reading past the end returns zero.");
  PASS_CF(CFReadStreamCopyProperty (r, CFSTR ("bogus")) == NULL,
          "An unknown read property returns NULL.");
  CFReadStreamClose (r);
  PASS_CF(CFReadStreamGetStatus (r) == kCFStreamStatusClosed,
          "Read status is Closed after closing.");
  CFRelease (r);

  wa = CFWriteStreamCreateWithAllocatedBuffers (NULL, NULL);
  PASS_CF(wa != NULL, "Created an allocated-buffer write stream.");
  PASS_CF(CFWriteStreamGetStatus (wa) == kCFStreamStatusNotOpen,
          "Write status is NotOpen before opening.");
  PASS_CF(CFWriteStreamOpen (wa), "Opening the write stream succeeds.");
  PASS_CF(CFWriteStreamGetStatus (wa) == kCFStreamStatusOpen,
          "Write status is Open after opening.");
  PASS_CF(CFWriteStreamCanAcceptBytes (wa) == true,
          "A growable stream can accept bytes.");
  PASS_CF(CFWriteStreamWrite (wa, (const UInt8 *)"hello", 5) == 5,
          "Wrote five bytes.");
  PASS_CF(CFWriteStreamWrite (wa, (const UInt8 *)" world", 6) == 6,
          "Wrote six more bytes.");
  d = CFWriteStreamCopyProperty (wa, kCFStreamPropertyDataWritten);
  PASS_CF(d != NULL && CFDataGetLength (d) == 11
          && memcmp (CFDataGetBytePtr (d), "hello world", 11) == 0,
          "The written-data property holds all written bytes.");
  CFRelease (d);
  CFWriteStreamClose (wa);
  CFRelease (wa);

  w = CFWriteStreamCreateWithBuffer (NULL, fb, 8);
  PASS_CF(CFWriteStreamOpen (w), "Opening the fixed-buffer stream succeeds.");
  PASS_CF(CFWriteStreamWrite (w, (const UInt8 *)"ABCDE", 5) == 5,
          "Wrote five bytes into the fixed buffer.");
  PASS_CF(memcmp (fb, "ABCDE", 5) == 0, "The fixed buffer holds the bytes.");
  PASS_CF(CFWriteStreamCanAcceptBytes (w) == true,
          "A fixed buffer with room can accept bytes.");
  CFRelease (w);

  return 0;
}
