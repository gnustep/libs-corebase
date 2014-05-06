/* CFArray.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: October, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"

#include "GSCArray.h"
#include "GSObjCRuntime.h"

#include <string.h>

struct __CFArray
{
  CFRuntimeBase _parent;
  const CFArrayCallBacks *_callBacks;
  const void **_contents;
  CFIndex _count;
};

struct __CFMutableArray
{
  CFRuntimeBase _parent;
  const CFArrayCallBacks *_callBacks;
  const void **_contents;
  CFIndex _count;
  CFIndex _capacity;
};

static CFTypeID _kCFArrayTypeID = 0;

enum
{
  _kCFArrayIsMutable = (1 << 0)
};

CF_INLINE Boolean
CFArrayIsMutable (CFArrayRef array)
{
  return ((CFRuntimeBase *) array)->_flags.info & _kCFArrayIsMutable ?
    true : false;
}

CF_INLINE void
CFArraySetMutable (CFArrayRef array)
{
  ((CFRuntimeBase *) array)->_flags.info |= _kCFArrayIsMutable;
}

const CFArrayCallBacks kCFTypeArrayCallBacks = {
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual
};

/* Internal structure in case NULL is passed as the callback */
static CFArrayCallBacks _kCFNullArrayCallBacks = {
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

static void
CFArrayFinalize (CFTypeRef cf)
{
  CFIndex idx;
  CFArrayRef array = (CFArrayRef) cf;
  CFArrayReleaseCallBack release = array->_callBacks->release;
  CFAllocatorRef alloc = CFGetAllocator (array);

  if (release)
    {
      for (idx = 0; idx < array->_count; ++idx)
        release (alloc, array->_contents[idx]);
    }

  if (CFArrayIsMutable (array))
    CFAllocatorDeallocate (alloc, array->_contents);
}

static Boolean
CFArrayEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFArrayRef a1 = (CFArrayRef) cf1;
  CFArrayRef a2 = (CFArrayRef) cf2;

  if (a1->_count != a2->_count)
    return false;

  if (a1->_count > 0)
    {
      Boolean result;
      CFIndex idx;
      CFArrayEqualCallBack equal = a1->_callBacks->equal;

      for (idx = 0; idx < a1->_count; ++idx)
        {
          result = equal ? equal (a1->_contents[idx], a2->_contents[idx]) :
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
  return CFArrayGetCount (cf);
}

static CFStringRef
CFArrayCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  CFIndex idx;
  CFStringRef ret;
  CFMutableStringRef str;
  CFArrayRef array = (CFArrayRef) cf;
  CFArrayCopyDescriptionCallBack copyDesc = array->_callBacks->copyDescription;

  str = CFStringCreateMutable (NULL, 0);
  CFStringAppend (str, CFSTR ("{"));

  if (copyDesc)
    {
      for (idx = 0; idx < array->_count; ++idx)
        {
          CFStringRef desc = copyDesc (array->_contents[idx]);
          CFStringAppendFormat (str, formatOptions, CFSTR ("%@, "), desc);
          CFRelease (desc);
        }
    }
  else
    {
      for (idx = 0; idx < array->_count; ++idx)
        CFStringAppendFormat (str, formatOptions, CFSTR ("%p, "),
                              array->_contents[idx]);
    }

  CFStringDelete (str, CFRangeMake (CFStringGetLength (str), 2));
  CFStringAppend (str, CFSTR ("}"));

  ret = CFStringCreateCopy (NULL, str);
  CFRelease (str);

  return ret;
}

static CFRuntimeClass CFArrayClass = {
  0,
  "CFArray",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) CFArrayCreateCopy,
  CFArrayFinalize,
  CFArrayEqual,
  CFArrayHash,
  CFArrayCopyFormattingDesc,
  NULL
};

void
CFArrayInitialize (void)
{
  _kCFArrayTypeID = _CFRuntimeRegisterClass (&CFArrayClass);
}



#define CFARRAY_SIZE sizeof(struct __CFArray) - sizeof(CFRuntimeBase)

