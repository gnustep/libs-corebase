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
  
  return 0;
}

