#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

#include "Testing.h"

#define CFTEST_BUFFER_SIZE 1024

#define PASS_CFEQ(cf1, cf2, testFormat__, ...) do \
{ \
  pass((CFEqual((cf1), (cf2)) ? YES : NO), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (NO == testPassed) \
    { \
      CFStringRef str1; \
      CFStringRef str2; \
      char expect[CFTEST_BUFFER_SIZE] = {0}; \
      char expr[CFTEST_BUFFER_SIZE] = {0}; \
      str1 = CFCopyDescription((cf1)); \
      str2 = CFCopyDescription((cf2)); \
      CFStringGetCString (str1, expr, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFStringGetCString (str2, expect, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFRelease ((CFTypeRef)str1); \
      CFRelease ((CFTypeRef)str2); \
      fprintf (stderr, "expected %s, but got %s\n", \
        expect, \
        expr); \
    } \
} while (0)

#define PASS_CFNEQ(cf1, cf2, testFormat__, ...) do \
{ \
  pass((CFEqual(cf1, cf2) ? NO : YES), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (NO == testPassed) \
    { \
      CFStringRef str1; \
      CFStringRef str2; \
      char expect[CFTEST_BUFFER_SIZE]; \
      char expr[CFTEST_BUFFER_SIZE]; \
      str1 = CFCopyDescription((CFTypeRef)cf1); \
      str2 = CFCopyDescription((CFTypeRef)cf2); \
      CFStringGetCString (str1, expr, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFStringGetCString (str2, expect, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFRelease ((CFTypeRef)str1); \
      CFRelease ((CFTypeRef)str2); \
      fprintf (stderr, "Did not expect %s, got %s\n", \
        expect, \
        expr); \
    } \
} while 0
