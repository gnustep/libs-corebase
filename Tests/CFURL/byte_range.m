#include "CoreFoundation/CFURL.h"
#include "../CFTesting.h"

static CFURLRef
mk (const char *s)
{
  CFStringRef str = CFStringCreateWithCString (NULL, s, kCFStringEncodingUTF8);
  CFURLRef u = CFURLCreateWithString (NULL, str, NULL);
  CFRelease (str);
  return u;
}

int main (void)
{
  CFURLRef u = mk ("http://host/dir/file.txt");
  CFRange r;

  r = CFURLGetByteRangeForComponent (u, kCFURLComponentScheme, NULL);
  PASS_CF(r.location == 0 && r.length == 4,
    "CFURLGetByteRangeForComponent returns the scheme range.");

  r = CFURLGetByteRangeForComponent (u, kCFURLComponentHost, NULL);
  PASS_CF(r.location == 7 && r.length == 4,
    "CFURLGetByteRangeForComponent returns the host range.");

  r = CFURLGetByteRangeForComponent (u, kCFURLComponentPath, NULL);
  PASS_CF(r.location == 11 && r.length == 13,
    "CFURLGetByteRangeForComponent returns the path range.");

  r = CFURLGetByteRangeForComponent (u, kCFURLComponentPort, NULL);
  PASS_CF(r.location == kCFNotFound,
    "CFURLGetByteRangeForComponent returns kCFNotFound for an absent component.");

  CFRelease (u);

  return 0;
}
