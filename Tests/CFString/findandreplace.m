#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableStringRef s;
  CFIndex n;

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("foo boo"));
  n = CFStringFindAndReplace (s, CFSTR("o"), CFSTR("0"),
    CFRangeMake (0, CFStringGetLength (s)), 0);
  PASS_CF(n == 4, "CFStringFindAndReplace reports the number of replacements.");
  PASS_CFEQ(s, CFSTR("f00 b00"), "CFStringFindAndReplace replaces every match.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("aaa"));
  n = CFStringFindAndReplace (s, CFSTR("a"), CFSTR("bb"), CFRangeMake (0, 3), 0);
  PASS_CF(n == 3,
    "CFStringFindAndReplace counts replacements that grow the string.");
  PASS_CFEQ(s, CFSTR("bbbbbb"),
    "CFStringFindAndReplace handles longer replacements.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("hello"));
  n = CFStringFindAndReplace (s, CFSTR("z"), CFSTR("Z"), CFRangeMake (0, 5), 0);
  PASS_CF(n == 0 && CFStringGetLength (s) == 5,
    "CFStringFindAndReplace makes no change when the target is absent.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("xxhelloxx"));
  CFStringTrim (s, CFSTR("x"));
  PASS_CFEQ(s, CFSTR("hello"),
    "CFStringTrim removes leading and trailing occurrences.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("abcabc"));
  CFStringTrim (s, CFSTR("abc"));
  PASS_CF(CFStringGetLength (s) == 0, "CFStringTrim can empty the string.");
  CFRelease (s);

  return 0;
}
