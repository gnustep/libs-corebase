/* CFRuntime.c
   
   Copyright (C) 2010-2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
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

#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFString.h"
#include "atomic_ops.h"
#include "objc_interface.h"



// CFRuntimeClass Table
CFRuntimeClass **__CFRuntimeClassTable = NULL;
Class *__CFRuntimeObjCClassTable = NULL;
UInt32 __CFRuntimeClassTableCount = 0;
UInt32 __CFRuntimeClassTableSize = 1024;  // Initial size

static pthread_mutex_t _kCFRuntimeTableLock = PTHREAD_MUTEX_INITIALIZER;

static Class NSCFTypeClass = Nil;



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
  char	padding[ALIGN - ((UNP % ALIGN) ? (UNP % ALIGN) : ALIGN)];
  CFAllocatorRef allocator;
  CFIndex        retained;
};
typedef	struct obj_layout *obj;
/******************************/

// CFNotATypeClass declaration for index 0 of the class table.
static CFRuntimeClass CFNotATypeClass = 
{
  0, // Version
  "CFNotATypeClass", // Class name
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
  
  pthread_mutex_lock (&_kCFRuntimeTableLock);
  if(__CFRuntimeClassTableCount >= __CFRuntimeClassTableSize)
    {
      /* FIXME NSLog (@"CoreBase class table is full, cannot register class %s",
        cls->className); */
      return _kCFRuntimeNotATypeID;
    }
  
  __CFRuntimeClassTable[__CFRuntimeClassTableCount] = (CFRuntimeClass *)cls;
  __CFRuntimeObjCClassTable[__CFRuntimeClassTableCount] = NSCFTypeClass;
  ret = __CFRuntimeClassTableCount++;
  pthread_mutex_unlock (&_kCFRuntimeTableLock);
  
  return ret;
}

const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID)
{
  if (typeID > __CFRuntimeClassTableCount)
    typeID = 0;
  return __CFRuntimeClassTable[typeID];
}

void
_CFRuntimeUnregisterClassWithTypeID (CFTypeID typeID)
{
  __CFRuntimeClassTable[typeID] = NULL;
}



CFTypeRef
_CFRuntimeCreateInstance (CFAllocatorRef allocator, CFTypeID typeID,
                          CFIndex extraBytes, unsigned char *category)
{ // category is not used and should be NULL.
  CFIndex instSize;
  CFRuntimeClass *cls;
  CFRuntimeBase *new;
  
  // Return NULL if typeID is unknown.
  if (_kCFRuntimeNotATypeID == typeID
      || typeID >= __CFRuntimeClassTableCount)
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
      new->_isa = __CFRuntimeObjCClassTable[typeID];
      new->_typeID = typeID;
      
      cls = __CFRuntimeClassTable[typeID];
      if (NULL != cls->init)
        {
          // Init instance...
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
  obj->_typeID = typeID;
  if (cls->init != NULL)
    {
      // Init instance...
      cls->init(memory);
    }
}



//
// CFType Functions
//
extern CFStringRef
__CFStringMakeConstantString (const char *str) __attribute__ ((pure));

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
  return CFRetain(__CFStringMakeConstantString(cfclass->className));
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
  
  // Can't compare here if either objects are ObjC objects.
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
  if (cf == NULL)
    return NULL;
  
  /* FIXME: This will crash for any ObjC objects.  Need to check for
      this case before returning the allocator. */
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return ((obj)cf)[-1].allocator;
  
  return kCFAllocatorSystemDefault;
}

CFIndex
CFGetRetainCount (CFTypeRef cf)
{
  if (cf == NULL)
    return 0;
  
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
  CF_OBJC_FUNCDISPATCH0(0, CFTypeID, cf, "_cfTypeID");
  
  return ((CFRuntimeBase*)cf)->_typeID;
}

CFHashCode
CFHash (CFTypeRef cf)
{
  CFRuntimeClass *cls;
  
  if (cf == NULL)
    return 0;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFHashCode, cf, "hash");
  
  cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
  if (cls->hash)
    return cls->hash (cf);
  
  return (CFHashCode)((uintptr_t)cf >> 3);
}

CFTypeRef
CFMakeCollectable (CFTypeRef cf)
{
// FIXME
  return NULL;
}

void
CFRelease (CFTypeRef cf)
{
  if (cf == NULL)
    return;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), void, cf, "release");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    {
      CFIndex result = CF_ATOMIC_DEC (&(((obj)cf)[-1].retained));
      if (result < 0)
        {
          assert (result == -1);
          
          CFRuntimeClass *cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
          
          if (cls->finalize)
            cls->finalize (cf);
          CFAllocatorDeallocate (CFGetAllocator(cf), (void*)&((obj)cf)[-1]);
        }
    }
}

CFTypeRef
CFRetain (CFTypeRef cf)
{
  if (cf == NULL)
    return NULL;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFTypeRef, cf, "retain");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    {
      CFIndex result = CF_ATOMIC_INC (&(((obj)cf)[-1].retained));
      assert (result < INT_MAX);
    }
  
  return cf;
}



extern void CFBooleanInitialize (void);
extern void CFCalendarInitialize (void);
extern void CFLocaleInitialize (void);
extern void CFBundleInitialize (void);
extern void CFNullInitialize (void);
extern void CFNumberFormatterInitialize (void);
extern void CFStringInitialize (void);
extern void CFTimeZoneInitialize (void);
extern void CFUUIDInitialize (void);

void CFInitialize (void)
{
  // Initialize CFRuntimeClassTable
  __CFRuntimeClassTable = (CFRuntimeClass **) calloc (__CFRuntimeClassTableSize,
                            sizeof(CFRuntimeClass *));
  __CFRuntimeObjCClassTable = (Class *) calloc (__CFRuntimeClassTableSize,
                      	        sizeof(Class));
  
  NSCFTypeClass = objc_getClass ("NSCFType");
  
  // CFNotATypeClass should be at index = 0
  _CFRuntimeRegisterClass (&CFNotATypeClass);
  
  CFBooleanInitialize ();
  CFCalendarInitialize ();
  CFLocaleInitialize ();
  CFBundleInitialize();
  CFNullInitialize ();
  CFNumberFormatterInitialize ();
  CFStringInitialize ();
  CFTimeZoneInitialize ();
  CFUUIDInitialize ();
}

