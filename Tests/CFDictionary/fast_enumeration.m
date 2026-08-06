#import <Foundation/Foundation.h>
#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

static CFMutableDictionaryRef makeCFDict(int count)
{
  CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  for (int i = 0; i < count; i++)
    {
      id key = [[NSNumber alloc] initWithInt: i];
      id val = [[NSNumber alloc] initWithInt: i * 10];
      CFDictionarySetValue(d, (const void *)key, (const void *)val);
      [key release];
      [val release];
    }
  return d;
}

static void testFastEnumeration(int count)
{
  CFMutableDictionaryRef cfdict = makeCFDict(count);
  NSDictionary *dict = (NSDictionary *)cfdict;

  int *seen = calloc(count > 0 ? count : 1, sizeof(int));
  NSUInteger visited = 0, guard = 0, limit = (NSUInteger)count * 4 + 16;
  Boolean duplicate = false, invalid = false;

  for (NSNumber *key in dict)
    {
      int k = [key intValue];
      if (k < 0 || k >= count)          invalid = true;
      else if (seen[k]++)               duplicate = true;

      NSNumber *val = [dict objectForKey: key];   /* usable mid-enumeration */
      if (!val || [val intValue] != k * 10)        invalid = true;

      visited++;
      if (++guard > limit) break;        /* regression: never terminates */
    }

  PASS_CF(guard <= limit,
    "fast enumeration of a %d-key CFDictionary terminates", count);
  PASS_CF(visited == (NSUInteger)count,
    "fast enumeration visits every key exactly once (%d keys, visited %lu)",
    count, (unsigned long)visited);
  PASS_CF(!duplicate, "fast enumeration does not repeat keys (%d keys)", count);
  PASS_CF(!invalid,   "fast enumeration yields valid keys/values (%d keys)", count);

  free(seen);
  CFRelease(cfdict);
}

/* A fresh for/in must restart cleanly (state->state resets to 0). */
static void testReEnumeration(void)
{
  CFMutableDictionaryRef cfdict = makeCFDict(10);
  NSDictionary *dict = (NSDictionary *)cfdict;
  NSUInteger a = 0, b = 0, guard = 0;

  for (id k in dict) { (void)k; if (++guard > 100) break; a++; }
  guard = 0;
  for (id k in dict) { (void)k; if (++guard > 100) break; b++; }

  PASS_CF(a == 10 && b == 10,
    "a CFDictionary can be fast-enumerated repeatedly (%lu, then %lu)",
    (unsigned long)a, (unsigned long)b);

  CFRelease(cfdict);
}

int main(void)
{
  testFastEnumeration(0);     /* empty: loop body never runs */
  testFastEnumeration(1);     /* the originally reported case */
  testFastEnumeration(20);    /* > 16-element batch: cursor resume across calls */
  testFastEnumeration(500);   /* many batches + hash collisions */
  testReEnumeration();
  return 0;
}
