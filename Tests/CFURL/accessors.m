#include "CoreFoundation/CFURL.h"
#include "../CFTesting.h"
#include <string.h>

int main (void)
{
  CFURLRef u, base, rel;
  CFStringRef s;
  const char *b = "http://host/p";

  PASS_CF(CFURLGetTypeID () != 0, "CFURLGetTypeID is not zero.");

  u = CFURLCreateWithBytes (NULL, (const UInt8 *) b, strlen (b),
    kCFStringEncodingUTF8, NULL);
  PASS_CF(u != NULL, "CFURLCreateWithBytes creates a URL.");
  PASS_CFEQ(CFURLGetString (u), CFSTR("http://host/p"),
    "CFURLCreateWithBytes preserves the URL string.");
  CFRelease (u);

  u = CFURLCreateWithString (NULL, CFSTR("http://user@host:8080/x"), NULL);
  s = CFURLCopyNetLocation (u);
  PASS_CFEQ(s, CFSTR("user@host:8080"),
    "CFURLCopyNetLocation returns the net location.");
  CFRelease (s);
  CFRelease (u);

  u = CFURLCreateWithString (NULL, CFSTR("http://host/x"), NULL);
  PASS_CF(CFURLGetPortNumber (u) == -1,
    "CFURLGetPortNumber returns -1 when there is no port.");
  CFRelease (u);

  base = CFURLCreateWithString (NULL, CFSTR("http://host/a/"), NULL);
  rel = CFURLCreateWithString (NULL, CFSTR("b"), base);
  PASS_CF(CFURLGetBaseURL (rel) == base,
    "CFURLGetBaseURL returns the base URL.");
  CFRelease (rel);
  CFRelease (base);

  return 0;
}
