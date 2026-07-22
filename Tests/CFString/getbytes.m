#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

/* CFStringGetBytes encoding, range, loss and partial-fill semantics. */

int main (void)
{
  UniChar ec = 0x00E9;
  CFStringRef eacute = CFStringCreateWithCharacters (NULL, &ec, 1);
  CFStringRef abcde = CFSTR ("ABCDE");
  UInt8 buf[16];
  CFIndex used;
  CFIndex n;

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingISOLatin1,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 1 && buf[0] == 0xE9,
    "GetBytes converts U+00E9 to ISO Latin 1 0xE9.");

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingUTF8,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 2 && buf[0] == 0xC3 && buf[1] == 0xA9,
    "GetBytes converts U+00E9 to UTF-8 C3 A9.");

  used = 0;
  n = CFStringGetBytes (eacute, CFRangeMake (0, 1), kCFStringEncodingASCII,
    '?', false, buf, sizeof (buf), &used);
  PASS_CF (n == 1 && used == 1 && buf[0] == '?',
    "GetBytes substitutes the loss byte for an unrepresentable character.");

  used = 0;
  n = CFStringGetBytes (abcde, CFRangeMake (0, 5), kCFStringEncodingASCII,
    0, false, buf, 3, &used);
  PASS_CF (n == 3 && used == 3 && memcmp (buf, "ABC", 3) == 0,
    "GetBytes fills as much as the buffer allows and returns the count.");

  used = 0;
  n = CFStringGetBytes (abcde, CFRangeMake (1, 3), kCFStringEncodingASCII,
    0, false, buf, sizeof (buf), &used);
  PASS_CF (n == 3 && used == 3 && memcmp (buf, "BCD", 3) == 0,
    "GetBytes honours the range.");

  return 0;
}
