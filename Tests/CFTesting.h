#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

#include "Testing.h"

#define CFTEST_BUFFER_SIZE 1024

inline void PASS_CFEQ (CFTypeRef cf1, CFTypeRef cf2, const char *message)
{
  pass((CFEqual(cf1, cf2) ? YES : NO), message);
  if (NO == testPassed)
    {
      CFStringRef str1;
      CFStringRef str2;
      char expect[CFTEST_BUFFER_SIZE] = {0};
      char expr[CFTEST_BUFFER_SIZE] = {0};
      str1 = CFCopyDescription(cf1);
      str2 = CFCopyDescription(cf2);
      CFStringGetCString (str1, expr, CFTEST_BUFFER_SIZE,
        CFStringGetSystemEncoding());
      CFStringGetCString (str2, expect, CFTEST_BUFFER_SIZE,
        CFStringGetSystemEncoding());
      CFRelease ((CFTypeRef)str1);
      CFRelease ((CFTypeRef)str2);
      fprintf (stderr, "cf2ed %s, but got %s\n",
        expect,
        expr);
    }
}

inline void PASS_CFNEQ(CFTypeRef cf1, CFTypeRef cf2, const char *message)
{
  pass((CFEqual(cf1, cf2) ? NO : YES), message);
  if (NO == testPassed)
    {
      CFStringRef str1;
      CFStringRef str2;
      char expect[CFTEST_BUFFER_SIZE];
      char expr[CFTEST_BUFFER_SIZE];
      str1 = CFCopyDescription((CFTypeRef)cf1);
      str2 = CFCopyDescription((CFTypeRef)cf2);
      CFStringGetCString (str1, expr, CFTEST_BUFFER_SIZE,
        CFStringGetSystemEncoding());
      CFStringGetCString (str2, expect, CFTEST_BUFFER_SIZE,
        CFStringGetSystemEncoding());
      CFRelease ((CFTypeRef)str1);
      CFRelease ((CFTypeRef)str2);
      fprintf (stderr, "Did not cf2 %s, got %s\n",
        expect,
        expr);
    }
}
