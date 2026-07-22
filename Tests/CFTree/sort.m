#include "CoreFoundation/CFTree.h"
#include "../CFTesting.h"

static CFTreeRef
mkc (int *val)
{
  CFTreeContext c;
  c.version = 0;
  c.info = val;
  c.retain = NULL;
  c.release = NULL;
  c.copyDescription = NULL;
  return CFTreeCreate (NULL, &c);
}

static CFComparisonResult
cmpInt (const void *a, const void *b, void *context)
{
  CFTreeContext ca, cb;
  int x, y;
  (void)context;
  CFTreeGetContext ((CFTreeRef) a, &ca);
  CFTreeGetContext ((CFTreeRef) b, &cb);
  x = *(const int *) ca.info;
  y = *(const int *) cb.info;
  if (x < y)
    return kCFCompareLessThan;
  if (x > y)
    return kCFCompareGreaterThan;
  return kCFCompareEqualTo;
}

int main (void)
{
  int v3 = 3, v1 = 1, v2 = 2;
  CFTreeContext rc;
  CFTreeRef root, c3, c1, c2;
  const void *kids[3];

  rc.version = 0;
  rc.info = NULL;
  rc.retain = NULL;
  rc.release = NULL;
  rc.copyDescription = NULL;
  root = CFTreeCreate (NULL, &rc);

  c3 = mkc (&v3);
  c1 = mkc (&v1);
  c2 = mkc (&v2);
  CFTreeAppendChild (root, c3);
  CFTreeAppendChild (root, c1);
  CFTreeAppendChild (root, c2);

  CFTreeSortChildren (root, cmpInt, NULL);
  CFTreeGetChildren (root, kids);
  PASS_CF(kids[0] == c1 && kids[1] == c2 && kids[2] == c3,
    "CFTreeSortChildren orders the children by the comparator.");

  return 0;
}
