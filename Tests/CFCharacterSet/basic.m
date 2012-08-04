#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFCharacterSetRef charset;
  CFCharacterSetRef charset2;
  
  charset = CFCharacterSetCreateWithCharactersInRange (NULL,
    CFRangeMake(0x20, 0x7F));
  PASS_CF(CFCharacterSetIsCharacterMember(charset, 'a'),
    "Letter 'a' is part of character set.");
  
  charset2 = CFCharacterSetCreateWithCharactersInString (NULL, CFSTR("abcABC"));
  PASS_CF(CFCharacterSetIsSupersetOfSet(charset, charset2),
    "Character set with all ASCII characters is a superset of a charcter set"
    "including 'abcABC'.");
  PASS_CF(CFCharacterSetIsSupersetOfSet(charset2, charset) == false,
    "Characters set with 'abcABC' is not a superset of a character set"
    "including all ASCII characters");
  
  CFRelease (charset2);
  
  charset2 = CFCharacterSetCreateInvertedSet (NULL, charset);
  PASS_CF(CFCharacterSetIsCharacterMember(charset2, 'a') == false,
    "Inverted character set does not include letter 'a'.");
  PASS_CF(CFCharacterSetIsCharacterMember(charset2, 0x00DD),
    "Inverted character set includes character 0x00DD.");
  
  CFRelease (charset);
  
  charset = CFCharacterSetGetPredefined (kCFCharacterSetWhitespace);
  PASS_CF(CFCharacterSetIsCharacterMember(charset, ' ') == true,
    "Whitespace character set includes ' '.");
  PASS_CF(CFCharacterSetIsCharacterMember(charset, '5') == false,
    "Whitespace character set does not includes '5'.");
  
  CFRelease (charset);
  
  return 0;
}