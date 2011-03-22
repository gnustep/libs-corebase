/* CFRuntime.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of CoreBase.
   
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

#import "NSCFType.h"



// CFRuntimeClass Table
CFRuntimeClass **__CFRuntimeClassTable = NULL;
Class *__CFRuntimeObjCClassTable = NULL;
static UInt32 __CFRuntimeClassTableCount = 0;
static UInt32 __CFRuntimeClassTableSize = 1024;  // Initial size

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



// These functions are declared in CFInternal.h, but since corebase
// doesn't have this file, they'll be done in here.
static inline void *
__CFISAForTypeID (CFTypeID typeID)
{
  if (typeID >= __CFRuntimeClassTableSize)
    {
      // Assume typeID is actually the address to a ObjC class.
      return (void *)typeID;
    }
  return (void *)__CFRuntimeObjCClassTable[typeID];
}

static inline Boolean
CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
  return (typeID >= __CFRuntimeClassTableSize
          || (obj != NULL
              && (((CFRuntimeBase*)obj)->_isa != __CFISAForTypeID (typeID))));
}

#define IS_OBJC(cf) CF_IS_OBJC(CFGetTypeID(cf), cf)

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
  // FIXME: This is absolutely NOT thread safe.
  CFTypeID ret;
  
  if(__CFRuntimeClassTableCount >= __CFRuntimeClassTableSize)
    {
      NSLog (@"CoreBase class table is full, cannot register class %s",
        cls->className);
      return _kCFRuntimeNotATypeID;
    }
  
  __CFRuntimeClassTable[__CFRuntimeClassTableCount] = (CFRuntimeClass *)cls;
  __CFRuntimeObjCClassTable[__CFRuntimeClassTableCount] = NSCFTypeClass;
  ret = __CFRuntimeClassTableCount++;
  return ret;
}

const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID)
{
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
  
  new = (CFRuntimeBase *)NSAllocateObject (NSCFTypeClass, extraBytes, allocator);
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
  ((CFRuntimeBase *)cf)->_isa = __CFISAForTypeID (typeID);
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
  CFTypeID typeID;
  if (NULL == cf)
    return NULL;
  
  typeID = CFGetTypeID(cf);
  if (typeID < __CFRuntimeClassTableSize)
    if (NULL == __CFRuntimeClassTable[typeID])
      return NULL;

  if (IS_OBJC(cf))
    {
      return (CFStringRef)CFRetain([(id)cf description]);
    }
  else
    {
      CFRuntimeClass *cfclass = __CFRuntimeClassTable[CFGetTypeID(cf)];
      if (NULL != cfclass->copyFormattingDesc)
        {
          return cfclass->copyFormattingDesc(cf, NULL);
        }
      else
        {
          CFStringCreateWithFormat (NULL, NULL, CFSTR("<%s: %p>"),
            cfclass->className, cf);
        }
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
  
  if (cf1 == cf2)
    return true;
  
  cls = __CFRuntimeClassTable[CFGetTypeID(cf1)];
  if (NULL != cls->equal)
    return cls->equal(cf1, cf2);
  
  return false;
}

CFAllocatorRef
CFGetAllocator (CFTypeRef cf)
{
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return (CFAllocatorRef)(((obj)cf)[-1]).zone;
  
  return kCFAllocatorSystemDefault;
}

CFIndex
CFGetRetainCount (CFTypeRef cf)
{
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    return (CFIndex)NSExtraRefCount (cf) + 1;
  
  return 1;
}

CFTypeID
CFGetTypeID (CFTypeRef cf)
{
  return (CFTypeID)((CFRuntimeBase*)cf)->_typeID;
}

CFHashCode
CFHash (CFTypeRef cf)
{
  CFRuntimeClass *cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
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
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    {
      CFIndex refCount = NSDecrementExtraRefCountWasZero(cf);
      if (refCount == 0)
        {
          CFRuntimeClass *cls = __CFRuntimeClassTable[CFGetTypeID(cf)];
          
          if (cls->finalize)
            cls->finalize (cf);
          NSDeallocateObject (cf);
        }
    }
}

CFTypeRef
CFRetain (CFTypeRef cf)
{
  if (!((CFRuntimeBase*)cf)->_flags.ro)
    NSIncrementExtraRefCount (cf);
  return cf;
}



extern void CFLocaleInitialize (void);
extern void CFNullInitialize (void);

void CFInitialize (void)
{
  // Initialize CFRuntimeClassTable
  __CFRuntimeClassTable = (CFRuntimeClass **) calloc (__CFRuntimeClassTableSize,
                            sizeof(CFRuntimeClass *));
  __CFRuntimeObjCClassTable = (Class *) calloc (__CFRuntimeClassTableSize,
                      	        sizeof(Class));

  _CFRuntimeRegisterClass (&CFNotATypeClass);
  CFNullInitialize ();
  CFLocaleInitialize ();
  
  NSCFTypeClass = [NSCFType class];
}

