/* CFBinaryHeap.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFBinaryHeap.h"

#include <string.h>

static CFTypeID _kCFBinaryHeapTypeID = 0;

struct __CFBinaryHeap
{
  CFRuntimeBase _parent;
  CFBinaryHeapCompareContext _context;
  const CFBinaryHeapCallBacks *_callBacks;
  CFIndex _count;
  CFIndex _capacity;
  const void **_values;
};

static void
CFBinaryHeapFinalize (CFTypeRef cf)
{
  CFBinaryHeapRef heap = (CFBinaryHeapRef)cf;
  CFAllocatorRef allocator = CFGetAllocator(heap);
  
  if (heap->_context.info && heap->_context.release)
    heap->_context.release (heap->_context.info);
  
  if (heap->_callBacks->release)
    {
      const void *cur = heap->_values;
      const void *end = cur + heap->_count;
      while (cur < end)
        heap->_callBacks->release (allocator, cur++);
    }
  CFAllocatorDeallocate (allocator, (void*)heap->_values);
}

static CFRuntimeClass CFBinaryHeapClass =
{
  0,
  "CFBinaryHeap",
  NULL,
  NULL,
  CFBinaryHeapFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFBinaryHeapInitialize (void)
{
  _kCFBinaryHeapTypeID = _CFRuntimeRegisterClass (&CFBinaryHeapClass);
}



CFTypeID
CFBinaryHeapGetTypeID (void)
{
  return _kCFBinaryHeapTypeID;
}

#define CFBINARYHEAP_SIZE sizeof(struct __CFBinaryHeap) - sizeof(CFRuntimeBase)
#define _kCFBinaryHeapMinimumSize 16

static const CFBinaryHeapCallBacks _kCFNullBinaryHeapCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

CFBinaryHeapRef
CFBinaryHeapCreate (CFAllocatorRef alloc, CFIndex capacity,
  const CFBinaryHeapCallBacks *callBacks,
  const CFBinaryHeapCompareContext *compareContext)
{
  CFBinaryHeapRef new;
  
  new = (CFBinaryHeapRef)_CFRuntimeCreateInstance (alloc, _kCFBinaryHeapTypeID,
    CFBINARYHEAP_SIZE, 0);
  if (new)
    {
      if (capacity < _kCFBinaryHeapMinimumSize)
        capacity = _kCFBinaryHeapMinimumSize;
      
      new->_values = CFAllocatorAllocate (alloc, sizeof(void*) * capacity, 0);
      memset (new->_values, 0, sizeof(void*) * capacity);
      new->_capacity = capacity;
      
      if (callBacks == NULL)
        callBacks = &_kCFNullBinaryHeapCallBacks;
      new->_callBacks = callBacks;
      
      if (compareContext && compareContext->info)
        {
          new->_context.version = compareContext->version;
          new->_context.info = compareContext->retain ?
            (void*)compareContext->retain (compareContext->info) :
            compareContext->info;
          new->_context.retain = compareContext->retain;
          new->_context.release = compareContext->release;
          new->_context.copyDescription = compareContext->copyDescription;
        }
    }
  
  return new;
}

CFBinaryHeapRef
CFBinaryHeapCreateCopy (CFAllocatorRef alloc, CFIndex capacity,
  CFBinaryHeapRef heap)
{
  return CFBinaryHeapCreate (alloc, capacity, heap->_callBacks,
    &heap->_context);
}

void
CFBinaryHeapAddValue (CFBinaryHeapRef heap, const void *value)
{
  
}

void
CFBinaryHeapApplyFunction (CFBinaryHeapRef heap,
  CFBinaryHeapApplierFunction applier, void *context)
{
  
}

Boolean
CFBinaryHeapContainsValue (CFBinaryHeapRef heap, const void *value)
{
  return false;
}

CFIndex
CFBinaryHeapGetCount (CFBinaryHeapRef heap)
{
  return heap->_count;
}

CFIndex
CFBinaryHeapGetCountOfValue (CFBinaryHeapRef heap, const void *value)
{
  return 0;
}

const void *
CFBinaryHeapGetMinimum (CFBinaryHeapRef heap)
{
  return heap->_values[0];
}

Boolean
CFBinaryHeapGetMinimumIfPresent (CFBinaryHeapRef heap, const void **value)
{
  if (heap->_count == 0)
    return false;
  
  if (value)
    *value = heap->_values[0];
  return true;
}

void
CFBinaryHeapGetValues (CFBinaryHeapRef heap, const void **values)
{
  
}

void
CFBinaryHeapRemoveAllValues (CFBinaryHeapRef heap)
{
  if (heap->_callBacks->release)
    {
      CFAllocatorRef allocator = CFGetAllocator(heap);
      const void *cur = heap->_values;
      const void *end = cur + heap->_count;
      while (cur < end)
        heap->_callBacks->release (allocator, cur++);
    }
  memset (heap->_values, 0, sizeof(void*) * heap->_count);
}

void
CFBinaryHeapRemoveMinimumValue (CFBinaryHeapRef heap)
{
  
}
