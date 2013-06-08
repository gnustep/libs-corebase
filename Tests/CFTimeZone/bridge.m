#import <Foundation/NSTimeZone.h>
#include "CoreFoundation/CFTimeZone.h"
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
  CFTimeZoneRef cftz = CFTimeZoneCopySystem();
  NSTimeZone* nstz = (NSTimeZone*) cftz;
  CFStringRef abbrev;

  PASS_CFEQ(CFTimeZoneGetName(cftz), [nstz name],
    "-name works on a CFTimeZone");
  PASS_CF(CFTimeZoneGetSecondsFromGMT(cftz, CFAbsoluteTimeGetCurrent()) == [nstz secondsFromGMT],
    "-secondsFromGMT works on a CFTimeZone");
  
  // TODO: this test is not "stable" enough
  PASS_CF(CFTimeZoneIsDaylightSavingTime(cftz, CFAbsoluteTimeGetCurrent()) == [nstz isDaylightSavingTime],
    "-isDaylightSavingTime works on a CFTimeZone");
  
  abbrev = CFTimeZoneCopyAbbreviation(cftz, CFAbsoluteTimeGetCurrent());
  PASS_CFEQ(abbrev, [nstz abbreviation],
    "-abbreviation works on a CFTimeZone");
  CFRelease(abbrev);
    
  // TODO: test other functions
  
  [nstz release];
}

void testCFonNS(void)
{
  NSTimeZone* nstz = [NSTimeZone systemTimeZone];
  CFTimeZoneRef cftz = (CFTimeZoneRef) nstz;
  CFStringRef abbrev;
  
  PASS_CFEQ([nstz name], CFTimeZoneGetName(cftz),
    "CFTimeZoneGetName() works on an NSTimeZone");
  PASS_CF(CFTimeZoneGetSecondsFromGMT(cftz, CFAbsoluteTimeGetCurrent()) == [nstz secondsFromGMT],
    "CFTimeZoneGetSecondsFromGMT works on an NSTimeZone");
  
  // TODO: this test is not "stable" enough
  PASS_CF(CFTimeZoneIsDaylightSavingTime(cftz, CFAbsoluteTimeGetCurrent()) == [nstz isDaylightSavingTime],
    "CFTimeZoneIsDaylightSavingTime() works on an NSTimeZone");
  
  abbrev = CFTimeZoneCopyAbbreviation(cftz, CFAbsoluteTimeGetCurrent());
  PASS_CFEQ([nstz abbreviation], abbrev,
    "CFTimeZoneCopyAbbreviation() works on an NSTimeZone");
  CFRelease(abbrev);
}

