#include "CoreFoundation/CFSet.h"
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  const void *values[3];
  CFSetRef set;

  values[0] = CFSTR ("a");
  values[1] = CFSTR ("b");
  values[2] = CFSTR ("a");

  set = CFSetCreate (NULL, values, 3, &kCFTypeSetCallBacks);
  PASS_CF(CFSetGetCount (set) == 2,
    "A value repeated in the input array is stored once.");
  PASS_CF(CFSetContainsValue (set, CFSTR ("a"))
    && CFSetContainsValue (set, CFSTR ("b")),
    "Both distinct values are present.");

  CFRelease (set);

  return 0;
}
