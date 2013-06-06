#import <Foundation/NSString.h>
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

void refcounting_tests(void);
void basic_ops(void);

int main (void)
{
	refcounting_tests();
	basic_ops();
	return 0;
}

void refcounting_tests(void)
{
  NSString *str;
  CFStringRef str2;
  
  str = (NSString*)CFSTR("test");
  PASS_CF ([str retainCount] == UINT_MAX,
    "Constant string has maximum retain count.");
  [str release];
  PASS_CF ([str retainCount] == UINT_MAX,
    "Release has no effect of constant string.");
  
  str2 = CFStringCreateWithBytes (NULL, "Test2", 6, kCFStringEncodingASCII, 0);
  [str2 retain];
  PASS_CF ([str2 retainCount] == 2,
    "CFStringRef instances accept -retain method calls");
}

void basic_ops(void)
{
	NSString* str = [NSString stringWithString: CFSTR("Hello world!")];

	PASS_CF([str isEqual: @"Hello world!"],
		"NSString stringWithString accepts a CFStringRef");

	PASS_CF(CFStringCompare((CFStringRef) str, CFSTR("Hello world!"), 0) == 0,
		"CFStringCompare accepts an NSString");

	[str release];
}

