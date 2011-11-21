/* CFArray.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: October, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "objc_interface.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"

#include <string.h>
#include <assert.h>

struct __CFArray
{
  CFRuntimeBase           _parent;
  const CFArrayCallBacks *_callbacks;
  const void            **_contents;
  CFIndex                 _count;
};

struct __CFMutableArray
{
  CFRuntimeBase           _parent;
  const CFArrayCallBacks *_callbacks;
  const void            **_contents;
  CFIndex                 _count;
  CFIndex                 _capacity;
};

static CFTypeID _kCFArrayTypeID = 0;

enum
{
  _kCFArrayIsMutable = (1<<0)
};

CF_INLINE Boolean
CFArrayIsMutable (CFArrayRef array)
{
  return ((CFRuntimeBase *)array)->_flags.info & _kCFArrayIsMutable ?
    true : false;
}

CF_INLINE void
CFArraySetMutable (CFArrayRef array)
{
  ((CFRuntimeBase *)array)->_flags.info |= _kCFArrayIsMutable;
}

static const void *
CFTypeRetain (CFAllocatorRef alloc, const void *value)
{
  return CFRetain (value);
}

static void
CFTypeRelease (CFAllocatorRef alloc, const void *value)
{
  CFRelease (value);
}

const CFArrayCallBacks kCFTypeArrayCallBacks =
{
  0,
  CFTypeRetain,
  CFTypeRelease,
  CFCopyDescription,
  CFEqual
};

/* Internal structure in case NULL is passed as the callback */
static CFArrayCallBacks _kCFNullArrayCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

static void
CFArrayDealloc (CFTypeRef cf)
{
  CFIndex idx;
  CFArrayRef array = (CFArrayRef)cf;
  CFArrayReleaseCallBack release = array->_callbacks->release;
  CFAllocatorRef alloc = CFGetAllocator(array);
  
  if (release)
    {
      for (idx = 0 ; idx < array->_count ; ++idx)
        release (alloc, array->_contents[idx]);
    }
  
  if (CFArrayIsMutable(array))
    CFAllocatorDeallocate (CFGetAllocator(array), array->_contents);
}

static Boolean
CFArrayEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFArrayRef a1 = (CFArrayRef)cf1;
  CFArrayRef a2 = (CFArrayRef)cf2;
  
  if (a1->_count != a2->_count)
    return false;
  
  if (a1->_count > 0)
    {
      Boolean result;
      CFIndex idx;
      CFArrayEqualCallBack equal = a1->_callbacks->equal;
      
      for (idx = 0 ; idx < a1->_count ; ++idx)
        {
          result = equal ? equal(a1->_contents[idx], a2->_contents[idx]) :
            a1->_contents[idx] == a2->_contents[idx];
          if (result == false)
            return false;
        }
    }
  
  return true;
}

static CFHashCode
CFArrayHash (CFTypeRef cf)
{
  return CFArrayGetCount(cf);
}

static CFStringRef
CFArrayCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  CFIndex idx;
  CFStringRef ret;
  CFMutableStringRef str;
  CFArrayRef array = (CFArrayRef)cf;
  CFArrayCopyDescriptionCallBack copyDesc = array->_callbacks->copyDescription;
  
  str = CFStringCreateMutable (NULL, 0);
  CFStringAppend (str, CFSTR("{"));
  
  if (copyDesc)
    {
      for (idx = 0 ; idx < array->_count ; ++idx)
        {
          CFStringRef desc = copyDesc(array->_contents[idx]);
          CFStringAppendFormat (str, formatOptions, CFSTR("%@, "), desc);
          CFRelease (desc);
        }
    }
  else
    {
      for (idx = 0 ; idx < array->_count ; ++idx)
        CFStringAppendFormat (str, formatOptions, CFSTR("%p, "),
          array->_contents[idx]);
    }
  
  CFStringDelete (str, CFRangeMake(CFStringGetLength(str), 2));
  CFStringAppend (str, CFSTR("}"));
  
  ret = CFStringCreateCopy (NULL, str);
  CFRelease (str);
  
  return ret;
}

