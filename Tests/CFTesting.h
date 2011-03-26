#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

#include "Testing.h"

#define CFTEST_BUFFER_SIZE 1024

#define PASS_CFEQ(cf1__, cf2__, testFormat__, ...) do \
{ \
  pass((CFEqual((cf1__), (cf2__)) ? YES : NO), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (NO == testPassed) \
    { \
      CFStringRef str1__; \
      CFStringRef str2__; \
      char expect[CFTEST_BUFFER_SIZE] = {0}; \
      char expr[CFTEST_BUFFER_SIZE] = {0}; \
      str1__ = CFCopyDescription((cf1__)); \
      str2__ = CFCopyDescription((cf2__)); \
      CFStringGetCString (str1__, expr, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFStringGetCString (str2__, expect, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFRelease ((CFTypeRef)str1__); \
      CFRelease ((CFTypeRef)str2__); \
      fprintf (stderr, "expected %s, but got %s\n", \
        expect, \
        expr); \
    } \
} while (0)

#define PASS_CFNEQ(cf1__, cf2__, testFormat__, ...) do \
{ \
  pass((CFEqual(cf1__, cf2__) ? NO : YES), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (NO == testPassed) \
    { \
      CFStringRef str1__; \
      CFStringRef str2__; \
      char expect[CFTEST_BUFFER_SIZE]; \
      char expr[CFTEST_BUFFER_SIZE]; \
      str1__ = CFCopyDescription((CFTypeRef)cf1__); \
      str2__ = CFCopyDescription((CFTypeRef)cf2__); \
      CFStringGetCString (str1__, expr, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFStringGetCString (str2__, expect, CFTEST_BUFFER_SIZE, \
        CFStringGetSystemEncoding()); \
      CFRelease ((CFTypeRef)str1__); \
      CFRelease ((CFTypeRef)str2__); \
      fprintf (stderr, "Did not expect %s, got %s\n", \
        expect, \
        expr); \
    } \
} while 0
