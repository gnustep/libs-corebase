#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  double d;

  PASS_CF(CFStringGetIntValue (CFSTR("  42")) == 42,
    "CFStringGetIntValue skips leading whitespace.");
  PASS_CF(CFStringGetIntValue (CFSTR("42")) == 42,
    "CFStringGetIntValue parses without leading whitespace.");
  PASS_CF(CFStringGetIntValue (CFSTR("  -7")) == -7,
    "CFStringGetIntValue skips whitespace before a sign.");

  d = CFStringGetDoubleValue (CFSTR("  3.14"));
  PASS_CF(d > 3.139 && d < 3.141,
    "CFStringGetDoubleValue skips leading whitespace.");
  d = CFStringGetDoubleValue (CFSTR("3.14"));
  PASS_CF(d > 3.139 && d < 3.141,
    "CFStringGetDoubleValue parses without leading whitespace.");

  return 0;
}
