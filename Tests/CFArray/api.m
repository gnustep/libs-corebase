#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

static CFIndex g_count;

static void
counter (const void *value, void *context)
{
  (void)value;
  (void)context;
  g_count++;
}

int main (void)
{
  const void *vals[3] = { CFSTR("a"), CFSTR("b"), CFSTR("c") };
  CFArrayRef arr, copy;
  CFMutableArrayRef m;
  const void *rep[1];

  PASS_CF(CFArrayGetTypeID () != 0, "CFArrayGetTypeID is not zero.");

  arr = CFArrayCreate (NULL, vals, 3, &kCFTypeArrayCallBacks);
  PASS_CF(CFGetTypeID (arr) == CFArrayGetTypeID (),
    "An array has the array type ID.");

  PASS_CF(CFArrayContainsValue (arr, CFRangeMake (0, 3), CFSTR("b")),
    "CFArrayContainsValue finds a present value.");
  PASS_CF(!CFArrayContainsValue (arr, CFRangeMake (0, 3), CFSTR("z")),
    "CFArrayContainsValue rejects an absent value.");

  copy = CFArrayCreateCopy (NULL, arr);
  PASS_CF(CFEqual (copy, arr), "CFArrayCreateCopy equals the original.");
  CFRelease (copy);

  g_count = 0;
  CFArrayApplyFunction (arr, CFRangeMake (0, 3), counter, NULL);
  PASS_CF(g_count == 3, "CFArrayApplyFunction visits every value.");

  m = CFArrayCreateMutableCopy (NULL, 0, arr);
  CFArrayInsertValueAtIndex (m, 0, CFSTR("x"));
  PASS_CF(CFEqual (CFArrayGetValueAtIndex (m, 0), CFSTR("x"))
      && CFArrayGetCount (m) == 4,
    "CFArrayInsertValueAtIndex inserts a value.");

  CFArraySetValueAtIndex (m, 0, CFSTR("y"));
  PASS_CF(CFEqual (CFArrayGetValueAtIndex (m, 0), CFSTR("y")),
    "CFArraySetValueAtIndex replaces a value.");

  CFArrayExchangeValuesAtIndices (m, 0, 1);
  PASS_CF(CFEqual (CFArrayGetValueAtIndex (m, 0), CFSTR("a"))
      && CFEqual (CFArrayGetValueAtIndex (m, 1), CFSTR("y")),
    "CFArrayExchangeValuesAtIndices swaps two values.");

  CFArrayRemoveValueAtIndex (m, 0);
  PASS_CF(CFArrayGetCount (m) == 3
      && CFEqual (CFArrayGetValueAtIndex (m, 0), CFSTR("y")),
    "CFArrayRemoveValueAtIndex removes a value.");

  CFArrayAppendArray (m, arr, CFRangeMake (0, 3));
  PASS_CF(CFArrayGetCount (m) == 6, "CFArrayAppendArray appends a range.");

  rep[0] = CFSTR("Z");
  CFArrayReplaceValues (m, CFRangeMake (0, 2), rep, 1);
  PASS_CF(CFArrayGetCount (m) == 5
      && CFEqual (CFArrayGetValueAtIndex (m, 0), CFSTR("Z")),
    "CFArrayReplaceValues replaces a range.");

  CFArrayRemoveAllValues (m);
  PASS_CF(CFArrayGetCount (m) == 0,
    "CFArrayRemoveAllValues empties the array.");

  CFRelease (m);
  CFRelease (arr);

  return 0;
}
