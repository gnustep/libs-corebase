#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef s = CFSTR("Hello, World");
  CFRange r;
  Boolean found;

  PASS_CF(CFStringHasPrefix (s, CFSTR("Hello")),
    "CFStringHasPrefix matches a leading substring.");
  PASS_CF(CFStringHasSuffix (s, CFSTR("World")),
    "CFStringHasSuffix matches a trailing substring.");
  PASS_CF(!CFStringHasPrefix (s, CFSTR("")),
    "The empty string is not a prefix.");

  r = CFStringFind (s, CFSTR("o"), 0);
  PASS_CF(r.location == 4 && r.length == 1,
    "CFStringFind returns the first match.");
  r = CFStringFind (s, CFSTR("o"), kCFCompareBackwards);
  PASS_CF(r.location == 8 && r.length == 1,
    "CFStringFind with kCFCompareBackwards returns the last match.");
  r = CFStringFind (s, CFSTR("z"), 0);
  PASS_CF(r.location == kCFNotFound,
    "CFStringFind reports kCFNotFound for an absent substring.");

  found = CFStringFindWithOptions (s, CFSTR("o"),
    CFRangeMake (5, CFStringGetLength (s) - 5), 0, &r);
  PASS_CF(found && r.location == 8,
    "CFStringFindWithOptions searches within the given range.");

  PASS_CF(CFStringCompare (CFSTR("abc"), CFSTR("abc"), 0) == kCFCompareEqualTo,
    "Equal strings compare equal.");

  return 0;
}
