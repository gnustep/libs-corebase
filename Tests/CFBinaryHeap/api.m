#include "CoreFoundation/CFBinaryHeap.h"
#include "../CFTesting.h"

static int v1 = 3, v2 = 1, v3 = 2;
static CFIndex g_count;

static void
applier (const void *val, void *context)
{
  (void)val;
  (void)context;
  g_count++;
}

static CFComparisonResult
cmp (const void *a, const void *b, void *context)
{
  int x = *(const int *) a;
  int y = *(const int *) b;
  (void)context;
  if (x < y)
    return kCFCompareLessThan;
  if (x > y)
    return kCFCompareGreaterThan;
  return kCFCompareEqualTo;
}

int main (void)
{
  CFBinaryHeapCallBacks cb = { 0, NULL, NULL, NULL, cmp };
  CFBinaryHeapRef h, copy;
  const void *minv;

  PASS_CF(CFBinaryHeapGetTypeID () != 0, "CFBinaryHeapGetTypeID is not zero.");

  h = CFBinaryHeapCreate (NULL, 0, &cb, NULL);
  PASS_CF(CFGetTypeID (h) == CFBinaryHeapGetTypeID (),
    "A heap has the heap type ID.");

  CFBinaryHeapAddValue (h, &v1);
  CFBinaryHeapAddValue (h, &v2);
  CFBinaryHeapAddValue (h, &v3);

  PASS_CF(CFBinaryHeapGetMinimumIfPresent (h, &minv)
      && *(const int *) minv == 1,
    "CFBinaryHeapGetMinimumIfPresent returns the minimum.");

  g_count = 0;
  CFBinaryHeapApplyFunction (h, applier, NULL);
  PASS_CF(g_count == 3, "CFBinaryHeapApplyFunction visits every value.");

  copy = CFBinaryHeapCreateCopy (NULL, 0, h);
  PASS_CF(CFBinaryHeapGetCount (copy) == 3
      && *(const int *) CFBinaryHeapGetMinimum (copy) == 1,
    "CFBinaryHeapCreateCopy copies the values.");
  CFRelease (copy);

  CFBinaryHeapRemoveAllValues (h);
  PASS_CF(CFBinaryHeapGetCount (h) == 0,
    "CFBinaryHeapRemoveAllValues empties the heap.");

  CFRelease (h);

  return 0;
}
