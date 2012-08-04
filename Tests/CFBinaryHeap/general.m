#include <CoreFoundation/CFBinaryHeap.h>
#include "../CFTesting.h"
#include <string.h>

#define ARRAY_SIZE 6
const CFIndex array[ARRAY_SIZE] = { 7, 10, 5, 10, 25, 1 };
const CFIndex sorted[ARRAY_SIZE] = { 1, 5, 7, 10, 10, 25 };

int main (void)
{
  CFIndex idx;
  CFIndex n;
  CFBinaryHeapRef heap;
  CFIndex values[ARRAY_SIZE];
  
  heap = CFBinaryHeapCreate (NULL, ARRAY_SIZE, NULL, NULL);
  PASS_CF(heap != NULL, "Binary heap was created.");
  for (idx = 0 ; idx < ARRAY_SIZE ; ++idx)
    CFBinaryHeapAddValue (heap, (const void*)array[idx]);
  
  n = CFBinaryHeapGetCount(heap);
  PASS_CF(n == ARRAY_SIZE, "Binary heap has %d values.", (int)n);
  
  PASS_CF(CFBinaryHeapContainsValue(heap, (const void*)5), "Binary heap has value.");
  
  n = CFBinaryHeapGetCountOfValue(heap, (const void*)10);
  PASS_CF(n == 2, "Value 10 appears %d times in binary heap.", (int)n);
  
  CFBinaryHeapGetValues (heap, (const void**)values);
  PASS_CF(memcmp(values, sorted, sizeof(void*) * ARRAY_SIZE) == 0,
    "All values are in the binary heap.");
  
  CFBinaryHeapRemoveMinimumValue (heap);
  n = (CFIndex)CFBinaryHeapGetMinimum (heap);
  PASS_CF(n == 5, "Minimum value in binary heap is %d.", (int)n);
  
  CFRelease (heap);
  
  return 0;
}