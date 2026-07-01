#include "CoreFoundation/CFBag.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableBagRef bag;
  CFMutableBagRef copy;

  bag = CFBagCreateMutable (NULL, 0, &kCFTypeBagCallBacks);
  PASS_CF(bag != NULL, "CFMutableBag created.");
  PASS_CF(CFBagGetCount (bag) == 0, "A new mutable bag is empty.");

  CFBagAddValue (bag, CFSTR("a"));
  CFBagAddValue (bag, CFSTR("b"));
  CFBagAddValue (bag, CFSTR("c"));
  PASS_CF(CFBagGetCount (bag) == 3,
    "CFBagAddValue adds distinct values.");
  PASS_CF(CFBagContainsValue (bag, CFSTR("b")),
    "An added value is present.");

  /* CFBagSetValue adds a value that is not yet present. */
  CFBagSetValue (bag, CFSTR("d"));
  PASS_CF(CFBagGetCount (bag) == 4,
    "CFBagSetValue adds an absent value.");
  /* Setting a value that is already present does not change the count. */
  CFBagSetValue (bag, CFSTR("d"));
  PASS_CF(CFBagGetCount (bag) == 4,
    "CFBagSetValue leaves an already-present value alone.");

  /* CFBagReplaceValue only acts when the value is already present. */
  CFBagReplaceValue (bag, CFSTR("a"));
  PASS_CF(CFBagGetCount (bag) == 4 && CFBagContainsValue (bag, CFSTR("a")),
    "CFBagReplaceValue keeps a present value.");
  CFBagReplaceValue (bag, CFSTR("z"));
  PASS_CF(CFBagGetCount (bag) == 4 && !CFBagContainsValue (bag, CFSTR("z")),
    "CFBagReplaceValue ignores an absent value.");

  copy = CFBagCreateMutableCopy (NULL, 0, bag);
  PASS_CF(copy != NULL, "CFBagCreateMutableCopy returns a bag.");
  PASS_CFEQ(copy, bag, "A mutable copy is equal to the original.");
  CFRelease (copy);

  CFBagRemoveValue (bag, CFSTR("b"));
  PASS_CF(CFBagGetCount (bag) == 3,
    "CFBagRemoveValue removes a value.");
  PASS_CF(!CFBagContainsValue (bag, CFSTR("b")),
    "A removed value is no longer present.");

  CFBagRemoveAllValues (bag);
  PASS_CF(CFBagGetCount (bag) == 0,
    "CFBagRemoveAllValues empties the bag.");

  CFRelease (bag);

  return 0;
}
