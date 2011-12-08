/* CFBag.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFBag.h"
#include "CoreFoundation/CFString.h"
#include "GSHashTable.h"
#include "GSPrivate.h"


static CFTypeID _kCFBagTypeID = 0;

struct __CFBag
{
  CFRuntimeBase         _parent;
  const CFBagCallBacks *_callBacks;
  struct GSHashTable    _ht;
};

enum
{
  _kCFBagIsMutable = (1<<0)
};

CF_INLINE Boolean
CFBagIsMutable (CFBagRef bag)
{
  return ((CFRuntimeBase *)bag)->_flags.info & _kCFBagIsMutable ?
    true : false;
}

CF_INLINE void
CFBagSetMutable (CFBagRef bag)
{
  ((CFRuntimeBase *)bag)->_flags.info |= _kCFBagIsMutable;
}

static void
CFBagFinalize (CFTypeRef cf)
{
  struct __CFBag *b = (struct __CFBag*)cf;
  
  CFBagRemoveAllValues (b);
  if (CFBagIsMutable(b))
    CFAllocatorDeallocate (CFGetAllocator(b), b->_ht.array);
}

static Boolean
CFBagEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  struct __CFBag *b1 = (struct __CFBag*)cf1;
  struct __CFBag *b2 = (struct __CFBag*)cf2;
  
  if (b1->_ht.count == b2->_ht.count
      && b1->_callBacks == b2->_callBacks)
    {
      CFIndex idx;
      const void *value1;
      const void *value2;
      CFBagEqualCallBack equal;
      
      idx = 0;
      equal = b1->_callBacks->equal;
      while ((value1 = GSHashTableNext ((struct GSHashTable*)&b1->_ht, &idx)))
        {
          value1 = b1->_ht.array[idx + b1->_ht.size];
          value2 = CFBagGetValue (b2, value1);
          
          if (!(equal ? equal (value1, value2) : value1 == value2))
            return false;
          
          ++idx;
        }
      return true;
    }
  
  return false;
}

static CFHashCode
CFBagHash (CFTypeRef cf)
{
  return ((CFBagRef)cf)->_ht.count;
}

static CFStringRef
CFBagCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFSTR("");
}

static CFRuntimeClass CFBagClass =
{
  0,
  "CFBag",
  NULL,
  (CFTypeRef(*)(CFAllocatorRef, CFTypeRef))CFBagCreateCopy,
  CFBagFinalize,
  CFBagEqual,
  CFBagHash,
  CFBagCopyFormattingDesc,
  NULL
};

void CFBagInitialize (void)
{
  _kCFBagTypeID = _CFRuntimeRegisterClass (&CFBagClass);
}



const CFBagCallBacks kCFCopyStringBagCallBacks =
{
  0,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFStringCreateCopy,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

const CFBagCallBacks kCFTypeBagCallBacks =
{
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

static const CFBagCallBacks _kCFNullBagCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



static void
CFBagMoveValueAction (struct GSHashTable *ht1, CFIndex idx1,
  struct GSHashTable *ht2, CFIndex idx2, void *context)
{
  CFIndex *counts1 = (CFIndex*)&(ht1->array[ht1->size]);
  CFIndex *counts2 = (CFIndex*)&(ht2->array[ht2->size]);
  
  counts2[idx2] = counts1[idx1];
}

static Boolean
CFBagSetValueAction (struct GSHashTable *ht, CFIndex idx,
  Boolean matched, void *context)
{
  CFIndex *counts = (CFIndex*)&(ht->array[ht->size]);
  counts[idx] += 1;
  
  return true;
}

static Boolean
CFBagRemoveValueAction (struct GSHashTable *ht, CFIndex idx,
  Boolean matched, void *context)
{
  CFIndex *counts = (CFIndex*)&(ht->array[ht->size]);
  counts[idx] -= 1;
  
  return (counts[idx] > 0);
}

static void
CFBagInit (CFAllocatorRef alloc, struct __CFBag *bag, const void **array,
  CFIndex arraySize, const void **values, CFIndex numValues,
  const CFBagCallBacks *callBacks)
{
  CFIndex idx;
  CFIndex arrayIdx;
  CFIndex *counts;
  CFBagRetainCallBack retain;
  
  bag->_callBacks =
    callBacks ? callBacks : &_kCFNullBagCallBacks;
  bag->_ht.size = arraySize;
  bag->_ht.array = array;
  
  if (numValues == 0)
    return;
  
  counts = (CFIndex*)&(array[arraySize]);
  retain = callBacks->retain;
  if (retain)
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&bag->_ht, values[idx],
            callBacks->hash, callBacks->equal);
          
          array[arrayIdx] = retain(alloc, values[idx]);
          counts[idx] += 1;
          bag->_ht.count += 1;
        }
    }
  else
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&bag->_ht, values[idx],
            callBacks->hash, callBacks->equal);
          
          array[arrayIdx] = values[idx];
          counts[idx] += 1;
          bag->_ht.count += 1;
        }
    }
}

#define CFBAG_SIZE \
  (sizeof(struct __CFBag) - sizeof(CFRuntimeBase))

#define GET_ARRAY_SIZE(size) ((size) * (sizeof(void*) + sizeof(CFIndex)))

CFBagRef
CFBagCreate (CFAllocatorRef alloc, const void **values, CFIndex numValues,
  const CFBagCallBacks *callBacks)
{
  CFIndex size;
  struct __CFBag *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, numValues);
  
  new = (struct __CFBag *)_CFRuntimeCreateInstance (alloc,
    _kCFBagTypeID, CFBAG_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFBagInit (alloc, new, (const void**)&new[1], size, values, numValues,
        callBacks);
    }
  
  return new;
}

CFBagRef
CFBagCreateCopy (CFAllocatorRef alloc, CFBagRef bag)
{
  CFIndex size;
  struct __CFBag *new;
  struct GSHashTable ht;
  
  if (alloc == CFGetAllocator(bag) && !CFBagIsMutable(bag))
    return CFRetain (bag);
  
  size = bag->_ht.size;
  
  new = (struct __CFBag *)_CFRuntimeCreateInstance (alloc,
    _kCFBagTypeID, CFBAG_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFBagInit (alloc, new, (const void**)&new[1], size, NULL, 0,
        bag->_callBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = (const void**)&new[1];
      
      GSHashTableCopyValues ((struct GSHashTable *)&bag->_ht, &ht, alloc,
        new->_callBacks->retain, new->_callBacks->hash, new->_callBacks->equal,
        NULL, NULL);
      
      new->_ht.count = ht.count;
    }
  
  return new;
}

void
CFBagApplyFunction (CFBagRef bag, CFBagApplierFunction applier, void *context)
{
  CFIndex idx;
  const void *value;
  
  if (applier == NULL)
    return;
  
  idx = 0;
  while ((value = GSHashTableNext ((struct GSHashTable*)&bag->_ht, &idx)))
    applier (value, context);
}

Boolean
CFBagContainsValue (CFBagRef bag, const void *value)
{
  CFIndex idx;
  
  if (value == NULL)
    return false;
  
  idx = GSHashTableFind ((struct GSHashTable*)&bag->_ht, value,
    bag->_callBacks->hash, bag->_callBacks->equal);
  
  return bag->_ht.array[idx] ? true : false;
}

CFIndex
CFBagGetCount (CFBagRef bag)
{
  return bag->_ht.count;
}

CFIndex
CFBagGetCountOfKey (CFBagRef bag, const void *value)
{
  CFIndex idx;
  
  if (value == NULL)
    return 0;
  
  idx = GSHashTableFind ((struct GSHashTable*)&bag->_ht, value,
    bag->_callBacks->hash, bag->_callBacks->equal);
  
  return (CFIndex)(bag->_ht.array[idx + bag->_ht.size]);
}

void
CFBagGetValues (CFBagRef bag, const void **values)
{
}

const void *
CFBagGetValue (CFBagRef bag, const void *value)
{
  CFIndex idx;
  
  if (value == NULL)
    return NULL;
  
  idx = GSHashTableFind ((struct GSHashTable*)&bag->_ht, value,
    bag->_callBacks->hash, bag->_callBacks->equal);
  return bag->_ht.array[idx];
}

Boolean
CFBagGetValueIfPresent (CFBagRef bag, const void *candidate,
  const void **value)
{
  CFIndex idx;
  const void *v;
  
  if (candidate == NULL)
    return false;
  
  idx = GSHashTableFind ((struct GSHashTable*)&bag->_ht, candidate,
    bag->_callBacks->hash, bag->_callBacks->equal);
  
  v = bag->_ht.array[idx];
  if (v == NULL)
    return false;
  
  if (value)
    *value = v;
  return true;
}

CFTypeID
CFBagGetTypeID (void)
{
  return _kCFBagTypeID;
}



//
// CFMutableBag
//
static void
CFBagCheckCapacityAndGrow (CFMutableBagRef bag)
{
  CFIndex oldSize;
  
  oldSize = bag->_ht.size;
  if (!GSHashTableIsSuitableSize (oldSize, bag->_ht.count + 1))
    {
      CFIndex newSize;
      const void **newArray;
      const void **oldArray;
      struct GSHashTable ht;
      
      newSize = GSHashTableNextSize (oldSize);
      newArray =
        CFAllocatorAllocate (CFGetAllocator(bag), GET_ARRAY_SIZE(newSize), 0);
      memset (newArray, 0, GET_ARRAY_SIZE(newSize));
      
      ht.size = newSize;
      ht.count = 0;
      ht.array = newArray;
      GSHashTableCopyValues ((struct GSHashTable *)&bag->_ht, &ht, NULL,
        NULL, bag->_callBacks->hash, bag->_callBacks->equal,
        CFBagMoveValueAction, NULL);
      
      oldArray = bag->_ht.array;
      bag->_ht.array = newArray;
      bag->_ht.size = newSize;
      CFAllocatorDeallocate (CFGetAllocator(bag), oldArray);
    }
}

CFMutableBagRef
CFBagCreateMutable (CFAllocatorRef alloc, CFIndex capacity,
  const CFBagCallBacks *callBacks)
{
  CFIndex size;
  const void **array;
  struct __CFBag *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFBag *)_CFRuntimeCreateInstance (alloc,
    _kCFBagTypeID, CFBAG_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (alloc, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFBagSetMutable (new);
      CFBagInit (alloc, new, array, size, NULL, 0, callBacks);
    }
  
  return new;
}

CFMutableBagRef
CFBagCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity, CFBagRef bag)
{
  CFIndex size;
  const void **array;
  struct __CFBag *new;
  struct GSHashTable ht;
  
  size = bag->_ht.size;
  if (size < capacity)
    size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFBag *)_CFRuntimeCreateInstance (alloc,
    _kCFBagTypeID, CFBAG_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (alloc, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFBagSetMutable (new);
      CFBagInit (alloc, new, array, size, NULL, 0, bag->_callBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = array;
      
      GSHashTableCopyValues ((struct GSHashTable *)&bag->_ht, &ht, alloc,
        new->_callBacks->retain, new->_callBacks->hash, new->_callBacks->equal,
        NULL, NULL);
      
      new->_ht.count = ht.count;
    }
  
  return new;
}

void
CFBagAddValue (CFMutableBagRef bag, const void *value)
{
  if (value == NULL || !CFBagIsMutable(bag))
    return;
  
  CFBagCheckCapacityAndGrow (bag);
  
  GSHashTableAddValue (&bag->_ht, value, CFGetAllocator(bag),
    bag->_callBacks->retain, bag->_callBacks->hash, bag->_callBacks->equal,
    CFBagSetValueAction, NULL);
}

void
CFBagRemoveAllValues (CFMutableBagRef bag)
{
}

void
CFBagRemoveValue (CFMutableBagRef bag, const void *value)
{
  if (value == NULL || !CFBagIsMutable(bag))
    return;
  
  CFBagCheckCapacityAndGrow (bag);
  
  GSHashTableRemoveValue (&bag->_ht, value, CFGetAllocator(bag),
    bag->_callBacks->release, bag->_callBacks->hash,
    bag->_callBacks->equal, CFBagRemoveValueAction, NULL);
}

void
CFBagReplaceValue (CFMutableBagRef bag, const void *value)
{
  GSHashTableReplaceValue (&bag->_ht, value, CFGetAllocator(bag),
    bag->_callBacks->retain, bag->_callBacks->hash,
    bag->_callBacks->equal, CFBagSetValueAction, NULL);
}

void
CFBagSetValue (CFMutableBagRef bag, const void *value)
{
  if (value == NULL || !CFBagIsMutable(bag))
    return;
  
  CFBagCheckCapacityAndGrow (bag);
  GSHashTableAddValue (&bag->_ht, value, CFGetAllocator(bag),
    bag->_callBacks->retain, bag->_callBacks->hash, bag->_callBacks->equal,
    CFBagSetValueAction, NULL);
}
