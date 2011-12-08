/* CFSet.c
   
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
#include "CoreFoundation/CFSet.h"
#include "CoreFoundation/CFString.h"
#include "GSHashTable.h"
#include "GSPrivate.h"


static CFTypeID _kCFSetTypeID = 0;

struct __CFSet
{
  CFRuntimeBase         _parent;
  const CFSetCallBacks *_callBacks;
  struct GSHashTable    _ht;
};

enum
{
  _kCFSetIsMutable = (1<<0)
};

CF_INLINE Boolean
CFSetIsMutable (CFSetRef set)
{
  return ((CFRuntimeBase *)set)->_flags.info & _kCFSetIsMutable ?
    true : false;
}

CF_INLINE void
CFSetSetMutable (CFSetRef set)
{
  ((CFRuntimeBase *)set)->_flags.info |= _kCFSetIsMutable;
}

static void
CFSetFinalize (CFTypeRef cf)
{
  struct __CFSet *s = (struct __CFSet*)cf;
  
  CFSetRemoveAllValues (s);
  if (CFSetIsMutable(s))
    CFAllocatorDeallocate (CFGetAllocator(s), s->_ht.array);
}

static Boolean
CFSetEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  struct __CFSet *s1 = (struct __CFSet*)cf1;
  struct __CFSet *s2 = (struct __CFSet*)cf2;
  
  if (s1->_ht.count == s2->_ht.count
      && s1->_callBacks == s2->_callBacks)
    {
      CFIndex idx;
      const void *value1;
      const void *value2;
      CFSetEqualCallBack equal;
      
      idx = 0;
      equal = s1->_callBacks->equal;
      while ((value1 = GSHashTableNext ((struct GSHashTable*)&s1->_ht, &idx)))
        {
          value2 = CFSetGetValue (s2, value1);
          
          if (!(equal ? equal (value1, value2) : value1 == value2))
            return false;
          
          ++idx;
        }
      return true;
    }
  
  return false;
}

static CFHashCode
CFSetHash (CFTypeRef cf)
{
  return ((CFSetRef)cf)->_ht.count;
}

static CFStringRef
CFSetCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFSTR("");
}

static CFRuntimeClass CFSetClass =
{
  0,
  "CFSet",
  NULL,
  (CFTypeRef(*)(CFAllocatorRef, CFTypeRef))CFSetCreateCopy,
  CFSetFinalize,
  CFSetEqual,
  CFSetHash,
  CFSetCopyFormattingDesc,
  NULL
};

void CFSetInitialize (void)
{
  _kCFSetTypeID = _CFRuntimeRegisterClass (&CFSetClass);
}



const CFSetCallBacks kCFCopyStringSetCallBacks =
{
  0,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFStringCreateCopy,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

const CFSetCallBacks kCFTypeSetCallBacks =
{
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

static const CFSetCallBacks _kCFNullSetCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



static void
CFSetInit (CFAllocatorRef alloc, struct __CFSet *set, const void **array,
  CFIndex arraySize, const void **values, CFIndex numValues,
  const CFSetCallBacks *callBacks)
{
  CFIndex idx;
  CFIndex arrayIdx;
  CFSetRetainCallBack retain;
  
  set->_callBacks =
    callBacks ? callBacks : &_kCFNullSetCallBacks;
  set->_ht.size = arraySize;
  set->_ht.array = array;
  
  if (numValues == 0)
    return;
  
  retain = callBacks->retain;
  if (retain)
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&set->_ht, values[idx],
            callBacks->hash, callBacks->equal);
          
          array[arrayIdx] = retain(alloc, values[idx]);
          set->_ht.count += 1;
        }
    }
  else
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&set->_ht, values[idx],
            callBacks->hash, callBacks->equal);
          
          array[arrayIdx] = values[idx];
          set->_ht.count += 1;
        }
    }
}

#define CFSET_SIZE (sizeof(struct __CFSet) - sizeof(CFRuntimeBase))

#define GET_ARRAY_SIZE(size) ((size) * sizeof(void*))

CFSetRef
CFSetCreate (CFAllocatorRef alloc, const void **values, CFIndex numValues,
  const CFSetCallBacks *callBacks)
{
  CFIndex size;
  struct __CFSet *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, numValues);
  
  new = (struct __CFSet *)_CFRuntimeCreateInstance (alloc,
    _kCFSetTypeID, CFSET_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFSetInit (alloc, new, (const void**)&new[1], size, values, numValues,
        callBacks);
    }
  
  return new;
}

CFSetRef
CFSetCreateCopy (CFAllocatorRef alloc, CFSetRef set)
{
  CFIndex size;
  struct __CFSet *new;
  struct GSHashTable ht;
  
  if (alloc == CFGetAllocator(set) && !CFSetIsMutable(set))
    return CFRetain (set);
  
  size = set->_ht.size;
  
  new = (struct __CFSet *)_CFRuntimeCreateInstance (alloc,
    _kCFSetTypeID, CFSET_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFSetInit (alloc, new, (const void**)&new[1], size, NULL, 0,
        set->_callBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = (const void**)&new[1];
      
      GSHashTableCopyValues ((struct GSHashTable *)&set->_ht, &ht, alloc,
        new->_callBacks->retain, new->_callBacks->hash, new->_callBacks->equal,
        NULL, NULL);
      
      new->_ht.count = ht.count;
    }
  
  return new;
}

void
CFSetApplyFunction (CFSetRef set, CFSetApplierFunction applier, void *context)
{
  CFIndex idx;
  const void *value;
  
  if (applier == NULL)
    return;
  
  idx = 0;
  while ((value = GSHashTableNext ((struct GSHashTable*)&set->_ht, &idx)))
    applier (value, context);
}

Boolean
CFSetContainsValue (CFSetRef set, const void *value)
{
  const void *v;
  
  if (value == NULL)
    return 0;
  
  v = CFSetGetValue (set, value);
  
  return v ? true : false;
}

CFIndex
CFSetGetCount (CFSetRef set)
{
  return set->_ht.count;
}

CFIndex
CFSetGetCountOfValue (CFSetRef set, const void *value)
{
  const void *v;
  
  if (value == NULL)
    return 0;
  
  v = CFSetGetValue (set, value);
  
  return v ? 1 : 0;
}

void
CFSetGetValues (CFSetRef set, const void **values)
{
}

const void *
CFSetGetValue (CFSetRef set, const void *value)
{
  CFIndex idx;
  
  if (value == NULL)
    return NULL;
  
  idx = GSHashTableFind ((struct GSHashTable*)&set->_ht, value,
    set->_callBacks->hash, set->_callBacks->equal);
  return set->_ht.array[idx];
}

Boolean
CFSetGetValueIfPresent (CFSetRef set, const void *candidate,
  const void **value)
{
  const void *v;
  
  if (value == NULL)
    return 0;
  
  v = CFSetGetValue (set, value);
  if (v == NULL)
    return false;
  
  if (value)
    *value = v;
  return true;
}

CFTypeID
CFSetGetTypeID (void)
{
  return _kCFSetTypeID;
}



//
// CFMutableSet
//
static void
CFSetCheckCapacityAndGrow (CFMutableSetRef set)
{
  CFIndex oldSize;
  
  oldSize = set->_ht.size;
  if (!GSHashTableIsSuitableSize (oldSize, set->_ht.count + 1))
    {
      CFIndex newSize;
      const void **newArray;
      const void **oldArray;
      struct GSHashTable ht;
      
      newSize = GSHashTableNextSize (oldSize);
      newArray =
        CFAllocatorAllocate (CFGetAllocator(set), GET_ARRAY_SIZE(newSize), 0);
      memset (newArray, 0, GET_ARRAY_SIZE(newSize));
      
      ht.size = newSize;
      ht.count = 0;
      ht.array = newArray;
      GSHashTableCopyValues ((struct GSHashTable *)&set->_ht, &ht, NULL,
        NULL, set->_callBacks->hash, set->_callBacks->equal, NULL, NULL);
      
      oldArray = set->_ht.array;
      set->_ht.array = newArray;
      set->_ht.size = newSize;
      CFAllocatorDeallocate (CFGetAllocator(set), oldArray);
    }
}

CFMutableSetRef
CFSetCreateMutable (CFAllocatorRef alloc, CFIndex capacity,
  const CFSetCallBacks *callBacks)
{
  CFIndex size;
  const void **array;
  struct __CFSet *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFSet *)_CFRuntimeCreateInstance (alloc,
    _kCFSetTypeID, CFSET_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (alloc, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFSetSetMutable (new);
      CFSetInit (alloc, new, array, size, NULL, 0, callBacks);
    }
  
  return new;
}

CFMutableSetRef
CFSetCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity, CFSetRef set)
{
  CFIndex size;
  const void **array;
  struct __CFSet *new;
  struct GSHashTable ht;
  
  size = set->_ht.size;
  if (size < capacity)
    size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFSet *)_CFRuntimeCreateInstance (alloc,
    _kCFSetTypeID, CFSET_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (alloc, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFSetSetMutable (new);
      CFSetInit (alloc, new, array, size, NULL, 0, set->_callBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = array;
      
      GSHashTableCopyValues ((struct GSHashTable *)&set->_ht, &ht, alloc,
        new->_callBacks->retain, new->_callBacks->hash, new->_callBacks->equal,
        NULL, NULL);
      
      new->_ht.count = ht.count;
    }
  
  return new;
}

void
CFSetAddValue (CFMutableSetRef set, const void *value)
{
  if (value == NULL || !CFSetIsMutable(set))
    return;
  
  CFSetCheckCapacityAndGrow (set);
  
  GSHashTableAddValue (&set->_ht, value, CFGetAllocator(set),
    set->_callBacks->retain, set->_callBacks->hash, set->_callBacks->equal,
    NULL, NULL);
}

void
CFSetRemoveAllValues (CFMutableSetRef set)
{
  CFIndex idx;
  const void *v;
  CFAllocatorRef alloc = CFGetAllocator(set);
  
  if (set->_callBacks->release)
    {
      for (idx = 0 ; (v = GSHashTableNext(&set->_ht, &idx)) ; ++idx)
        set->_callBacks->release (alloc, v);
    }
  
  memset (set->_ht.array, 0, GET_ARRAY_SIZE(set->_ht.size));
}

void
CFSetRemoveValue (CFMutableSetRef set, const void *value)
{
  if (value == NULL || !CFSetIsMutable(set))
    return;
  
  CFSetCheckCapacityAndGrow (set);
  
  GSHashTableRemoveValue (&set->_ht, value, CFGetAllocator(set),
    set->_callBacks->release, set->_callBacks->hash,
    set->_callBacks->equal, NULL, NULL);
}

void
CFSetReplaceValue (CFMutableSetRef set, const void *value)
{
  GSHashTableReplaceValue (&set->_ht, value, CFGetAllocator(set),
    set->_callBacks->retain, set->_callBacks->hash,
    set->_callBacks->equal, NULL, NULL);
}

void
CFSetSetValue (CFMutableSetRef set, const void *value)
{
  if (value == NULL || !CFSetIsMutable(set))
    return;
  
  CFSetCheckCapacityAndGrow (set);
  GSHashTableAddValue (&set->_ht, value, CFGetAllocator(set),
    set->_callBacks->retain, set->_callBacks->hash, set->_callBacks->equal,
    NULL, NULL);
}
