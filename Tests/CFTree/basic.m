#include "CoreFoundation/CFTree.h"
#include "../CFTesting.h"

int main (void)
{
  CFTreeRef tree;
  CFTreeRef child1;
  CFTreeRef child2;
  CFTreeRef child3;
  CFTreeContext ctxt;
  
  ctxt.version = 0;
  ctxt.info = NULL;
  ctxt.retain = NULL;
  ctxt.release = NULL;
  ctxt.copyDescription = NULL;
  
  tree = CFTreeCreate (NULL, &ctxt);
  child1 = CFTreeCreate (NULL, &ctxt);
  child2 = CFTreeCreate (NULL, &ctxt);
  child3 = CFTreeCreate (NULL, &ctxt);
  
  CFTreeAppendChild (tree, child2);
  CFTreePrependChild (tree, child1);
  CFTreeInsertSibling (child2, child3);
  
  PASS_CF(CFTreeGetChildCount (tree) == 3, "Tree has three children.");
  PASS_CF(CFTreeGetParent (child3) == tree, "Parent is the original tree object.");
  PASS_CF(CFTreeGetFirstChild (tree) == child1, "First child is child1.");
  PASS_CF(CFTreeGetNextSibling (child1) == child2,
    "Next sibling for child1 is child2.");
  PASS_CF(CFTreeGetChildAtIndex (tree, 2) == child3, "Child3 is at index 2");

  {
    /* Prepending to an empty tree must set _lastChild, otherwise the next
       append dereferences NULL.  An out-of-range index must return NULL
       rather than walking off the end of the child list. */
    CFTreeRef t2 = CFTreeCreate (NULL, &ctxt);
    CFTreeRef a = CFTreeCreate (NULL, &ctxt);
    CFTreeRef b = CFTreeCreate (NULL, &ctxt);

    CFTreePrependChild (t2, a);
    CFTreeAppendChild (t2, b);
    PASS_CF(CFTreeGetChildCount (t2) == 2
      && CFTreeGetChildAtIndex (t2, 1) == b,
      "Append after prepend-to-empty works.");
    PASS_CF(CFTreeGetChildAtIndex (t2, 5) == NULL,
      "Out-of-range child index returns NULL.");

    CFRelease (a);
    CFRelease (b);
    CFRelease (t2);
  }

  CFRelease (child1);
  CFRelease (child2);
  CFRelease (child3);
  CFRelease (tree);
  
  return 0;
}
