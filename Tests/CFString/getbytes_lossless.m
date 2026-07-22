#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFStringGetBytes refuses to convert unrepresentable characters when no
   loss byte is given. */

int main (void)
{
  UniChar e = 0x00E9;
  UniChar amacron = 0x0100;
  UniChar ae[] = { 0x0041, 0x00E9 };
  CFStringRef eacute = CFStringCreateWithCharacters (NULL, &e, 1);
  CFStringRef wide = CFStringCreateWithCharacters (NULL, &amacron, 1);
  CFStringRef mixed = CFStringCreateWithCharacters (NULL, ae, 2);
  UInt8 buf[16];
  CFIndex used, n;

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingASCII,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 0 && used == 0,
    "GetBytes to ASCII converts nothing for U+00E9 without a loss byte.");

  used = 0;
  n = CFStringGetBytes (wide, CFRangeMake (0, 1), kCFStringEncodingISOLatin1,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 0 && used == 0,
    "GetBytes to ISO Latin 1 converts nothing for U+0100 without a loss byte.");

  used = 0;
  n = CFStringGetBytes (mixed, CFRangeMake (0, 2), kCFStringEncodingASCII,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 1 && buf[0] == 'A',
    "GetBytes converts the representable prefix and stops at the first loss.");

  return 0;
}
