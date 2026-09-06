#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef s = CFSTR("Hello, World");
  CFStringRef sub;
  CFStringRef copy;
  UniChar buf[8];
  double d;

  PASS_CF(CFStringGetCharacterAtIndex (s, 0) == 'H',
    "CFStringGetCharacterAtIndex returns the first character.");
  PASS_CF(CFStringGetCharacterAtIndex (s, 7) == 'W',
    "CFStringGetCharacterAtIndex returns a character at an index.");

  CFStringGetCharacters (s, CFRangeMake (7, 5), buf);
  PASS_CF(buf[0] == 'W' && buf[4] == 'd',
    "CFStringGetCharacters copies the requested range.");

  sub = CFStringCreateWithSubstring (NULL, s, CFRangeMake (7, 5));
  PASS_CFEQ(sub, CFSTR("World"),
    "CFStringCreateWithSubstring extracts the range.");
  CFRelease (sub);

  copy = CFStringCreateCopy (NULL, s);
  PASS_CFEQ(copy, s, "CFStringCreateCopy equals the original.");
  CFRelease (copy);

  PASS_CF(CFStringGetIntValue (CFSTR("42")) == 42,
    "CFStringGetIntValue parses a positive integer.");
  PASS_CF(CFStringGetIntValue (CFSTR("-7")) == -7,
    "CFStringGetIntValue parses a negative integer.");
  PASS_CF(CFStringGetIntValue (CFSTR("15abc")) == 15,
    "CFStringGetIntValue stops at the first non-digit.");
  PASS_CF(CFStringGetIntValue (CFSTR("abc")) == 0,
    "CFStringGetIntValue returns zero for a non-number.");

  d = CFStringGetDoubleValue (CFSTR("3.14"));
  PASS_CF(d > 3.139 && d < 3.141, "CFStringGetDoubleValue parses a decimal.");
  d = CFStringGetDoubleValue (CFSTR("-2.5"));
  PASS_CF(d > -2.501 && d < -2.499,
    "CFStringGetDoubleValue parses a negative decimal.");
  d = CFStringGetDoubleValue (CFSTR("abc"));
  PASS_CF(d == 0.0, "CFStringGetDoubleValue returns zero for a non-number.");

  return 0;
}
