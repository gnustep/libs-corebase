#include "CoreFoundation/CFCharacterSet.h"
#include "../CFTesting.h"

static Boolean
mem (CFCharacterSetPredefinedSet which, UniChar c)
{
  return CFCharacterSetIsCharacterMember (CFCharacterSetGetPredefined (which), c);
}

int main (void)
{
  PASS_CF(mem (kCFCharacterSetControl, 0x0001)
      && !mem (kCFCharacterSetControl, 'A'),
    "kCFCharacterSetControl contains control characters only.");

  PASS_CF(mem (kCFCharacterSetWhitespace, ' ')
      && mem (kCFCharacterSetWhitespace, '\t')
      && !mem (kCFCharacterSetWhitespace, '\n')
      && !mem (kCFCharacterSetWhitespace, 'A'),
    "kCFCharacterSetWhitespace contains space and tab but not newline.");

  PASS_CF(mem (kCFCharacterSetWhitespaceAndNewline, ' ')
      && mem (kCFCharacterSetWhitespaceAndNewline, '\n')
      && !mem (kCFCharacterSetWhitespaceAndNewline, 'A'),
    "kCFCharacterSetWhitespaceAndNewline contains space and newline.");

  PASS_CF(mem (kCFCharacterSetDecimalDigit, '7')
      && !mem (kCFCharacterSetDecimalDigit, 'A'),
    "kCFCharacterSetDecimalDigit contains digits only.");

  PASS_CF(mem (kCFCharacterSetLetter, 'A')
      && !mem (kCFCharacterSetLetter, '7'),
    "kCFCharacterSetLetter contains letters only.");

  PASS_CF(mem (kCFCharacterSetLowercaseLetter, 'a')
      && !mem (kCFCharacterSetLowercaseLetter, 'A'),
    "kCFCharacterSetLowercaseLetter contains lowercase letters only.");

  PASS_CF(mem (kCFCharacterSetUppercaseLetter, 'A')
      && !mem (kCFCharacterSetUppercaseLetter, 'a'),
    "kCFCharacterSetUppercaseLetter contains uppercase letters only.");

  PASS_CF(mem (kCFCharacterSetAlphaNumeric, 'a')
      && mem (kCFCharacterSetAlphaNumeric, '7')
      && !mem (kCFCharacterSetAlphaNumeric, ' '),
    "kCFCharacterSetAlphaNumeric contains letters and digits.");

  PASS_CF(mem (kCFCharacterSetPunctuation, '.')
      && !mem (kCFCharacterSetPunctuation, 'A'),
    "kCFCharacterSetPunctuation contains punctuation only.");

  PASS_CF(mem (kCFCharacterSetSymbol, '+')
      && !mem (kCFCharacterSetSymbol, 'A'),
    "kCFCharacterSetSymbol contains symbols only.");

  PASS_CF(mem (kCFCharacterSetNewline, '\n')
      && !mem (kCFCharacterSetNewline, ' '),
    "kCFCharacterSetNewline contains newline but not space.");

  PASS_CF(mem (kCFCharacterSetCapitalizedLetter, 0x01C5)
      && !mem (kCFCharacterSetCapitalizedLetter, 'A'),
    "kCFCharacterSetCapitalizedLetter contains titlecase letters only.");

  PASS_CF(mem (kCFCharacterSetNonBase, 0x0301)
      && !mem (kCFCharacterSetNonBase, 'A'),
    "kCFCharacterSetNonBase contains combining marks.");

  return 0;
}
