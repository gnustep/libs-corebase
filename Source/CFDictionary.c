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
#include "GSPrivate.h"


static CFTypeID _kCFDictionaryTypeID = 0;

struct __CFDictionary
{
  CFRuntimeBase _parent;
  const CFDictionaryKeyCallBacks   *_keyCallBacks;
  const CFDictionaryValueCallBacks *_valueCallBacks;
  struct GSHashTable _ht;
};

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

static void
CFDictionaryFinalize (CFTypeRef cf)
{
  struct __CFDictionary *d = (struct __CFDictionary*)cf;
  
  CFDictionaryRemoveAllValues (d);
  if (CFDictionaryIsMutable(d))
    CFAllocatorDeallocate (CFGetAllocator(d), d->_ht.array);
}

static Boolean
CFDictionaryEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  struct __CFDictionary *d1 = (struct __CFDictionary*)cf1;
  struct __CFDictionary *d2 = (struct __CFDictionary*)cf2;
  
  if (d1->_ht.count == d2->_ht.count
      && d1->_keyCallBacks == d2->_keyCallBacks
      && d1->_valueCallBacks == d2->_valueCallBacks)
    {
      CFIndex idx;
      const void *key;
      const void *value1;
      const void *value2;
      CFDictionaryEqualCallBack equal;
      
      idx = 0;
      equal = d1->_valueCallBacks->equal;
      while ((key = GSHashTableNext ((struct GSHashTable*)&d1->_ht, &idx)))
        {
          value1 = d1->_ht.array[idx + d1->_ht.size];
          value2 = CFDictionaryGetValue (d2, key);
          
          if (!(equal ? equal (value1, value2) : value1 == value2))
            return false;
          
          ++idx;
        }
      return true;
    }
  
  return false;
}

static CFHashCode
CFDictionaryHash (CFTypeRef cf)
{
  return ((CFDictionaryRef)cf)->_ht.count;
}

static CFStringRef
CFDictionaryCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFSTR("");
}

static CFRuntimeClass CFDictionaryClass =
{
  0,
  "CFDictionary",
  NULL,
  (CFTypeRef(*)(CFAllocatorRef, CFTypeRef))CFDictionaryCreateCopy,
  CFDictionaryFinalize,
  CFDictionaryEqual,
  CFDictionaryHash,
  CFDictionaryCopyFormattingDesc,
  NULL
};

void CFDictionaryInitialize (void)
{
  _kCFDictionaryTypeID = _CFRuntimeRegisterClass (&CFDictionaryClass);
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



struct CFDictionaryContext
{
  CFAllocatorRef a;
  CFDictionaryRef d;
  const void *v;
};

static void
CFDictionaryMoveValueAction (struct GSHashTable *ht1, CFIndex idx1,
  struct GSHashTable *ht2, CFIndex idx2, void *context)
{
  ht2->array[idx2 + ht2->size] = ht1->array[idx1 + ht1->size];
}

static void
CFDictionaryCopyValueAction (struct GSHashTable *ht1, CFIndex idx1,
  struct GSHashTable *ht2, CFIndex idx2, void *context)
{
  struct CFDictionaryContext *c = (struct CFDictionaryContext *)context;
  CFDictionaryRetainCallBack retain;
  
  retain = c->d->_valueCallBacks->retain;
  
  ht2->array[idx2 + ht2->size] = retain ?
    retain(c->a, ht1->array[idx1 + ht1->size]) : ht1->array[idx1 + ht1->size];
}

static Boolean
CFDictionarySetValueAction (struct GSHashTable *ht, CFIndex idx,
  Boolean matched, void *context)
{
  CFDictionaryRetainCallBack retain;
  struct CFDictionaryContext *c = (struct CFDictionaryContext *)context;
  
  if (matched)
    {
      CFDictionaryReleaseCallBack release = c->d->_valueCallBacks->release;
      if (release)
        release (c->a, ht->array[idx + ht->size]);
    }
  
  retain = c->d->_valueCallBacks->retain;
  ht->array[idx + ht->size] = retain ? retain (c->a, c->v) : c->v;
  
  return true;
}

static Boolean
CFDictionaryRemoveValueAction (struct GSHashTable *ht, CFIndex idx,
  Boolean matched, void *context)
{
  struct CFDictionaryContext *c = (struct CFDictionaryContext *)context;
  
  
  CFDictionaryReleaseCallBack release = c->d->_valueCallBacks->release;
  if (release)
    release (c->a, ht->array[idx + ht->size]);
  
  ht->array[idx + ht->size] = NULL;
  
  return true;
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

#define CFDICTIONARY_SIZE \
  (sizeof(struct __CFDictionary) - sizeof(CFRuntimeBase))

#define GET_ARRAY_SIZE(size) ((size) * 2 * sizeof(void*))

CFDictionaryRef
CFDictionaryCreate (CFAllocatorRef allocator, const void **keys,
                    const void **values, CFIndex numValues,
                    const CFDictionaryKeyCallBacks *keyCallBacks,
                    const CFDictionaryValueCallBacks *valueCallBacks)
{
  CFIndex size;
  struct __CFDictionary *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, numValues);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID, CFDICTIONARY_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFDictionaryInit (allocator, new, (const void**)&new[1], size, keys,
        values, numValues, keyCallBacks, valueCallBacks);
    }
  
  return new;
}