static CFRuntimeClass CFArrayClass =
{
  0,
  "CFArray",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFArrayCreateCopy,
  CFArrayDealloc,
  CFArrayEqual,
  CFArrayHash,
  CFArrayCopyFormattingDesc,
  NULL
};

void CFArrayInitialize (void)
{
  _kCFArrayTypeID = _CFRuntimeRegisterClass (&CFArrayClass);
}



//
// CFArray
//
void
CFArrayApplyFunction (CFArrayRef array, CFRange range,
                      CFArrayApplierFunction applier, void *context)
{
  CFIndex i;

  for (i = range.location; i < range.location + range.length; i++)
    applier(CFArrayGetValueAtIndex(array, i), context);
}

CFIndex
CFArrayBSearchValues (CFArrayRef array, CFRange range, const void *value,
                      CFComparatorFunction comparator, void *context)
{
  CFIndex min, max, mid;

  min = range.location;
  max = range.location + range.length - 1;

  while (min <= max)
    {
      const void *midValue;
      CFComparisonResult res;

      mid = (min + max) / 2;
      midValue = CFArrayGetValueAtIndex(array, mid);
      res = comparator(midValue, value, context);
      if (res == kCFCompareEqualTo)
        {
          max = mid - 1;
          break;
        }
      else if (res == kCFCompareGreaterThan)
        {
          max = mid - 1;
        }
      else
        {
          min = mid + 1;
        }
    }
  return max + 1;
}

Boolean
CFArrayContainsValue (CFArrayRef array, CFRange range, const void *value)
{
  return (CFArrayGetFirstIndexOfValue(array, range, value) != -1);
}

CFArrayRef
CFArrayCreate (CFAllocatorRef allocator, const void **values,
               CFIndex numValues, const CFArrayCallBacks *callBacks)
{
  struct __CFArray *new;
  CFIndex size;
  CFIndex idx;
  CFArrayRetainCallBack retain;
  
  size = sizeof(struct __CFArray) - sizeof(CFRuntimeBase) +
    (sizeof(void*) * numValues);
  new = (struct __CFArray*) _CFRuntimeCreateInstance (allocator,
    _kCFArrayTypeID, size, 0);
  
  if (new)
    {
      if (callBacks == NULL)
        callBacks = &_kCFNullArrayCallBacks;
      
      new->_callbacks = callBacks;
      new->_contents = (const void**)&new[1];
      new->_count = numValues;
      
      memcpy (new->_contents, values, numValues * sizeof(void *));
      
      retain = callBacks->retain;
      if (retain)
        for (idx = 0 ; idx < numValues ; ++idx)
          retain (allocator, values[idx]);
    }
  
  return (CFArrayRef)new;
}

CFArrayRef
CFArrayCreateCopy (CFAllocatorRef allocator, CFArrayRef array)
{
  return CFArrayCreate (allocator, array->_contents, array->_count,
    array->_callbacks);
}

CFIndex
CFArrayGetCount (CFArrayRef array)
{
  CF_OBJC_FUNCDISPATCH0(_kCFArrayTypeID, CFIndex, array, "count");
  
  return array->_count;
}

CFIndex
CFArrayGetCountOfValue (CFArrayRef array, CFRange range, const void *value)
{
  CFIndex count = 0;
  CFIndex i;

  while (( i = CFArrayGetFirstIndexOfValue(array, range, value)) != -1)
    {
      count++;
      range.location = i + 1;
      range.length = range.length - range.location;
    }
  return count;
}

