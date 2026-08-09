#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"

static void
observe (CFRunLoopObserverRef o, CFRunLoopActivity a, void *info)
{
}

int main (void)
{
  CFRunLoopObserverRef obs = CFRunLoopObserverCreate (NULL,
    kCFRunLoopBeforeWaiting, false, 0, observe, NULL);

  PASS_CF (CFRunLoopObserverIsValid (obs) == true, "A new observer is valid.");

  CFRunLoopObserverInvalidate (obs);
  PASS_CF (CFRunLoopObserverIsValid (obs) == false,
    "An invalidated observer is no longer valid.");

  CFRelease (obs);
  return 0;
}
