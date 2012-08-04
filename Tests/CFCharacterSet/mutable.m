#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableCharacterSetRef charset;
  
  charset = CFCharacterSetCreateMutable (NULL);
  CFCharacterSetAddCharactersInRange (charset, CFRangeMake(0x20, 0x7F));
  PASS_CF(CFCharacterSetIsCharacterMember(charset, 'a'),
    "Letter 'a' is part of mutable character set.");
  
  CFCharacterSetRemoveCharactersInString (charset, CFSTR("a"));
  PASS_CF(CFCharacterSetIsCharacterMember(charset, 'a') == false,
    "Letter 'a' was successfully removed from mutable character set.");
  
  return 0;
}