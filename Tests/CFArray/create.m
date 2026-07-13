#include "CoreFoundation/CFArray.h"
#include "../CFTesting.h"

#define ARRAY_SIZE 5
const CFIndex array[ARRAY_SIZE] = { 5, 2, 1, 4, 3 };

int main (void)
{
  CFArrayRef a;
  CFIndex n;
  
  a = CFArrayCreate (NULL, (const void**)&array, ARRAY_SIZE, NULL);
  PASS_CF(a != NULL, "CFArray created.");
  
  n = CFArrayGetCount (a);
  PASS_CF(n == ARRAY_SIZE, "CFArray has correct number of values.");
  n = (CFIndex)CFArrayGetCountOfValue (a, CFRangeMake(0, ARRAY_SIZE), (const void*)3);
  PASS_CF(n == 1, "Found 1 occurence of the %d.", (int)n);
  
  n = (CFIndex)CFArrayGetValueAtIndex (a, 1);
  PASS_CF(n == 2, "Found value at index %d.", (int)n);

  n = CFArrayGetLastIndexOfValue (a, CFRangeMake(0, ARRAY_SIZE), (const void*)4);
  PASS_CF(n == 3, "Last index of value 4 is %d.", (int)n);
  /* Searching the whole array must not read one past the end of the range. */
  n = CFArrayGetLastIndexOfValue (a, CFRangeMake(0, ARRAY_SIZE), (const void*)99);
  PASS_CF(n == -1, "Absent value is reported as not found (%d).", (int)n);

  CFRelease (a);
  
  a = CFArrayCreate (NULL, NULL, 0, NULL);
  PASS_CF(a != NULL, "An array with no values return non-NULL.");
  
  return 0;
}

