#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

/* CFStringGetBytes UTF-16 encoding, including byte order and multi-unit
   output. */

int main (void)
{
  UniChar e = 0x00E9;
  UniChar two[] = { 0x00E9, 0x0041 };
  UniChar surr[] = { 0xD83D, 0xDE00 };          /* U+1F600 */
  CFStringRef eacute = CFStringCreateWithCharacters (NULL, &e, 1);
  CFStringRef pair = CFStringCreateWithCharacters (NULL, two, 2);
  CFStringRef emoji = CFStringCreateWithCharacters (NULL, surr, 2);
  UInt8 buf[16];
  CFIndex used, n;

  UInt8 be1[] = { 0x00, 0xE9 };
  UInt8 le1[] = { 0xE9, 0x00 };
  UInt8 be2[] = { 0x00, 0xE9, 0x00, 0x41 };
  UInt8 le2[] = { 0xE9, 0x00, 0x41, 0x00 };
  UInt8 beS[] = { 0xD8, 0x3D, 0xDE, 0x00 };

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingUTF16BE,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 2 && memcmp (buf, be1, 2) == 0,
    "GetBytes UTF-16BE of U+00E9 is 00 E9.");

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingUTF16LE,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 2 && memcmp (buf, le1, 2) == 0,
    "GetBytes UTF-16LE of U+00E9 is E9 00.");

  used = 0;
  n = CFStringGetBytes (pair, CFRangeMake (0, 2), kCFStringEncodingUTF16BE,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 2 && used == 4 && memcmp (buf, be2, 4) == 0,
    "GetBytes UTF-16BE of two characters is 00 E9 00 41.");

  used = 0;
  n = CFStringGetBytes (pair, CFRangeMake (0, 2), kCFStringEncodingUTF16LE,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 2 && used == 4 && memcmp (buf, le2, 4) == 0,
    "GetBytes UTF-16LE of two characters is E9 00 41 00.");

  used = 0;
  n = CFStringGetBytes (emoji, CFRangeMake (0, 2), kCFStringEncodingUTF16BE,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 2 && used == 4 && memcmp (buf, beS, 4) == 0,
    "GetBytes UTF-16BE of a surrogate pair is D8 3D DE 00.");

  CFRelease (eacute);
  CFRelease (pair);
  CFRelease (emoji);
  return 0;
}
