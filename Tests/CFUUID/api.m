#include "CoreFoundation/CFUUID.h"
#include "../CFTesting.h"

int main (void)
{
  CFUUIDRef u, u2, c1, c2;

  PASS_CF(CFUUIDGetTypeID () != 0, "CFUUIDGetTypeID is not zero.");

  u = CFUUIDCreateWithBytes (NULL, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde,
    0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0);
  PASS_CF(CFGetTypeID (u) == CFUUIDGetTypeID (),
    "A UUID has the UUID type ID.");

  u2 = CFUUIDCreateFromString (NULL,
    CFSTR("12345678-9ABC-DEF0-1234-56789ABCDEF0"));
  PASS_CF(CFEqual (u, u2),
    "CFUUIDCreateWithBytes and CFUUIDCreateFromString agree on the bytes.");
  CFRelease (u2);
  CFRelease (u);

  c1 = CFUUIDGetConstantUUIDWithBytes (NULL, 0x11, 0x22, 0x33, 0x44, 0x55,
    0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00);
  c2 = CFUUIDGetConstantUUIDWithBytes (NULL, 0x11, 0x22, 0x33, 0x44, 0x55,
    0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00);
  PASS_CF(c1 == c2,
    "CFUUIDGetConstantUUIDWithBytes returns the same constant instance.");

  return 0;
}
