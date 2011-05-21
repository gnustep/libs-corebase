#include "CoreFoundation/CFString.h"
#include "Testing.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFIndex num;
  
  num = CFStringGetMaximumSizeForEncoding (4, kCFStringEncodingUTF16);
  PASS (num == 8, "Maximum size for 4 UTF-16 characters is 8 bytes.");
  
  num = CFStringGetMaximumSizeForEncoding (4, kCFStringEncodingUTF8);
  PASS (num == 12, "Maximum size for 4 UTF-8 characters is 12 bytes.");
  
  str = CFStringConvertEncodingToIANACharSetName (kCFStringEncodingUTF16);
  PASS_CFEQ (str, __CFStringMakeConstantString("UTF-16"),
    "Correctly converts to IANA Char Set.");
  
  return 0;
}
