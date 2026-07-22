#include "CoreFoundation/CFCharacterSet.h"
#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef s, rebuilt;
  CFDataRef d;
  const UInt8 *p;

  s = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("A"));
  d = CFCharacterSetCreateBitmapRepresentation (NULL, s);
  PASS_CF(d != NULL && CFDataGetLength (d) == 8192,
    "A basic multilingual plane bitmap is 8192 bytes.");
  p = CFDataGetBytePtr (d);
  PASS_CF((p[8] & (1 << 1)) != 0 && (p[8] & ~(1 << 1)) == 0,
    "Only the bit for 'A' is set in its byte.");
  rebuilt = CFCharacterSetCreateWithBitmapRepresentation (NULL, d);
  PASS_CF(rebuilt != NULL && CFEqual (rebuilt, s),
    "A basic multilingual plane set round-trips through its bitmap.");
  CFRelease (rebuilt);
  CFRelease (d);
  CFRelease (s);

  s = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("aeiou"));
  d = CFCharacterSetCreateBitmapRepresentation (NULL, s);
  rebuilt = CFCharacterSetCreateWithBitmapRepresentation (NULL, d);
  PASS_CF(rebuilt != NULL && CFEqual (rebuilt, s),
    "A multi-character set round-trips through its bitmap.");
  CFRelease (rebuilt);
  CFRelease (d);
  CFRelease (s);

  s = CFCharacterSetCreateWithCharactersInRange (NULL, CFRangeMake (0x1F600, 1));
  d = CFCharacterSetCreateBitmapRepresentation (NULL, s);
  PASS_CF(d != NULL && CFDataGetLength (d) == 8192 + 1 + 8192,
    "A supplementary set adds a plane byte and a plane bitmap.");
  p = CFDataGetBytePtr (d);
  PASS_CF(p[8192] == 1, "The supplementary plane number byte is 1.");
  rebuilt = CFCharacterSetCreateWithBitmapRepresentation (NULL, d);
  PASS_CF(rebuilt != NULL && CFEqual (rebuilt, s),
    "A supplementary set round-trips through its bitmap.");
  CFRelease (rebuilt);
  CFRelease (d);
  CFRelease (s);

  return 0;
}
