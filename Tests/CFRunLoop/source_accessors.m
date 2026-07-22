#include "CoreFoundation/CFRunLoop.h"
#include "../CFTesting.h"
#include <string.h>

static void
perform (void *info)
{
}

int main (void)
{
  int marker = 99;
  CFRunLoopSourceContext ctx;
  CFRunLoopSourceContext got;
  CFRunLoopSourceRef src;

  memset (&ctx, 0, sizeof (ctx));
  ctx.version = 0;
  ctx.info = &marker;
  ctx.perform = perform;
  src = CFRunLoopSourceCreate (NULL, 4, &ctx);

  PASS_CF (src != NULL, "A source is created.");
  PASS_CF (CFGetTypeID (src) == CFRunLoopSourceGetTypeID (),
    "A source has the source type ID.");
  PASS_CF (CFRunLoopSourceGetOrder (src) == 4, "The order round-trips.");
  PASS_CF (CFRunLoopSourceIsValid (src) == true, "A new source is valid.");

  memset (&got, 0, sizeof (got));
  CFRunLoopSourceGetContext (src, &got);
  PASS_CF (got.info == &marker, "The context info pointer round-trips.");
  PASS_CF (got.version == 0, "The context version round-trips.");

  CFRunLoopSourceInvalidate (src);
  PASS_CF (CFRunLoopSourceIsValid (src) == false,
    "An invalidated source is no longer valid.");

  return 0;
}
