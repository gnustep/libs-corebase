#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  UniChar uStr[] = { 's', 't', 'r', 0 };
  CFStringRef str1 = CFSTR ("str");
  CFStringRef str2 = CFStringCreateWithCharacters (NULL, uStr, 3);

  PASS_CF(CFHash (str1) == CFHash (str2),
    "Identical ASCII and UTF-16 string hashes match");

  CFRelease(str2);
  return 0;
}

