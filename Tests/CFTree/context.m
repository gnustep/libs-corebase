#include <CoreFoundation/CFTree.h>
#include <CoreFoundation/CFString.h>

#include "../CFTesting.h"

int main (void)
{
  CFStringRef info;
  CFTreeContext ctx;
  CFTreeContext out;
  CFTreeRef tree;

  info = CFStringCreateWithCString (NULL, "node", kCFStringEncodingUTF8);

  ctx.version = 0;
  ctx.info = (void *)info;
  ctx.retain = CFRetain;
  ctx.release = CFRelease;
  ctx.copyDescription = CFCopyDescription;

  tree = CFTreeCreate (NULL, &ctx);
  PASS_CF(tree != NULL, "Created a tree with a retaining context.");

  CFTreeGetContext (tree, &out);
  PASS_CF(out.info == (void *)info, "CFTreeGetContext returns the info.");
  PASS_CF(out.retain == CFRetain && out.release == CFRelease,
          "CFTreeGetContext returns the context callbacks.");

  CFRelease (tree);
  PASS_CF(CFStringGetLength (info) == 4,
          "The context info outlives the tree that retained it.");

  CFRelease (info);

  return 0;
}
