/* CFBinaryHeap.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFBinaryHeap.h"

#include <string.h>

static CFTypeID _kCFBinaryHeapTypeID = 0;

static const CFBinaryHeapCallBacks _kCFNullBinaryHeapCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

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
  
  if (heap->_context.release)
    heap->_context.release (heap->_context.info);
  
  if (heap->_callBacks->release)
    {
      const void **cur = heap->_values;
      const void **end = cur + heap->_count;
      while (cur < end)
        heap->_callBacks->release (allocator, cur++);
    }
  CFAllocatorDeallocate (allocator, (void*)heap->_values);
}

static Boolean
CFBinaryHeapEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFBinaryHeapRef heap1 = (CFBinaryHeapRef)cf1;
  CFBinaryHeapRef heap2 = (CFBinaryHeapRef)cf2;
  
  if (heap1->_count != heap2->_count
      || heap1->_callBacks->compare != heap2->_callBacks->compare
      || heap1->_context.info != heap2->_context.info)
    return false;
  
  if (heap1->_count > 0)
    {
      CFIndex idx;
      Boolean result;
      CFBinaryHeapCompareCallBack compare = heap1->_callBacks->compare;
      
      for (idx = 0 ; idx < heap1->_count ; ++idx)
        {
          if (compare)
            result = (compare(heap1->_values[idx], heap2->_values[idx],
              heap1->_context.info) == 0);
          else
            result = ((heap1->_values[idx] == heap2->_values[idx]) == false);
          
          if (result == false)
            return false;
        }
    }
  
  return true;
}

static CFHashCode
CFBinaryHeapHash (CFTypeRef cf)
{
  return (CFHashCode)((CFBinaryHeapRef)cf)->_count;
}

static CFRuntimeClass CFBinaryHeapClass =
{
  0,
  "CFBinaryHeap",
  NULL,
  NULL,
  CFBinaryHeapFinalize,
  CFBinaryHeapEqual,
  CFBinaryHeapHash,
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

CFIndex
CFBinaryHeapGetCount (CFBinaryHeapRef heap)
{
  return heap->_count;
}

void
CFBinaryHeapApplyFunction (CFBinaryHeapRef heap,
  CFBinaryHeapApplierFunction applier, void *context)
{
  CFIndex i;

  for (i = 0 ; i < heap->_count ; i++)
    applier(heap->_values[i], context);
}

#define CFBINARYHEAP_SIZE sizeof(struct __CFBinaryHeap) - sizeof(CFRuntimeBase)
#define DEFAULT_HEAP_CAPACITY 15 /* Equivalent to 3 levels */

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
      /* Make sure we always have at least 3 complete levels */
      if (capacity < DEFAULT_HEAP_CAPACITY)
        {
          capacity = DEFAULT_HEAP_CAPACITY;
        }
      else
        {
          CFIndex tmp = DEFAULT_HEAP_CAPACITY;
          while (capacity > tmp)
            tmp = (tmp << 1) + 1;
          capacity = tmp;
        }
      
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
  CFBinaryHeapRef ret;
  
  ret = CFBinaryHeapCreate (alloc, capacity, heap->_callBacks,
    &heap->_context);
  memcpy (ret->_values, heap->_values, sizeof(void*) * heap->_count);
  ret->_count = heap->_count;
  
  return ret;
}

CF_INLINE void
CFBinaryHeapCheckCapacityAndGrow (CFBinaryHeapRef heap)
{
  if (heap->_count == heap->_capacity)
    {
      CFIndex newCapacity = (heap->_capacity << 1) + 1;
      
      heap->_values = CFAllocatorReallocate (CFGetAllocator(heap),
        heap->_values, (newCapacity * sizeof(const void *)), 0);
      heap->_capacity = newCapacity;
    }
}

