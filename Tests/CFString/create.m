#include "CoreFoundation/CFString.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  CFStringRef str2;
  UniChar ubuffer[] = { 'O', 'K', '!', 0 };
  
  str1 = CFStringCreateWithBytes (NULL, "OK", 2, kCFStringEncodingASCII, false);
  CFShowStr(str1);
  
  str2 = CFStringCreateWithBytes (NULL, (const UInt8*)ubuffer, 2 * sizeof(UniChar), kCFStringEncodingUTF16LE, false);
  CFShowStr(str2);
  
//  str1 = CFStringCreateWithBytes (NULL, (const UInt8*)ubuffer, 2 * sizeof(UniChar), kCFStringEncodingUTF16LE, false);
//  CFShowStr(str1);
  
  PASS_CFEQ(str1, str2, "Unicode and C-string are equal.");
  
  CFRelease(str1);
  CFRelease(str2);
  
  return 0;
}
