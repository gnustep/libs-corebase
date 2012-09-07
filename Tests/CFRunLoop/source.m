#include <CoreFoundation/CFRunLoop.h>

#include "../CFTesting.h"

void perform1 (void *info)
{
  SInt32 ret;
  CFIndex *i = (CFIndex*)info;
  ret = CFRunLoopRunInMode (CFSTR("another_mode"), 0.0, true);
  PASS_CF(ret == kCFRunLoopRunFinished, "Run loop run in 'another_mode' returned"
       " '%d'", ret);
  (*i)++;
}

void perform2 (void *info)
{
  CFIndex *i = (CFIndex*)info;
  (*i)++;
}



int main (void)
{
  CFRunLoopRef rl;
  CFRunLoopSourceRef rls1;
  CFRunLoopSourceRef rls2;
  CFIndex i1 = 0;
  CFIndex i2 = 0;
  CFRunLoopSourceContext c1 = { 0 };
  CFRunLoopSourceContext c2 = { 0 };
  SInt32 ret;
  
  rl = CFRunLoopGetMain ();
  PASS_CF(rl != NULL, "Got main run loop.");
  
  c1.info = &i1;
  c1.perform = perform1;
  c2.info = &i2;
  c2.perform = perform2;
  rls1 = CFRunLoopSourceCreate (NULL, 0, &c1);
  PASS_CF(rls1 != NULL, "First run loop source create.");
  rls2 = CFRunLoopSourceCreate (NULL, 0, &c2);
  PASS_CF(rls2 != NULL, "Second run loop source create.");
  
  CFRunLoopAddSource (rl, rls1, kCFRunLoopDefaultMode);
  CFRunLoopAddSource (rl, rls2, kCFRunLoopDefaultMode);
  CFRunLoopAddSource (rl, rls2, CFSTR("another_mode"));
  
  ret = CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.0, true);
  PASS_CF(ret == kCFRunLoopRunHandledSource, "Run loop handled sources.  Exit"
          " code '%d'.", ret);
  PASS_CF(i1 == 1, "First run loop source was performed %d time(s).", (int)i1);
  PASS_CF(i2 == 2, "Second run loop source was performed %d time(s).", (int)i2);
  
  CFRunLoopRemoveSource (rl, rls2, CFSTR("another_mode"));
  CFRunLoopSourceInvalidate (rls2);
  
  ret = CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.0, true);
  
  CFRelease (rls1);
  CFRelease (rls2);
  CFRelease (rl);
  
  return 0;
}
