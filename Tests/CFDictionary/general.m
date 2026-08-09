#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableDictionaryRef cfdict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  int keys[] = {48, 33, 39, 44, 47, 56, 59, 54, 58, 71, 63, 61, 77, 64, 68, 60, 65, 70, 73, 84, 82, 75};
  int key, value, i;
  int removeCount, count = sizeof(keys)/sizeof(*keys);
  CFTypeRef cfkey, cfvalue;
  Boolean ok;
  int firstBad;

  ok = true;
  firstBad = -1;
  for (i = 0; i < count; i++) {
    key = keys[i];
    cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
    cfvalue = CFNumberCreate (NULL, kCFNumberIntType, &i);
    CFDictionarySetValue(cfdict, cfkey, cfvalue);
    CFNumberRef cfvalue2 = CFDictionaryGetValue(cfdict, cfkey);
    if (cfvalue2 == NULL || !CFEqual(cfvalue, cfvalue2)) {
      if (ok) firstBad = key;
      ok = false;
    }
    CFRelease(cfkey);
    CFRelease(cfvalue);
  }
  if (!ok) fprintf(stderr, "  first mismatch at key %d\n", firstBad);
  PASS_CF(ok, "set and get values are equal for all %d keys", count);

  key = 57;
  cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
  PASS_CF(CFDictionaryGetValue(cfdict, cfkey) == NULL, "CFDictionaryGetValue returns NULL for nonexistant key");
  CFRelease(cfkey);

  {
    Boolean allPresent = true;
    Boolean allCorrect = true;
    int firstMissing = -1;
    int firstWrong = -1;
    for (i = 0; i < count; i++) {
      key = keys[i];
      cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
      cfvalue = CFDictionaryGetValue(cfdict, cfkey);
      if (cfvalue == NULL) {
        if (allPresent) firstMissing = key;
        allPresent = false;
      } else {
        CFNumberGetValue(cfvalue, kCFNumberIntType, &value);
        if (value != i) {
          if (allCorrect) firstWrong = key;
          allCorrect = false;
        }
      }
      CFRelease(cfkey);
    }
    if (!allPresent) fprintf(stderr, "  first missing key %d\n", firstMissing);
    PASS_CF(allPresent, "CFDictionaryGetValue returns a value for all %d existant keys", count);
    if (!allCorrect) fprintf(stderr, "  first incorrect value at key %d\n", firstWrong);
    PASS_CF(allCorrect, "CFDictionaryGetValue returns the correct value for all existant keys");
  }

  PASS_CF(CFDictionaryGetCount(cfdict) == count, "CFDictionaryGetCount returns correct value");

  CFDictionaryRemoveAllValues(cfdict);
  PASS_CF(CFDictionaryGetCount(cfdict) == 0, "CFDictionaryRemoveAllValues removes all values");

  // test large dictionary

  count = 5000;
  removeCount = 0;

  CFStringRef keyFormat = CFSTR("key-%d");

  ok = true;
  firstBad = -1;
  for (i = 0; i < count; i++) {

    cfkey = CFStringCreateWithFormat(NULL, NULL, keyFormat, i);
    cfvalue = CFNumberCreate(NULL, kCFNumberIntType, &i);
    CFDictionarySetValue(cfdict, cfkey, cfvalue);
    CFRelease(cfkey);
    CFRelease(cfvalue);

    // start removing keys while we are adding new ones after filling 1/10
    if (i > count/10) {
      int keyToRemove = removeCount++;
      cfkey = CFStringCreateWithFormat(NULL, NULL, keyFormat, keyToRemove);
      cfvalue = CFDictionaryGetValue(cfdict, cfkey);
      if (cfvalue == NULL) {
        if (ok) firstBad = keyToRemove;
        ok = false;
      }
      CFDictionaryRemoveValue(cfdict, cfkey);
      CFRelease(cfkey);
    }
  }
  if (!ok) fprintf(stderr, "  first missing key before removal: key-%d\n", firstBad);
  PASS_CF(ok, "CFDictionaryGetValue returns a value for each of the %d keys before removal", removeCount);

  {
    Boolean removedAbsent = true;
    Boolean remainingCorrect = true;
    int firstPresent = -1;
    int firstWrong = -1;
    for (i = 0; i < count; i++) {
      cfkey = CFStringCreateWithFormat(NULL, NULL, keyFormat, i);
      cfvalue = CFDictionaryGetValue(cfdict, cfkey);
      if (i < removeCount) {
        if (cfvalue != NULL) {
          if (removedAbsent) firstPresent = i;
          removedAbsent = false;
        }
      } else {
        CFNumberRef cfvalue2 = CFNumberCreate(NULL, kCFNumberIntType, &i);
        if (cfvalue == NULL || !CFEqual(cfvalue, cfvalue2)) {
          if (remainingCorrect) firstWrong = i;
          remainingCorrect = false;
        }
        CFRelease(cfvalue2);
      }
      CFRelease(cfkey);
    }
    if (!removedAbsent) fprintf(stderr, "  removed key still present: key-%d\n", firstPresent);
    PASS_CF(removedAbsent, "CFDictionaryGetValue returns no value for the %d removed keys", removeCount);
    if (!remainingCorrect) fprintf(stderr, "  first incorrect value at key-%d\n", firstWrong);
    PASS_CF(remainingCorrect, "CFDictionaryGetValue returns the correct value for all remaining keys");
  }

  CFRelease(cfdict);

  return 0;
}
