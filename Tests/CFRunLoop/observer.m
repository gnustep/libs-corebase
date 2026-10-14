#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"
#include <string.h>

static void
observe (CFRunLoopObserverRef o, CFRunLoopActivity a, void *info)
{
}

int main (void)
{
  int marker = 42;
  CFRunLoopObserverContext ctx = { 0, &marker, NULL, NULL, NULL };
  CFRunLoopObserverContext got;
  CFOptionFlags acts = kCFRunLoopBeforeWaiting | kCFRunLoopExit;
  CFRunLoopObserverRef obs = CFRunLoopObserverCreate (NULL, acts, true, 5,
    observe, &ctx);

  PASS_CF (obs != NULL, "An observer is created.");
  PASS_CF (CFGetTypeID (obs) == CFRunLoopObserverGetTypeID (),
    "An observer has the observer type ID.");
  PASS_CF (CFRunLoopObserverGetActivities (obs) == acts,
    "The observed activities round-trip.");
  PASS_CF (CFRunLoopObserverDoesRepeat (obs) == true,
    "The repeat flag round-trips.");
  PASS_CF (CFRunLoopObserverGetOrder (obs) == 5, "The order round-trips.");

  memset (&got, 0, sizeof (got));
  CFRunLoopObserverGetContext (obs, &got);
  PASS_CF (got.info == &marker, "The context info pointer round-trips.");

  CFRelease (obs);
  return 0;
}