CFDictionaryRef
CFDictionaryCreateCopy (CFAllocatorRef allocator, CFDictionaryRef dict)
{
  CFIndex size;
  struct __CFDictionary *new;
  struct GSHashTable ht;
  struct CFDictionaryContext context;
  
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, CFDictionaryRef, dict, "copy");
  
  if (allocator == CFGetAllocator(dict) && !CFDictionaryIsMutable(dict))
    return CFRetain (dict);
  
  size = dict->_ht.size;
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID, CFDICTIONARY_SIZE + GET_ARRAY_SIZE(size), NULL);
  if (new)
    {
      CFDictionaryInit (allocator, new, (const void**)&new[1], size, NULL,
        NULL, 0, dict->_keyCallBacks, dict->_valueCallBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = (const void**)&new[1];
      context.a = allocator;
      context.d = new;
      
      GSHashTableCopyValues ((struct GSHashTable *)&dict->_ht, &ht, NULL,
        NULL, new->_keyCallBacks->hash, new->_keyCallBacks->equal,
        CFDictionaryCopyValueAction, &context);
      
      new->_ht.count = ht.count;
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
  while (GSHashTableNext ((struct GSHashTable*)&dict->_ht, &idx))
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
static void
CFDictionaryCheckCapacityAndGrow (CFMutableDictionaryRef d)
{
  CFIndex oldSize;
  
  oldSize = d->_ht.size;
  if (!GSHashTableIsSuitableSize (oldSize, d->_ht.count + 1))
    {
      CFIndex newSize;
      const void **newArray;
      const void **oldArray;
      struct GSHashTable ht;
      
      newSize = GSHashTableNextSize (oldSize);
      newArray =
        CFAllocatorAllocate (CFGetAllocator(d), GET_ARRAY_SIZE(newSize), 0);
      memset (newArray, 0, GET_ARRAY_SIZE(newSize));
      
      ht.size = newSize;
      ht.count = 0;
      ht.array = newArray;
      GSHashTableCopyValues ((struct GSHashTable *)&d->_ht, &ht, NULL,
        NULL, d->_keyCallBacks->hash, d->_keyCallBacks->equal,
        CFDictionaryMoveValueAction, NULL);
      
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
  CFIndex size;
  const void **array;
  struct __CFDictionary *new;
  
  size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID, CFDICTIONARY_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (allocator, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFDictionarySetMutable (new);
      CFDictionaryInit (allocator, new, array, size, NULL, NULL, 0,
        keyCallBacks, valueCallBacks);
    }
  
  return new;
}

CFMutableDictionaryRef
CFDictionaryCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                               CFDictionaryRef dict)
{
  CFIndex size;
  const void **array;
  struct __CFDictionary *new;
  struct GSHashTable ht;
  struct CFDictionaryContext context;
  
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, CFMutableDictionaryRef, dict,
    "mutableCopyWithZone:", NULL);
  
  size = dict->_ht.size;
  if (size < capacity)
    size = GSHashTableGetSuitableSize (_kGSHashTableDefaultSize, capacity);
  
  new = (struct __CFDictionary *)_CFRuntimeCreateInstance (allocator,
    _kCFDictionaryTypeID, CFDICTIONARY_SIZE, NULL);
  if (new)
    {
      array = CFAllocatorAllocate (allocator, GET_ARRAY_SIZE(size), 0);
      memset (array, 0, GET_ARRAY_SIZE(size));
      
      CFDictionarySetMutable (new);
      CFDictionaryInit (allocator, new, array, size, NULL, NULL, 0,
        dict->_keyCallBacks, dict->_valueCallBacks);
      
      ht.size = size;
      ht.count = 0;
      ht.array = array;
      context.a = allocator;
      context.d = new;
      
      GSHashTableCopyValues ((struct GSHashTable *)&dict->_ht, &ht, NULL,
        NULL, new->_keyCallBacks->hash, new->_keyCallBacks->equal,
        CFDictionaryCopyValueAction, &context);
      
      new->_ht.count = ht.count;
    }
  
  return new;
}

void
CFDictionaryAddValue (CFMutableDictionaryRef dict, const void *key,
                      const void *value)
{
  struct CFDictionaryContext context;
  
  if (key == NULL || !CFDictionaryIsMutable(dict))
    return;
  
  CFDictionaryCheckCapacityAndGrow (dict);
  
  context.a = CFGetAllocator (dict);
  context.d = dict;
  context.v = value;
  
  GSHashTableAddValue (&dict->_ht, key, context.a, dict->_keyCallBacks->retain,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal,
    CFDictionarySetValueAction, &context);
}

void
CFDictionaryRemoveAllValues (CFMutableDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, void, dict, "removeAllObjects");
}

