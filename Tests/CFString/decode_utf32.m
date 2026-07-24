#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* UTF-32 decoding in GSUnicodeFromEncoding. */

static CFStringRef
decode (const UInt8 *bytes, CFIndex len, CFStringEncoding enc)
{
  return CFStringCreateWithBytes (NULL, bytes, len, enc, false);
}

int main (void)
{
  UInt8 be_a[] = { 0x00, 0x00, 0x00, 0x41 };
  UInt8 le_a[] = { 0x41, 0x00, 0x00, 0x00 };
  UInt8 be_emoji[] = { 0x00, 0x01, 0xF6, 0x00 };        /* U+1F600 */
  UInt8 be_bom_a[] = { 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x41 };
  UniChar emoji[] = { 0xD83D, 0xDE00 };
  CFStringRef emojiStr = CFStringCreateWithCharacters (NULL, emoji, 2);
  CFStringRef s;

  s = decode (be_a, 4, kCFStringEncodingUTF32BE);
  PASS_CFEQ (s, CFSTR ("A"), "UTF-32BE 00 00 00 41 decodes to A.");
  CFRelease (s);
  s = decode (le_a, 4, kCFStringEncodingUTF32LE);
  PASS_CFEQ (s, CFSTR ("A"), "UTF-32LE 41 00 00 00 decodes to A.");
  CFRelease (s);
  s = decode (be_emoji, 4, kCFStringEncodingUTF32BE);
  PASS_CFEQ (s, emojiStr, "UTF-32BE of U+1F600 decodes to a surrogate pair.");
  CFRelease (s);
  s = decode (be_bom_a, 8, kCFStringEncodingUTF32);
  PASS_CFEQ (s, CFSTR ("A"), "UTF-32 with a big-endian BOM decodes to A.");
  CFRelease (s);

  CFRelease (emojiStr);
  return 0;
}
