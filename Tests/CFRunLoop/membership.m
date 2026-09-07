#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFDate.h>

#include "../CFTesting.h"
#include <pthread.h>

static Boolean
allModesContain (CFRunLoopRef rl, CFStringRef mode)
{
  Boolean found = false;
  CFArrayRef modes = CFRunLoopCopyAllModes (rl);
  if (modes != NULL)
    {
      CFRange r = CFRangeMake (0, CFArrayGetCount (modes));
      found = CFArrayContainsValue (modes, r, mode);
      CFRelease (modes);
    }
  return found;
}

static void *
body (void *unused)
{
  CFRunLoopRef rl;
  CFRunLoopSourceContext sc = {0};
  CFRunLoopSourceRef src;
  CFRunLoopSourceRef csrc;
  CFRunLoopObserverRef obs;
  CFRunLoopTimerRef timer;
  CFStringRef cur;
  CFArrayRef modes;

  rl = CFRunLoopGetCurrent ();
  PASS_CF(rl != NULL, "CFRunLoopGetCurrent returns a run loop.");
  PASS_CF(CFRunLoopGetMain () != NULL, "CFRunLoopGetMain returns a run loop.");

  cur = CFRunLoopCopyCurrentMode (rl);
  PASS_CF(cur == NULL, "CFRunLoopCopyCurrentMode is NULL when not running.");

  modes = CFRunLoopCopyAllModes (rl);
  PASS_CF(modes != NULL, "CFRunLoopCopyAllModes returns a non-NULL array.");
  if (modes != NULL)
    CFRelease (modes);

  src = CFRunLoopSourceCreate (NULL, 0, &sc);
  PASS_CF(src != NULL, "Created a version 0 run loop source.");
  PASS_CF(CFRunLoopContainsSource (rl, src, CFSTR ("testMode1")) == false,
          "Source is not contained before being added.");
  CFRunLoopAddSource (rl, src, CFSTR ("testMode1"));
  PASS_CF(CFRunLoopContainsSource (rl, src, CFSTR ("testMode1")) == true,
          "Source is contained after being added to its mode.");
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopDefaultMode) == false,
          "Source is not contained in a mode it was not added to.");
  PASS_CF(allModesContain (rl, CFSTR ("testMode1")) == true,
          "CFRunLoopCopyAllModes lists a mode after a source is added to it.");
  CFRunLoopRemoveSource (rl, src, CFSTR ("testMode1"));
  PASS_CF(CFRunLoopContainsSource (rl, src, CFSTR ("testMode1")) == false,
          "Source is not contained after being removed.");
  CFRelease (src);

  obs = CFRunLoopObserverCreate (NULL, kCFRunLoopAllActivities, false, 0, NULL,
                                 NULL);
  PASS_CF(obs != NULL, "Created a run loop observer.");
  PASS_CF(CFRunLoopContainsObserver (rl, obs, kCFRunLoopDefaultMode) == false,
          "Observer is not contained before being added.");
  CFRunLoopAddObserver (rl, obs, kCFRunLoopDefaultMode);
  PASS_CF(CFRunLoopContainsObserver (rl, obs, kCFRunLoopDefaultMode) == true,
          "Observer is contained after being added.");
  CFRunLoopRemoveObserver (rl, obs, kCFRunLoopDefaultMode);
  PASS_CF(CFRunLoopContainsObserver (rl, obs, kCFRunLoopDefaultMode) == false,
          "Observer is not contained after being removed.");
  CFRelease (obs);

  timer = CFRunLoopTimerCreate (NULL, CFAbsoluteTimeGetCurrent () + 1000, 0, 0,
                                0, NULL, NULL);
  PASS_CF(timer != NULL, "Created a run loop timer.");
  PASS_CF(CFRunLoopContainsTimer (rl, timer, kCFRunLoopDefaultMode) == false,
          "Timer is not contained before being added.");
  CFRunLoopAddTimer (rl, timer, kCFRunLoopDefaultMode);
  PASS_CF(CFRunLoopContainsTimer (rl, timer, kCFRunLoopDefaultMode) == true,
          "Timer is contained after being added.");
  CFRunLoopRemoveTimer (rl, timer, kCFRunLoopDefaultMode);
  PASS_CF(CFRunLoopContainsTimer (rl, timer, kCFRunLoopDefaultMode) == false,
          "Timer is not contained after being removed.");
  CFRelease (timer);

  CFRunLoopAddCommonMode (rl, CFSTR ("testCommonMode"));
  csrc = CFRunLoopSourceCreate (NULL, 0, &sc);
  CFRunLoopAddSource (rl, csrc, kCFRunLoopCommonModes);
  PASS_CF(CFRunLoopContainsSource (rl, csrc, kCFRunLoopCommonModes) == true,
          "Source added to the common modes is contained in kCFRunLoopCommonModes.");
  PASS_CF(CFRunLoopContainsSource (rl, csrc, kCFRunLoopDefaultMode) == true,
          "Source added to the common modes reaches kCFRunLoopDefaultMode.");
  PASS_CF(CFRunLoopContainsSource (rl, csrc, CFSTR ("testCommonMode")) == true,
          "Source added to the common modes reaches a registered common mode.");
  CFRelease (csrc);

  return NULL;
}

int main (void)
{
  pthread_t th;
  pthread_create (&th, NULL, body, NULL);
  pthread_join (th, NULL);
  return 0;
}
