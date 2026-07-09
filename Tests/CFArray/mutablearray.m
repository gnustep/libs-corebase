#include "CoreFoundation/CFArray.h"
#include "../CFTesting.h"
#include <string.h>

#define ARRAY_SIZE 5
const CFIndex array[ARRAY_SIZE] = { 5, 2, 3, 4, 1 };
const CFIndex sorted[ARRAY_SIZE+1] = { 1, 2, 3, 4, 5, 7 };

CFComparisonResult comp (const void *val1, const void *val2, void *context)
{
  return val1 == val2 ? kCFCompareEqualTo : (val1 < val2 ? kCFCompareLessThan :
    kCFCompareGreaterThan);
}

int main (void)
{
  CFArrayRef a;
  CFMutableArrayRef ma;
  CFIndex n;
  CFIndex len;
  CFIndex buf[ARRAY_SIZE + 1];
  
  a = CFArrayCreate (NULL, (const void**)&array, ARRAY_SIZE, NULL);
  PASS_CF(a != NULL, "CFArray created.");
  
  ma = CFArrayCreateMutableCopy (NULL, 6, a);
  PASS_CF(ma != NULL, "CFMutableArray created.");
  
  n = 7;
  CFArrayAppendValue (ma, (const void*)n);
  len = CFArrayGetCount ((CFArrayRef)ma);
  PASS_CF(len == ARRAY_SIZE + 1, "CFMutableArray has correct number of values.");
  
  CFArraySortValues (ma, CFRangeMake(0, len), comp, NULL);
  CFArrayGetValues (ma, CFRangeMake(0, len), (const void**)buf);
  PASS_CF(memcmp(buf, sorted, sizeof(CFIndex) * ARRAY_SIZE) == 0,
    "Array sorted correctly.");
  
  n = CFArrayBSearchValues (ma, CFRangeMake(2, len - 2), (const void*)5, comp, NULL);
  PASS_CF(n == 4, "Index of number 5 is %d.", (int)n);
  
  n = CFArrayBSearchValues (ma, CFRangeMake(0, len), (const void*)6, comp, NULL);
  PASS_CF(n == 5, "Index of value between values is %d.", (int)n);

  {
    const void *big[20];
    CFArrayRef bigArr;
    CFMutableArrayRef mc;
    CFMutableArrayRef target;
    int i;

    for (i = 0; i < 20; i++)
      big[i] = (const void *)(CFIndex)(i + 1);
    bigArr = CFArrayCreate (NULL, big, 20, NULL);

    /* A mutable copy whose requested capacity is smaller than the source
       count must still hold every value (no write past the buffer). */
    mc = CFArrayCreateMutableCopy (NULL, 0, bigArr);
    PASS_CF(CFArrayGetCount (mc) == 20
      && CFArrayGetValueAtIndex (mc, 19) == (const void *)(CFIndex)20,
      "MutableCopy with capacity 0 holds all 20 values.");
    CFRelease (mc);

    /* Appending a whole array grows by more than the default chunk at once. */
    target = CFArrayCreateMutable (NULL, 0, NULL);
    CFArrayAppendArray (target, bigArr, CFRangeMake (0, 20));
    PASS_CF(CFArrayGetCount (target) == 20
      && CFArrayGetValueAtIndex (target, 19) == (const void *)(CFIndex)20,
      "AppendArray of 20 values grows the array correctly.");
    CFRelease (target);
    CFRelease (bigArr);
  }

  return 0;
}

