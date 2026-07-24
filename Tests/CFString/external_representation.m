#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFStringGetBytes sizing pass (null buffer) and the
   CFStringCreateExternalRepresentation that depends on it. */

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
  CFStringRef hello = CFSTR ("Hello");
  UInt8 hbytes[] = { 'H', 'e', 'l', 'l', 'o' };
  CFIndex used, n;
  CFDataRef d;

  used = 0;
  n = CFStringGetBytes (hello, CFRangeMake (0, 5), kCFStringEncodingUTF8,
    0, false, NULL, 0, &used);
  PASS_CF (n == 5 && used == 5,
    "GetBytes with a null buffer reports the count and byte length.");

  d = CFStringCreateExternalRepresentation (NULL, hello,
    kCFStringEncodingASCII, 0);
  PASS_CF (databytes (d, hbytes, 5),
    "External ASCII of an ASCII string is the ASCII bytes.");
  if (d != NULL)
    CFRelease (d);

  d = CFStringCreateExternalRepresentation (NULL, hello,
    kCFStringEncodingISOLatin1, 0);
  PASS_CF (databytes (d, hbytes, 5),
    "External ISO Latin 1 of an ASCII string is the ASCII bytes.");
  if (d != NULL)
    CFRelease (d);

  return 0;
}
