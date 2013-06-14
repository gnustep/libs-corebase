#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#include "CoreFoundation/CFDictionary.h"
#include "../CFTesting.h"

void testCFonNS(void);
void testNSonCF(void);

int main (void)
{
  testCFonNS();
  testNSonCF();
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

