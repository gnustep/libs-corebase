/* CFSet.c
   
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
#include "CoreFoundation/CFSet.h"
#include "CoreFoundation/CFString.h"

#include "GSHashTable.h"
#include "GSPrivate.h"
#include "GSObjCRuntime.h"



static CFTypeID _kCFSetTypeID = 0;

static void
CFSetFinalize (CFTypeRef cf)
{
  GSHashTableFinalize ((GSHashTableRef) cf);
}

static Boolean
CFSetEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return GSHashTableEqual ((GSHashTableRef) cf1, (GSHashTableRef) cf2);
}

static CFHashCode
CFSetHash (CFTypeRef cf)
{
  return GSHashTableHash ((GSHashTableRef) cf);
}

static CFStringRef
CFSetCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFSTR ("");
}

static CFRuntimeClass CFSetClass = {
  0,
  "CFSet",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) CFSetCreateCopy,
  CFSetFinalize,
  CFSetEqual,
  CFSetHash,
  CFSetCopyFormattingDesc,
  NULL
};

void
CFSetInitialize (void)
{
  _kCFSetTypeID = _CFRuntimeRegisterClass (&CFSetClass);
}



const CFSetCallBacks kCFCopyStringSetCallBacks = {
  0,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef)) CFStringCreateCopy,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};

const CFSetCallBacks kCFTypeSetCallBacks = {
  0,
  CFTypeRetainCallBack,
  CFTypeReleaseCallBack,
  CFCopyDescription,
  CFEqual,
  CFHash
};



CFSetRef
CFSetCreate (CFAllocatorRef allocator, const void **values, CFIndex numValues,
             const CFSetCallBacks * callBacks)
{
  return (CFSetRef) GSHashTableCreate (allocator, _kCFSetTypeID,
                                       values, values, numValues,
                                       (const GSHashTableKeyCallBacks *)
                                       callBacks, NULL);
}

CFSetRef
CFSetCreateCopy (CFAllocatorRef allocator, CFSetRef set)
{
  if (CF_IS_OBJC (_kCFSetTypeID, set))
    {
      CFSetRef result;
      const CFIndex count = CFSetGetCount (set);
      void **values =
        (void **) CFAllocatorAllocate (allocator, sizeof (void *) * count, 0);

      CFSetGetValues (set, (const void **) values);
      result =
        CFSetCreate (allocator, (const void **) values, count,
                     &kCFTypeSetCallBacks);

      CFAllocatorDeallocate (allocator, (void *) values);
      return result;
    }

  return (CFSetRef) GSHashTableCreateCopy (allocator, (GSHashTableRef) set);
}

void
CFSetApplyFunction (CFSetRef set, CFSetApplierFunction applier, void *context)
{
  // TODO: could be made more efficient by providing a specialized
  // implementation for the CF_IS_OBJC case

  const CFIndex count = CFSetGetCount (set);
  void **values =
    (void **) CFAllocatorAllocate (NULL, sizeof (void *) * count, 0);
  CFIndex i;

  CFSetGetValues (set, (const void **) values);

  for (i = 0; i < count; i++)
    {
      applier (values[i], context);
    }

  CFAllocatorDeallocate (NULL, (void *) values);
}

Boolean
CFSetContainsValue (CFSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, Boolean, set, "containsObject:", value);

  return GSHashTableContainsKey ((GSHashTableRef) set, value);
}

CFIndex
CFSetGetCount (CFSetRef set)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, CFIndex, set, "count");

  return GSHashTableGetCount ((GSHashTableRef) set);
}

CFIndex
CFSetGetCountOfValue (CFSetRef set, const void *value)
{
  return CFSetContainsValue (set, value) ? 1 : 0;
}

void
CFSetGetValues (CFSetRef set, const void **values)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "_cfGetValues:", values);

  GSHashTableGetKeysAndValues ((GSHashTableRef) set, values, NULL);
}

const void *
CFSetGetValue (CFSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void *, set, "_cfGetValue:", value);

  return GSHashTableGetValue ((GSHashTableRef) set, value);
}

Boolean
CFSetGetValueIfPresent (CFSetRef set, const void *candidate, const void **value)
{
  const void *v;

  v = CFSetGetValue (set, candidate);
  if (v)
    {
      if (value)
        *value = v;
      return true;
    }

  return false;
}

CFTypeID
CFSetGetTypeID (void)
{
  return _kCFSetTypeID;
}



CFMutableSetRef
CFSetCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
                    const CFSetCallBacks * callBacks)
{
  return (CFMutableSetRef) GSHashTableCreateMutable (allocator, _kCFSetTypeID,
                                                     capacity,
                                                     (const
                                                      GSHashTableKeyCallBacks *)
                                                     callBacks, NULL);
}

CFMutableSetRef
CFSetCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                        CFSetRef set)
{
  if (CF_IS_OBJC (_kCFSetTypeID, set))
    {
      CFMutableSetRef result;
      const CFIndex count = CFSetGetCount (set);
      void **values =
        (void **) CFAllocatorAllocate (allocator, sizeof (void *) * count, 0);
      CFIndex i;

      CFSetGetValues (set, (const void **) values);
      result = CFSetCreateMutable (allocator, count, &kCFTypeSetCallBacks);

      for (i = 0; i < count; i++)
        GSHashTableAddValue ((GSHashTableRef) result, values[i], values[i]);

      CFAllocatorDeallocate (allocator, (void *) values);
      return result;
    }

  return (CFMutableSetRef) GSHashTableCreateMutableCopy (allocator,
                                                         (GSHashTableRef) set,
                                                         capacity);
}

void
CFSetAddValue (CFMutableSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "addObject:", value);

  GSHashTableAddValue ((GSHashTableRef) set, value, value);
}

void
CFSetRemoveAllValues (CFMutableSetRef set)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "removeAllObjects");

  GSHashTableRemoveAll ((GSHashTableRef) set);
}

void
CFSetRemoveValue (CFMutableSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "removeObject:", value);

  GSHashTableRemoveValue ((GSHashTableRef) set, value);
}

void
CFSetReplaceValue (CFMutableSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "_cfReplaceValue:", value);

  GSHashTableReplaceValue ((GSHashTableRef) set, value, value);
}

void
CFSetSetValue (CFMutableSetRef set, const void *value)
{
  CF_OBJC_FUNCDISPATCHV (_kCFSetTypeID, void, set, "_cfSetValue:", value);

  GSHashTableSetValue ((GSHashTableRef) set, value, value);
}
