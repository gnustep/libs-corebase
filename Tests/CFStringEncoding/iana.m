#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  PASS_CFEQ(CFStringConvertEncodingToIANACharSetName (kCFStringEncodingUTF8),
    CFSTR("utf-8"), "UTF-8 has the IANA name utf-8.");
  PASS_CFEQ(CFStringConvertEncodingToIANACharSetName (kCFStringEncodingASCII),
    CFSTR("us-ascii"), "ASCII has the IANA name us-ascii.");
  PASS_CFEQ(
    CFStringConvertEncodingToIANACharSetName (kCFStringEncodingISOLatin1),
    CFSTR("iso-8859-1"), "ISO Latin 1 has the IANA name iso-8859-1.");
  PASS_CFEQ(CFStringConvertEncodingToIANACharSetName (kCFStringEncodingUTF16),
    CFSTR("utf-16"), "UTF-16 has the IANA name utf-16.");

  return 0;
}
