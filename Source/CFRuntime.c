/* CFRuntime.c
   
   Copyright (C) 2010-2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
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
#include "CoreFoundation/CFString.h"
#include "GSPrivate.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>



/* GC stuff... */
Boolean kCFUseCollectableAllocator = false;
Boolean (*__CFObjCIsCollectable)(void *) = NULL;

/* CFRuntimeClass Table */
CFRuntimeClass **__CFRuntimeClassTable = NULL;
void **__CFRuntimeObjCClassTable = NULL;
UInt32 __CFRuntimeClassTableCount = 0;
UInt32 __CFRuntimeClassTableSize = 1024;  /* Initial size */

static GSMutex _kCFRuntimeTableLock;

void *NSCFTypeClass = NULL;



/******************************/
/* Object header... lifted from base's NSObject.m */
#ifdef ALIGN
#undef ALIGN
#endif
#define	ALIGN __alignof__(double)

/*
 *	Define a structure to hold information that is held locally
 *	(before the start) in each object.
 */
struct obj_layout_unpadded {
  CFAllocatorRef allocator;
  CFIndex        retained;
};
#define	UNP sizeof(struct obj_layout_unpadded)

/*
 *	Now do the REAL version - using the other version to determine
 *	what padding (if any) is required to get the alignment of the
 *	structure correct.
 */
struct obj_layout {
#if !defined(_MSC_VER)
  char	padding[ALIGN - ((UNP % ALIGN) ? (UNP % ALIGN) : ALIGN)];
#endif
  CFAllocatorRef allocator;
  CFIndex        retained;
};
typedef	struct obj_layout *obj;
/******************************/

/* CFNotATypeClass declaration for index 0 of the class table. */
static CFRuntimeClass CFNotATypeClass = 
{
  0, /* Version */
  "CFNotATypeClass", /* Class name */
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

CFTypeID
_CFRuntimeRegisterClass (const CFRuntimeClass * const cls)
{
  CFTypeID ret;
  
  GSMutexLock (&_kCFRuntimeTableLock);
  if(__CFRuntimeClassTableCount >= __CFRuntimeClassTableSize)
    {
      GSMutexUnlock (&_kCFRuntimeTableLock);
      return _kCFRuntimeNotATypeID;
    }
  
  __CFRuntimeClassTable[__CFRuntimeClassTableCount] = (CFRuntimeClass *)cls;
  if (__CFRuntimeObjCClassTable)
    __CFRuntimeObjCClassTable[__CFRuntimeClassTableCount] = NSCFTypeClass;
  ret = __CFRuntimeClassTableCount++;
  GSMutexUnlock (&_kCFRuntimeTableLock);
  
  return ret;
}

const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID)
{
  GSMutexLock (&_kCFRuntimeTableLock);
  if (typeID > __CFRuntimeClassTableCount)
    typeID = 0;
  GSMutexUnlock (&_kCFRuntimeTableLock);
  return __CFRuntimeClassTable[typeID];
}

void
_CFRuntimeUnregisterClassWithTypeID (CFTypeID typeID)
{
  GSMutexLock (&_kCFRuntimeTableLock);
  __CFRuntimeClassTable[typeID] = NULL;
  GSMutexUnlock (&_kCFRuntimeTableLock);
}



CFTypeRef
_CFRuntimeCreateInstance (CFAllocatorRef allocator, CFTypeID typeID,
                          CFIndex extraBytes, unsigned char *category)
{ /* category is not used and should be NULL. */
  CFIndex instSize;
  CFRuntimeClass *cls;
  CFRuntimeBase *new;
  
  /* Return NULL if typeID is unknown. */
  if (_kCFRuntimeNotATypeID == typeID
      || typeID > __CFRuntimeClassTableCount)
    {
      return NULL;
    }
  if (NULL == allocator)
    allocator = CFAllocatorGetDefault ();
  
  instSize = sizeof(struct obj_layout) + sizeof(CFRuntimeBase) + extraBytes;
  new = (CFRuntimeBase*)CFAllocatorAllocate (allocator, instSize, 0);
  if (new)
    {
      new = memset (new, 0, instSize);
      ((obj)new)->allocator = allocator;
      new = (CFRuntimeBase*)&((obj)new)[1];
      new->_isa =
        __CFRuntimeObjCClassTable ? __CFRuntimeObjCClassTable[typeID] : NULL;
      new->_typeID = typeID;
      
      cls = __CFRuntimeClassTable[typeID];
      if (NULL != cls->init)
        {
          /* Init instance... */
          cls->init(new);
        }
    }
  
  return new;
}

void
_CFRuntimeSetInstanceTypeID (CFTypeRef cf, CFTypeID typeID)
{
  ((CFRuntimeBase *)cf)->_typeID = typeID;
}

