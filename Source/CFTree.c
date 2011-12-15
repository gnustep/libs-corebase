/* CFTree.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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



static CFTypeID _kCFTreeTypeID = 0;

struct __CFTree
{
  CFRuntimeBase _parent;
};

static void
CFTreeFinalize (CFTypeRef cf)
{
  
}

static CFStringRef
CFTreeCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFSTR("");
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
  CFTreeCopyFormattingDesc,
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

CFTreeRef
CFTreeCreate (CFTreeRef tree, const CFTreeContext *context)
{
  return NULL;
}

void
CFTreeAppendChild (CFTreeRef tree, CFTreeRef newChild)
{
  
}

void
CFTreeInsertSibling (CFTreeRef tree, CFTreeRef newSibling)
{
  
}

void
CFTreeRemoveAllChildren (CFTreeRef tree)
{
  
}

void
CFTreePrependChild (CFTreeRef tree, CFTreeRef newChild)
{
  
}

void
CFTreeRemove (CFTreeRef tree)
{
  
}

void
CFTreeSetContext (CFTreeRef tree, const CFTreeContext *context)
{
  
}

void
CFTreeSortChildren (CFTreeRef tree, CFComparatorFunction comp, void *context)
{
  
}

CFTreeRef
CFTreeFindRoot (CFTreeRef tree)
{
  return NULL;
}

CFTreeRef
CFTreeGetChildAtIndex (CFTreeRef tree, CFIndex idx)
{
  return NULL;
}

CFIndex
CFTreeGetChildCount (CFTreeRef tree)
{
  return 0;
}

void
CFTreeGetChildren (CFTreeRef tree, CFTreeRef *children)
{
  
}

void
CFTreeGetContext (CFTreeRef tree, CFTreeContext *context)
{
  
}

CFTreeRef
CFTreeGetFirstChild (CFTreeRef tree)
{
  return NULL;
}

CFTreeRef
CFTreeGetNextSibling (CFTreeRef tree)
{
  return NULL;
}

CFTreeRef
CFTreeGetParent (CFTreeRef tree)
{
  return NULL;
}

void
CFTreeApplyFunctionToChildren (CFTreeRef tree, CFTreeApplierFunction applier,
  void *context)
{
  
}
