#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFStringCreateExternalRepresentation to UTF-8 does not prepend a byte
   order mark. */

static int
databytes (CFDataRef d, const UInt8 *expect, CFIndex n)
{
  const UInt8 *p;
  CFIndex i;

  if (d == NULL || CFDataGetLength (d) != n)
    return 0;
  p = CFDataGetBytePtr (d);
  for (i = 0; i < n; i++)
    if (p[i] != expect[i])
      return 0;
  return 1;
}

int main (void)
{
  UniChar chars[] = { 0x00E9, 0x0041 };            /* é A */
  CFStringRef s = CFStringCreateWithCharacters (NULL, chars, 2);
  UInt8 expect[] = { 0xC3, 0xA9, 0x41 };
  CFDataRef d = CFStringCreateExternalRepresentation (NULL, s,
    kCFStringEncodingUTF8, 0);

  PASS_CF (databytes (d, expect, 3), "External UTF-8 has no byte order mark.");

  if (d != NULL)
    CFRelease (d);
  CFRelease (s);
  return 0;
}
