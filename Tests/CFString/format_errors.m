#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef o;
  
  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%z"), "test");
#ifdef __APPLE__
  PASS_CFEQ (o, CFSTR(""), "Format with invalid spec (%%z) matches Apple.");
  CFRelease (o);
#else
  PASS_CF(o == NULL, "Format with invalid spec (%%z) returns NULL.");
#endif

  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%s %01.2z"), "test", NULL);
#ifdef __APPLE__
  PASS_CFEQ (o, CFSTR("test "),
             "Complex format with invalid spec (%%01.2z) matches Apple.");
  CFRelease (o);
#else
  PASS_CF(o == NULL, "Complex format with invalid spec (%%01.2z) returns NULL.");
#endif

  o = CFStringCreateWithFormat (NULL, NULL, CFSTR("%s %1$s"), "test1", "test2");
#ifdef __APPLE__
  PASS_CFEQ (o, CFSTR("test1 test1"),
             "Mixed positional formatting matches Apple.");
  CFRelease (o);
#else
  PASS_CF(o == NULL, "Specifying only 1 positional parameter returns NULL.");
#endif

  return 0;
}
