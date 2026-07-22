#include "CoreFoundation/CFBase.h"
#include "../CFTesting.h"
#include <stdlib.h>
#include <string.h>

static void *
my_alloc (CFIndex size, CFOptionFlags hint, void *info)
{
  ++(*(int *) info);
  return malloc (size);
}

static void
my_dealloc (void *ptr, void *info)
{
  free (ptr);
}

int main (void)
{
  int counter = 0;
  void *p;
  CFAllocatorContext ctx;
  CFAllocatorContext got;
  CFAllocatorRef myAlloc;

  PASS_CF (CFAllocatorGetTypeID () != 0,
    "CFAllocatorGetTypeID is registered.");
  PASS_CF (CFGetTypeID (kCFAllocatorSystemDefault) == CFAllocatorGetTypeID (),
    "The system allocator has the allocator type ID.");
  PASS_CF (CFAllocatorGetDefault () != NULL,
    "There is a default allocator.");
  PASS_CFEQ (CFCopyTypeIDDescription (CFAllocatorGetTypeID ()),
    CFSTR ("CFAllocator"), "The allocator type is described as CFAllocator.");

  PASS_CF (CFAllocatorAllocate (kCFAllocatorNull, 16, 0) == NULL,
    "The null allocator allocates nothing.");

  p = CFAllocatorAllocate (kCFAllocatorSystemDefault, 16, 0);
  PASS_CF (p != NULL, "The system allocator returns memory.");
  CFAllocatorDeallocate (kCFAllocatorSystemDefault, p);

  memset (&ctx, 0, sizeof (ctx));
  ctx.info = &counter;
  ctx.allocate = my_alloc;
  ctx.deallocate = my_dealloc;
  myAlloc = CFAllocatorCreate (kCFAllocatorDefault, &ctx);
  PASS_CF (myAlloc != NULL, "A custom allocator is created.");

  p = CFAllocatorAllocate (myAlloc, 8, 0);
  PASS_CF (p != NULL && counter == 1,
    "Allocation goes through the custom callback.");
  CFAllocatorDeallocate (myAlloc, p);

  CFAllocatorGetContext (myAlloc, &got);
  PASS_CF (got.info == &counter,
    "CFAllocatorGetContext returns the stored info pointer.");

  CFRelease (myAlloc);
  return 0;
}