CFIndex
CFArrayGetFirstIndexOfValue (CFArrayRef array, CFRange range,
                             const void *value)
{
  const void **contents;
  CFIndex idx;
  CFIndex end;
  CFArrayEqualCallBack equal;
  
  assert (range.location + range.length <= array->_count);
  
  contents = array->_contents;
  idx = range.location;
  end = idx + range.length;
  equal = array->_callbacks->equal;
  if (equal)
    {
      while (idx < end)
        {
          if (equal (value, contents[idx++]))
            break;
        }
    }
  else
    {
      while (idx < end)
        {
          if (value == contents[idx++])
            break;
        }
    }
  if (idx >= end)
    idx = -1;
  
  return idx;
}

CFIndex
CFArrayGetLastIndexOfValue (CFArrayRef array, CFRange range,
                            const void *value)
{
  const void **contents;
  CFIndex idx;
  CFIndex start;
  CFArrayEqualCallBack equal;
  
  assert (range.location + range.length <= array->_count);
  
  contents = array->_contents;
  start = range.location;
  idx = start + range.length;
  equal = array->_callbacks->equal;
  if (equal)
    {
      while (idx >= start)
        {
          if (equal (value, contents[idx]))
            break;
          --idx;
        }
    }
  else
    {
      while (idx >= start)
        {
          if (value == contents[idx])
            break;
          --idx;
        }
    }
  if (idx < start)
    idx = -1;
  
  return idx;
}

CFTypeID
CFArrayGetTypeID (void)
{
  return _kCFArrayTypeID;
}

const void *
CFArrayGetValueAtIndex (CFArrayRef array, CFIndex idx)
{
  CF_OBJC_FUNCDISPATCH1(_kCFArrayTypeID, const void *, array,
    "objectAtIndex:", idx);
  
  assert (idx < array->_count);
  return (array->_contents)[idx];
}

void
CFArrayGetValues (CFArrayRef array, CFRange range, const void **values)
{
  CF_OBJC_FUNCDISPATCH2(_kCFArrayTypeID, void, array,
    "getObjects:range:", values, range);
  
  assert (range.location + range.length < array->_count);
  memcpy (values, (array->_contents + range.location),
    range.length * sizeof(const void*));
}



//
// CFMutableArray
//

#define DEFAULT_ARRAY_CAPACITY 16

static Boolean
CFArrayCheckCapacityAndGrow (CFMutableArrayRef array, CFIndex newCapacity)
{
  void *currentContents;
  void *newContents;
  struct __CFMutableArray *mArray = (struct __CFMutableArray *)array;
  
  if (mArray->_capacity >= newCapacity)
    return true;
  
  currentContents = mArray->_contents;
  newCapacity =
    ((newCapacity / DEFAULT_ARRAY_CAPACITY) + 1) * DEFAULT_ARRAY_CAPACITY;
  
  newContents = CFAllocatorAllocate (CFGetAllocator(mArray),
    (newCapacity * sizeof(const void *)), 0);
  if (newContents == NULL)
    return false;
  
  memcpy (newContents, currentContents, sizeof(const void *) * mArray->_count);
  mArray->_capacity = newCapacity;
  mArray->_contents = newContents;
  
  CFAllocatorDeallocate (CFGetAllocator(mArray), currentContents);
  
  return true;
}

CFMutableArrayRef
CFArrayCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
                      const CFArrayCallBacks *callBacks)
{
  struct __CFMutableArray *new;
  
  new = (struct __CFMutableArray*) _CFRuntimeCreateInstance (allocator,
    _kCFArrayTypeID, sizeof(struct __CFArray) - sizeof(CFRuntimeBase), 0);
  
  if (new)
    {
      if (callBacks == NULL)
        callBacks = &_kCFNullArrayCallBacks;
      
      new->_callbacks = callBacks;
      
      if (capacity < DEFAULT_ARRAY_CAPACITY)
        capacity = DEFAULT_ARRAY_CAPACITY;
      
      new->_contents =
        CFAllocatorAllocate (allocator, capacity * sizeof(void*), 0);
      new->_count = 0;
      new->_capacity = capacity;
      
      CFArraySetMutable ((CFArrayRef)new);
    }
  
  return (CFMutableArrayRef)new;
}

