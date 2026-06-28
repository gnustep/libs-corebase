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
    /* CFTreeRemove must unlink the node from its parent.  It used to walk
       the wrong list, never unlink, and prematurely finalize the node,
       leaving a dangling pointer in the parent. */
    CFTreeRef r  = CFTreeCreate (NULL, &ctxt);
    CFTreeRef r1 = CFTreeCreate (NULL, &ctxt);
    CFTreeRef r2 = CFTreeCreate (NULL, &ctxt);
    CFTreeRef r3 = CFTreeCreate (NULL, &ctxt);

    CFTreeAppendChild (r, r1);
    CFTreeAppendChild (r, r2);
    CFTreeAppendChild (r, r3);

    CFTreeRemove (r2);
    PASS_CF(CFTreeGetChildCount (r) == 2
      && CFTreeGetNextSibling (r1) == r3
      && CFTreeGetParent (r2) == NULL,
      "CFTreeRemove unlinks a child from its parent.");

    /* Removing the last child updates _lastChild, so a following append
       links onto the right node. */
    CFTreeRemove (r3);
    CFTreeAppendChild (r, r2);
    PASS_CF(CFTreeGetChildCount (r) == 2
      && CFTreeGetChildAtIndex (r, 1) == r2,
      "Append after removing the last child works.");

    CFRelease (r);
    CFRelease (r1);
    CFRelease (r2);
    CFRelease (r3);
  }

  /* Release the parent before its children: CFTreeAppendChild() does not
     retain the child, so the children must outlive the tree that links
     them. */
  CFRelease (tree);
  CFRelease (child1);
  CFRelease (child2);
  CFRelease (child3);

  return 0;
}
