#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef s;
  CFMutableCharacterSetRef m;

  s = CFCharacterSetCreateWithCharactersInRange (NULL, CFRangeMake ('A', 3));
  PASS_CF(CFCharacterSetIsCharacterMember (s, 'A')
      && CFCharacterSetIsCharacterMember (s, 'C')
      && !CFCharacterSetIsCharacterMember (s, 'D'),
    "CFCharacterSetCreateWithCharactersInRange includes exactly the range.");
  CFRelease (s);

  s = CFCharacterSetCreateWithCharactersInRange (NULL, CFRangeMake ('A', 0));
  PASS_CF(!CFCharacterSetIsCharacterMember (s, 'A'),
    "CFCharacterSetCreateWithCharactersInRange of an empty range is empty.");
  CFRelease (s);

  s = CFCharacterSetCreateWithCharactersInRange (NULL, CFRangeMake (0x1F600, 1));
  PASS_CF(CFCharacterSetIsLongCharacterMember (s, 0x1F600)
      && !CFCharacterSetIsLongCharacterMember (s, 0x1F601),
    "CFCharacterSetIsLongCharacterMember matches exactly a one-character range.");
  CFRelease (s);

  m = CFCharacterSetCreateMutable (NULL);
  CFCharacterSetAddCharactersInRange (m, CFRangeMake ('0', 2));
  PASS_CF(CFCharacterSetIsCharacterMember (m, '0')
      && CFCharacterSetIsCharacterMember (m, '1')
      && !CFCharacterSetIsCharacterMember (m, '2'),
    "CFCharacterSetAddCharactersInRange adds exactly the range.");

  CFCharacterSetAddCharactersInString (m, CFSTR("xy"));
  CFCharacterSetRemoveCharactersInRange (m, CFRangeMake ('x', 1));
  PASS_CF(!CFCharacterSetIsCharacterMember (m, 'x')
      && CFCharacterSetIsCharacterMember (m, 'y'),
    "CFCharacterSetRemoveCharactersInRange removes exactly the range.");
  CFRelease (m);

  return 0;
}
