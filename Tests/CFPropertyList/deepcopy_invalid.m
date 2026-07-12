/* Regression test: CFPropertyListCreateDeepCopy returns NULL for input that is
 * not a valid property list, matching Apple CoreFoundation. */

#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFURL.h>
#include "../CFTesting.h"

int main (void)
{
  SInt32		i32;
  CFNumberRef		n;
  CFURLRef		url;
  const void	       *aValues[1];
  CFArrayRef		validArray;
  CFArrayRef		badValueArray;
  CFDictionaryRef	badKeyDict;
  CFMutableArrayRef	cyclicArray;
  CFPropertyListRef	copy;

  /* A valid property list still copies. */
  i32 = 42;
  n = CFNumberCreate (NULL, kCFNumberSInt32Type, &i32);
  aValues[0] = n;
  validArray = CFArrayCreate (NULL, aValues, 1, &kCFTypeArrayCallBacks);
  copy = CFPropertyListCreateDeepCopy (NULL, validArray,
                                       kCFPropertyListImmutable);
  PASS_CF(copy != NULL, "a valid property list is copied");
  if (copy)
    CFRelease (copy);
  CFRelease (validArray);

  /* An array holding a non-property-list value (a CFURL) is invalid. */
  url = CFURLCreateWithString (NULL, CFSTR("http://example.com/"), NULL);
  aValues[0] = url;
  badValueArray = CFArrayCreate (NULL, aValues, 1, &kCFTypeArrayCallBacks);
  copy = CFPropertyListCreateDeepCopy (NULL, badValueArray,
                                       kCFPropertyListImmutable);
  PASS_CF(copy == NULL,
    "an array containing a non-property-list value returns NULL");
  if (copy)
    CFRelease (copy);
  CFRelease (badValueArray);
  CFRelease (url);

  /* A dictionary with a non-string key is invalid. */
  {
    const void *keys[1];
    const void *vals[1];

    keys[0] = n;                /* a number key */
    vals[0] = CFSTR("v");
    badKeyDict = CFDictionaryCreate (NULL, keys, vals, 1,
                                     &kCFTypeDictionaryKeyCallBacks,
                                     &kCFTypeDictionaryValueCallBacks);
    copy = CFPropertyListCreateDeepCopy (NULL, badKeyDict,
                                         kCFPropertyListImmutable);
    PASS_CF(copy == NULL,
      "a dictionary with a non-string key returns NULL");
    if (copy)
      CFRelease (copy);
    CFRelease (badKeyDict);
  }

  /* A container that contains itself is cyclic and cannot be copied. */
  cyclicArray = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
  CFArrayAppendValue (cyclicArray, cyclicArray);
  copy = CFPropertyListCreateDeepCopy (NULL, cyclicArray,
                                       kCFPropertyListImmutable);
  PASS_CF(copy == NULL, "a cyclic container returns NULL");
  if (copy)
    CFRelease (copy);
  CFArrayRemoveAllValues (cyclicArray);         /* break the cycle */
  CFRelease (cyclicArray);

  CFRelease (n);

  return 0;
}
