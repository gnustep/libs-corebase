#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

static void
countApplier (const void *key, const void *value, void *context)
{
  (void)key;
  (void)value;
  *((CFIndex *)context) += 1;
}

int main (void)
{
  CFDictionaryRef dict;
  CFDictionaryRef copy;
  CFDictionaryRef other;
  const void *keys[3];
  const void *values[3];
  const void *outKeys[3];
  const void *outValues[3];
  const void *found;
  CFIndex applied;
  CFIndex i;
  Boolean present;

  keys[0] = CFSTR("one");
  keys[1] = CFSTR("two");
  keys[2] = CFSTR("three");
  values[0] = CFSTR("1");
  values[1] = CFSTR("2");
  values[2] = CFSTR("1");           /* a duplicate value */

  dict = CFDictionaryCreate (NULL, keys, values, 3,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  PASS_CF(dict != NULL, "CFDictionary created.");

  PASS_CF(CFDictionaryGetTypeID () != 0, "CFDictionary type ID is not zero.");
  PASS_CF(CFGetTypeID (dict) == CFDictionaryGetTypeID (),
    "CFDictionary has the CFDictionary type ID.");

  PASS_CF(CFDictionaryGetCount (dict) == 3,
    "CFDictionaryGetCount returns the entry count.");

  found = CFDictionaryGetValue (dict, CFSTR("two"));
  PASS_CFEQ(found, CFSTR("2"), "CFDictionaryGetValue returns the value.");
  PASS_CF(CFDictionaryGetValue (dict, CFSTR("nope")) == NULL,
    "CFDictionaryGetValue returns NULL for an absent key.");

  PASS_CF(CFDictionaryContainsKey (dict, CFSTR("one")),
    "CFDictionaryContainsKey finds a present key.");
  PASS_CF(!CFDictionaryContainsKey (dict, CFSTR("nope")),
    "CFDictionaryContainsKey rejects an absent key.");
  PASS_CF(CFDictionaryContainsValue (dict, CFSTR("2")),
    "CFDictionaryContainsValue finds a present value.");
  PASS_CF(!CFDictionaryContainsValue (dict, CFSTR("9")),
    "CFDictionaryContainsValue rejects an absent value.");

  PASS_CF(CFDictionaryGetCountOfKey (dict, CFSTR("one")) == 1,
    "CFDictionaryGetCountOfKey returns 1 for a present key.");
  PASS_CF(CFDictionaryGetCountOfKey (dict, CFSTR("nope")) == 0,
    "CFDictionaryGetCountOfKey returns 0 for an absent key.");
  PASS_CF(CFDictionaryGetCountOfValue (dict, CFSTR("1")) == 2,
    "CFDictionaryGetCountOfValue counts every key holding the value.");
  PASS_CF(CFDictionaryGetCountOfValue (dict, CFSTR("9")) == 0,
    "CFDictionaryGetCountOfValue returns 0 for an absent value.");

  found = NULL;
  present = CFDictionaryGetValueIfPresent (dict, CFSTR("three"), &found);
  PASS_CF(present && found != NULL,
    "CFDictionaryGetValueIfPresent reports a present key.");
  present = CFDictionaryGetValueIfPresent (dict, CFSTR("nope"), &found);
  PASS_CF(!present,
    "CFDictionaryGetValueIfPresent reports an absent key.");

  CFDictionaryGetKeysAndValues (dict, outKeys, outValues);
  for (i = 0; i < 3; i++)
    PASS_CF(CFDictionaryContainsKey (dict, outKeys[i])
      && CFEqual (CFDictionaryGetValue (dict, outKeys[i]), outValues[i]),
      "CFDictionaryGetKeysAndValues returns matching key/value pairs.");

  applied = 0;
  CFDictionaryApplyFunction (dict, countApplier, &applied);
  PASS_CF(applied == 3, "CFDictionaryApplyFunction visits every entry.");

  copy = CFDictionaryCreateCopy (NULL, dict);
  PASS_CF(copy != NULL, "CFDictionaryCreateCopy returns a dictionary.");
  PASS_CFEQ(copy, dict, "A copy is equal to the original.");
  PASS_CF(CFHash (copy) == CFHash (dict),
    "A copy hashes the same as the original.");
  CFRelease (copy);

  keys[0] = CFSTR("one");
  values[0] = CFSTR("different");
  other = CFDictionaryCreate (NULL, keys, values, 1,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  PASS_CFNEQ(other, dict, "Dictionaries with different entries are not equal.");
  CFRelease (other);

  CFRelease (dict);

  return 0;
}