CFArrayRef
CFArrayCreate (CFAllocatorRef allocator, const void **values,
               CFIndex numValues, const CFArrayCallBacks * callBacks)
{
  struct __CFArray *new;
  CFIndex size;
  CFIndex idx;
  CFArrayRetainCallBack retain;

  size = CFARRAY_SIZE + (sizeof (void *) * numValues);
  new = (struct __CFArray *) _CFRuntimeCreateInstance (allocator,
                                                       _kCFArrayTypeID, size,
                                                       0);

  if (new)
    {
      if (callBacks == NULL)
        callBacks = &_kCFNullArrayCallBacks;

      new->_callBacks = callBacks;
      new->_contents = (const void **) &new[1];
      new->_count = numValues;

      memcpy (new->_contents, values, numValues * sizeof (void *));

      retain = callBacks->retain;
      if (retain)
        for (idx = 0; idx < numValues; ++idx)
          retain (allocator, values[idx]);
    }

  return (CFArrayRef) new;
}

CFArrayRef
CFArrayCreateCopy (CFAllocatorRef allocator, CFArrayRef array)
{
  if (CF_IS_OBJC (_kCFArrayTypeID, array))
    {
      const void **values;
      CFIndex count;
      CFArrayRef copy;

      count = CFArrayGetCount (array);
      values = (const void **) CFAllocatorAllocate (allocator,
                                                    sizeof (const void *) *
                                                    count, 0);

      CFArrayGetValues (array, CFRangeMake (0, count), values);
      copy = CFArrayCreate (allocator, values, count, &kCFTypeArrayCallBacks);

      CFAllocatorDeallocate (allocator, (void *) values);

      return copy;
    }

  return CFArrayCreate (allocator, array->_contents, array->_count,
                        array->_callBacks);
}

void
CFArrayApplyFunction (CFArrayRef array, CFRange range,
                      CFArrayApplierFunction applier, void *context)
{
  CFIndex i;

  for (i = range.location; i < range.location + range.length; i++)
    applier (CFArrayGetValueAtIndex (array, i), context);
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
      midValue = CFArrayGetValueAtIndex (array, mid);
      res = comparator (midValue, value, context);
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
  return (CFArrayGetFirstIndexOfValue (array, range, value) != -1);
}

CFIndex
CFArrayGetCount (CFArrayRef array)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, CFIndex, array, "count");

  return array->_count;
}

CFIndex
CFArrayGetCountOfValue (CFArrayRef array, CFRange range, const void *value)
{
  CFIndex count = 0;
  CFIndex i;

  while ((i = CFArrayGetFirstIndexOfValue (array, range, value)) != -1)
    {
      count++;
      range.location = i + 1;
      range.length = range.length - range.location;
    }
  return count;
}

CFIndex
CFArrayGetFirstIndexOfValue (CFArrayRef array, CFRange range, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, CFIndex, array,
                         "indexOfObject:inRange:", value, range);

  const void **contents;
  CFIndex idx;
  CFIndex end;
  CFArrayEqualCallBack equal;

  contents = array->_contents;
  idx = range.location;
  end = idx + range.length;
  equal = array->_callBacks->equal;
  if (equal)
    {
      while (idx < end)
        {
          if (equal (value, contents[idx]))
            break;
          idx++;
        }
    }
  else
    {
      while (idx < end)
        {
          if (value == contents[idx])
            break;
          idx++;
        }
    }
  if (idx >= end)
    idx = -1;

  return idx;
}

CFIndex
CFArrayGetLastIndexOfValue (CFArrayRef array, CFRange range, const void *value)
{
  CFArrayEqualCallBack equal;
  CFIndex idx;
  CFIndex start;

  if (CF_IS_OBJC (_kCFArrayTypeID, array))
    equal = kCFTypeArrayCallBacks.equal;
  else
    equal = array->_callBacks->equal;

  start = range.location;
  idx = start + range.length;

  if (equal)
    {
      while (idx >= start)
        {
          if (equal (value, CFArrayGetValueAtIndex (array, idx)))
            break;
          --idx;
        }
    }
  else
    {
      while (idx >= start)
        {
          if (value == CFArrayGetValueAtIndex (array, idx))
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
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, const void *, array,
                         "objectAtIndex:", idx);

  return (array->_contents)[idx];
}

void
CFArrayGetValues (CFArrayRef array, CFRange range, const void **values)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "getObjects:range:", values, range);

  memcpy (values, (array->_contents + range.location),
          range.length * sizeof (const void *));
}



