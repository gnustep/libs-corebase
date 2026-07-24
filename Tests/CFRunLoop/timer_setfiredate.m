#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"

static void
fire (CFRunLoopTimerRef t, void *info)
{
}

int main (void)
{
  CFRunLoopTimerRef t = CFRunLoopTimerCreate (NULL, 100.0, 5.0, 0, 0,
    fire, NULL);

  CFRunLoopTimerSetNextFireDate (t, 250.0);
  PASS_CF (CFRunLoopTimerGetNextFireDate (t) == 250.0,
    "Setting the fire date of a timer not in a run loop updates it.");

  CFRelease (t);
  return 0;
}
