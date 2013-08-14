#import <Foundation/NSLocale.h>
#import <Foundation/NSString.h>
#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

void testCFonNS(void);
void testNSonCF(void);

int main(void)
{
	testCFonNS();
	testNSonCF();
	return 0;
}

void testCFonNS(void)
{
	NSLocale* locale = [NSLocale currentLocale];
	CFLocaleRef cfRef = (CFLocaleRef) locale;

	PASS_CF(CFStringCompare(CFLocaleGetIdentifier(cfRef), (CFStringRef) [locale localeIdentifier], 0) == 0,
			"CFLocaleGetIdentifier works");
	
	PASS_CF(CFStringCompare(CFLocaleGetValue(cfRef, kCFLocaleCurrencySymbol),
				(CFStringRef) [locale objectForKey: NSLocaleCurrencySymbol], 0) == 0,
			"CFLocaleGetValue works");
	PASS_CF(CFStringCompare(CFLocaleCopyDisplayNameForPropertyValue(cfRef, kCFLocaleIdentifier, CFSTR("fr_FR")),
				(CFStringRef) [locale displayNameForKey: NSLocaleIdentifier value: @"fr_FR"], 0) == 0,
			"CFLocaleCopyDisplayNameForPropertyValue works");
}

void testNSonCF(void)
{
  CFLocaleRef cfRef = CFLocaleCopyCurrent();
  NSLocale* locale = (NSLocale*)cfRef;

  PASS_CF([[(NSLocale*)cfRef localeIdentifier] isEqual:[locale localeIdentifier]] == YES, 
    "-localeIdentifier works");
  PASS_CF([[(NSLocale*)cfRef objectForKey:NSLocaleCurrencySymbol] isEqual:[locale objectForKey:NSLocaleCurrencySymbol]] == YES, 
    "-objectForKey: works");
  PASS_CF([[(NSLocale*)cfRef displayNameForKey:NSLocaleIdentifier value:@"fr_FR"] isEqual:[locale displayNameForKey:NSLocaleIdentifier value:@"fr_FR"]] == YES, 
    "-displayNameForKey:value: works"); 

  CFRelease(cfRef);
}

