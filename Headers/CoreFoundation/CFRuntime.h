/* CFRuntime.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of CoreBase.
   
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

#ifndef __COREFOUNDATION_CFRUNTIME_H__
#define __COREFOUNDATION_CFRUNTIME_H__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <stddef.h>

CF_EXTERN_C_BEGIN

/** \defgroup CFRuntimeUtils Runtime Utilities
    \brief 
    \par Examples:
      \ref ExampleEXUInt32
    \warning The CFRuntime functions are not thread-safe.
    \{
 */

/** \name Garbage Collection
    \brief Garbage Collection is not supported.  All macros and functions
      relating to GC return false or NULL, but are provided for compatibility.
    \{
 */
CF_EXPORT Boolean kCFUseCollectableAllocator;
CF_EXPORT Boolean (*__CFObjCIsCollectable)(void *);

#define CF_USING_COLLECTABLE_MEMORY (kCFUseCollectableAllocator)
#define CF_IS_COLLECTABLE_ALLOCATOR(allocator) 0
#define CF_IS_COLLECTABLE(obj) 0
/** \} */

enum
{
  _kCFRuntimeNotATypeID = 0
};

enum
{
  _kCFRuntimeScannedObject =     (1UL<<0),
  _kCFRuntimeResourcefulObject = (1UL<<2),
  _kCFRuntimeCustomRefCount =    (1UL<<3)
};

typedef struct __CFRuntimeClass CFRuntimeClass;
struct __CFRuntimeClass
{
  CFIndex version;
  const char *className;
  void (*init)(CFTypeRef cf);
  CFTypeRef (*copy)(CFAllocatorRef allocator, CFTypeRef cf);
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  void (*finalize)(CFTypeRef cf);
#else
  void (*dealloc)(CFTypeRef cf);
#endif
  Boolean (*equal)(CFTypeRef cf1, CFTypeRef cf2);
  CFHashCode (*hash)(CFTypeRef cf);
  CFStringRef (*copyFormattingDesc)(CFTypeRef cf, CFDictionaryRef formatOptions);
  CFStringRef (*copyDebugDesc)(CFTypeRef cf);
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
#define CF_RECLAIM_AVAILABLE 1
  void (*reclaim)(CFTypeRef cf); /* _kCFRuntimeResourcefulObject */
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
#define CF_REFCOUNT_AVAILABLE 1
  UInt32 (*refcount)(intptr_t op, CFTypeRef cf); /* _kCFRuntimeCustomRefCount */
#endif
};



/** Registers a new CF class with the runtime.  This function locks the
    class table and so is thread-safe.
    \param cls A constant CFRuntimeClass.
    \return The next available CFTypeID or _kCFRuntimeNotATypeID
    if none are available.
    \see _CFRuntimeUnregisterClassWithTypeID()
 */
CF_EXPORT CFTypeID
_CFRuntimeRegisterClass (const CFRuntimeClass * const cls);

/** Gets the class structure associated with the \a typeID.
    
    \param typeID A CFTypeID to look up.
    \return The CFRuntimeClass for the \b typeID
 */
CF_EXPORT const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID);

/** Unregisters a class.
    \param typeID The CFTypeID to unregister.
    \see _CFRuntimeRegisterClass()
    \warning This function is not thread-safe.
 */
CF_EXPORT void
_CFRuntimeUnregisterClassWithTypeID (CFTypeID typeID);



typedef struct __CFRuntimeBase CFRuntimeBase;
struct __CFRuntimeBase
{
  void *_isa;
    /**< The Objective-C class for this object.  Used for the Objective-C
	 bridge.  <em>For internal use only.</em> */
  SInt16 _typeID;
  struct
    {
      SInt16 ro:       1;
      SInt16 reserved: 7;
      SInt16 info:     8;
    } _flags;
};

#define INIT_CFRUNTIME_BASE(...) { 0, 0, { 1, 0, 0 } }

/** Creates a new CF type instance.
    \param allocator The CFAllocatorRef to use or NULL for the default
    allocator.
    \param typeID The CFTypeID of the class.
    \param extraBytes The amount of extra bytes over a CFRuntimeBase type
    needed by this instance.
    \param category Currently unused, use NULL.
    \return A newly allocated object.
    \see CFRetain()
    \see CFRelease()
 */
CF_EXPORT CFTypeRef
_CFRuntimeCreateInstance (CFAllocatorRef allocator, CFTypeID typeID,
                          CFIndex extraBytes, unsigned char *category);

/** Set the CFTypeID for an instance.
    \param cf The object instance to set the type ID.
    \param typeID The new CFTypeID.
 */
CF_EXPORT void
_CFRuntimeSetInstanceTypeID (CFTypeRef cf, CFTypeID typeID);

/** Initializes a static CF object instance.
    \param memory A pointer to a static CF object instance.
    \param typeID The CFTypeID of the instance.
 */
CF_EXPORT void
_CFRuntimeInitStaticInstance (void *memory, CFTypeID typeID);
#define CF_HAS_INIT_STATIC_INSTANCE 1

/** \} */

/** \example EXUInt32.h
    Example of how to create a new CF class.

    See \ref EXUInt32.c for the implementation details of this simple CF type.
 */

/** \example EXUInt32.c
    Example of how to create a new CF class.

    See \ref EXUInt32.h for the interface details of this simple CF type.
 */

CF_EXTERN_C_END

#endif /* __COREFOUNDATION_CFRUNTIME_H__ */

