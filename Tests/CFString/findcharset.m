#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef digits;
  CFRange r;

  digits = CFCharacterSetGetPredefined (kCFCharacterSetDecimalDigit);

  PASS_CF(CFStringFindCharacterFromSet (CFSTR("abc123"), digits,
      CFRangeMake (0, 6), 0, &r) && r.location == 3 && r.length == 1,
    "CFStringFindCharacterFromSet finds the first set member.");
  PASS_CF(CFStringFindCharacterFromSet (CFSTR("abc123"), digits,
      CFRangeMake (0, 6), kCFCompareBackwards, &r) && r.location == 5,
    "CFStringFindCharacterFromSet honours kCFCompareBackwards.");
  PASS_CF(!CFStringFindCharacterFromSet (CFSTR("abcdef"), digits,
      CFRangeMake (0, 6), 0, &r),
    "CFStringFindCharacterFromSet reports no match when the set is absent.");

  return 0;
}
