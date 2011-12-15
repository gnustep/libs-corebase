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
  return (CFDictionaryRef)GSHashTableCreateCopy (allocator,
    (GSHashTableRef)dict);
}

void
CFDictionaryApplyFunction (CFDictionaryRef dict,
  CFDictionaryApplierFunction applier, void *context)
{
  
}

Boolean
CFDictionaryContainsKey (CFDictionaryRef dict, const void *key)
{
  return GSHashTableContainsKey ((GSHashTableRef)dict, key);
}

Boolean
CFDictionaryContainsValue (CFDictionaryRef dict, const void *value)
{
  return GSHashTableContainsValue ((GSHashTableRef)dict, value);
}

CFIndex
CFDictionaryGetCount (CFDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, CFIndex, dict, "count");
  
  return GSHashTableGetCount ((GSHashTableRef)dict);
}

CFIndex
CFDictionaryGetCountOfKey (CFDictionaryRef dict, const void *key)
{
  return GSHashTableGetCountOfKey ((GSHashTableRef)dict, key);
}

CFIndex
CFDictionaryGetCountOfValue (CFDictionaryRef dict, const void *value)
{
  return GSHashTableGetCountOfValue ((GSHashTableRef)dict, value);
}

void
CFDictionaryGetKeysAndValues (CFDictionaryRef dict, const void **keys,
  const void **values)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDictionaryTypeID, void, dict,
    "getObjects:andKeys:", values, keys);
  
  GSHashTableGetKeysAndValues ((GSHashTableRef)dict, keys, values);
}

const void *
CFDictionaryGetValue (CFDictionaryRef dict, const void *key)
{
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, const void *, dict,
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



//
// CFMutableDictionary
//
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
  return (CFMutableDictionaryRef)GSHashTableCreateMutableCopy (allocator,
    (GSHashTableRef)dict, capacity);
}

void
CFDictionaryAddValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  GSHashTableAddValue ((GSHashTableRef)dict, key, value);
}

void
CFDictionaryRemoveAllValues (CFMutableDictionaryRef dict)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDictionaryTypeID, void, dict, "removeAllObjects");
  
  GSHashTableRemoveAll ((GSHashTableRef)dict);
}

void
CFDictionaryRemoveValue (CFMutableDictionaryRef dict, const void *key)
{
  CF_OBJC_FUNCDISPATCH1(_kCFDictionaryTypeID, void, dict,
    "removeObjectForKey:", key);
  
  GSHashTableRemoveValue ((GSHashTableRef)dict, key);
}

void
CFDictionaryReplaceValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  GSHashTableReplaceValue ((GSHashTableRef)dict, key, value);
}

void
CFDictionarySetValue (CFMutableDictionaryRef dict, const void *key,
  const void *value)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDictionaryTypeID, void, dict,
    "setObject:forKey:", value, key);
  
  GSHashTableSetValue ((GSHashTableRef)dict, key, value);
}
