#include "CoreFoundation/CFArray.h"
#include "../CFTesting.h"
#include <string.h>
#include <stdlib.h>

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
    /* Sort a shuffled permutation of 1..sz for a range of sizes that straddle
       the 16-element threshold between the insertion sort and the quicksort
       path (the latter used to read one element past the end of the array).
       The sorted result must be exactly 1, 2, ..., sz. */
    const CFIndex sizes[] = { 1, 2, 5, 15, 16, 17, 33, 64, 100 };
    const void *shuf[100];
    CFIndex s;

    srand (1);
    for (s = 0; s < (CFIndex)(sizeof(sizes) / sizeof(sizes[0])); s++)
      {
        CFIndex sz = sizes[s];
        CFArrayRef src;
        CFMutableArrayRef big;
        Boolean ordered = true;
        CFIndex i;

        for (i = 0; i < sz; i++)
          shuf[i] = (const void *)(CFIndex)(i + 1);
        for (i = sz - 1; i > 0; i--)
          {
            CFIndex j = rand () % (i + 1);
            const void *t = shuf[i];
            shuf[i] = shuf[j];
            shuf[j] = t;
          }

        src = CFArrayCreate (NULL, shuf, sz, NULL);
        big = CFArrayCreateMutableCopy (NULL, sz, src);
        CFArraySortValues (big, CFRangeMake (0, sz), comp, NULL);

        for (i = 0; i < sz; i++)
          if ((CFIndex) CFArrayGetValueAtIndex (big, i) != i + 1)
            ordered = false;

        PASS_CF(ordered && CFArrayGetCount (big) == sz,
          "Shuffled range of %d values sorts to ascending order.", (int)sz);
        CFRelease (big);
        CFRelease (src);
      }
  }

  return 0;
}

