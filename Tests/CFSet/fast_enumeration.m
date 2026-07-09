#import <Foundation/NSSet.h>
#import <Foundation/NSNumber.h>
#include "CoreFoundation/CFSet.h"
#include "../CFTesting.h"

/* Build a set via the CF API so it is backed by NSCFSet, exercising the
 * bridge's fast-enumeration path. */
static CFMutableSetRef makeCFSet(int count)
{
  CFMutableSetRef s = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
  for (int i = 0; i < count; i++)
    {
      id v = [[NSNumber alloc] initWithInt: i];
      CFSetAddValue(s, (const void *)v);
      [v release];
    }
  return s;
}

static void testFastEnumeration(int count)
{
  CFMutableSetRef cfset = makeCFSet(count);
  NSSet *set = (NSSet *)cfset;

  int *seen = calloc(count > 0 ? count : 1, sizeof(int));
  NSUInteger visited = 0, guard = 0, limit = (NSUInteger)count * 4 + 16;
  Boolean duplicate = false, invalid = false;

  for (NSNumber *v in set)
    {
      int k = [v intValue];
      if (k < 0 || k >= count) invalid = true;
      else if (seen[k]++)      duplicate = true;
      visited++;
      if (++guard > limit) break;        /* regression: never terminates */
    }

  PASS_CF(guard <= limit,
    "fast enumeration of a %d-element CFSet terminates", count);
  PASS_CF(visited == (NSUInteger)count,
    "fast enumeration visits every element once (%d, visited %lu)",
    count, (unsigned long)visited);
  PASS_CF(!duplicate, "fast enumeration does not repeat elements (%d)", count);
  PASS_CF(!invalid,   "fast enumeration yields valid elements (%d)", count);

  free(seen);
  CFRelease(cfset);
}

int main(void)
{
  testFastEnumeration(0);
  testFastEnumeration(1);
  testFastEnumeration(20);
  testFastEnumeration(500);
  return 0;
}
