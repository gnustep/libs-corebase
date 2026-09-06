#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  UniChar chars[4] = { 'A', 0xD83D, 0xDE00, 'B' };      /* A, U+1F600, B */
  CFStringRef s;
  CFRange r;

  PASS_CF(CFStringIsSurrogateHighCharacter (0xD83D),
    "CFStringIsSurrogateHighCharacter accepts a high surrogate.");
  PASS_CF(!CFStringIsSurrogateHighCharacter (0xDE00),
    "CFStringIsSurrogateHighCharacter rejects a low surrogate.");
  PASS_CF(!CFStringIsSurrogateHighCharacter (0x0041),
    "CFStringIsSurrogateHighCharacter rejects a BMP character.");
  PASS_CF(CFStringIsSurrogateLowCharacter (0xDE00),
    "CFStringIsSurrogateLowCharacter accepts a low surrogate.");
  PASS_CF(!CFStringIsSurrogateLowCharacter (0xD83D),
    "CFStringIsSurrogateLowCharacter rejects a high surrogate.");

  s = CFStringCreateWithCharacters (NULL, chars, 4);
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 0);
  PASS_CF(r.location == 0 && r.length == 1,
    "A BMP character occupies a single-unit range.");
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 1);
  PASS_CF(r.location == 1 && r.length == 2,
    "The high surrogate resolves to the whole pair range.");
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 2);
  PASS_CF(r.location == 1 && r.length == 2,
    "The low surrogate resolves to the whole pair range.");
  r = CFStringGetRangeOfComposedCharactersAtIndex (s, 3);
  PASS_CF(r.location == 3 && r.length == 1,
    "A trailing BMP character occupies a single-unit range.");
  CFRelease (s);

  return 0;
}
