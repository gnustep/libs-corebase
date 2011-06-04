#include "CoreFoundation/CFString.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableStringRef str1;
  CFMutableStringRef str2;
  CFStringRef constant1 = __CFStringMakeConstantString("Test string.");
  CFStringRef constant2 = __CFStringMakeConstantString("   test  ");
  
  str1 = CFStringCreateMutable (NULL, 0);
  CFStringReplaceAll (str1, constant1);
  PASS_CFEQ(str1, constant1, "String is replaced correctly.");
  
  str2 = CFStringCreateMutableCopy (NULL, CFStringGetLength(str1), str1);
  
  CFStringUppercase (str2, NULL);
  PASS_CFEQ(str2, __CFStringMakeConstantString("TEST STRING."), "Uppercase mapping works.");
  
  CFStringCapitalize (str2, NULL);
  PASS_CFEQ(str2, __CFStringMakeConstantString("Test String."), "Capitalize mapping works.");
  
  CFStringLowercase (str2, NULL);
  PASS_CFEQ(str2, __CFStringMakeConstantString("test string."), "Lowercase mapping works");
  
  CFStringReplace (str2, CFRangeMake(0, CFStringGetLength(str1)), constant1);
  PASS_CFEQ(str2, constant1, "String is replaced whole.");
  
  CFRelease(str1);
  CFRelease(str2);
  
  str1 = CFStringCreateMutable (NULL, 0);
  CFStringReplaceAll (str1, constant2);
  CFStringTrimWhitespace (str1);
  PASS_CFEQ(str1, __CFStringMakeConstantString("test"),
    "String is trimmed correctly.");
  
  CFStringReplace (str1, CFRangeMake(1, 2), constant1);
  PASS_CFEQ(str1, __CFStringMakeConstantString("tTest string.t"),
    "String replacement works");
  
  CFRelease(str1);
  
  return 0;
}
