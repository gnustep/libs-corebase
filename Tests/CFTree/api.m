#include "CoreFoundation/CFTree.h"
#include "../CFTesting.h"

static CFIndex g_count;

static void
counter (const void *info, void *context)
{
  (void)info;
  (void)context;
  g_count++;
}

static CFTreeRef
mk (void)
{
  CFTreeContext c;
  c.version = 0;
  c.info = NULL;
  c.retain = NULL;
  c.release = NULL;
  c.copyDescription = NULL;
  return CFTreeCreate (NULL, &c);
}

int main (void)
{
  CFTreeRef root, a, b, gc;
  const void *kids[2];

  PASS_CF(CFTreeGetTypeID () != 0, "CFTreeGetTypeID is not zero.");

  root = mk ();
  a = mk ();
  b = mk ();
  gc = mk ();
  CFTreeAppendChild (root, a);
  CFTreeAppendChild (root, b);
  CFTreeAppendChild (a, gc);

  PASS_CF(CFGetTypeID (root) == CFTreeGetTypeID (),
    "A tree has the tree type ID.");
  PASS_CF(CFTreeFindRoot (gc) == root,
    "CFTreeFindRoot returns the root of the hierarchy.");
  PASS_CF(CFTreeFindRoot (root) == root,
    "CFTreeFindRoot of the root is itself.");

  g_count = 0;
  CFTreeApplyFunctionToChildren (root, counter, NULL);
  PASS_CF(g_count == 2,
    "CFTreeApplyFunctionToChildren visits each immediate child.");

  CFTreeGetChildren (root, kids);
  PASS_CF(kids[0] == a && kids[1] == b,
    "CFTreeGetChildren returns the children in order.");

  return 0;
}