void
_CFRuntimeInitStaticInstance (void *memory, CFTypeID typeID)
{
  CFRuntimeClass *cls;
  CFRuntimeBase  *obj = (CFRuntimeBase *)memory;
  
  if (_kCFRuntimeNotATypeID == typeID
      || typeID >= __CFRuntimeClassTableCount
      || NULL == memory)
    {
      return;
    }
  
  cls = __CFRuntimeClassTable[typeID];
  obj->_isa = __CFISAForTypeID (typeID);
  obj->_flags.ro = 1;
  obj->_flags.reserved = 0;
  obj->_flags.info = 0;
  obj->_typeID = typeID;
  if (cls->init != NULL)
    {
      /* Init instance... */
      cls->init(memory);
    }
}



CFStringRef
CFCopyDescription (CFTypeRef cf)
{
  CFRuntimeClass *cfclass;
  CFTypeID typeID = CFGetTypeID(cf);
  
  if (NULL == cf)
    return NULL;
  
  CF_OBJC_FUNCDISPATCH0(typeID, CFStringRef, cf, "description");
  
  if (_kCFRuntimeNotATypeID == typeID)
    return NULL;
  
  cfclass = __CFRuntimeClassTable[typeID];
  if (NULL != cfclass->copyFormattingDesc)
    {
      return cfclass->copyFormattingDesc(cf, NULL);
    }
  else
    {
      return CFStringCreateWithFormat (NULL, NULL, CFSTR("<%s: %p>"),
        cfclass->className, cf);
    }
  
  return NULL;
}

CFStringRef
CFCopyTypeIDDescription (CFTypeID typeID)
{
  CFRuntimeClass *cfclass;
  
  if (_kCFRuntimeNotATypeID == typeID
      || typeID >= __CFRuntimeClassTableCount)
    return NULL;
  
  cfclass = __CFRuntimeClassTable[typeID];
  return __CFStringMakeConstantString(cfclass->className);
}

Boolean
CFEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFRuntimeClass *cls;
  CFTypeID tID1, tID2;
  
  if (cf1 == cf2)
    return true;
  
  if (cf1 == NULL || cf2 == NULL)
    return false;
  
  /* Can't compare here if either objects are ObjC objects. */
  CF_OBJC_FUNCDISPATCH1(CFGetTypeID(cf1), Boolean, cf1, "isEqual:", cf2);
  CF_OBJC_FUNCDISPATCH1(CFGetTypeID(cf2), Boolean, cf2, "isEqual:", cf1);
  
  tID1 = CFGetTypeID(cf1);
  tID2 = CFGetTypeID(cf2);
  if (tID1 != tID2)
    return false;
  
  cls = __CFRuntimeClassTable[tID1];
  if (NULL != cls->equal)
    return cls->equal(cf1, cf2);
  
  return false;
}

CFAllocatorRef
CFGetAllocator (CFTypeRef cf)
{
  if (CF_IS_OBJC(CFGetTypeID(cf), cf) || ((CFRuntimeBase*)cf)->_flags.ro)
    return kCFAllocatorSystemDefault;
  
  return ((obj)cf)[-1].allocator;
}

CFIndex
CFGetRetainCount (CFTypeRef cf)
{
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFIndex, cf, "retainCount");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return ((obj)cf)[-1].retained + 1;
  
  return 1;
}

CFTypeID
CFGetTypeID (CFTypeRef cf)
{
  /* This is unsafe, but I don't see any other way of getting the typeID
     for this call. */
  CF_OBJC_FUNCDISPATCH0(((CFRuntimeBase*)cf)->_typeID, CFTypeID, cf, "_cfTypeID");
  
  return ((CFRuntimeBase*)cf)->_typeID;
}

CFHashCode
CFHash (CFTypeRef cf)
{
  CFRuntimeClass *cls;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFHashCode, cf, "hash");
  
  cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
  if (cls->hash)
    return cls->hash (cf);
  
  return (CFHashCode)((uintptr_t)cf >> 3);
}

CFTypeRef
CFMakeCollectable (CFTypeRef cf)
{
/* FIXME */
  return cf;
}

void
CFRelease (CFTypeRef cf)
{
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), void, cf, "release");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    {
      CFIndex result = GSAtomicDecrementCFIndex (&(((obj)cf)[-1].retained));
      if (result < 0)
        {
          CFRuntimeClass *cls;
          assert (result == -1);
          
          cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
          
          if (cls->finalize)
            cls->finalize (cf);
          CFAllocatorDeallocate (CFGetAllocator(cf), (void*)&((obj)cf)[-1]);
        }
    }
}

CFTypeRef
CFRetain (CFTypeRef cf)
{
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFTypeRef, cf, "retain");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    {
      CFIndex result = GSAtomicIncrementCFIndex (&(((obj)cf)[-1].retained));
      assert (result < INT_MAX);
    }
  
  return cf;
}

