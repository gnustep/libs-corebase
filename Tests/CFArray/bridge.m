#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#include "CoreFoundation/CFArray.h"
#include "../CFTesting.h"

void testNSonCF(void);
void testCFonNS(void);

int main(void)
{
	testNSonCF();
	testCFonNS();
	return 0;
}

void testNSonCF(void)
{
  CFMutableArrayRef cfarray = CFArrayCreateMutable(NULL, 4, &kCFTypeArrayCallBacks);
  NSMutableArray* nsarray = (NSMutableArray*) cfarray;
  CFIndex count;

  CFArrayAppendValue(cfarray, CFSTR("1"));
  CFArrayAppendValue(cfarray, CFSTR("2"));
  CFArrayAppendValue(cfarray, CFSTR("3"));
  CFArrayAppendValue(cfarray, CFSTR("4"));

  count = CFArrayGetCount(cfarray);

  PASS_CF([nsarray count] == count,
    "-count works on a CFArray");

  [nsarray addObject: (id)CFSTR("5")];

  PASS_CF([nsarray count] == count+1,
    "-addObject: adds a value into a CFArray");

  PASS_CF(CFArrayGetValueAtIndex(cfarray, 1) == [nsarray objectAtIndex: 1],
  "-objectAtIndex: works on a CFArray (#1)");
  PASS_CF(CFArrayGetValueAtIndex(cfarray, 4) == [nsarray objectAtIndex: 4],
  "-objectAtIndex: works on a CFArray (#2)");

  PASS_CF(CFArrayGetFirstIndexOfValue(cfarray, CFRangeMake(0, count+1), @"5")
    == [nsarray indexOfObject: @"5"],
    "-indexOfObject: works on a CFArray");

  [nsarray release];
}

void testCFonNS(void)
{
  NSMutableArray* nsarray = [[NSMutableArray alloc] initWithCapacity: 4];
  CFMutableArrayRef cfarray = (CFMutableArrayRef) nsarray;
  CFIndex count;

  [nsarray addObject: @"1"];
  [nsarray addObject: @"2"];
  [nsarray addObject: @"3"];
  [nsarray addObject: @"4"];

  count = [nsarray count];

  PASS_CF(CFArrayGetCount(cfarray) == count,
    "CFArrayGetCount() works on an NSArray");

  CFArrayAppendValue(cfarray, @"5");

  PASS_CF(CFArrayGetCount(cfarray) == count+1,
    "CFArrayAppendValue() adds a value into an NSArray");

  PASS_CF(CFArrayGetValueAtIndex(cfarray, 1) == [nsarray objectAtIndex: 1],
    "CFArrayGetValueAtIndex() works on an NSArray (#1)");
  PASS_CF(CFArrayGetValueAtIndex(cfarray, 4) == [nsarray objectAtIndex: 4],
    "CFArrayGetValueAtIndex() works on an NSArray (#2)");

  PASS_CF(CFArrayGetFirstIndexOfValue(cfarray, CFRangeMake(0, count+1), @"5")
    == [nsarray indexOfObject: @"5"],
    "CFArrayGetFirstIndexOfValue() works on an NSArray");

  CFRelease(cfarray);
}

