#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef bmp, supp;

  bmp = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("abc"));
  PASS_CF(CFCharacterSetHasMemberInPlane (bmp, 0),
    "A basic multilingual plane set has a member in plane 0.");
  PASS_CF(!CFCharacterSetHasMemberInPlane (bmp, 1),
    "A basic multilingual plane set has no member in plane 1.");
  CFRelease (bmp);

  supp = CFCharacterSetCreateWithCharactersInRange (NULL,
    CFRangeMake (0x1F600, 1));
  PASS_CF(CFCharacterSetHasMemberInPlane (supp, 1),
    "A supplementary set has a member in plane 1.");
  PASS_CF(!CFCharacterSetHasMemberInPlane (supp, 0),
    "A supplementary set has no member in plane 0.");
  CFRelease (supp);

  return 0;
}
