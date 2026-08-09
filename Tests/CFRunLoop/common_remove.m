#include <CoreFoundation/CFRunLoop.h>

#include "../CFTesting.h"
#include <pthread.h>

static void *
body (void *unused)
{
  CFRunLoopRef rl;
  CFRunLoopSourceContext sc = {0};
  CFRunLoopSourceRef src;

  rl = CFRunLoopGetCurrent ();
  CFRunLoopAddCommonMode (rl, CFSTR ("testCommonMode"));

  src = CFRunLoopSourceCreate (NULL, 0, &sc);
  CFRunLoopAddSource (rl, src, kCFRunLoopCommonModes);
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopCommonModes) == true,
          "Source is in the common modes after being added.");
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopDefaultMode) == true,
          "Source reaches the default mode after being added to common modes.");

  CFRunLoopRemoveSource (rl, src, kCFRunLoopCommonModes);
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopCommonModes) == false,
          "Source is not in the common modes after being removed.");
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopDefaultMode) == false,
          "Source no longer reaches the default mode after common-modes removal.");
  PASS_CF(CFRunLoopContainsSource (rl, src, CFSTR ("testCommonMode")) == false,
          "Source no longer reaches a registered common mode after removal.");
  CFRelease (src);

  return NULL;
}

int main (void)
{
  pthread_t th;
  pthread_create (&th, NULL, body, NULL);
  pthread_join (th, NULL);
  return 0;
}
