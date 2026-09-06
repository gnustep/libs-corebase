#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableStringRef s;

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("Hello"));
  CFStringAppend (s, CFSTR(", World"));
  PASS_CFEQ(s, CFSTR("Hello, World"),
    "CFStringAppend appends to existing content.");
  CFRelease (s);

  s = CFStringCreateMutable (NULL, 0);
  CFStringAppend (s, CFSTR("start"));
  PASS_CFEQ(s, CFSTR("start"), "CFStringAppend appends to an empty string.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR("n="));
  CFStringAppendFormat (s, NULL, CFSTR("%d"), 42);
  PASS_CFEQ(s, CFSTR("n=42"),
    "CFStringAppendFormat appends formatted output.");
  CFRelease (s);

  s = CFStringCreateMutableCopy (NULL, 0, CFSTR(">"));
  CFStringAppendFormat (s, NULL, CFSTR("%@!"), CFSTR("hi"));
  PASS_CFEQ(s, CFSTR(">hi!"),
    "CFStringAppendFormat handles a CFString argument.");
  CFRelease (s);

  return 0;
}
