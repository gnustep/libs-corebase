#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFStringCreateWithBytes decoding for the byte encodings backed by
   GSUnicodeFromEncoding. */

static CFStringRef
decode (const UInt8 *bytes, CFIndex len, CFStringEncoding enc)
{
  return CFStringCreateWithBytes (NULL, bytes, len, enc, false);
}

int main (void)
{
  UniChar ec = 0x00E9;
  CFStringRef eacute = CFStringCreateWithCharacters (NULL, &ec, 1);

  UInt8 ascii[] = { 0x41 };
  UInt8 utf8[] = { 0xC3, 0xA9 };
  UInt8 u16be[] = { 0x00, 0x41 };
  UInt8 u16le[] = { 0x41, 0x00 };
  UInt8 latin1[] = { 0xE9 };

  PASS_CFEQ (decode (ascii, 1, kCFStringEncodingASCII), CFSTR ("A"),
    "ASCII 0x41 decodes to A.");
  PASS_CFEQ (decode (utf8, 2, kCFStringEncodingUTF8), eacute,
    "UTF-8 C3 A9 decodes to U+00E9.");
  PASS_CFEQ (decode (u16be, 2, kCFStringEncodingUTF16BE), CFSTR ("A"),
    "UTF-16BE 00 41 decodes to A.");
  PASS_CFEQ (decode (u16le, 2, kCFStringEncodingUTF16LE), CFSTR ("A"),
    "UTF-16LE 41 00 decodes to A.");
  PASS_CFEQ (decode (latin1, 1, kCFStringEncodingISOLatin1), eacute,
    "ISO Latin 1 0xE9 decodes to U+00E9.");

  return 0;
}
