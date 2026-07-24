#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CF(CFStringConvertEncodingToNSStringEncoding (kCFStringEncodingUTF8)
      == 4,
    "UTF-8 maps to NSUTF8StringEncoding (4).");
  PASS_CF(CFStringConvertNSStringEncodingToEncoding (4)
      == kCFStringEncodingUTF8,
    "NSUTF8StringEncoding maps back to UTF-8.");

  PASS_CF(CFStringConvertIANACharSetNameToEncoding (CFSTR("utf-8"))
      == kCFStringEncodingUTF8,
    "The IANA name utf-8 maps to UTF-8.");

  PASS_CF(CFStringConvertEncodingToWindowsCodepage (kCFStringEncodingUTF8)
      == 65001,
    "UTF-8 maps to Windows codepage 65001.");
  PASS_CF(CFStringConvertWindowsCodepageToEncoding (65001)
      == kCFStringEncodingUTF8,
    "Windows codepage 65001 maps to UTF-8.");

  PASS_CF(CFStringGetMaximumSizeForEncoding (1, kCFStringEncodingASCII) == 1,
    "One ASCII character needs at most one byte.");

  return 0;
}
