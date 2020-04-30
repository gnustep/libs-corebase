#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableDictionaryRef cfdict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  int keys[] = {48, 33, 39, 44, 47, 56, 59, 54, 58, 71, 63, 61, 77, 64, 68, 60, 65, 70, 73, 84, 82, 75};
  int key, value, i;
  CFNumberRef cfkey, cfvalue;

  for (i = 0; i < sizeof(keys)/sizeof(*keys); i++) {
    key = keys[i];
    cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
    cfvalue = CFNumberCreate (NULL, kCFNumberIntType, &i);
    CFDictionarySetValue(cfdict, cfkey, cfvalue);
    CFNumberRef cfvalue2 = CFDictionaryGetValue(cfdict, cfkey);
    PASS_CF(CFEqual(cfvalue, cfvalue2), "set and get values are equal");
    CFRelease(cfkey);
    CFRelease(cfvalue);
  }
  
  key = 57;
  cfkey = CFNumberCreate (NULL, kCFNumberIntType, &key);
  PASS_CF(CFDictionaryGetValue(cfdict, cfkey) == NULL, "CFDictionaryGetValue returns NULL for nonexistant key");
  CFRelease(cfkey);
  
  for (i = 0; i < sizeof(keys)/sizeof(*keys); i++) {
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
  
  CFRelease(cfdict);
  
  return 0;
}