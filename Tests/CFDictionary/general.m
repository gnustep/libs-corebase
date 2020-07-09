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

  for (i = 0; i < count; i++) {
    key = keys[i];
    cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
    cfvalue = CFNumberCreate (NULL, kCFNumberIntType, &i);
    CFDictionarySetValue(cfdict, cfkey, cfvalue);
    CFNumberRef cfvalue2 = CFDictionaryGetValue(cfdict, cfkey);
    PASS_CFEQ(cfvalue, cfvalue2, "set and get values are equal");
    CFRelease(cfkey);
    CFRelease(cfvalue);
  }
  
  key = 57;
  cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
  PASS_CF(CFDictionaryGetValue(cfdict, cfkey) == NULL, "CFDictionaryGetValue returns NULL for nonexistant key");
  CFRelease(cfkey);
  
  for (i = 0; i < count; i++) {
    key = keys[i];
    cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
    cfvalue = CFDictionaryGetValue(cfdict, cfkey);
    PASS_CF(cfvalue != NULL, "CFDictionaryGetValue returns value for existant key %d", key);
    if (cfvalue) {
      CFNumberGetValue(cfvalue, kCFNumberIntType, &value);
      PASS_CF(value == i, "CFDictionaryGetValue returns correct value %d == %d for key %d", value, i, key);
    }
    CFRelease(cfkey);
  }
  
  PASS_CF(CFDictionaryGetCount(cfdict) == count, "CFDictionaryGetCount returns correct value");
  
  CFDictionaryRemoveAllValues(cfdict);
  PASS_CF(CFDictionaryGetCount(cfdict) == 0, "CFDictionaryRemoveAllValues removes all values");
  
  // test large dictionary
  
  count = 5000;
  removeCount = 0;

  CFStringRef keyFormat = CFSTR("key-%d");

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
      PASS_CF(cfvalue != NULL, "CFDictionaryGetValue returns value for key 'key-%d' to remove", keyToRemove);
      CFDictionaryRemoveValue(cfdict, cfkey);
      CFRelease(cfkey);
    }
  }
  
  for (i = 0; i < count; i++) {
    cfkey = CFStringCreateWithFormat(NULL, NULL, keyFormat, i);
    cfvalue = CFDictionaryGetValue(cfdict, cfkey);
    if (i < removeCount) {
      PASS_CF(cfvalue == NULL, "CFDictionaryGetValue returns no value for non-existant key 'key-%d': %p", i, cfvalue);
    } else {
      CFNumberRef cfvalue2 = CFNumberCreate(NULL, kCFNumberIntType, &i);
      PASS_CFEQ(cfvalue, cfvalue2, "CFDictionaryGetValue returns correct value for existant key 'key-%d': %p", i, cfvalue);
    }
    CFRelease(cfkey);
  }
  
  CFRelease(cfdict);
  
  return 0;
}