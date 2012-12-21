#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"
#include <string.h>

/* These are all the same text. */
const UInt8 utf8_string[] = { 't', 0xC2, 0xA2,
                              'e', 0xE2, 0x82, 0xAC,
                              's', 0xF0, 0xA4, 0xAD, 0xA2,
                              't', '\0' };
const UInt8 utf8_ext_string[] = { 0xEF, 0xBB, 0xBF,
                                 't', 0xC2, 0xA2,
                                 'e', 0xE2, 0x82, 0xAC,
                                 's', 0xF0, 0xA4, 0xAD, 0xA2,
                                 't', '\0' };

const UniChar utf16_string[] = { 't', 0x00A2,
                                 'e', 0x20AC,
                                 's', 0xD852, 0xDF62,
                                 't', '\0' };
const UniChar utf16_ext_string[] = { 0xFEFF,
                                    't', 0x00A2,
                                    'e', 0x20AC,
                                    's', 0xD852, 0xDF62,
                                    't', '\0' };

const UTF32Char utf32_string[] = { 't', 0x00A2,
                                   'e', 0x20AC,
                                   's', 0xD852, 0xDF62,
                                   't', '\0' };
const UTF32Char utf32_ext_string[] = { 0xFEFF,
                                      't', 0xA2,
                                      'e', 0x20AC,
                                      's', 0x24B62,
                                      't', '\0' };

int main (void)
{
  UInt8 buf1[256];
  UInt8 buf2[256];
  CFStringRef str_utf8;
  CFStringRef str_utf16;
  CFIndex num;
  CFIndex used1;
  CFIndex used2;
  
  num = CFStringGetMaximumSizeForEncoding (4, kCFStringEncodingUTF16);
  PASS_CF(num == 8, "Maximum size for 4 UTF-16 characters is 8 bytes.");
  
  num = CFStringGetMaximumSizeForEncoding (4, kCFStringEncodingUTF8);
  PASS_CF(num == 12, "Maximum size for 4 UTF-8 characters is 12 bytes.");
  
  str_utf16 = CFStringConvertEncodingToIANACharSetName (kCFStringEncodingUTF16);
  PASS_CFEQ (str_utf16, CFSTR("UTF-16"), "Correctly converts to IANA Char Set.");
  
  CFRelease(str_utf16);
  
  str_utf8 = CFStringCreateWithBytes (NULL, utf8_string, sizeof(utf8_string),
                                      kCFStringEncodingUTF8, false);
  str_utf16 = CFStringCreateWithCharacters (NULL, utf16_string,
                                            sizeof(utf16_string) / sizeof(UniChar));
  PASS_CFEQ (str_utf8, str_utf16,
             "UTF-8 string is equal to UTF-16 string.");
  
  memset (buf1, 0, 256);
  memset (buf2, 0, 256);
  used1 = 0;
  used2 = 0;
  CFStringGetBytes (str_utf8, CFRangeMake (0, CFStringGetLength(str_utf8)),
                    kCFStringEncodingUTF16, 0, false, buf1, 256, &used1);
  CFStringGetBytes (str_utf16, CFRangeMake (0, CFStringGetLength(str_utf16)),
                    kCFStringEncodingUTF16, 0, true, buf2, 256, &used2);
  PASS_CF (used1 == used2, "UTF-16 convesion used the same from buffer.");
  PASS_CF (memcmp (buf1, utf16_string, used1) == 0,
           "UTF-16 conversion successful.");
  PASS_CF (memcmp (buf2, utf16_ext_string, used2) == 0,
           "External UTF-16 conversion successful.");
  
  CFRelease (str_utf8);
  CFRelease (str_utf16);
  
  return 0;
}
