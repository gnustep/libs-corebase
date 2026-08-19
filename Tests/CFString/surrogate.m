#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFStringGetSurrogatePairForLongCharacter for supplementary-plane and BMP characters. */

int main (void)
{
  UniChar pair[2];

  PASS_CF(CFStringGetSurrogatePairForLongCharacter(0x1F600, pair) == true,
    "Supplementary character U+1F600 yields a surrogate pair.");
  PASS_CF(pair[0] == 0xD83D && pair[1] == 0xDE00,
    "Surrogate pair for U+1F600 is D83D DE00.");
  PASS_CF(CFStringGetLongCharacterForSurrogatePair(pair[0], pair[1]) == 0x1F600,
    "Surrogate pair round-trips back to U+1F600.");

  PASS_CF(CFStringGetSurrogatePairForLongCharacter(0x10000, pair) == true,
    "The first supplementary character U+10000 yields a pair.");

  PASS_CF(CFStringGetSurrogatePairForLongCharacter(0x0041, pair) == false,
    "A BMP character (U+0041) yields no surrogate pair.");

  return 0;
}
