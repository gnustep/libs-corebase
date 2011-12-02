/* CFDictionary.c
   
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
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "GSHashTable.h"
#include "callbacks.h"
#include "objc_interface.h"
#include "atomic_ops.h"



struct __CFDictionary
{
  CFRuntimeBase _parent;
  const CFDictionaryKeyCallBacks   *_keyCallBacks;
  const CFDictionaryValueCallBacks *_valueCallBacks;
  struct GSHashTable _ht;
};

static CFTypeID _kCFDictionaryTypeID = 0;

static CFRuntimeClass CFDictionaryClass =
{
  0,
  "CFDictionary",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFDictionaryInitialize (void)
{
  _kCFDictionaryTypeID = _CFRuntimeRegisterClass (&CFDictionaryClass);
}

enum
{
  _kCFDictionaryIsMutable = (1<<0)
};

CF_INLINE Boolean
CFDictionaryIsMutable (CFDictionaryRef dict)
{
  return ((CFRuntimeBase *)dict)->_flags.info & _kCFDictionaryIsMutable ?
    true : false;
}

CF_INLINE void
CFDictionarySetMutable (CFDictionaryRef dict)
{
  ((CFRuntimeBase *)dict)->_flags.info |= _kCFDictionaryIsMutable;
}



const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks =
{
  0,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFStringCreateCopy,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks =
{
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks =
{
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual
};

const CFDictionaryKeyCallBacks _kCFNullDictionaryKeyCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const CFDictionaryValueCallBacks _kCFNullDictionaryValueCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};



static void
CFDictionaryMoveAllKeysAndValues (CFDictionaryRef d, struct GSHashTable *ht)
{
  CFIndex idx;
  CFIndex newIdx;
  CFIndex newSize;
  CFIndex oldSize;
  const void **newArray;
  const void **oldArray;
  CFDictionaryRetainCallBack keyRetain;
  CFDictionaryRetainCallBack valueRetain;
  CFAllocatorRef alloc = CFGetAllocator (d);
  
  idx = 0;
  newSize = ht->size;
  oldSize = d->_ht.size;
  newArray = ht->array;
  oldArray = d->_ht.array;
  keyRetain = d->_keyCallBacks->retain;
  valueRetain = d->_valueCallBacks->retain;
  while (CFHashTableNext ((struct GSHashTable*)&d->_ht, &idx))
    {
      newIdx = GSHashTableFind (ht, oldArray[idx], d->_keyCallBacks->hash,
        NULL);
      newArray[newIdx] =
        keyRetain ? keyRetain(alloc, oldArray[idx]) : oldArray[idx];
      newArray[newIdx + newSize] = valueRetain ?
        valueRetain(alloc, oldArray[idx + oldSize]) : oldArray[idx + oldSize];
      ++idx;
    }
}

static void
CFDictionaryInit (CFAllocatorRef alloc, struct __CFDictionary *dict,
  const void **array, CFIndex arraySize, const void **keys,
  const void **values, CFIndex numValues,
  const CFDictionaryKeyCallBacks *keyCallBacks,
  const CFDictionaryValueCallBacks *valueCallBacks)
{
  CFIndex idx;
  CFIndex arrayIdx;
  CFDictionaryRetainCallBack keyRetain;
  CFDictionaryRetainCallBack valueRetain;
  
  dict->_keyCallBacks =
    keyCallBacks ? keyCallBacks : &_kCFNullDictionaryKeyCallBacks;
  dict->_valueCallBacks =
    valueCallBacks ? valueCallBacks : &_kCFNullDictionaryValueCallBacks;
  dict->_ht.size = arraySize;
  dict->_ht.array = array;
  
  if (numValues == 0)
    return;
  
  keyRetain = keyCallBacks->retain;
  valueRetain = valueCallBacks->retain;
  if (keyRetain && valueRetain)
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&dict->_ht, keys[idx],
            keyCallBacks->hash, keyCallBacks->equal);
          if (array[arrayIdx])
            continue;
          
          array[arrayIdx] = keyRetain(alloc, keys[idx]);
          array[arrayIdx + arraySize] = valueRetain(alloc, values[idx]);
          dict->_ht.count += 1;
        }
    }
  else if (keyRetain)
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&dict->_ht, keys[idx],
            keyCallBacks->hash, keyCallBacks->equal);
          if (array[arrayIdx])
            continue;
          
          array[arrayIdx] = keyRetain(alloc, keys[idx]);
          array[arrayIdx + arraySize] = values[idx];
          dict->_ht.count += 1;
        }
    }
  else if (valueRetain)
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&dict->_ht, keys[idx],
            keyCallBacks->hash, keyCallBacks->equal);
          if (array[arrayIdx])
            continue;
          
          array[arrayIdx] = keys[idx];
          array[arrayIdx + arraySize] = valueRetain(alloc, values[idx]);
          dict->_ht.count += 1;
        }
    }
  else
    {
      for (idx = 0 ; idx < numValues ; ++idx)
        {
          arrayIdx = GSHashTableFind (&dict->_ht, keys[idx],
            keyCallBacks->hash, keyCallBacks->equal);
          if (array[arrayIdx])
            continue;
          
          array[arrayIdx] = keys[idx];
          array[arrayIdx + arraySize] = values[idx];
          dict->_ht.count += 1;
        }
    }
}

#define CF_DICTIONARY_ADJUST_SIZE(size, old, min) do { \
  size = old; \
  while (!GSHashTableIsAppropriateSize (size, min)) \
    size = GSHashTableNextSize (size); \
} while(0)

CFDictionaryRef
CFDictionaryCreate (CFAllocatorRef allocator, const void **keys,
                    const void **values, CFIndex numValues,
                    const CFDictionaryKeyCallBacks *keyCallBacks,
                    const CFDictionaryValueCallBacks *valueCallBacks)
{
  CFIndex tableSize;
  CFIndex arraySize;
  struct __CFDictionary *new;
  
  CF_DICTIONARY_ADJUST_SIZE (tableSize, _kGSHashTableDefaultSize, numValues);
  
  /* Multiply the size by two because we need space for the values, too. */
  arraySize = tableSize * 2 * sizeof(const void *);
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID,
    sizeof(struct __CFDictionary) - sizeof(CFRuntimeBase) + arraySize, NULL);
  if (new)
    {
      CFDictionaryInit (allocator, new, (const void**)&new[1], tableSize, keys,
        values, numValues, keyCallBacks, valueCallBacks);
    }
  
  return new;
}