const void *
CFTypeRetainCallBack (CFAllocatorRef allocator, const void *value)
{
  return CFRetain (value);
}

void
CFTypeReleaseCallBack (CFAllocatorRef alloc, const void *value)
{
  CFRelease (value);
}

CFTypeRef
GSTypeCreateCopy (CFAllocatorRef alloc, CFTypeRef cf, CFTypeID typeID)
{
  CFRuntimeClass *cls;
  
  CF_OBJC_FUNCDISPATCH0(typeID, CFTypeRef, cf, "copy");
  
  cls = __CFRuntimeClassTable[typeID];
  if (cls->copy)
    return cls->copy (alloc, cf);
  
  return CFRetain (cf);
}



static CFTypeRef GSRuntimeConstantTable[512];
static CFIndex GSRuntimeConstantTableSize = 0;

void
GSRuntimeConstantInit (CFTypeRef cf, CFTypeID typeID)
{
  ((CFRuntimeBase*)cf)->_typeID = typeID;
  GSRuntimeConstantTable[GSRuntimeConstantTableSize++] = cf;
}

void
GSRuntimeInitializeConstants (void)
{
  CFIndex i;
  CFTypeID tid;
  
  for (i = 0 ; i < GSRuntimeConstantTableSize ; ++i)
    {
      tid = ((CFRuntimeBase*)GSRuntimeConstantTable[i])->_typeID;
      ((CFRuntimeBase*)GSRuntimeConstantTable[i])->_isa =
        __CFRuntimeObjCClassTable[tid];
    }
}

extern void CFAllocatorInitialize (void);
extern void CFArrayInitialize (void);
extern void CFAttributedStringInitialize (void);
extern void CFBagInitialize (void);
extern void CFBinaryHeapInitialize (void);
extern void CFBitVectorInitialize (void);
extern void CFBooleanInitialize (void);
extern void CFCalendarInitialize (void);
extern void CFCharacterSetInitialize (void);
extern void CFDataInitialize (void);
extern void CFDateInitialize (void);
extern void CFDateFormatterInitialize (void);
extern void CFDictionaryInitialize (void);
extern void CFErrorInitialize (void);
extern void CFLocaleInitialize (void);
extern void CFNullInitialize (void);
extern void CFNumberInitialize (void);
extern void CFNumberFormatterInitialize (void);
extern void CFSetInitialize (void);
extern void CFStringInitialize (void);
extern void CFStringEncodingInitialize (void);
extern void CFTimeZoneInitialize (void);
extern void CFTreeInitialize (void);
extern void CFURLInitialize (void);
extern void CFUUIDInitialize (void);
extern void CFXMLNodeInitialize (void);

#if !defined(_MSC_VER)
void CFInitialize (void) __attribute__((constructor));
#endif

static CFIndex CFInitialized = 0;
void CFInitialize (void)
{
  /* Only initialize once. */
  if (GSAtomicCompareAndSwapCFIndex(&CFInitialized, 0, 1) == 1)
    return;
  
  /* Initialize CFRuntimeClassTable */
  __CFRuntimeClassTable = (CFRuntimeClass **) calloc (__CFRuntimeClassTableSize,
                            sizeof(CFRuntimeClass *));
  
  GSMutexInitialize (&_kCFRuntimeTableLock);
  
  /* CFNotATypeClass should be at index = 0 */
  _CFRuntimeRegisterClass (&CFNotATypeClass);
  
  CFAllocatorInitialize ();
  CFArrayInitialize ();
  CFAttributedStringInitialize ();
  CFBagInitialize ();
  CFBinaryHeapInitialize ();
  CFBitVectorInitialize ();
  CFBooleanInitialize ();
  CFCalendarInitialize ();
  CFCharacterSetInitialize ();
  CFDataInitialize ();
  CFDateInitialize ();
  CFDateFormatterInitialize ();
  CFDictionaryInitialize ();
  CFErrorInitialize ();
  CFLocaleInitialize ();
  CFNullInitialize ();
  CFNumberInitialize ();
  CFNumberFormatterInitialize ();
  CFSetInitialize ();
  CFStringInitialize ();
  CFStringEncodingInitialize ();
  CFTimeZoneInitialize ();
  CFTreeInitialize ();
  CFURLInitialize ();
  CFUUIDInitialize ();
  CFXMLNodeInitialize ();
}

#if defined(_MSC_VER)
#include <windows.h>

BOOL DllMain (HINSTANCE hinst, DWORD fdwReason, LPVOID ldvReserved)
{
  if (fdwReason == DLL_PROCESS_ATTACH)
    {
      CFInitialize ();
    }
  
  return TRUE;
}
#endif

