#include "CoreFoundation/CFPropertyList.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

/* CFPropertyListCreateDeepCopy of mutable containers. */

int main (void)
{
  CFMutableArrayRef arr;
  CFArrayRef acopy;
  CFMutableDictionaryRef dict;
  CFDictionaryRef dcopy;

  arr = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
  CFArrayAppendValue (arr, CFSTR ("a"));
  CFArrayAppendValue (arr, CFSTR ("b"));
  acopy = CFPropertyListCreateDeepCopy (NULL, arr,
                                        kCFPropertyListMutableContainers);
  PASS_CF(acopy != NULL && CFArrayGetCount (acopy) == 2,
    "Deep copy of a mutable array has 2 elements.");
  PASS_CF(CFArrayGetCount (acopy) == 2
    && CFEqual (CFArrayGetValueAtIndex (acopy, 0), CFSTR ("a"))
    && CFEqual (CFArrayGetValueAtIndex (acopy, 1), CFSTR ("b")),
    "Deep-copied array elements are preserved.");

  dict = CFDictionaryCreateMutable (NULL, 0,
                                    &kCFCopyStringDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks);
  CFDictionarySetValue (dict, CFSTR ("k"), CFSTR ("v"));
  dcopy = CFPropertyListCreateDeepCopy (NULL, dict,
                                        kCFPropertyListMutableContainers);
  PASS_CF(dcopy != NULL && CFDictionaryGetCount (dcopy) == 1,
    "Deep copy of a mutable dictionary has 1 entry.");
  PASS_CF(CFDictionaryGetCount (dcopy) == 1
    && CFEqual (CFDictionaryGetValue (dcopy, CFSTR ("k")), CFSTR ("v")),
    "Deep-copied dictionary value is preserved.");

  return 0;
}
