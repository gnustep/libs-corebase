#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

/* Non-lossy ASCII encoding: escapes for characters outside printable
   ASCII, both directions. */

static int
encodes_to (const UniChar *c, CFIndex n, const char *expect)
{
  CFStringRef s = CFStringCreateWithCharacters (NULL, c, n);
  CFDataRef d = CFStringCreateExternalRepresentation (NULL, s,
    kCFStringEncodingNonLossyASCII, 0);
  CFIndex len = (CFIndex) strlen (expect);
  int result;

  if (d == NULL || CFDataGetLength (d) != len)
    result = 0;
  else
    result = memcmp (CFDataGetBytePtr (d), expect, len) == 0;

  if (d != NULL)
    CFRelease (d);
  CFRelease (s);
  return result;
}

static CFStringRef
decode (const char *bytes)
{
  return CFStringCreateWithBytes (NULL, (const UInt8 *) bytes,
    strlen (bytes), kCFStringEncodingNonLossyASCII, false);
}

int main (void)
{
  UniChar eacute = 0x00E9, euro = 0x20AC, bs = 0x005C, u80 = 0x0080;
  UniChar yuml = 0x00FF;
  UniChar mixed[] = { 0x0041, 0x00E9, 0x0042 };
  CFStringRef eacuteStr = CFStringCreateWithCharacters (NULL, &eacute, 1);
  CFStringRef euroStr = CFStringCreateWithCharacters (NULL, &euro, 1);
  CFStringRef bsStr = CFStringCreateWithCharacters (NULL, &bs, 1);
  CFStringRef mixedStr = CFStringCreateWithCharacters (NULL, mixed, 3);
  CFStringRef d;

  PASS_CF (encodes_to (&eacute, 1, "\\351"),
    "U+00E9 encodes to \\351.");
  PASS_CF (encodes_to (&u80, 1, "\\200"),
    "U+0080 encodes to \\200.");
  PASS_CF (encodes_to (&yuml, 1, "\\377"),
    "U+00FF encodes to \\377.");
  PASS_CF (encodes_to (&euro, 1, "\\u20ac"),
    "U+20AC encodes to \\u20ac.");
  PASS_CF (encodes_to (&bs, 1, "\\\\"),
    "A backslash encodes to a doubled backslash.");
  PASS_CF (encodes_to (mixed, 3, "A\\351B"),
    "A mixed string escapes only the non-ASCII character.");

  d = decode ("\\351");
  PASS_CFEQ (d, eacuteStr, "\\351 decodes to U+00E9.");
  CFRelease (d);
  d = decode ("\\u20ac");
  PASS_CFEQ (d, euroStr, "\\u20ac decodes to U+20AC.");
  CFRelease (d);
  d = decode ("\\u20AC");
  PASS_CFEQ (d, euroStr, "Uppercase hex \\u20AC decodes to U+20AC.");
  CFRelease (d);
  d = decode ("\\\\");
  PASS_CFEQ (d, bsStr, "\\\\ decodes to a backslash.");
  CFRelease (d);
  d = decode ("A\\351B");
  PASS_CFEQ (d, mixedStr, "A mixed escaped string decodes correctly.");
  CFRelease (d);

  CFRelease (eacuteStr);
  CFRelease (euroStr);
  CFRelease (bsStr);
  CFRelease (mixedStr);
  return 0;
}
