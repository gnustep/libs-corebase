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
  CFURLRef u;
  CFStringRef s;

  u = mk ("http://host:8080/x");
  PASS_CF(CFURLGetPortNumber (u) == 8080,
    "CFURLGetPortNumber returns the port.");
  s = CFURLCopyHostName (u);
  PASS_CFEQ(s, CFSTR("host"),
    "CFURLCopyHostName returns the host when a port is present.");
  CFRelease (s);
  CFRelease (u);

  u = mk ("http://user@host:80/x");
  PASS_CF(CFURLGetPortNumber (u) == 80,
    "CFURLGetPortNumber returns the port after user info.");
  CFRelease (u);

  u = mk ("http://host/x");
  PASS_CF(CFURLGetPortNumber (u) == -1,
    "CFURLGetPortNumber returns -1 when there is no port.");
  CFRelease (u);

  return 0;
}