#define DEFAULT_ARRAY_CAPACITY 16
#define CFMUTABLEARRAY_SIZE sizeof(struct __CFMutableArray) - sizeof(CFRuntimeBase)

CF_INLINE void
CFArrayCheckCapacityAndGrow (CFMutableArrayRef array, CFIndex newCapacity)
{
  struct __CFMutableArray *mArray = (struct __CFMutableArray *) array;

  if (mArray->_capacity < newCapacity)
    {
      newCapacity = mArray->_capacity + DEFAULT_ARRAY_CAPACITY;

      mArray->_contents = CFAllocatorReallocate (CFGetAllocator (mArray),
                                                 mArray->_contents,
                                                 (newCapacity *
                                                  sizeof (const void *)), 0);
      mArray->_capacity = newCapacity;
    }
}

CFMutableArrayRef
CFArrayCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
                      const CFArrayCallBacks * callBacks)
{
  struct __CFMutableArray *new;

  new = (struct __CFMutableArray *) _CFRuntimeCreateInstance (allocator,
                                                              _kCFArrayTypeID,
                                                              CFMUTABLEARRAY_SIZE,
                                                              0);

  if (new)
    {
      if (callBacks == NULL)
        callBacks = &_kCFNullArrayCallBacks;

      new->_callBacks = callBacks;

      if (capacity < DEFAULT_ARRAY_CAPACITY)
        capacity = DEFAULT_ARRAY_CAPACITY;

      new->_contents =
        CFAllocatorAllocate (allocator, capacity * sizeof (void *), 0);
      new->_count = 0;
      new->_capacity = capacity;

      CFArraySetMutable ((CFArrayRef) new);
    }

  return (CFMutableArrayRef) new;
}

CFMutableArrayRef
CFArrayCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                          CFArrayRef array)
{
  if (CF_IS_OBJC (_kCFArrayTypeID, array))
    {
      const void **values;
      CFIndex count, i;
      CFMutableArrayRef copy;

      count = CFArrayGetCount (array);
      values = (const void **) CFAllocatorAllocate (allocator,
                                                    sizeof (const void *) *
                                                    count, 0);

      CFArrayGetValues (array, CFRangeMake (0, count), values);

      if (count > capacity)
        capacity = count;

      copy = CFArrayCreateMutable (allocator, capacity, &kCFTypeArrayCallBacks);

      for (i = 0; i < count; i++)
        CFArrayAppendValue (copy, values[i]);

      CFAllocatorDeallocate (allocator, (void *) values);

      return copy;
    }

  CFMutableArrayRef new;
  const CFArrayCallBacks *callbacks;

  if (!array)
    return NULL;

  if (CF_IS_OBJC (_kCFArrayTypeID, array))
    callbacks = &kCFTypeArrayCallBacks;
  else
    callbacks = array->_callBacks;

  new = CFArrayCreateMutable (allocator, capacity, callbacks);
  if (new)
    {
      CFIndex idx;
      CFIndex count;

      for (idx = 0, count = CFArrayGetCount (array); idx < count; ++idx)
        {
          new->_contents[idx] = callbacks->retain
            ? callbacks->retain (NULL, CFArrayGetValueAtIndex (array, idx))
            : CFArrayGetValueAtIndex (array, idx);
        }
      new->_count = count;
    }

  return new;
}

void
CFArrayAppendArray (CFMutableArrayRef array, CFArrayRef oArray, CFRange oRange)
{
  CFIndex oLen;
  const void **values;

  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "replaceObjectsInRange:withObjectsFromArray:range:",
                         CFRangeMake (CFArrayGetCount (array), 0), oArray,
                         oRange);

  oLen = oRange.length;
  values = CFAllocatorAllocate (NULL, oLen * sizeof (void *), 0);
  CFArrayGetValues (oArray, oRange, values);

  CFArrayReplaceValues (array, CFRangeMake (array->_count, 0), values, oLen);

  CFAllocatorDeallocate (NULL, values);
}

