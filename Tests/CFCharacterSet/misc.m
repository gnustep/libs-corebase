#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef a;
  CFMutableCharacterSetRef mc;

  PASS_CF(CFCharacterSetGetTypeID () != 0,
    "CFCharacterSetGetTypeID is not zero.");

  a = CFCharacterSetGetPredefined (kCFCharacterSetDecimalDigit);
  PASS_CF(CFGetTypeID (a) == CFCharacterSetGetTypeID (),
    "A character set has the character-set type ID.");

  a = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("abc"));
  mc = CFCharacterSetCreateMutableCopy (NULL, a);
  PASS_CF(CFEqual (mc, a),
    "CFCharacterSetCreateMutableCopy equals the original.");
  CFCharacterSetAddCharactersInString (mc, CFSTR("d"));
  PASS_CF(CFCharacterSetIsCharacterMember (mc, 'd')
      && !CFCharacterSetIsCharacterMember (a, 'd'),
    "A mutable copy is independent of the original.");
  CFRelease (mc);
  CFRelease (a);

  return 0;
}
