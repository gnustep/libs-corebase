#include <stdio.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

#define CFTEST_BUFFER_SIZE 1024

#if defined(__OBJC__) && defined(__clang__) && defined(_MSC_VER)
/* Work around Clang bug on Windows MSVC when tests contain no
 * Objective-C constructs: https://bugs.llvm.org/show_bug.cgi?id=49681
 */
id __work_around_clang_bug = @"__unused__";
#endif

static Boolean testPassed = true;
static Boolean testHopeful = false;

static void cfpass(int passed, const char *format, ...)  __attribute__((unused)) __attribute__ ((format(printf, 2, 3)));
static void cfpass(int passed, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  if (passed)
    {
      fprintf(stderr, "Passed test:     ");
      testPassed = true;
    }
#if	!defined(TESTDEV)
  else if (true == testHopeful)
    {
      fprintf(stderr, "Dashed hope:     ");
      testPassed = false;
    }
#endif
  else
    {
      fprintf(stderr, "Failed test:     ");
      testPassed = false;
    }
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
#if	defined(FAILFAST)
  if (false == testPassed && false == testHopeful)
    {
      exit(1);	// Abandon testing now.
    }
#endif
}

#define PASS_CFEQ(cf1__, cf2__, testFormat__, ...) do \
{ \
  cfpass((CFEqual((CFTypeRef)(cf1__), (CFTypeRef)(cf2__)) ? true : false), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (false == testPassed) \
    { \
      CFStringRef str1__; \
      CFStringRef str2__; \
      char expect[CFTEST_BUFFER_SIZE] = {0}; \
      char expr[CFTEST_BUFFER_SIZE] = {0}; \
      str1__ = CFCopyDescription((CFTypeRef)(cf1__)); \
      str2__ = CFCopyDescription((CFTypeRef)(cf2__)); \
      CFStringGetCString (str1__, expr, CFTEST_BUFFER_SIZE, \
        kCFStringEncodingUTF8); \
      CFStringGetCString (str2__, expect, CFTEST_BUFFER_SIZE, \
        kCFStringEncodingUTF8); \
      CFRelease ((CFTypeRef)str1__); \
      CFRelease ((CFTypeRef)str2__); \
      fprintf (stderr, "expected %s, but got %s\n", \
        expect, \
        expr); \
    } \
} while (0)

#define PASS_CFNEQ(cf1__, cf2__, testFormat__, ...) do \
{ \
  cfpass((CFEqual((CFTypeRef)(cf1__), (CFTypeRef)(cf2__)) ? false : true), "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
  if (false == testPassed) \
    { \
      CFStringRef str1__; \
      CFStringRef str2__; \
      char expect[CFTEST_BUFFER_SIZE] = {0}; \
      char expr[CFTEST_BUFFER_SIZE] = {0}; \
      str1__ = CFCopyDescription((CFTypeRef)(cf1__)); \
      str2__ = CFCopyDescription((CFTypeRef)(cf2__)); \
      CFStringGetCString (str1__, expr, CFTEST_BUFFER_SIZE, \
        kCFStringEncodingUTF8); \
      CFStringGetCString (str2__, expect, CFTEST_BUFFER_SIZE, \
        kCFStringEncodingUTF8); \
      CFRelease ((CFTypeRef)str1__); \
      CFRelease ((CFTypeRef)str2__); \
      fprintf (stderr, "Did not expect %s, got %s\n", \
        expect, \
        expr); \
    } \
} while (0)

#define PASS_CF(exp__, testFormat__, ...) do \
{ \
  cfpass(exp__, "%s:%d ... " testFormat__, __FILE__, __LINE__, ## __VA_ARGS__); \
} while (0)

