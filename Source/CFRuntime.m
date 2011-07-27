/* CFRuntime.m
   
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

#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFString.h"
#include "objc_interface.h"

#include <pthread.h>



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
  CFIndex        retained;
  CFAllocatorRef zone;
};
#define	UNP sizeof(struct obj_layout_unpadded)

/*
 *	Now do the REAL version - using the other version to determine
 *	what padding (if any) is required to get the alignment of the
 *	structure correct.
 */
struct obj_layout {
  CFIndex        retained;
  CFAllocatorRef zone;
  char	padding[ALIGN - ((UNP % ALIGN) ? (UNP % ALIGN) : ALIGN)];
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
      NSLog (@"CoreBase class table is full, cannot register class %s",
        cls->className);
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
  CFRuntimeClass *cls;
  CFRuntimeBase *new;
  
  // Return NULL if typeID is unknown.
  if (_kCFRuntimeNotATypeID == typeID
      || NULL == (cls = __CFRuntimeClassTable[typeID]))
    {
      return NULL;
    }
  if (NULL == allocator)
    allocator = CFAllocatorGetDefault ();
  
  new = (CFRuntimeBase *)NSAllocateObject (NSCFTypeClass, extraBytes,
    allocator);
  new->_isa = __CFRuntimeObjCClassTable[typeID];
  if (new)
    {
      new->_typeID = typeID;
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
      || NULL == (cls = __CFRuntimeClassTable[typeID])
      || NULL == memory)
    {
      return;
    }
  
  obj->_isa = __CFISAForTypeID (typeID);
  if (obj)
    {
      obj->_typeID = typeID;
      if (cls->init != NULL)
        {
          // Init instance...
          cls->init(memory);
        }
    }
}



//
// CFType Functions
//
CFStringRef
CFCopyDescription (CFTypeRef cf)
{
  CFRuntimeClass *cfclass;
  CFTypeID typeID = CFGetTypeID(cf);
  
  if (NULL == cf)
    return NULL;
  
  CF_OBJC_FUNCDISPATCH0(typeID, CFStringRef, cf, "description");
  
  if (typeID < __CFRuntimeClassTableSize)
    if (NULL == __CFRuntimeClassTable[typeID])
      return NULL;
  
  cfclass = __CFRuntimeClassTable[CFGetTypeID(cf)];
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
  if (typeID >= __CFRuntimeClassTableSize)
    {
      return (CFStringRef)CFRetain([(Class)typeID description]);
    }
  else
    {
      if (NULL == __CFRuntimeClassTable[typeID])
        return NULL;
      CFRuntimeClass *cfclass = __CFRuntimeClassTable[typeID];
      return CFStringCreateWithCStringNoCopy (NULL, cfclass->className,
	       CFStringGetSystemEncoding(), kCFAllocatorNull);
    }
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
  
  cls = __CFRuntimeClassTable[CFGetTypeID(cf1)];
  if (NULL != cls->equal)
    return cls->equal(cf1, cf2);
  
  return false;
}

CFAllocatorRef
CFGetAllocator (CFTypeRef cf)
{
  if (cf == NULL)
    return NULL;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFAllocatorRef, cf, "zone");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return (CFAllocatorRef)(((obj)cf)[-1]).zone;
  
  return kCFAllocatorSystemDefault;
}

CFIndex
CFGetRetainCount (CFTypeRef cf)
{
  if (cf == NULL)
    return 0;
  
  CF_OBJC_FUNCDISPATCH0(CFGetTypeID(cf), CFIndex, cf, "retainCount");
  
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return (CFIndex)NSExtraRefCount ((id)cf) + 1;
  
  return 1;
}

CFTypeID
CFGetTypeID (CFTypeRef cf)
{
  /* This is unsafe, but I don't see any other way of getting the typeID
     for this call. */
  CF_OBJC_FUNCDISPATCH0(0, CFTypeID, cf,
    "_cfTypeID");
  
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
      if (NSDecrementExtraRefCountWasZero((id)cf))
        {
          CFRuntimeClass *cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
          
          if (cls->finalize)
            cls->finalize (cf);
          NSDeallocateObject ((id)cf);
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
    NSIncrementExtraRefCount ((id)cf);
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