CFMutableArrayRef
CFArrayCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                          CFArrayRef array)
{
  CFMutableArrayRef new;
  const CFArrayCallBacks *callbacks;
  
  if (!array)
    return NULL;
  
  if (CF_IS_OBJC(_kCFArrayTypeID, array))
	  callbacks = &kCFTypeArrayCallBacks;
	else
	  callbacks = array->_callbacks;
  
  new = CFArrayCreateMutable (allocator, capacity, callbacks);
  if (new)
    {
      CFIndex idx;
      CFIndex count;
      
      for (idx = 0, count = CFArrayGetCount(array) ; idx < count ; ++idx)
        {
          new->_contents[idx] = callbacks->retain
            ? callbacks->retain(NULL, CFArrayGetValueAtIndex(array, idx))
            : CFArrayGetValueAtIndex(array, idx);
        }
      new->_count = count;
    }
  
  return new;
}

void
CFArrayAppendArray (CFMutableArrayRef array, CFArrayRef otherArray,
                    CFRange otherRange)
{
  CFIndex count;
  CFIndex idx;
  CFIndex otherIdx;
  const CFArrayCallBacks *callbacks;
  
  assert (otherRange.location + otherRange.length < CFArrayGetCount (otherArray));
  
  if (!otherArray)
    return;
  
  count = CFArrayGetCount (array);
  if (CFArrayCheckCapacityAndGrow (array, count + otherRange.length))
    {
      callbacks = array->_callbacks;
      for (idx = count, otherIdx = otherRange.location
          ; idx < count + otherRange.length
          ; ++idx, ++otherIdx)
        {
          array->_contents[idx] = callbacks->retain
            ? callbacks->retain(NULL, CFArrayGetValueAtIndex (otherArray, otherIdx))
            : CFArrayGetValueAtIndex (otherArray, otherIdx);
        }
      array->_count = count + otherRange.length;
    }
}

void
CFArrayAppendValue (CFMutableArrayRef array, const void *value)
{
  CFIndex newCount;
  const CFArrayCallBacks *callbacks;
  
  CF_OBJC_FUNCDISPATCH1(_kCFArrayTypeID, void, array,
    "addObject:", value);
  
  newCount = CFArrayGetCount (array) + 1;
  if (value && CFArrayCheckCapacityAndGrow (array, newCount))
    {
      callbacks = array->_callbacks;
      array->_contents[newCount - 1] = callbacks->retain
        ? callbacks->retain (NULL, value) : value;
      array->_count++;
    }
}

void
CFArrayExchangeValuesAtIndices (CFMutableArrayRef array, CFIndex idx1,
                                CFIndex idx2)
{
  const void *tmp;
  
  CF_OBJC_FUNCDISPATCH2(_kCFArrayTypeID, void, array,
    "exchangeObjectAtIndex:withObjectAtIndex:", idx1, idx2);
  
  tmp = array->_contents[idx1];
  array->_contents[idx1] = array->_contents[idx2];
  array->_contents[idx2] = tmp;
}

void
CFArrayInsertValueAtIndex (CFMutableArrayRef array, CFIndex idx,
                           const void *value)
{
  CF_OBJC_FUNCDISPATCH2(_kCFArrayTypeID, void, array,
    "insertObject:AtIndex:", value, idx);
  
  if (value && CFArrayCheckCapacityAndGrow (array, array->_count + 1))
    {
      const CFArrayCallBacks *callbacks;
      const void *moveFrom;
      void *moveTo;
      CFIndex moveLength;
      
      moveFrom = array->_contents + idx;
      moveTo = array->_contents + idx + 1;
      moveLength = array->_count - idx;
      
      memmove (moveTo, moveFrom, moveLength);
      
      callbacks = array->_callbacks;
      array->_contents[idx] = callbacks->retain
        ? callbacks->retain (NULL, value) : value;
      
      array->_count++;
    }
}