void
CFDictionaryRemoveValue (CFMutableDictionaryRef dict, const void *key)
{
  struct CFDictionaryContext context;
  
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, void, dict,
    "removeObjectForKey:", key);
  
  if (key == NULL || !CFDictionaryIsMutable(dict))
    return;
  
  CFDictionaryCheckCapacityAndGrow (dict);
  
  context.a = CFGetAllocator (dict);
  context.d = dict;
  
  GSHashTableRemoveValue (&dict->_ht, key, context.a,
    dict->_keyCallBacks->retain, dict->_keyCallBacks->hash,
    dict->_keyCallBacks->equal, CFDictionaryRemoveValueAction, &context);
}

void
CFDictionaryReplaceValue (CFMutableDictionaryRef dict, const void *key,
                          const void *value)
{
  struct CFDictionaryContext context;
  context.a = CFGetAllocator (dict);
  context.d = dict;
  context.v = value;
  
  GSHashTableReplaceValue (&dict->_ht, key, context.a,
    dict->_keyCallBacks->retain, dict->_keyCallBacks->hash,
    dict->_keyCallBacks->equal, CFDictionarySetValueAction, &context);
}

void
CFDictionarySetValue (CFMutableDictionaryRef dict, const void *key,
                      const void *value)
{
  struct CFDictionaryContext context;
  
  CF_OBJC_FUNCDISPATCH2(_kCFDictionaryTypeID, void, dict,
    "setObject:forKey:", value, key);
  
  if (key == NULL || !CFDictionaryIsMutable(dict))
    return;
  
  CFDictionaryCheckCapacityAndGrow (dict);
  
  context.a = CFGetAllocator (dict);
  context.d = dict;
  context.v = value;
  
  GSHashTableAddValue (&dict->_ht, key, context.a, dict->_keyCallBacks->retain,
    dict->_keyCallBacks->hash, dict->_keyCallBacks->equal,
    CFDictionarySetValueAction, &context);
}