CFDictionaryRef
CFDictionaryCreateCopy (CFAllocatorRef allocator, CFDictionaryRef dict)
{
  CFIndex size;
  CFIndex arraySize;
  struct __CFDictionary *new;
  
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, CFDictionaryRef, dict, "copy");
  
  if (allocator == CFGetAllocator(dict) && !CFDictionaryIsMutable(dict))
    return CFRetain (dict);
  
  size = dict->_ht.size;
  arraySize = size * 2 * sizeof(void*);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID,
    sizeof(struct __CFDictionary) - sizeof(CFRuntimeBase) + arraySize, NULL);
  if (new)
    {
      CFDictionaryInit (allocator, new, (const void**)&new[1], size, NULL,
        NULL, 0, dict->_keyCallBacks, dict->_valueCallBacks);
      CFDictionaryMoveAllKeysAndValues (dict, &new->_ht);
    }
  
  return new;
}

void
CFDictionaryApplyFunction (CFDictionaryRef dict,
                           CFDictionaryApplierFunction applier, void *context)
{
  CFIndex idx;
  CFIndex size;
  const void **array;
  
  if (applier == NULL)
    return;
  
  idx = 0;
  size = dict->_ht.size;
  array = dict->_ht.array;
  while (CFHashTableNext ((struct GSHashTable*)&dict->_ht, &idx))
    {
      applier (array[idx], array[idx + size], context);
    }
}

Boolean
CFDictionaryContainsKey (CFDictionaryRef dict, const void *key)
{
  CFIndex idx;
  
  if (key == NULL)
    return false;
  
  idx = GSHashTableFind ((struct GSHashTable*)&dict->_ht, key,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal);
  
  return dict->_ht.array[idx] ? true : false;
}

Boolean
CFDictionaryContainsValue (CFDictionaryRef dict, const void *value)
{
  return false;
}

CFIndex
CFDictionaryGetCount (CFDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, CFIndex, dict, "count");
  
  return dict->_ht.count;
}

CFIndex
CFDictionaryGetCountOfKey (CFDictionaryRef dict, const void *key)
{
  CFIndex idx;
  
  if (key == NULL)
    return 0;
  
  idx = GSHashTableFind ((struct GSHashTable*)&dict->_ht, key,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal);
  
  return dict->_ht.array[idx] ? 1 : 0;
}

CFIndex
CFDictionaryGetCountOfValue (CFDictionaryRef dict, const void *value)
{
  return 0;
}

void
CFDictionaryGetKeysAndValues (CFDictionaryRef dict, const void **keys,
                              const void **values)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDictionaryTypeID, void, dict,
    "getObjects:andKeys:", values, keys);
}

const void *
CFDictionaryGetValue (CFDictionaryRef dict, const void *key)
{
  CFIndex idx;
  
  if (dict == NULL)
    return NULL;
  
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, const void *, dict,
    "objectForKey:", key);
  
  if (key == NULL)
    return NULL;
  
  idx = GSHashTableFind ((struct GSHashTable*)&dict->_ht, key,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal);
  return dict->_ht.array[idx + dict->_ht.size];
}

