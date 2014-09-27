#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  void *ptr;
  int num;
  
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
  PASS_CF(num == 4, "Getting number of bytes written works.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8s"), NULL);
  PASS_CFEQ(str1, CFSTR("(null)  "),
	    "Left-aligned, NULL string formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8s"), NULL);
  PASS_CFEQ(str1, CFSTR("  (null)"),
	    "Right-aligned, NULL string formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%4.2s"), "string");
  PASS_CFEQ(str1, CFSTR("  st"), "Truncated string formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%% %s"), "%");
  PASS_CFEQ(str1, CFSTR("% %"), "Percent sign formatted correctly.");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%p"), NULL);
  PASS_CFEQ(str1, CFSTR("(nil)"), "Percent sign formatted correctly.");
  CFRelease (str1);

  return 0;
}
