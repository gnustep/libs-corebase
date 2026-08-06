#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  void *ptr;
  int num = -1;
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%c%2c%c"), 'r', 'u', 'n');
  PASS_CFEQ(str1, CFSTR("r un"), "Characters are formatted correctly");
  CFRelease(str1);
  
  ptr = (void*)12;
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%p %s"), ptr,
				   "A longer than usual string.");
  PASS_CFEQ(str1, CFSTR ("0xc A longer than usual string."),
	    "Strings are formatted correctly");
  CFRelease(str1);
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%@"),
				   CFSTR("Object test"));
  PASS_CFEQ(str1, CFSTR("Object test"),
	    "CFString objects are formatted correctly");
  CFRelease(str1);
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("1234%n"), &num);
#ifdef __APPLE__
  PASS_CF(num == -1, "Getting number of bytes written matches Apple.");
#else
  PASS_CF(num == 4, "Getting number of bytes written works.");
#endif
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8s"), NULL);
#ifdef __APPLE__
  PASS_CFEQ(str1, CFSTR("(null)"),
	    "Left-aligned, NULL string formatted correctly.");
#else
  PASS_CFEQ(str1, CFSTR("(null)  "),
	    "Left-aligned, NULL string formatted correctly.");
#endif
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8s"), NULL);
#ifdef __APPLE__
  PASS_CFEQ(str1, CFSTR("(null)"),
	    "Right-aligned, NULL string formatted correctly.");
#else
  PASS_CFEQ(str1, CFSTR("  (null)"),
	    "Right-aligned, NULL string formatted correctly.");
#endif
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%4.2s"), "string");
  PASS_CFEQ(str1, CFSTR("  st"), "Truncated string formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%% %s"), "%");
  PASS_CFEQ(str1, CFSTR("% %"), "Percent sign formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%p"), NULL);
#ifdef __APPLE__
  PASS_CFEQ(str1, CFSTR("0x0"), "Percent sign formatted correctly.");
#else
  PASS_CFEQ(str1, CFSTR("(nil)"), "Percent sign formatted correctly.");
#endif
  CFRelease (str1);

  return 0;
}
