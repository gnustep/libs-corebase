/* CFRuntime.h
   
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

#ifndef __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE
#define __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

CF_EXTERN_C_BEGIN

enum
{
  _kCFRuntimeNotATypeID = 0,
  _kCFRuntimeScannedObject = (1UL << 0),
  _kCFRuntimeResourcefulObject = (1UL << 2)
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
#if 0 // FIXME: OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
#define CF_RECLAIM_AVAILABLE 1
  void (*reclaim)(CFTypeRef cf);
#endif
};



CFTypeID
_CFRuntimeRegisterClass (const CFRuntimeClass * const cls);

const CFRuntimeClass *
_CFRuntimeGetClassWithTypeID (CFTypeID typeID);

void
_CFRuntimeUnregisterClassWithTypeID (CFTypeID typeID);



typedef struct __CFRuntimeBase CFRuntimeBase;
struct __CFRuntimeBase
{
  void *_isa;
  int16_t _typeID;
  struct
    {
      int16_t ro:       1; // 0 = read-only object
      int16_t unused:   7;
      int16_t reserved: 8;
    } _flags;
};

#define INIT_CFRUNTIME_BASE(...) { 0, 0, { 1, 0, 0 } }

CFTypeRef
_CFRuntimeCreateInstance (CFAllocatorRef allocator, CFTypeID typeID,
                          CFIndex extraBytes, unsigned char *category);

void
_CFRuntimeSetInstanceTypeID (CFTypeRef cf, CFTypeID typeID);

void
_CFRuntimeInitStaticInstance (void *memory, CFTypeID typeID);
#define CF_HAS_INIT_STATIC_INSTANCE 0

CF_EXTERN_C_END

#endif /* __CFRuntime_h_GNUSTEP_COREBASE_INCLUDE */
