#include "CoreFoundation/CFUUID.h"
#include "../CFTesting.h"

int main (void)
{
  CFUUIDRef u;
  CFStringRef s;

  u = CFUUIDCreateWithBytes (NULL, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde,
    0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0);
  s = CFUUIDCreateString (NULL, u);
  PASS_CFEQ(s, CFSTR("12345678-9ABC-DEF0-1234-56789ABCDEF0"),
    "CFUUIDCreateString formats hex digits in uppercase.");
  CFRelease (s);
  CFRelease (u);

  u = CFUUIDCreateWithBytes (NULL, 0x00, 0x01, 0x02, 0x0a, 0x0b, 0x0c, 0x00,
    0x0f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
  s = CFUUIDCreateString (NULL, u);
  PASS_CFEQ(s, CFSTR("0001020A-0B0C-000F-0102-030405060708"),
    "CFUUIDCreateString zero-pads each byte to two digits.");
  CFRelease (s);
  CFRelease (u);

  return 0;
}
