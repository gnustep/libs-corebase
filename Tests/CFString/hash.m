#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1 = CFSTR ("str");
  CFStringRef str2 = CFStringCreateWithCharacters (NULL,
    (UniChar*) "s\0t\0r\0", 3); // "str" in UTF-16

  PASS_CF(CFHash (str1) == CFHash (str2),
    "Identical ASCII and UTF-16 string hashes match");

  CFRelease(str2);
  return 0;
}

