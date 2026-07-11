#include "CoreFoundation/CFSet.h"
#include "../CFTesting.h"

static void
countApplier (const void *value, void *context)
{
  (void)value;
  *((CFIndex *)context) += 1;
}

int main (void)
{
  CFSetRef set;
  CFSetRef copy;
  CFSetRef other;
  CFSetRef empty;
  const void *values[3];
  const void *out[3];
  const void *found;
  const void *otherValues[2];
  CFIndex applied;
  CFIndex i;
  Boolean present;

  values[0] = CFSTR("a");
  values[1] = CFSTR("b");
  values[2] = CFSTR("c");

  set = CFSetCreate (NULL, values, 3, &kCFTypeSetCallBacks);
  PASS_CF(set != NULL, "CFSet created.");

  PASS_CF(CFSetGetTypeID () != 0, "CFSet type ID is not zero.");
  PASS_CF(CFGetTypeID (set) == CFSetGetTypeID (),
    "CFSet has the CFSet type ID.");

  PASS_CF(CFSetGetCount (set) == 3, "CFSetGetCount returns the value count.");

  PASS_CF(CFSetContainsValue (set, CFSTR("b")),
    "CFSetContainsValue finds a present value.");
  PASS_CF(!CFSetContainsValue (set, CFSTR("z")),
    "CFSetContainsValue rejects an absent value.");

  PASS_CF(CFSetGetCountOfValue (set, CFSTR("b")) == 1,
    "CFSetGetCountOfValue returns 1 for a present value.");
  PASS_CF(CFSetGetCountOfValue (set, CFSTR("z")) == 0,
    "CFSetGetCountOfValue returns 0 for an absent value.");

  found = CFSetGetValue (set, CFSTR("c"));
  PASS_CFEQ(found, CFSTR("c"), "CFSetGetValue returns the stored value.");
  PASS_CF(CFSetGetValue (set, CFSTR("z")) == NULL,
    "CFSetGetValue returns NULL for an absent value.");

  found = NULL;
  present = CFSetGetValueIfPresent (set, CFSTR("a"), &found);
  PASS_CF(present && found != NULL,
    "CFSetGetValueIfPresent reports a present value.");
  present = CFSetGetValueIfPresent (set, CFSTR("z"), &found);
  PASS_CF(!present,
    "CFSetGetValueIfPresent reports an absent value.");

  CFSetGetValues (set, out);
  for (i = 0; i < 3; i++)
    PASS_CF(CFSetContainsValue (set, out[i]),
      "CFSetGetValues returns stored values.");

  /* CFSetApplyFunction must visit every value exactly once. */
  applied = 0;
  CFSetApplyFunction (set, countApplier, &applied);
  PASS_CF(applied == 3, "CFSetApplyFunction visits every value.");

  copy = CFSetCreateCopy (NULL, set);
  PASS_CF(copy != NULL, "CFSetCreateCopy returns a set.");
  PASS_CFEQ(copy, set, "A copy is equal to the original.");
  PASS_CF(CFHash (copy) == CFHash (set),
    "A copy hashes the same as the original.");
  CFRelease (copy);

  /* A set with different members is not equal. */
  otherValues[0] = CFSTR("a");
  otherValues[1] = CFSTR("x");
  other = CFSetCreate (NULL, otherValues, 2, &kCFTypeSetCallBacks);
  PASS_CFNEQ(other, set, "Sets with different members are not equal.");
  CFRelease (other);

  /* An empty set. */
  empty = CFSetCreate (NULL, NULL, 0, &kCFTypeSetCallBacks);
  PASS_CF(empty != NULL && CFSetGetCount (empty) == 0,
    "An empty set has count 0.");
  PASS_CF(!CFSetContainsValue (empty, CFSTR("a")),
    "An empty set contains no value.");
  PASS_CF(CFSetGetValue (empty, CFSTR("a")) == NULL,
    "CFSetGetValue on an empty set returns NULL.");
  CFRelease (empty);

  CFRelease (set);

  return 0;
}
