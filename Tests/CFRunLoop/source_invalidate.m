#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"
#include <string.h>

static void perform (void *info)
{
}

int main (void)
{
  CFRunLoopRef rl;
  CFRunLoopSourceContext ctx;
  CFRunLoopSourceRef src;

  rl = CFRunLoopGetCurrent ();
  memset (&ctx, 0, sizeof (ctx));
  ctx.perform = perform;
  src = CFRunLoopSourceCreate (NULL, 0, &ctx);
  CFRunLoopAddSource (rl, src, kCFRunLoopDefaultMode);
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopDefaultMode) == true,
    "Source is in the run loop after being added.");

  /* No timers are registered. */
  CFRunLoopSourceInvalidate (src);
  PASS_CF(CFRunLoopContainsSource (rl, src, kCFRunLoopDefaultMode) == false,
    "Invalidated source is removed from the run loop.");

  CFRelease (src);
  return 0;
}
