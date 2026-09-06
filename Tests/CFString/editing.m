#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableStringRef s;

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("Helo"));
  CFStringInsert (s, 2, CFSTR("l"));
  PASS_CFEQ(s, CFSTR("Hello"), "CFStringInsert inserts at the index.");
  CFStringInsert (s, 0, CFSTR(">"));
  PASS_CFEQ(s, CFSTR(">Hello"), "CFStringInsert at zero prepends.");
  CFStringInsert (s, CFStringGetLength (s), CFSTR("!"));
  PASS_CFEQ(s, CFSTR(">Hello!"), "CFStringInsert at the length appends.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("Hello, World"));
  CFStringDelete (s, CFRangeMake (5, 7));
  PASS_CFEQ(s, CFSTR("Hello"), "CFStringDelete removes the range.");
  CFStringDelete (s, CFRangeMake (0, 0));
  PASS_CFEQ(s, CFSTR("Hello"), "CFStringDelete of an empty range is a no-op.");
  CFStringDelete (s, CFRangeMake (0, CFStringGetLength (s)));
  PASS_CF(CFStringGetLength (s) == 0,
    "CFStringDelete of the whole range empties the string.");
  CFRelease (s);

  return 0;
}
