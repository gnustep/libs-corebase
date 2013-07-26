/* CFBag.c
   
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
#include "CoreFoundation/CFBag.h"
#include "CoreFoundation/CFString.h"

#include "GSHashTable.h"
#include "GSObjCRuntime.h"



static CFTypeID _kCFBagTypeID = 0;

static void
CFBagFinalize (CFTypeRef cf)
{
  GSHashTableFinalize ((GSHashTableRef)cf);
}

static Boolean
CFBagEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return GSHashTableEqual ((GSHashTableRef)cf1, (GSHashTableRef)cf2);
}

static CFHashCode
CFBagHash (CFTypeRef cf)
{
  return GSHashTableHash ((GSHashTableRef)cf);
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



CFBagRef
CFBagCreate (CFAllocatorRef allocator, const void **values, CFIndex numValues,
  const CFBagCallBacks *callBacks)
{
  return (CFBagRef)GSHashTableCreate (allocator, _kCFBagTypeID,
    values, values, numValues,
    (const GSHashTableKeyCallBacks*)callBacks, NULL);
}

CFBagRef
CFBagCreateCopy (CFAllocatorRef allocator, CFBagRef bag)
{
  return (CFBagRef)GSHashTableCreateCopy (allocator, (GSHashTableRef)bag);
}

void
CFBagApplyFunction (CFBagRef bag,
  CFBagApplierFunction applier, void *context)
{
  
}

Boolean
CFBagContainsValue (CFBagRef bag, const void *value)
{
  return GSHashTableContainsKey ((GSHashTableRef)bag, value);
}

CFIndex
CFBagGetCount (CFBagRef bag)
{
  return GSHashTableGetCount ((GSHashTableRef)bag);
}

CFIndex
CFBagGetCountOfValue (CFBagRef bag, const void *value)
{
  return GSHashTableGetCountOfKey ((GSHashTableRef)bag, value);
}

void
CFBagGetValues (CFBagRef bag, const void **values)
{
  GSHashTableGetKeysAndValues ((GSHashTableRef)bag, values, NULL);
}

const void *
CFBagGetValue (CFBagRef bag, const void *value)
{
  return GSHashTableGetValue ((GSHashTableRef)bag, value);
}

Boolean
CFBagGetValueIfPresent (CFBagRef bag,
  const void *candidate, const void **value)
{
  const void *v;
  
  v = CFBagGetValue (bag, candidate);
  if (v)
    {
      if (value)
        *value = v;
      return true;
    }
  
  return false;
}

CFTypeID
CFBagGetTypeID (void)
{
  return _kCFBagTypeID;
}



CFMutableBagRef
CFBagCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
  const CFBagCallBacks *callBacks)
{
  return (CFMutableBagRef)GSHashTableCreateMutable (allocator, _kCFBagTypeID,
    capacity, (const GSHashTableKeyCallBacks*)callBacks, NULL);
}

CFMutableBagRef
CFBagCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFBagRef bag)
{
  return (CFMutableBagRef)GSHashTableCreateMutableCopy (allocator,
    (GSHashTableRef)bag, capacity);
}

void
CFBagAddValue (CFMutableBagRef bag, const void *value)
{
  GSHashTableAddValue ((GSHashTableRef)bag, value, value);
}

void
CFBagRemoveAllValues (CFMutableBagRef bag)
{
  GSHashTableRemoveAll ((GSHashTableRef)bag);
}

void
CFBagRemoveValue (CFMutableBagRef bag, const void *value)
{
  GSHashTableRemoveValue ((GSHashTableRef)bag, value);
}

void
CFBagReplaceValue (CFMutableBagRef bag, const void *value)
{
  GSHashTableReplaceValue ((GSHashTableRef)bag, value, value);
}

void
CFBagSetValue (CFMutableBagRef bag, const void *value)
{
  GSHashTableSetValue ((GSHashTableRef)bag, value, value);
}

