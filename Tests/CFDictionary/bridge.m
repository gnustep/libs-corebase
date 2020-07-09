#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

void testCFonNS(void);
void testNSonCF(void);
void testLargeDict(void);

int main (void)
{
  testCFonNS();
  testNSonCF();
  testLargeDict();
  return 0;
}

void testCFonNS(void)
{
  NSMutableDictionary* nsdict = [[NSMutableDictionary alloc] initWithCapacity: 4];
  CFMutableDictionaryRef cfdict = (CFMutableDictionaryRef) nsdict;
  CFIndex count;

  [nsdict setValue: @"A"
            forKey: @"a"];

  [nsdict setValue: @"B"
            forKey: @"b"];

  [nsdict setValue: @"C"
            forKey: @"c"];

  [nsdict setValue: @"D"
            forKey: @"d"];

  count = [nsdict count];
  PASS_CF(CFDictionaryGetCount(cfdict) == count,
    "CFDictionaryGetCount() works on an NSDictionary");

  PASS_CF(CFDictionaryGetValue(cfdict, @"b") == [nsdict objectForKey: @"b"],
    "CFDictionaryGetValue() works on an NSDictionary");

  CFDictionaryAddValue(cfdict, @"e", @"E");
  PASS_CF(CFDictionaryGetCount(cfdict) == count+1,
    "CFDictionaryAddValue() added a pair into an NSDictionary");

  NSLog(@"dict: %@\n", nsdict);

  PASS_CF(CFDictionaryGetCountOfKey(cfdict, @"e") == 1,
    "CFDictionaryGetCountOfKey() works on an NSDictionary (#1)");
  PASS_CF(CFDictionaryGetCountOfValue(cfdict, @"E") == 1,
    "CFDictionaryGetCountOfValue() works on an NSDictionary (#1)");

  PASS_CF(CFDictionaryGetCountOfKey(cfdict, @"b") == 1,
    "CFDictionaryGetCountOfKey() works on an NSDictionary (#2)");
  PASS_CF(CFDictionaryGetCountOfValue(cfdict, @"B") == 1,
    "CFDictionaryGetCountOfValue() works on an NSDictionary (#2)");

  CFRelease(cfdict);
}

void testNSonCF(void)
{
  CFMutableDictionaryRef cfdict = CFDictionaryCreateMutable(NULL, 4,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  NSMutableDictionary* nsdict = (NSMutableDictionary*) cfdict;
  CFIndex count;

  CFDictionaryAddValue(cfdict, @"a", @"A");
  CFDictionaryAddValue(cfdict, @"b", @"B");
  CFDictionaryAddValue(cfdict, @"c", @"C");
  CFDictionaryAddValue(cfdict, @"d", @"D");

  count = CFDictionaryGetCount(cfdict);
  PASS_CF([nsdict count] == count,
    "-count works on a CFDictionary");

  PASS_CF(CFDictionaryGetValue(cfdict, @"b") == [nsdict objectForKey: @"b"],
    "-objectForKey: works on a CFDictionary");

  [nsdict setObject: @"E"
             forKey: @"e"];

  PASS_CF([nsdict count] == count+1,
    "-setObject:forKey: works on a CFDictionary");

  PASS_CF([nsdict objectForKey: @"e"] != nil,
    "-objectForKey: works on a CFDictionary");

  [nsdict release];
}

void testLargeDict(void)
{
  CFMutableDictionaryRef cfdict = CFDictionaryCreateMutable(NULL, 0,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  int count = 5000;
  int removeCount = 0;

  for (int i = 0; i < count; i++) {
    id key = [[NSString alloc] initWithFormat:@"key-%d", i];
    id value = [[NSNumber alloc] initWithInt:i];
    CFDictionarySetValue(cfdict, (__bridge const void *)key, (__bridge const void *)value);
    [key release];
    [value release];

    // start removing keys while we are adding new ones after filling 1/10
    if (i > count/10) {
      id keyToEvict = [[NSString alloc] initWithFormat:@"key-%d", removeCount++];
      CFDictionaryRemoveValue(cfdict, (__bridge const void *)(keyToEvict));
      [keyToEvict release];
    }
  }

  for (int i = 0; i < count; i++) {
    id key = [[NSString alloc] initWithFormat:@"key-%d", i];
    void *value = CFDictionaryGetValue(cfdict, (__bridge const void *)key);
    
    if (i < removeCount) {
      PASS_CF(value == NULL, "CFDictionaryGetValue returns no value for non-existant key '%@'", key);
    } else {
      PASS_CF(value != NULL, "CFDictionaryGetValue returns value for existant key '%@'", key);
    }
    
    [key release];
  }

  CFRelease(cfdict);
}

