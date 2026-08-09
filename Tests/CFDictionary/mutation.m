#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableDictionaryRef dict;
  CFMutableDictionaryRef copy;

  dict = CFDictionaryCreateMutable (NULL, 0,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  PASS_CF(dict != NULL, "CFMutableDictionary created.");
  PASS_CF(CFDictionaryGetCount (dict) == 0, "A new mutable dictionary is empty.");

  CFDictionaryAddValue (dict, CFSTR("k"), CFSTR("a"));
  PASS_CF(CFDictionaryGetCount (dict) == 1
    && CFEqual (CFDictionaryGetValue (dict, CFSTR("k")), CFSTR("a")),
    "CFDictionaryAddValue inserts an absent key.");

  CFDictionaryAddValue (dict, CFSTR("k"), CFSTR("b"));
  PASS_CFEQ(CFDictionaryGetValue (dict, CFSTR("k")), CFSTR("a"),
    "CFDictionaryAddValue does not overwrite a present key.");

  CFDictionarySetValue (dict, CFSTR("k"), CFSTR("c"));
  PASS_CFEQ(CFDictionaryGetValue (dict, CFSTR("k")), CFSTR("c"),
    "CFDictionarySetValue overwrites a present key.");
  PASS_CF(CFDictionaryGetCount (dict) == 1,
    "Overwriting does not change the count.");

  CFDictionarySetValue (dict, CFSTR("k2"), CFSTR("d"));
  PASS_CF(CFDictionaryGetCount (dict) == 2,
    "CFDictionarySetValue adds an absent key.");

  CFDictionaryReplaceValue (dict, CFSTR("k2"), CFSTR("e"));
  PASS_CFEQ(CFDictionaryGetValue (dict, CFSTR("k2")), CFSTR("e"),
    "CFDictionaryReplaceValue overwrites a present key.");
  CFDictionaryReplaceValue (dict, CFSTR("absent"), CFSTR("x"));
  PASS_CF(!CFDictionaryContainsKey (dict, CFSTR("absent"))
    && CFDictionaryGetCount (dict) == 2,
    "CFDictionaryReplaceValue ignores an absent key.");

  copy = CFDictionaryCreateMutableCopy (NULL, 0, dict);
  PASS_CF(copy != NULL, "CFDictionaryCreateMutableCopy returns a dictionary.");
  PASS_CFEQ(copy, dict, "A mutable copy is equal to the original.");
  CFRelease (copy);

  CFDictionaryRemoveValue (dict, CFSTR("k"));
  PASS_CF(CFDictionaryGetCount (dict) == 1
    && !CFDictionaryContainsKey (dict, CFSTR("k")),
    "CFDictionaryRemoveValue removes an entry.");

  CFDictionaryRemoveAllValues (dict);
  PASS_CF(CFDictionaryGetCount (dict) == 0,
    "CFDictionaryRemoveAllValues empties the dictionary.");

  CFRelease (dict);

  return 0;
}
