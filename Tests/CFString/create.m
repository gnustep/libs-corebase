#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  CFStringRef str2;
  CFStringRef string;
  CFStringRef arrayValues[2];
  CFArrayRef array;
  UniChar ubuffer[] = { 'O', 'K', '!', 0 };
  
  str1 = CFStringCreateWithBytes (NULL, (const UInt8*)"OK", 2, kCFStringEncodingASCII, false);
  
  str2 = CFStringCreateWithBytes (NULL, (const UInt8*)ubuffer, 2 * sizeof(UniChar), kCFStringEncodingUTF16, false);
  
  PASS_CFEQ(str1, str2, "Unicode and C-string are equal.");
  
  arrayValues[0] = str1;
  arrayValues[1] = str2;
  array = CFArrayCreate (NULL, (const void **)arrayValues, 2,
    &kCFTypeArrayCallBacks);
  string = CFStringCreateByCombiningStrings (NULL, array, CFSTR("! "));
  
  PASS_CFEQ(string, CFSTR("OK! OK"),
    "Strings were combined successfully.");
  
  CFRelease(str1);
  CFRelease(str2);
  CFRelease(string);
  CFRelease(array);
  
  return 0;
}
