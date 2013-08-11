/*
 * This file contains tests focused on using CF functions
 * with non-CF objects
 */

#import <Foundation/NSString.h>
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

void testCaseTransformations(void);
void testWhitespace(void);
void testPointerAccess(void);
void testAppend(void);
void testPad(void);
void testGetBytes(void);
void testEquals(void);

int main(void)
{
	testEquals();
	testCaseTransformations();
	testWhitespace();
	testPointerAccess();
	testAppend();
	testPad();
	testGetBytes();
	return 0;
}

void testEquals(void)
{
	NSString* str = @"Hello";
	CFStringRef cstr = CFSTR("Hello");
	
	PASS_CF([str isEqual: cstr], "isEqual works #1");
	PASS_CF([cstr isEqual: str], "isEqual works #2");
	PASS_CF([str hash] == [cstr hash], "hashes are equal");
}

void testCaseTransformations(void)
{
	NSMutableString* str = [NSMutableString stringWithCapacity: 0];
	[str setString: @"hello world"];

	CFStringUppercase((CFMutableStringRef) str, /*CFLocaleGetSystem()*/ NULL);

	PASS_CF([str isEqual: @"HELLO WORLD"], "CFStringUppercase works");
	[str release];
}

void testWhitespace(void)
{
	NSMutableString* str = [NSMutableString stringWithCapacity: 0];
	[str setString: @"\nHello\tworld \t"];

	CFStringTrimWhitespace((CFMutableStringRef) str);

	PASS_CF([str isEqual: @"Hello\tworld"], "CFStringTrimWhitespace works");
	[str release];
}

void testPointerAccess(void)
{
	CFStringRef str = (CFStringRef) @"Hello world!";
	PASS_CF(strcmp(CFStringGetCStringPtr(str, kCFStringEncodingASCII), "Hello world!") == 0,
		"CFStringGetCStringPtr works on a Foundation object");
}

void testAppend(void)
{
	const UniChar trivialUtf16[] = { ' ', 'w', 'o', 'r', 'l', 'd', '!', 0 };
	NSMutableString* str = [NSMutableString stringWithCapacity: 0];
	[str setString: @"Hello"];

	CFStringAppendCString((CFMutableStringRef) str, " world!", kCFStringEncodingASCII);
	PASS_CF([str isEqual: @"Hello world!"], "CFStringAppendCString works");

	[str setString: @"Hello"];
	CFStringAppendCharacters((CFMutableStringRef) str, trivialUtf16, 7);
	PASS_CF([str isEqual: @"Hello world!"], "CFStringAppendCharacters works");

	[str release];
}

void testPad(void)
{
	NSMutableString* str = [NSMutableString stringWithCapacity: 0];
	[str setString: (NSString*) CFSTR("abcdef")];

	CFStringPad((CFMutableStringRef) str, CFSTR("123"), 9, 1);
	PASS_CF([str isEqual: @"abcdef231"], "CFStringPad works");

	[str release];
}


void testGetBytes(void)
{
	NSString* str = @"Hello world!";
	const UniChar uniStr[] = { 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l' };
	UniChar buffer[10];
	CFIndex usedLen = -1;
	
	CFStringGetBytes((CFStringRef) str, CFRangeMake(1, 9), kCFStringEncodingUnicode,
			0, false, (UInt8*) buffer, 9, &usedLen);

	testHopeful = true;
	PASS_CF(memcmp(buffer, uniStr, sizeof(UniChar)*9) == 0, "CFStringGetBytes works in simple case");

	// TODO: lossByte, ext representation...
	
	PASS_CF(usedLen == 9, "CFStringGetBytes returns correct usedLen");
	testHopeful = false;
}

