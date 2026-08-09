#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  UniChar combining[2] = { 0x0065, 0x0301 };            /* e + combining acute */
  UniChar mixed[4] = { 'A', 0xD83D, 0xDE00, 'B' };      /* A, U+1F600, B */
  CFStringRef s;
  CFRange r;

  s = CFStringCreateWithCharacters (NULL, combining, 2);
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 0);
  PASS_CF(r.location == 0 && r.length == 2,
    "A base and its combining mark form one composed sequence.");
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 1);
  PASS_CF(r.location == 0 && r.length == 2,
    "The combining mark resolves to the base sequence.");
  CFRelease (s);

  s = CFStringCreateWithCharacters (NULL, mixed, 4);
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 2);
  PASS_CF(r.location == 1 && r.length == 2,
    "A surrogate pair remains one composed sequence.");
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 0);
  PASS_CF(r.location == 0 && r.length == 1,
    "A plain character remains a single-unit sequence.");
  CFRelease (s);

  return 0;
}
