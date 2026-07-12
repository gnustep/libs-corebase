/* Regression test for CFPropertyListCreateDeepCopy mutable copies. */

#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include "../CFTesting.h"

int main (void)
{
  CFStringRef		s1;
  CFStringRef		s2;
  SInt32		i32;
  CFNumberRef		n;
  const void	       *aValues[3];
  CFArrayRef		srcArray;
  CFMutableArrayRef	mArrayCopy;
  CFArrayRef		iArrayCopy;
  CFMutableDictionaryRef srcDict;
  CFMutableDictionaryRef mDictCopy;
  CFArrayRef		inner;
  CFMutableArrayRef	outer;
  CFMutableArrayRef	mOuterCopy;

  s1 = CFSTR("hello");
  s2 = CFSTR("world");
  i32 = 42;
  n = CFNumberCreate (NULL, kCFNumberSInt32Type, &i32);

  aValues[0] = s1;
  aValues[1] = s2;
  aValues[2] = n;
  srcArray = CFArrayCreate (NULL, aValues, 3, &kCFTypeArrayCallBacks);

  /* Array: mutable deep copy preserves count. */
  mArrayCopy = (CFMutableArrayRef) CFPropertyListCreateDeepCopy (NULL,
    srcArray, kCFPropertyListMutableContainersAndLeaves);
  PASS_CF(mArrayCopy != NULL,
    "mutable deep copy of a 3-element array is non-NULL");
  PASS_CF(CFArrayGetCount (mArrayCopy) == 3,
    "mutable deep copy of a 3-element array has 3 elements "
    "(was 0 before the CFArrayApplyFunction source/destination fix)");

  /* Array: element values match. */
  if (mArrayCopy != NULL && CFArrayGetCount (mArrayCopy) == 3)
    {
      PASS_CFEQ(CFArrayGetValueAtIndex (mArrayCopy, 0), s1,
        "mutable array deep copy preserves element 0");
      PASS_CFEQ(CFArrayGetValueAtIndex (mArrayCopy, 1), s2,
        "mutable array deep copy preserves element 1");
      PASS_CFEQ(CFArrayGetValueAtIndex (mArrayCopy, 2), n,
        "mutable array deep copy preserves element 2");
    }

  /* Array: the result is genuinely mutable - if the fix is applied,
   * appending to the copy should succeed and the count should grow. */
  if (mArrayCopy != NULL)
    {
      CFArrayAppendValue (mArrayCopy, s1);
      PASS_CF(CFArrayGetCount (mArrayCopy) == 4,
        "mutable array deep copy is genuinely mutable (append grows count)");
      CFRelease (mArrayCopy);
    }

  /* Array: immutable deep copy was never broken - guard it against a
   * regression that mistakenly fixes both branches. */
  iArrayCopy = (CFArrayRef) CFPropertyListCreateDeepCopy (NULL, srcArray,
    kCFPropertyListImmutable);
  PASS_CF(iArrayCopy != NULL,
    "immutable deep copy of an array is non-NULL");
  PASS_CF(iArrayCopy != NULL && CFArrayGetCount (iArrayCopy) == 3,
    "immutable deep copy preserves count (non-buggy branch still works)");
  if (iArrayCopy != NULL)
    {
      CFRelease (iArrayCopy);
    }

  /* Dictionary: mutable deep copy preserves count.  The audit caught
   * the array bug but missed the identical bug in the dictionary
   * branch of CFPropertyListCreateDeepCopy - this test covers it. */
  srcDict = CFDictionaryCreateMutable (NULL, 0,
    &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFDictionaryAddValue (srcDict, CFSTR("first"), s1);
  CFDictionaryAddValue (srcDict, CFSTR("second"), s2);
  CFDictionaryAddValue (srcDict, CFSTR("number"), n);

  mDictCopy = (CFMutableDictionaryRef) CFPropertyListCreateDeepCopy (NULL,
    srcDict, kCFPropertyListMutableContainersAndLeaves);
  PASS_CF(mDictCopy != NULL,
    "mutable deep copy of a 3-entry dictionary is non-NULL");
  PASS_CF(mDictCopy != NULL && CFDictionaryGetCount (mDictCopy) == 3,
    "mutable deep copy of a 3-entry dictionary has 3 entries "
    "(was 0 before the CFDictionaryApplyFunction source/destination fix)");

  /* Dictionary: lookups by key return the right values. */
  if (mDictCopy != NULL && CFDictionaryGetCount (mDictCopy) == 3)
    {
      PASS_CFEQ(CFDictionaryGetValue (mDictCopy, CFSTR("first")), s1,
        "mutable dict deep copy preserves value for \"first\"");
      PASS_CFEQ(CFDictionaryGetValue (mDictCopy, CFSTR("second")), s2,
        "mutable dict deep copy preserves value for \"second\"");
      PASS_CFEQ(CFDictionaryGetValue (mDictCopy, CFSTR("number")), n,
        "mutable dict deep copy preserves value for \"number\"");
    }

  /* Dictionary: the result is genuinely mutable. */
  if (mDictCopy != NULL)
    {
      CFDictionaryAddValue (mDictCopy, CFSTR("extra"), s1);
      PASS_CF(CFDictionaryGetCount (mDictCopy) == 4,
        "mutable dict deep copy is genuinely mutable (add grows count)");
      CFRelease (mDictCopy);
    }

  CFRelease (srcDict);

  /* Nested: a mutable deep copy of an array-containing-array must
   * preserve both the outer and inner contents.  Because
   * CFPropertyListCreateDeepCopy recurses through itself, a nested
   * test catches bugs where the fix only works at the top level. */
  inner = srcArray;
  aValues[0] = inner;
  aValues[1] = s1;
  outer = CFArrayCreateMutable (NULL, 2, &kCFTypeArrayCallBacks);
  CFArrayAppendValue (outer, inner);
  CFArrayAppendValue (outer, s1);

  mOuterCopy = (CFMutableArrayRef) CFPropertyListCreateDeepCopy (NULL, outer,
    kCFPropertyListMutableContainersAndLeaves);
  PASS_CF(mOuterCopy != NULL,
    "mutable deep copy of a nested array is non-NULL");
  PASS_CF(mOuterCopy != NULL && CFArrayGetCount (mOuterCopy) == 2,
    "outer nested array deep copy preserves count");
  if (mOuterCopy != NULL && CFArrayGetCount (mOuterCopy) == 2)
    {
      CFArrayRef nestedCopy
	= (CFArrayRef) CFArrayGetValueAtIndex (mOuterCopy, 0);
      PASS_CF(nestedCopy != NULL && CFArrayGetCount (nestedCopy) == 3,
	"inner nested array is also deep-copied with all elements");
    }
  if (mOuterCopy != NULL)
    {
      CFRelease (mOuterCopy);
    }

  CFRelease (outer);
  CFRelease (srcArray);
  CFRelease (n);
  return 0;
}
