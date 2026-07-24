#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  const void *keys[2];
  const void *values[2];
  CFDictionaryRef dict;

  keys[0] = CFSTR ("k");
  keys[1] = CFSTR ("k");
  values[0] = CFSTR ("v1");
  values[1] = CFSTR ("v2");

  dict = CFDictionaryCreate (NULL, keys, values, 2,
                             &kCFCopyStringDictionaryKeyCallBacks,
                             &kCFTypeDictionaryValueCallBacks);
  PASS_CF(CFDictionaryGetCount (dict) == 1,
    "A key repeated in the input array is stored once.");
  PASS_CF(CFDictionaryContainsKey (dict, CFSTR ("k")),
    "The key is present.");

  CFRelease (dict);

  return 0;
}
