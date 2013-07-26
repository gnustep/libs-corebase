/* CFDictionary.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"

#include "GSHashTable.h"
#include "GSObjCRuntime.h"


static CFTypeID _kCFDictionaryTypeID = 0;

static void
CFDictionaryFinalize (CFTypeRef cf)
{
  GSHashTableFinalize ((GSHashTableRef)cf);
}

static Boolean
CFDictionaryEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return GSHashTableEqual ((GSHashTableRef)cf1, (GSHashTableRef)cf2);
}

static CFHashCode
CFDictionaryHash (CFTypeRef cf)
{
  return GSHashTableHash ((GSHashTableRef)cf);
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



CFDictionaryRef
CFDictionaryCreate (CFAllocatorRef allocator, const void **keys,
  const void **values, CFIndex numValues,
  const CFDictionaryKeyCallBacks *keyCallBacks,
  const CFDictionaryValueCallBacks *valueCallBacks)
{
  return (CFDictionaryRef)GSHashTableCreate (allocator, _kCFDictionaryTypeID,
    keys, values, numValues,
    (const GSHashTableKeyCallBacks*)keyCallBacks,
    (const GSHashTableValueCallBacks*)valueCallBacks);
}

CFDictionaryRef
CFDictionaryCreateCopy (CFAllocatorRef allocator, CFDictionaryRef dict)
{
  if (CF_IS_OBJC(_kCFDictionaryTypeID, dict))
    {
      CFIndex count = CFDictionaryGetCount(dict);
      const void **values, **keys;
      CFDictionaryRef copy;

      keys = (const void **) CFAllocatorAllocate(allocator,
        sizeof(const void *) * count, 0);
      values = (const void **) CFAllocatorAllocate(allocator,
        sizeof(const void *) * count, 0);

      CFDictionaryGetKeysAndValues(dict, keys, values);
      copy = CFDictionaryCreate(allocator, keys, values, count,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

      CFAllocatorDeallocate(allocator, keys);
      CFAllocatorDeallocate(allocator, values);

      return copy;
    }

  return (CFDictionaryRef)GSHashTableCreateCopy (allocator,
    (GSHashTableRef)dict);
}

void
CFDictionaryApplyFunction (CFDictionaryRef dict,
                           CFDictionaryApplierFunction applier, void *context)
{
  CFIndex i;
  CFIndex cnt;
  const void **keys;
  const void **values;
  CFAllocatorRef alloc;
  
  cnt = CFDictionaryGetCount (dict);
  alloc = CFGetAllocator(dict);
  keys = CFAllocatorAllocate (alloc, cnt * 2 * sizeof(void*), 0);
  values = keys + cnt;
  CFDictionaryGetKeysAndValues (dict, keys, values);
  
  for (i = 0; i < cnt; i++)
    applier (keys[i], values[i], context);
}

Boolean
CFDictionaryContainsKey (CFDictionaryRef dict, const void *key)
{
  if (CF_IS_OBJC(_kCFDictionaryTypeID, dict))
    {
      void *object;
      CF_OBJC_CALLV (void*, object, dict, "objectForKey:", key);
      return object != NULL;
    }
  
  return GSHashTableContainsKey ((GSHashTableRef)dict, key);
}

Boolean
CFDictionaryContainsValue (CFDictionaryRef dict, const void *value)
{
  if (CF_IS_OBJC(_kCFDictionaryTypeID, dict))
    return CFDictionaryGetCountOfValue(dict, value) ? true : false;
  
  return GSHashTableContainsValue ((GSHashTableRef)dict, value);
}

CFIndex
CFDictionaryGetCount (CFDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, CFIndex, dict, "count");
  
  return GSHashTableGetCount ((GSHashTableRef)dict);
}

CFIndex
CFDictionaryGetCountOfKey (CFDictionaryRef dict, const void *key)
{
  if (CF_IS_OBJC(_kCFDictionaryTypeID, dict))
    return CFDictionaryContainsKey(dict, key) ? 1 : 0;
  
  return GSHashTableGetCountOfKey ((GSHashTableRef)dict, key);
}

CFIndex
CFDictionaryGetCountOfValue (CFDictionaryRef dict, const void *value)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, Boolean, dict,
    "_cfCountOfValue:", value);
  
  return GSHashTableGetCountOfValue ((GSHashTableRef)dict, value);
}

void
CFDictionaryGetKeysAndValues (CFDictionaryRef dict, const void **keys,
  const void **values)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict,
    "getObjects:andKeys:", values, keys);
  
  GSHashTableGetKeysAndValues ((GSHashTableRef)dict, keys, values);
}

const void *
CFDictionaryGetValue (CFDictionaryRef dict, const void *key)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, const void *, dict,
    "objectForKey:", key);
  
  return GSHashTableGetValue ((GSHashTableRef)dict, key);
}

Boolean
CFDictionaryGetValueIfPresent (CFDictionaryRef dict,
  const void *key, const void **value)
{
  const void *v;
  
  v = CFDictionaryGetValue (dict, key);
  if (v)
    {
      if (value)
        *value = v;
      return true;
    }
  
  return false;
}

CFTypeID
CFDictionaryGetTypeID (void)
{
  return _kCFDictionaryTypeID;
}



CFMutableDictionaryRef
CFDictionaryCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
  const CFDictionaryKeyCallBacks *keyCallBacks,
  const CFDictionaryValueCallBacks *valueCallBacks)
{
  return (CFMutableDictionaryRef)GSHashTableCreateMutable (allocator,
    _kCFDictionaryTypeID, capacity,
    (const GSHashTableKeyCallBacks*)keyCallBacks,
    (const GSHashTableValueCallBacks*)valueCallBacks);
}

CFMutableDictionaryRef
CFDictionaryCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFDictionaryRef dict)
{
  if (CF_IS_OBJC(_kCFDictionaryTypeID, dict))
    {
      CFIndex count = CFDictionaryGetCount(dict);
      const void **values, **keys;
      CFMutableDictionaryRef copy;
      CFIndex i;

      keys = (const void **) CFAllocatorAllocate(allocator,
        sizeof(const void *) * count, 0);
      values = (const void **) CFAllocatorAllocate(allocator,
        sizeof(const void *) * count, 0);

      if (count > capacity)
        capacity = count;

      CFDictionaryGetKeysAndValues(dict, keys, values);
      copy = CFDictionaryCreateMutable(allocator, capacity,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

      for (i = 0; i < count; i++)
        CFDictionaryAddValue(copy, keys[i], values[i]);

      CFAllocatorDeallocate(allocator, keys);
      CFAllocatorDeallocate(allocator, values);

      return copy;
    }

  return (CFMutableDictionaryRef)GSHashTableCreateMutableCopy (allocator,
    (GSHashTableRef)dict, capacity);
}

void
CFDictionaryAddValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict,
    "setObject:forKey:", value, key);
  
  GSHashTableAddValue ((GSHashTableRef)dict, key, value);
}

void
CFDictionaryRemoveAllValues (CFMutableDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict, "removeAllObjects");
  
  GSHashTableRemoveAll ((GSHashTableRef)dict);
}

void
CFDictionaryRemoveValue (CFMutableDictionaryRef dict, const void *key)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict,
    "removeObjectForKey:", key);
  
  GSHashTableRemoveValue ((GSHashTableRef)dict, key);
}

void
CFDictionaryReplaceValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict,
    "_cfReplaceValue::", key, value);
  
  GSHashTableReplaceValue ((GSHashTableRef)dict, key, value);
}

void
CFDictionarySetValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDictionaryTypeID, void, dict,
    "_cfSetValue::", key, value);
  
  GSHashTableSetValue ((GSHashTableRef)dict, key, value);
}