void
CFBinaryHeapAddValue (CFBinaryHeapRef heap, const void *value)
{
  CFIndex cur;
  void *info;
  CFBinaryHeapCompareCallBack compare;
  CFBinaryHeapRetainCallBack retain;
  
  CFBinaryHeapCheckCapacityAndGrow (heap);
  
  compare = heap->_callBacks->compare;
  info = heap->_context.info;
  cur = heap->_count;
  while (cur > 0)
    {
      /* Shift down */
      CFIndex parent;
      const void *p;
      
      parent = (cur - 1) >> 1;
      p = heap->_values[parent];
      if (compare ? compare(p, value, info) != kCFCompareGreaterThan
          : p <= value)
        break;
      
      heap->_values[cur] = heap->_values[parent];
      cur = parent;
    }
  
  retain = heap->_callBacks->retain;
  heap->_values[cur] = retain ? retain(CFGetAllocator(heap), value) : value;
  heap->_count += 1;
}

Boolean
CFBinaryHeapContainsValue (CFBinaryHeapRef heap, const void *value)
{
  CFIndex idx;
  CFIndex count;
  CFBinaryHeapCompareCallBack compare;
  void *info;
  
  idx = 0;
  count = heap->_count;
  compare = heap->_callBacks->compare;
  info = heap->_context.info;
  while (idx < count)
    {
      const void *v;
      
      v = heap->_values[idx++];
      if (compare ? compare(v, value, info) == kCFCompareEqualTo : v == value)
        return true;
    }
  
  return false;
}

CFIndex
CFBinaryHeapGetCountOfValue (CFBinaryHeapRef heap, const void *value)
{
  CFIndex idx;
  CFIndex count;
  CFIndex counter;
  CFBinaryHeapCompareCallBack compare;
  void *info;
  
  idx = 0;
  count = heap->_count;
  counter = 0;
  compare = heap->_callBacks->compare;
  info = heap->_context.info;
  while (idx < count)
    {
      const void *v;
      
      v = heap->_values[idx++];
      if (compare ? compare(v, value, info) == kCFCompareEqualTo : v == value)
        counter++;
    }
  
  return counter;
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
  CFBinaryHeapRef copy;
  
  copy = CFBinaryHeapCreateCopy (NULL, heap->_capacity, heap);
  while (CFBinaryHeapGetMinimumIfPresent(copy, values))
    {
      values++;
      CFBinaryHeapRemoveMinimumValue (copy);
    }
  CFRelease (copy);
}

void
CFBinaryHeapRemoveAllValues (CFBinaryHeapRef heap)
{
  if (heap->_callBacks->release)
    {
      CFAllocatorRef allocator = CFGetAllocator(heap);
      const void **cur = heap->_values;
      const void **end = cur + heap->_count;
      while (cur < end)
        heap->_callBacks->release (allocator, cur++);
    }
  
  heap->_count = 0;
}

void
CFBinaryHeapRemoveMinimumValue (CFBinaryHeapRef heap)
{
  CFIndex idx;
  CFIndex child;
  CFIndex count;
  CFBinaryHeapReleaseCallBack release;
  CFBinaryHeapCompareCallBack compare;
  const void *last;
  void *info;
  
  release = heap->_callBacks->release;
  if (release)
    release (CFGetAllocator(heap), heap->_values[0]);
  count = heap->_count;
  heap->_count -= 1;
  
  compare = heap->_callBacks->compare;
  info = heap->_context.info;
  
  idx = 0;
  child = 1; /* Initialize to left child */
  last = heap->_values[count - 1];
  while (child < count)
    {
      const void *v;
      
      v = heap->_values[child];
      if (child + 1 < count) /* Check for a right child */
        {
          const void *right;
          
          right = heap->_values[child + 1];
          if (compare ? compare(v, right, info) == kCFCompareGreaterThan
              : v > right)
            {
              v = right;
              child += 1;
            }
        }
      
      heap->_values[idx] = v;
      idx = child;
      child = (idx << 1) + 1; /* Go to left child */
    }
  
  heap->_values[idx] = last;
}

