#define BUFFER_SIZE 1024
Boolean testPassed = false;

#define PASS(expression, message) do \
  { \
  if (expression) \
    { \
      fprintf (stderr, "Passed test:    "); \
      testPassed = true; \
    } \
  else \
    { \
      fprintf (stderr, "Failed test:    "); \
      testPassed = false; \
    } \
  fprintf (stderr, "%s:%d ... %s", __FILE__, __LINE__, message); \
  fprintf (stderr, "\n"); \
  } while (0)

#define PASS_EQ(expression, expect, message) do \
  { \
    PASS(CFEqual((CFTypeRef)expression, (CFTypeRef)expect), message); \
    if (testPassed == false) \
      { \
        CFStringRef str1; \
        CFStringRef str2; \
        char expected[BUFFER_SIZE]; \
        char expr[BUFFER_SIZE]; \
        str1 = CFCopyDescription((CFTypeRef)expect); \
        str2 = CFCopyDescription((CFTypeRef)expression); \
        CFStringGetCString (str1, expected, BUFFER_SIZE-1, \
          CFStringGetSystemEncoding()); \
        CFStringGetCString (str2, expr, BUFFER_SIZE-1, \
          CFStringGetSystemEncoding()); \
        CFRelease ((CFTypeRef)str1); \
        CFRelease ((CFTypeRef)str2); \
        fprintf (stderr, "Expected %s, but got %s\n", \
          expected, \
          expr); \
      } \
  } while(0)

#define PASS_NEQ(expression, expect, message) do \
  { \
    PASS(!CFEqual((CFTypeRef)expression, (CFTypeRef)expect), message); \
    if (testPassed == false) \
      { \
        CFStringRef str1; \
        CFStringRef str2; \
        char expected[BUFFER_SIZE]; \
        char expr[BUFFER_SIZE]; \
        str1 = CFCopyDescription((CFTypeRef)expect); \
        str2 = CFCopyDescription((CFTypeRef)expression); \
        CFStringGetCString (str1, expected, BUFFER_SIZE-1, \
          CFStringGetSystemEncoding()); \
        CFStringGetCString (str2, expr, BUFFER_SIZE-1, \
          CFStringGetSystemEncoding()); \
        CFRelease ((CFTypeRef)str1); \
        CFRelease ((CFTypeRef)str2); \
        fprintf (stderr, "Expected %s, but got %s\n", \
          expected, \
          expr); \
      } \
  } while(0)
