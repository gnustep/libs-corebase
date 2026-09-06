#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFRange r;

  PASS_CF(CFStringCompare (CFSTR("ABC"), CFSTR("abc"),
      kCFCompareCaseInsensitive) == kCFCompareEqualTo,
    "kCFCompareCaseInsensitive compares equal regardless of case.");

  r = CFStringFind (CFSTR("Hello, World"), CFSTR("hello"),
    kCFCompareCaseInsensitive);
  PASS_CF(r.location == 0 && r.length == 5,
    "CFStringFind with kCFCompareCaseInsensitive matches regardless of case.");

  return 0;
}
