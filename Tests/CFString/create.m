#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

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

  {
    char b[8];
    Boolean ok;

    memset (b, 'X', sizeof b);
    ok = CFStringGetCString (CFSTR("hello"), b, 5, kCFStringEncodingASCII);
    PASS_CF(ok == false, "GetCString fails when there is no room for the NUL.");
    PASS_CF(b[5] == 'X', "GetCString did not write past the requested size.");

    ok = CFStringGetCString (CFSTR("hello"), b, 6, kCFStringEncodingASCII);
    PASS_CF(ok && strcmp (b, "hello") == 0,
      "GetCString succeeds when the buffer has room for the NUL.");
  }

  return 0;
}
