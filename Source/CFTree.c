/* CFTree.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTree.h"

#include <string.h>



static CFTypeID _kCFTreeTypeID = 0;

struct __CFTree
{
  CFRuntimeBase parent;
  CFTreeContext _context;
  CFTreeRef     _parent;
  CFTreeRef     _nextSibling;
  CFTreeRef     _firstChild;
  CFTreeRef     _lastChild;
};

static CFTreeContext _kCFNullTreeContext =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};



static void
CFTreeFinalize (CFTypeRef cf)
{
  CFTreeRef tree = (CFTreeRef)cf;
  CFTreeRef child;
  CFTreeRef tmp;
  CFTreeReleaseCallBack release;
  
  child = tree->_firstChild;
  while (child)
    {
      tmp = child->_nextSibling;
      CFTreeFinalize (child); /* No need to go through CFRelease(). */
      child = tmp;
    }
  
  release = tree->_context.release;
  if (release)
    release (tree);
}

static CFRuntimeClass CFTreeClass =
{
  0,
  "CFTree",
  NULL,
  NULL,
  CFTreeFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFTreeInitialize (void)
{
  _kCFTreeTypeID = _CFRuntimeRegisterClass (&CFTreeClass);
}



CFTypeID
CFTreeGetTypeID (void)
{
  return _kCFTreeTypeID;
}

#define CFTREE_SIZE (sizeof(struct __CFTree) - sizeof(CFRuntimeBase))

CFTreeRef
CFTreeCreate (CFAllocatorRef allocator, const CFTreeContext *context)
{
  CFTreeRef new;
  
  new = (CFTreeRef)_CFRuntimeCreateInstance (allocator, _kCFTreeTypeID,
    CFTREE_SIZE, 0);
  if (new)
    {
      if (context == NULL)
        context = &_kCFNullTreeContext;
      memcpy (&new->_context, context, sizeof(CFTreeContext));
    }
  
  return new;
}

void
CFTreeAppendChild (CFTreeRef tree, CFTreeRef newChild)
{
  CFTreeRetainCallBack retain = newChild->_context.retain;
  
  newChild->_parent = tree;
  if (retain)
    retain (newChild);
  
  if (tree->_firstChild == NULL)
    {
      tree->_firstChild = newChild;
      tree->_lastChild = newChild;
    }
  else
    {
      tree->_lastChild->_nextSibling = newChild;
      tree->_lastChild = newChild;
    }
}

void
CFTreeInsertSibling (CFTreeRef tree, CFTreeRef newSibling)
{
  CFTreeRef parent = tree->_parent;
  
  if (parent != NULL && newSibling->_parent == NULL)
    {
      CFTreeRetainCallBack retain = newSibling->_context.retain;
      
      newSibling->_parent = parent;
      if (retain)
        retain (newSibling);
      
      if (parent->_lastChild == tree)
        parent->_lastChild = newSibling;
      else
        newSibling->_nextSibling = tree->_nextSibling;
      
      tree->_nextSibling = newSibling;
    }
}

void
CFTreeRemoveAllChildren (CFTreeRef tree)
{
  CFTreeRef child;
  CFTreeRef tmp;
  
  child = tree->_firstChild;
  while (child)
    {
      tmp = child->_nextSibling;
      CFTreeFinalize (child);
      child = tmp;
    }
  
  tree->_firstChild = NULL;
  tree->_lastChild = NULL;
}

void
CFTreePrependChild (CFTreeRef tree, CFTreeRef newChild)
{
  newChild->_parent = tree;
  newChild->_nextSibling = tree->_firstChild;
  tree->_firstChild = newChild;
  if (tree->_lastChild == NULL)
    tree->_lastChild = NULL;
}

void
CFTreeRemove (CFTreeRef tree)
{
  CFTreeRef parent = tree->_parent;

  if (parent == NULL)
    return;

  /* Unlink the tree from its parent's list of children. */
  if (parent->_firstChild == tree)
    {
      parent->_firstChild = tree->_nextSibling;
    }
  else
    {
      CFTreeRef previousSibling = parent->_firstChild;

      while (previousSibling != NULL
             && previousSibling->_nextSibling != tree)
        previousSibling = previousSibling->_nextSibling;
      if (previousSibling != NULL)
        previousSibling->_nextSibling = tree->_nextSibling;
    }

  /* Fix up the parent's last-child pointer if we removed the last child. */
  if (parent->_lastChild == tree)
    {
      CFTreeRef last = parent->_firstChild;

      while (last != NULL && last->_nextSibling != NULL)
        last = last->_nextSibling;
      parent->_lastChild = last;
    }

  tree->_parent = NULL;
  tree->_nextSibling = NULL;
}

void
CFTreeSetContext (CFTreeRef tree, const CFTreeContext *context)
{
  if (context == NULL)
    context = &_kCFNullTreeContext;
  memcpy (&tree->_context, context, sizeof(CFTreeContext));
}

void
CFTreeSortChildren (CFTreeRef tree, CFComparatorFunction comp, void *context)
{
  /* FIXME */
}

CFTreeRef
CFTreeFindRoot (CFTreeRef tree)
{
  while (tree->_parent != NULL)
    tree = tree->_parent;
  
  return tree;
}

CFTreeRef
CFTreeGetChildAtIndex (CFTreeRef tree, CFIndex idx)
{
  CFIndex j;
  CFTreeRef child;
  
  j = 0;
  child = tree->_firstChild;
  while (j++ < idx)
    child = child->_nextSibling;
  
  return child;
}

CFIndex
CFTreeGetChildCount (CFTreeRef tree)
{
  CFIndex count;
  CFTreeRef child;
  
  count = 0;
  child = tree->_firstChild;
  while (child)
    {
      child = child->_nextSibling;
      ++count;
    }
  
  return count;
}

void
CFTreeGetChildren (CFTreeRef tree, CFTreeRef *children)
{
  CFIndex idx;
  CFTreeRef child;
  
  idx = 0;
  child = tree->_firstChild;
  while (child)
    {
      children[idx++] = child;
      child = child->_nextSibling;
    }
}

void
CFTreeGetContext (CFTreeRef tree, CFTreeContext *context)
{
  memcpy (context, &tree->_context, sizeof(CFTreeContext));
}

CFTreeRef
CFTreeGetFirstChild (CFTreeRef tree)
{
  return tree->_firstChild;
}

CFTreeRef
CFTreeGetNextSibling (CFTreeRef tree)
{
  return tree->_nextSibling;
}

CFTreeRef
CFTreeGetParent (CFTreeRef tree)
{
  return tree->_parent;
}

void
CFTreeApplyFunctionToChildren (CFTreeRef tree, CFTreeApplierFunction applier,
  void *context)
{
  CFTreeRef child;
  
  child = tree->_firstChild;
  while (child)
    {
      applier (child, context);
      child = child->_nextSibling;
    }
}