void
CFArrayAppendValue (CFMutableArrayRef array, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array, "addObject:", value);

  CFArrayReplaceValues (array, CFRangeMake (array->_count, 0), &value, 1);
}

void
CFArrayExchangeValuesAtIndices (CFMutableArrayRef array, CFIndex idx1,
                                CFIndex idx2)
{
  const void *tmp;

  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "exchangeObjectAtIndex:withObjectAtIndex:", idx1,
                         idx2);

  tmp = array->_contents[idx1];
  array->_contents[idx1] = array->_contents[idx2];
  array->_contents[idx2] = tmp;
}

void
CFArrayInsertValueAtIndex (CFMutableArrayRef array, CFIndex idx,
                           const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "insertObject:AtIndex:", value, idx);

  CFArrayReplaceValues (array, CFRangeMake (idx, 0), &value, 1);
}

void
CFArrayRemoveAllValues (CFMutableArrayRef array)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array, "removeAllObjects");

  CFArrayReplaceValues (array, CFRangeMake (0, array->_count), NULL, 0);
  memset (array->_contents, 0, array->_count * sizeof (void *));
}

void
CFArrayRemoveValueAtIndex (CFMutableArrayRef array, CFIndex idx)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "removeObjectAtIndex:", idx);

  CFArrayReplaceValues (array, CFRangeMake (idx, 1), NULL, 0);
}

void
CFArrayReplaceValues (CFMutableArrayRef array, CFRange range,
                      const void **newValues, CFIndex newCount)
{
  if (CF_IS_OBJC (_kCFArrayTypeID, array))
    {
      CFArrayRef temp;

      temp = CFArrayCreate (kCFAllocatorDefault, newValues,
                            newCount, &kCFTypeArrayCallBacks);

      CF_OBJC_VOIDCALLV (array,
                         "replaceObjectsInRange:withObjectsFromArray:", range,
                         temp);

      CFRelease (temp);
      return;
    }

  const void **start;
  const void **end;
  CFAllocatorRef alloc;

  start = array->_contents + range.location;
  end = start + range.length;
  alloc = CFGetAllocator (array);

  /* Release values if needed */
  if (range.length > 0)
    {
      CFArrayReleaseCallBack release = array->_callBacks->release;
      if (release)
        {
          const void **current = start;
          while (current < end)
            release (alloc, *(current++));
        }
      array->_count -= range.length;
    }

  /* Move remaining values if required */
  if (range.length != newCount)
    {
      CFIndex newSize;

      newSize = array->_count - range.length + newCount;
      CFArrayCheckCapacityAndGrow (array, newSize);

      memmove (start + newCount, end,
               (array->_count - range.location +
                range.length) * sizeof (void *));
    }

  /* Insert new values */
  if (newCount > 0)
    {
      CFArrayRetainCallBack retain = array->_callBacks->retain;
      const void **current = start;
      end = current + newCount; /* New end... */
      if (retain)
        {
          while (current < end)
            *(current++) = retain (alloc, *(newValues++));
        }
      else
        {
          while (current < end)
            *(current++) = *(newValues++);
        }
      array->_count += newCount;
    }
}

void
CFArraySetValueAtIndex (CFMutableArrayRef array, CFIndex idx, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "replaceObjectAtIndex:withObject:", idx, value);

  CFArrayReplaceValues (array, CFRangeMake (idx, 1), &value, 1);
}

void
CFArraySortValues (CFMutableArrayRef array, CFRange range,
                   CFComparatorFunction comparator, void *context)
{
  CF_OBJC_FUNCDISPATCHV (_kCFArrayTypeID, void, array,
                         "sortUsingFunction:context:", comparator, context);

  GSCArrayQuickSort (array->_contents + range.location, range.length,
                     comparator, context);
}
