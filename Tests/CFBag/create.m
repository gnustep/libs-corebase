#include "CoreFoundation/CFBag.h"
#include "../CFTesting.h"

int main (void)
{
  CFBagRef bag;
  CFBagRef copy;
  CFBagRef distinct;
  const void *values[4];
  const void *out[3];
  const void *found;
  CFIndex i;
  Boolean present;

  /* "a" appears twice: a bag is a multiset, so the total count and the
     per-value count reflect the repeat. */
  values[0] = CFSTR("a");
  values[1] = CFSTR("b");
  values[2] = CFSTR("c");
  values[3] = CFSTR("a");

  bag = CFBagCreate (NULL, values, 4, &kCFTypeBagCallBacks);
  PASS_CF(bag != NULL, "CFBag created.");

  PASS_CF(CFBagGetTypeID () != 0, "CFBag type ID is not zero.");
  PASS_CF(CFGetTypeID (bag) == CFBagGetTypeID (),
    "CFBag has the CFBag type ID.");

  PASS_CF(CFBagGetCount (bag) == 4,
    "CFBagGetCount counts repeated values.");
  PASS_CF(CFBagGetCountOfValue (bag, CFSTR("a")) == 2,
    "CFBagGetCountOfValue reports the multiplicity of a value.");
  PASS_CF(CFBagGetCountOfValue (bag, CFSTR("b")) == 1,
    "CFBagGetCountOfValue reports a single occurrence.");
  PASS_CF(CFBagGetCountOfValue (bag, CFSTR("z")) == 0,
    "CFBagGetCountOfValue reports zero for an absent value.");

  PASS_CF(CFBagContainsValue (bag, CFSTR("b")),
    "CFBagContainsValue finds a present value.");
  PASS_CF(!CFBagContainsValue (bag, CFSTR("z")),
    "CFBagContainsValue rejects an absent value.");

  found = CFBagGetValue (bag, CFSTR("c"));
  PASS_CFEQ(found, CFSTR("c"), "CFBagGetValue returns the stored value.");
  PASS_CF(CFBagGetValue (bag, CFSTR("z")) == NULL,
    "CFBagGetValue returns NULL for an absent value.");

  found = NULL;
  present = CFBagGetValueIfPresent (bag, CFSTR("b"), &found);
  PASS_CF(present && found != NULL,
    "CFBagGetValueIfPresent reports a present value.");
  present = CFBagGetValueIfPresent (bag, CFSTR("z"), &found);
  PASS_CF(!present,
    "CFBagGetValueIfPresent reports an absent value.");

  /* CFBagGetValues writes one entry per distinct value, so exercise it on a
     bag whose values are all distinct.  Every retrieved value must be one we
     put in, and such a bag round-trips through CFBagCreateCopy. */
  distinct = CFBagCreate (NULL, values, 3, &kCFTypeBagCallBacks);
  CFBagGetValues (distinct, out);
  for (i = 0; i < 3; i++)
    PASS_CF(CFBagContainsValue (distinct, out[i]),
      "CFBagGetValues returns stored values.");

  copy = CFBagCreateCopy (NULL, distinct);
  PASS_CF(copy != NULL, "CFBagCreateCopy returns a bag.");
  PASS_CFEQ(copy, distinct, "A copy is equal to the original.");
  PASS_CF(CFHash (copy) == CFHash (distinct),
    "A copy hashes the same as the original.");
  CFRelease (copy);
  CFRelease (distinct);

  CFRelease (bag);

  return 0;
}
