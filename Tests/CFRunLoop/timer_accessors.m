#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"
#include <string.h>

static void
fire (CFRunLoopTimerRef t, void *info)
{
}

int main (void)
{
  int marker = 7;
  CFRunLoopTimerContext ctx = { 0, &marker, NULL, NULL, NULL };
  CFRunLoopTimerContext got;
  CFRunLoopTimerRef t = CFRunLoopTimerCreate (NULL, 100.0, 5.0, 0, 3,
    fire, &ctx);
  CFRunLoopTimerRef once = CFRunLoopTimerCreate (NULL, 100.0, 0.0, 0, 0,
    fire, NULL);

  PASS_CF (t != NULL, "A timer is created.");
  PASS_CF (CFGetTypeID (t) == CFRunLoopTimerGetTypeID (),
    "A timer has the timer type ID.");
  PASS_CF (CFRunLoopTimerGetNextFireDate (t) == 100.0,
    "The fire date round-trips.");
  PASS_CF (CFRunLoopTimerGetInterval (t) == 5.0, "The interval round-trips.");
  PASS_CF (CFRunLoopTimerGetOrder (t) == 3, "The order round-trips.");
  PASS_CF (CFRunLoopTimerDoesRepeat (t) == true,
    "A timer with a positive interval repeats.");
  PASS_CF (CFRunLoopTimerDoesRepeat (once) == false,
    "A timer with a zero interval does not repeat.");
  PASS_CF (CFRunLoopTimerIsValid (t) == true, "A new timer is valid.");

  memset (&got, 0, sizeof (got));
  CFRunLoopTimerGetContext (t, &got);
  PASS_CF (got.info == &marker, "The context info pointer round-trips.");

  CFRunLoopTimerInvalidate (t);
  PASS_CF (CFRunLoopTimerIsValid (t) == false,
    "An invalidated timer is no longer valid.");

  return 0;
}
