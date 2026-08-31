#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CF(CFStringCompare (CFSTR("abc"), CFSTR("abd"), 0)
      == kCFCompareLessThan,
    "CFStringCompare orders an earlier string before a later one.");
  PASS_CF(CFStringCompare (CFSTR("abd"), CFSTR("abc"), 0)
      == kCFCompareGreaterThan,
    "CFStringCompare orders a later string after an earlier one.");
  PASS_CF(CFStringCompare (CFSTR("abc"), CFSTR("abc"), 0)
      == kCFCompareEqualTo,
    "Equal strings compare equal.");
  PASS_CF(CFStringCompare (CFSTR("file10"), CFSTR("file9"),
      kCFCompareNumerically) == kCFCompareGreaterThan,
    "kCFCompareNumerically orders embedded numbers by value.");

  return 0;
}
