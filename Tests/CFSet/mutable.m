#include "CoreFoundation/CFSet.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableSetRef set;
  CFMutableSetRef copy;

  set = CFSetCreateMutable (NULL, 0, &kCFTypeSetCallBacks);
  PASS_CF(set != NULL, "CFMutableSet created.");
  PASS_CF(CFSetGetCount (set) == 0, "A new mutable set is empty.");

  CFSetAddValue (set, CFSTR("a"));
  CFSetAddValue (set, CFSTR("b"));
  PASS_CF(CFSetGetCount (set) == 2, "CFSetAddValue adds values.");

  /* A set holds each value once: adding a present value is a no-op. */
  CFSetAddValue (set, CFSTR("a"));
  PASS_CF(CFSetGetCount (set) == 2,
    "CFSetAddValue does not duplicate a present value.");

  /* CFSetSetValue adds a value that is not yet present. */
  CFSetSetValue (set, CFSTR("c"));
  PASS_CF(CFSetGetCount (set) == 3 && CFSetContainsValue (set, CFSTR("c")),
    "CFSetSetValue adds an absent value.");
  CFSetSetValue (set, CFSTR("c"));
  PASS_CF(CFSetGetCount (set) == 3,
    "CFSetSetValue leaves an already-present value alone.");

  /* CFSetReplaceValue only acts when the value is already present. */
  CFSetReplaceValue (set, CFSTR("a"));
  PASS_CF(CFSetGetCount (set) == 3 && CFSetContainsValue (set, CFSTR("a")),
    "CFSetReplaceValue keeps a present value.");
  CFSetReplaceValue (set, CFSTR("z"));
  PASS_CF(CFSetGetCount (set) == 3 && !CFSetContainsValue (set, CFSTR("z")),
    "CFSetReplaceValue ignores an absent value.");

  copy = CFSetCreateMutableCopy (NULL, 0, set);
  PASS_CF(copy != NULL, "CFSetCreateMutableCopy returns a set.");
  PASS_CFEQ(copy, set, "A mutable copy is equal to the original.");
  CFRelease (copy);

  CFSetRemoveValue (set, CFSTR("b"));
  PASS_CF(CFSetGetCount (set) == 2, "CFSetRemoveValue removes a value.");
  PASS_CF(!CFSetContainsValue (set, CFSTR("b")),
    "A removed value is no longer present.");

  CFSetRemoveValue (set, CFSTR("z"));
  PASS_CF(CFSetGetCount (set) == 2,
    "CFSetRemoveValue of an absent value is a no-op.");

  CFSetRemoveAllValues (set);
  PASS_CF(CFSetGetCount (set) == 0,
    "CFSetRemoveAllValues empties the set.");

  CFRelease (set);

  return 0;
}