Boolean
CFDictionaryGetValueIfPresent (CFDictionaryRef dict,
  const void *key, const void **value)
{
  CFIndex idx;
  const void *v;
  
  if (key == NULL)
    return false;
  
  idx = GSHashTableFind ((struct GSHashTable*)&dict->_ht, key,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal);
  
  v = dict->_ht.array[idx + dict->_ht.size];
  if (v == NULL)
    return false;
  
  if (value)
    *value = v;
  return true;
}

CFTypeID
CFDictionaryGetTypeID (void)
{
  return _kCFDictionaryTypeID;
}



//
// CFMutableDictionary
//
CF_INLINE void
CFDictionaryCheckCapacityAndGrow (CFMutableDictionaryRef d)
{
  CFIndex oldSize;
  
  oldSize = d->_ht.size;
  if (!GSHashTableIsAppropriateSize (oldSize, d->_ht.count + 1))
    {
      CFIndex actualSize;
      CFIndex newSize;
      const void **newArray;
      const void **oldArray;
      struct GSHashTable ht;
      
      newSize = GSHashTableNextSize (oldSize);
      actualSize = newSize * 2 * sizeof(void*);
      
      newArray = CFAllocatorAllocate (CFGetAllocator(d), actualSize, 0);
      memset (newArray, 0, actualSize);
      
      ht.size = newSize;
      ht.count = 0;
      CFDictionaryMoveAllKeysAndValues (d, &ht);
      
      oldArray = d->_ht.array;
      d->_ht.array = newArray;
      d->_ht.size = newSize;
      CFAllocatorDeallocate (CFGetAllocator(d), oldArray);
    }
}

CFMutableDictionaryRef
CFDictionaryCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
                           const CFDictionaryKeyCallBacks *keyCallBacks,
                           const CFDictionaryValueCallBacks *valueCallBacks)
{
  CFIndex arraySize;
  CFIndex size;
  const void **array;
  struct __CFDictionary *new;
  
  CF_DICTIONARY_ADJUST_SIZE (arraySize, _kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID,
    sizeof(struct __CFDictionary) - sizeof(CFRuntimeBase), NULL);
  if (new)
    {
      size = arraySize * 2 * sizeof(void*);
      array = CFAllocatorAllocate (allocator, size, 0);
      memset (array, 0, size);
      
      CFDictionarySetMutable (new);
      CFDictionaryInit (allocator, new, array, arraySize, NULL, NULL, 0,
        keyCallBacks, valueCallBacks);
    }
  
  return new;
}

CFMutableDictionaryRef
CFDictionaryCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                               CFDictionaryRef dict)
{
  CFIndex arraySize;
  CFIndex size;
  const void **array;
  struct __CFDictionary *new;
  
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, CFMutableDictionaryRef, dict,
    "mutableCopyWithZone:", NULL);
  
  arraySize = dict->_ht.size;
  if (arraySize < capacity)
    CF_DICTIONARY_ADJUST_SIZE (arraySize, arraySize, capacity);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID,
    sizeof(struct __CFDictionary) - sizeof(CFRuntimeBase), NULL);
  if (new)
    {
      size = arraySize * 2 * sizeof(void*);
      array = CFAllocatorAllocate (allocator, size, 0);
      memset (array, 0, size);
      
      CFDictionarySetMutable (new);
      CFDictionaryInit (allocator, new, array, arraySize, NULL, NULL, 0,
        dict->_keyCallBacks, dict->_valueCallBacks);
      CFDictionaryMoveAllKeysAndValues (dict, &new->_ht);
    }
  
  return new;
}

void
CFDictionaryAddValue (CFMutableDictionaryRef dict, const void *key,
                      const void *value)
{
  CFIndex idx;
  CFDictionaryRetainCallBack keyRetain;
  CFDictionaryRetainCallBack valueRetain;
  
  if (key == NULL || !CFDictionaryIsMutable(dict))
    return;
  
  keyRetain = dict->_keyCallBacks->retain;
  valueRetain = dict->_valueCallBacks->retain;
  CFDictionaryCheckCapacityAndGrow (dict);
  
  idx = GSHashTableFind ((struct GSHashTable*)&dict->_ht, key,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal);
  dict->_ht.array[idx] =
    keyRetain ? keyRetain(CFGetAllocator(dict), key) : key;
  dict->_ht.array[idx + dict->_ht.size] = 
    valueRetain ? valueRetain(CFGetAllocator(dict), value) : value;
  ++(dict->_ht.count);
}

void
CFDictionaryRemoveAllValues (CFMutableDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, void, dict, "removeAllObjects");
}

void
CFDictionaryRemoveValue (CFMutableDictionaryRef dict, const void *key)
{
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, void, dict,
    "removeObjectForKey:", key);
}

void
CFDictionaryReplaceValue (CFMutableDictionaryRef dict, const void *key,
                          const void *value)
{
  
}

void
CFDictionarySetValue (CFMutableDictionaryRef dict, const void *key,
                      const void *value)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDictionaryTypeID, void, dict,
    "setObject:forKey:", value, key);
}