void
CFArrayRemoveAllValues (CFMutableArrayRef array)
{
  CFArrayReleaseCallBack release;
  CFIndex idx;
  CFIndex count;
  
  CF_OBJC_FUNCDISPATCH0(_kCFArrayTypeID, void, array,
    "removeAllObjects");
  
  release = array->_callbacks->release;
  for (idx = 0, count = array->_count ; idx < count ; ++idx)
    release (NULL, array->_contents[idx]);
  
  array->_count = 0;
}

void
CFArrayRemoveValueAtIndex (CFMutableArrayRef array, CFIndex idx)
{
  CFArrayReleaseCallBack release;
  CFIndex count;
  
  CF_OBJC_FUNCDISPATCH1(_kCFArrayTypeID, void, array,
    "removeObjectAtIndex:", idx);
  
  release = array->_callbacks->release;
  release (NULL, array->_contents[idx]);
  
  --(array->_count);
  for (count = array->_count ; idx < count ; ++idx)
    array->_contents[idx] = array->_contents[idx+1];
}

void
CFArrayReplaceValues (CFMutableArrayRef array, CFRange range,
                      const void **newValues, CFIndex newCount)
{
  CFIndex i;

  for (i = 0 ; i < range.length ; i++)
    CFArrayRemoveValueAtIndex(array, range.location);

  for (i = newCount - 1 ; i >= 0 ; i--)
    CFArrayInsertValueAtIndex(array, range.location, newValues[i]);
}

void
CFArraySetValueAtIndex (CFMutableArrayRef array, CFIndex idx,
                        const void *value)
{
  CFArrayReleaseCallBack release;
  CFArrayRetainCallBack retain;
  
  CF_OBJC_FUNCDISPATCH2(_kCFArrayTypeID, void, array,
    "replaceObjectAtIndex:withObject:", idx, value);
  
  release = array->_callbacks->release;
  retain = array->_callbacks->retain;
  
  release (NULL, array->_contents[idx]);
  array->_contents[idx] = retain (NULL, value);
}

/* Using the quick-sort algorithm to sort CFArrays. */
CFIndex
CFArraySortValuesPartition (CFMutableArrayRef array, CFIndex left,
  CFIndex right, CFIndex pivot, CFComparatorFunction comp, void *ctxt)
{
  CFIndex idx;
  CFIndex storeIdx;
  CFComparisonResult result;
  const void *pivotValue;
  
  pivotValue = CFArrayGetValueAtIndex (array, pivot);
  CFArrayExchangeValuesAtIndices (array, pivot, right - 1);
  storeIdx = left;
  for (idx = left ; idx < right ; ++idx)
    {
      result = (*comp)(CFArrayGetValueAtIndex(array, idx), pivotValue, ctxt);
      if (result == kCFCompareLessThan)
        {
          CFArrayExchangeValuesAtIndices (array, idx, storeIdx);
          storeIdx += 1;
        }
    }
  CFArrayExchangeValuesAtIndices (array, storeIdx, right - 1);
  
  return storeIdx;
}

void
CFArraySortValuesQuickSort (CFMutableArrayRef array, CFIndex left,
  CFIndex right, CFComparatorFunction comp, void *ctxt)
{
  if (left < right)
    {
      CFIndex pivotIdx;
      CFIndex pivotNewIdx;
      
      pivotIdx = (right - left) / 2;
      pivotNewIdx = CFArraySortValuesPartition (array, left, right, pivotIdx,
        comp, ctxt);
      
      CFArraySortValuesQuickSort (array, left, pivotNewIdx - 1, comp, ctxt);
      CFArraySortValuesQuickSort (array, pivotNewIdx + 1, right, comp, ctxt);
    }
}

void
CFArraySortValues (CFMutableArrayRef array, CFRange range,
                   CFComparatorFunction comparator, void *context)
{
  CFArraySortValuesQuickSort (array, range.location,
    range.location + range.length, comparator, context);
}

