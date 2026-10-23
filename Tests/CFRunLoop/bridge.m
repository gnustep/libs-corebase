#import <Foundation/NSArray.h>
#import <Foundation/NSRunLoop.h>
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"

void testNSonCF(void);
void testCFonNS(void);

static Boolean
containsMode (CFArrayRef modes, CFStringRef mode)
{
  return CFArrayContainsValue (modes, CFRangeMake (0, CFArrayGetCount (modes)),
                               mode);
}

int main(void)
{
  testNSonCF();
  testCFonNS();
  return 0;
}

void testNSonCF(void)
{
  CFRunLoopRef cfrl = CFRunLoopGetCurrent();
  NSRunLoop *nsrl = (NSRunLoop*) cfrl;

  testHopeful = true;
  PASS_CF(cfrl != NULL, "CFRunLoopGetCurrent() returns a run loop");
  testHopeful = false;
  if (cfrl == NULL)
    return;

  [nsrl addCommonMode: (NSString*) CFSTR("ns_on_cf")];
  PASS_CF(containsMode((CFArrayRef) [nsrl allModes], CFSTR("ns_on_cf")),
    "-addCommonMode: and -allModes work on a CFRunLoop");
  PASS_CF([nsrl currentMode] == nil,
    "-currentMode works on a CFRunLoop");
}

void testCFonNS(void)
{
  NSRunLoop *nsrl = [NSRunLoop currentRunLoop];
  CFRunLoopRef cfrl = [nsrl getCFRunLoop];
  CFArrayRef modes;

  CFRunLoopAddCommonMode(cfrl, CFSTR("cf_on_ns"));
  modes = CFRunLoopCopyAllModes(cfrl);
  PASS_CF(containsMode(modes, CFSTR("cf_on_ns")),
    "CFRunLoopAddCommonMode() and CFRunLoopCopyAllModes() work on an NSRunLoop");
  CFRelease(modes);

  PASS_CF(CFRunLoopCopyCurrentMode(cfrl) == NULL,
    "CFRunLoopCopyCurrentMode() works on an NSRunLoop");
}
