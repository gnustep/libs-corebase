#include "CoreFoundation/CFString.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableStringRef str1;
  CFMutableStringRef str2;
  CFStringRef constant1 = __CFStringMakeConstantString("testing!");
  CFStringRef constant2 = __CFStringMakeConstantString("   test  ");
  
  str1 = CFStringCreateMutable (NULL, 0);
  CFStringReplaceAll (str1, constant1);
  PASS_CFEQ(str1, constant1, "String is replaced correctly.");
  
  str2 = CFStringCreateMutableCopy (NULL, CFStringGetLength(str1), str1);
  CFStringUppercase (str2, NULL);
  PASS_CFEQ(str2, __CFStringMakeConstantString("TESTING!"), "Uppercase mapping works.");
  
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateMutable (NULL, 0);
  CFStringReplaceAll (str1, constant2);
  CFStringTrimWhitespace (str1);
  PASS_CFEQ(str1, __CFStringMakeConstantString("test"),
    "String is trimmed correctly.");
  
  return 0;
}
