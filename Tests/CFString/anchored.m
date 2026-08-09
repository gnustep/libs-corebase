#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef s = CFSTR("Hello, World");
  CFRange r;

  PASS_CF(CFStringHasPrefix (s, CFSTR("Hello")),
    "CFStringHasPrefix accepts a genuine prefix.");
  PASS_CF(!CFStringHasPrefix (s, CFSTR("World")),
    "CFStringHasPrefix rejects a substring that is not at the start.");
  PASS_CF(CFStringHasSuffix (s, CFSTR("World")),
    "CFStringHasSuffix accepts a genuine suffix.");
  PASS_CF(!CFStringHasSuffix (s, CFSTR("Hello")),
    "CFStringHasSuffix rejects a substring that is not at the end.");

  PASS_CF(CFStringFindWithOptions (s, CFSTR("Hello"),
      CFRangeMake (0, CFStringGetLength (s)), kCFCompareAnchored, &r)
      && r.location == 0,
    "kCFCompareAnchored matches at the range start.");
  PASS_CF(!CFStringFindWithOptions (s, CFSTR("World"),
      CFRangeMake (0, CFStringGetLength (s)), kCFCompareAnchored, &r),
    "kCFCompareAnchored does not match away from the range start.");

  return 0;
}
