#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableCharacterSetRef m;
  CFCharacterSetRef a, b, copy;

  a = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("abc"));
  b = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("cd"));

  m = CFCharacterSetCreateMutableCopy (NULL, a);
  CFCharacterSetUnion (m, b);
  PASS_CF(CFCharacterSetIsCharacterMember (m, 'a')
      && CFCharacterSetIsCharacterMember (m, 'd'),
    "CFCharacterSetUnion adds the other set's members.");
  CFRelease (m);

  m = CFCharacterSetCreateMutableCopy (NULL, a);
  CFCharacterSetIntersect (m, b);
  PASS_CF(CFCharacterSetIsCharacterMember (m, 'c')
      && !CFCharacterSetIsCharacterMember (m, 'a'),
    "CFCharacterSetIntersect keeps only common members.");
  CFRelease (m);

  m = CFCharacterSetCreateMutableCopy (NULL, a);
  CFCharacterSetInvert (m);
  PASS_CF(!CFCharacterSetIsCharacterMember (m, 'a')
      && CFCharacterSetIsCharacterMember (m, 'z'),
    "CFCharacterSetInvert flips membership.");
  CFRelease (m);

  copy = CFCharacterSetCreateCopy (NULL, a);
  PASS_CF(CFEqual (copy, a), "CFCharacterSetCreateCopy equals the original.");
  CFRelease (copy);

  m = CFCharacterSetCreateMutable (NULL);
  CFCharacterSetAddCharactersInString (m, CFSTR("xy"));
  PASS_CF(CFCharacterSetIsCharacterMember (m, 'x')
      && !CFCharacterSetIsCharacterMember (m, 'z'),
    "CFCharacterSetAddCharactersInString adds the string's members.");
  CFRelease (m);

  CFRelease (a);
  CFRelease (b);

  return 0;
}
