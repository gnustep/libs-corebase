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
#import <Foundation/NSZone.h>

#include "CoreFoundation/CFRuntime.h"

// Taken from NSObject.m for compatibility with ObjC objects.
#define	ALIGN __alignof__(double)
/*
 *	Define a structure to hold information that is held locally
 *	(before the start) in each object.
 */
typedef struct obj_layout_unpadded {
    NSUInteger	retained;
    NSZone	*zone;
} unp;
#define	UNP sizeof(unp)
/*
 *	Now do the REAL version - using the other version to determine
 *	what padding (if any) is required to get the alignment of the
 *	structure correct.
 */
struct obj_layout {
    NSUInteger	retained;
    NSZone	*zone;
    char	padding[ALIGN - ((UNP % ALIGN) ? (UNP % ALIGN) : ALIGN)];
};
typedef	struct obj_layout *obj;



/* This is NSCFType, the ObjC class that all non-bridged CF types belong to.
 */
@interface NSCFType : NSObject
{
  CFTypeID _typeid;
}

- (CFTypeID) _cfTypeID;
@end



// CFRuntimeClass Table
#define CF_RUNTIME_CLASS_TABLE_DEFAULT_SIZE 1024
static CFRuntimeClass **__CFRuntimeClassTable = NULL;
static Class *__CFRuntimeObjCClassTable = NULL;
static UInt32 __CFRuntimeClassTableCount = 0;
static UInt32 __CFRuntimeClassTableSize = 0;



// These functions are declared in CFInternal.h, but since corebase
// doesn't have this file, they'll be done in here.
static inline void *__CFISAForTypeID (CFTypeID typeID)
{
  if (typeID >= __CFRuntimeClassTableSize)
    {
      // Assume typeID is actually the address to a ObjC class.
      return (void *)typeID;
    }
  return (void *)__CFRuntimeObjCClassTable[typeID];
}

static inline Boolean CF_IS_OBJC (CFTypeID typeID, const void *obj)
{
  return (typeID >= __CFRuntimeClassTableSize)
    || (((CFRuntimeBase *)obj)->_isa != __CFISAForTypeID (typeID));
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
  
  if(__CFRuntimeClassTableCount >= CF_RUNTIME_CLASS_TABLE_DEFAULT_SIZE)
    {
      NSLog (@"CoreBase class table is full, cannot register class %s",
        cls->className);
      return _kCFRuntimeNotATypeID;
    }
  
  __CFRuntimeClassTable[__CFRuntimeClassTableCount] = (CFRuntimeClass *)cls;
  __CFRuntimeObjCClassTable[__CFRuntimeClassTableCount] = [NSCFType class];
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
  CFIndex size;
  CFRuntimeBase *new;
  
  // Return NULL if typeID is unknown.
  if (_kCFRuntimeNotATypeID == typeID
      || NULL == (cls = __CFRuntimeClassTable[typeID]))
    {
      return NULL;
    }
  if (NULL == allocator)
    allocator = CFAllocatorGetDefault ();
  
  // extraBytes is the number of bytes needed in addition to CFRuntimeClass.
  size = sizeof(CFRuntimeClass) + extraBytes + sizeof(struct obj_layout);
  new = (CFTypeRef) CFAllocatorAllocate (allocator, size, 0);
  if (new == NULL)
    {
      // return NULL if allocator returns NULL.
      return NULL;
    }
  
  // this function does NOT init anything but CFRuntimeBase.
  ((obj)new)->zone = allocator;
  new = (CFRuntimeBase *)&((obj)new)[1];
  new->_isa = __CFISAForTypeID (typeID);
  new->_typeid = typeID;
  if (NULL != cls->init)
    {
      // Init instance...
      cls->init(new);
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
  if (_kCFRuntimeNotATypeID == typeID
      || NULL == (cls = __CFRuntimeClassTable[typeID])
      || NULL == memory)
    {
      return;
    }
  
  ((CFRuntimeBase *)memory)->_isa = __CFISAForTypeID (typeID);
  if (cls->init != NULL)
    {
      // Init instance...
      cls->init(memory);
    }
}



//
// CFType Functions
//
CFStringRef
CFCopyDescription (CFTypeRef cf)
{
  if (NULL == cf && NULL == __CFRuntimeClassTable[CFGetTypeID(cf)])
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
	  CFStringCreateWithFormat (NULL, NULL, CFSTR("<%s: %p"),
	    cfclass->className, cf);
	}
    }
  return NULL;
}

CFStringRef
CFCopyTypeIDDescription (CFTypeID typeID)
{
  // FIXME: is this the right way?
  if (NULL == __CFRuntimeClassTable[typeID])
    return NULL;

  if (typeID >= __CFRuntimeClassTableSize)
    {
      return (CFStringRef)CFRetain([(Class)typeID description]);
    }
  else
    {
      CFRuntimeClass *cfclass = __CFRuntimeClassTable[typeID];
      return CFStringCreateWithCStringNoCopy (NULL, cfclass->className,
	       CFStringGetSystemEncoding(), kCFAllocatorNull);
    }
}

Boolean
CFEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return [(id)cf1 isEqual: (id)cf2];
}

CFAllocatorRef
CFGetAllocator (CFTypeRef cf)
{
  return (CFAllocatorRef)[(id)cf zone];
}

CFIndex
CFGetRetainCount (CFTypeRef cf)
{
  return [(id)cf retainCount];
}

CFTypeID
CFGetTypeID (CFTypeRef cf)
{
  return [(id)cf _cfTypeID];
}

CFHashCode
CFHash (CFTypeRef cf)
{
  return [(id)cf hash];
}

CFTypeRef
CFMakeCollectable (CFTypeRef cf)
{
  return NULL;
}

void
CFRelease (CFTypeRef cf)
{
  RELEASE((id)cf);
}

CFTypeRef
CFRetain (CFTypeRef cf)
{
  return (CFTypeRef)RETAIN((id)cf);
}



static void __CFInitialize (void)
{
  // Initialize CFRuntimeClassTable
  __CFRuntimeClassTableSize = CF_RUNTIME_CLASS_TABLE_DEFAULT_SIZE;
  __CFRuntimeClassTable = (CFRuntimeClass **) calloc (__CFRuntimeClassTableSize,
                            sizeof(CFRuntimeClass *));
  __CFRuntimeObjCClassTable = (Class *) calloc (__CFRuntimeClassTableSize,
                      	        sizeof(Class));

  _CFRuntimeRegisterClass (&CFNotATypeClass);
}



@interface NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return _kCFRuntimeNotATypeID;
}
@end

@implementation NSCFType

+ (void) load
{
  __CFInitialize ();
}

- (void) dealloc
{
  CFRuntimeClass *cfclass = __CFRuntimeClassTable[([self _cfTypeID])];
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  if (NULL != cfclass->finalize)
    return cfclass->finalize((CFTypeRef)self);
#else
  if (NULL != cfclass->dealloc)
    return cfclass->dealloc((CFTypeRef)self);
#endif
  
  [super dealloc];
}
- (BOOL) isEqual: (id) anObject
{
  CFRuntimeClass *cfclass = __CFRuntimeClassTable[([self _cfTypeID])];
  if (NULL != cfclass->equal)
    return cfclass->equal((CFTypeRef)self, (CFTypeRef)anObject);
  else
    return [super isEqual: anObject];
}

- (CFTypeID) _cfTypeID
{
  /* This is an undocumented method.
     See: http://www.cocoadev.com/index.pl?HowToCreateTollFreeBridgedClass for
     more info.
  */
  return (CFTypeID)_typeid;
}

@end
