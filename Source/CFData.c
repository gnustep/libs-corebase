/* CFData.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
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

#include <assert.h>
#include <string.h>

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"

struct __CFData
{
  CFRuntimeBase  _parent;
  const UInt8   *_contents;
  CFIndex        _length;
  CFHashCode     _hash;
  CFAllocatorRef _deallocator;
};

struct __CFMutableData
{
  CFRuntimeBase  _parent;
  UInt8         *_contents;
  CFIndex        _length;
  CFHashCode     _hash;
  CFAllocatorRef _allocator;
  CFIndex        _capacity;
};

static CFTypeID _kCFDataTypeID = 0;

/* These are some masks to access the data in CFRuntimeBase's _flags.info
   field. */
enum
{
  _kCFDataIsMutable = (1<<0),
  _kCFDataFreeBytes = (1<<1)
};

CF_INLINE Boolean
CFDataIsMutable (CFDataRef d)
{
  return ((CFRuntimeBase *)d)->_flags.info & _kCFDataIsMutable ? true : false;
}

CF_INLINE Boolean
CFDataFreeBytes (CFDataRef d)
{
  return ((CFRuntimeBase *)d)->_flags.info & _kCFDataFreeBytes ? true : false;
}

CF_INLINE void
CFDataSetMutable (CFDataRef d)
{
  ((CFRuntimeBase *)d)->_flags.info |= _kCFDataIsMutable;
}

CF_INLINE void
CFDataSetFreeBytes (CFDataRef d)
{
  ((CFRuntimeBase *)d)->_flags.info |= _kCFDataFreeBytes;
}

static void
CFDataFinalize (CFTypeRef cf)
{
  CFDataRef d = (CFDataRef)cf;
  
  if (CFDataFreeBytes(d))
    {
      CFAllocatorDeallocate (d->_deallocator, (void*)d->_contents);
      CFRelease (d->_deallocator);
    }
}

static Boolean
CFDataEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFDataRef d1 = cf1;
  CFDataRef d2 = cf2;
  CFIndex length = CFDataGetLength (d1);
  
  if (length != CFDataGetLength(d2))
    return false;
  
  return (memcmp (d1->_contents, d2->_contents, length) == 0);
}

static CFHashCode
CFDataHash (CFTypeRef cf)
{
  struct __CFData *d = (struct __CFData*)cf;
  if (d->_hash == 0)
    d->_hash = GSHashBytes (d->_contents, d->_length);
  
  return d->_hash;
}

static const CFRuntimeClass CFDataClass =
{
  0,
  "CFData",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFDataCreateCopy,
  CFDataFinalize,
  CFDataEqual,
  CFDataHash,
  NULL,
  NULL
};

void CFDataInitialize (void)
{
  _kCFDataTypeID = _CFRuntimeRegisterClass(&CFDataClass);
}



#define CFDATA_SIZE sizeof(struct __CFData) - sizeof(CFRuntimeBase)

CFTypeID
CFDataGetTypeID (void)
{
  return _kCFDataTypeID;
}

static CFDataRef
CFDataCreate_internal (CFAllocatorRef allocator, const UInt8 *bytes,
  CFIndex length, CFAllocatorRef bytesDealloc, Boolean copy)
{
  struct __CFData *newData;
  CFIndex size;
  
  size = CFDATA_SIZE + (copy == true ? length : 0);
  
  if (allocator == NULL)
    allocator = CFAllocatorGetDefault ();
  
  newData = (struct __CFData*)_CFRuntimeCreateInstance (allocator,
    _kCFDataTypeID, size, NULL);
  if (newData)
    {
      newData->_length = length;
      
      if (copy)
        {
          memcpy (&(newData[1]), bytes, length);
          bytes = (const UInt8*)&(newData[1]);
        }
      else
        {
          if (bytesDealloc == NULL)
            bytesDealloc = CFAllocatorGetDefault ();
          newData->_deallocator = CFRetain(bytesDealloc);
          CFDataSetFreeBytes (newData);
        }
      newData->_contents = bytes;
    }
  
  return (CFDataRef)newData;
}

CFDataRef
CFDataCreate (CFAllocatorRef allocator, const UInt8 *bytes, CFIndex length)
{
  return CFDataCreate_internal (allocator, bytes, length, NULL, true);
}

CFDataRef
CFDataCreateWithBytesNoCopy (CFAllocatorRef allocator, const UInt8 *bytes,
  CFIndex length, CFAllocatorRef bytesDealloc)
{
  return CFDataCreate_internal (allocator, bytes, length, bytesDealloc, false);
}

CFDataRef
CFDataCreateCopy (CFAllocatorRef allocator, CFDataRef d)
{
  return CFDataCreate_internal (allocator, CFDataGetBytePtr(d),
    CFDataGetLength(d), NULL, true);
}

const UInt8 *
CFDataGetBytePtr (CFDataRef d)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, const UInt8 *, d, "bytes");
  
  return d->_contents;
}

void
CFDataGetBytes (CFDataRef d, CFRange range, UInt8 *buffer)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, void, d, "getBytes:range:", buffer,
    range);
  
  assert (range.location + range.length <= d->_length);
  memcpy (buffer, d->_contents + range.location, range.length);
}

CFIndex
CFDataGetLength (CFDataRef d)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, CFIndex, d, "length");
  
  return d->_length;
}



#define DEFAULT_CAPACITY 16
#define CFMUTABLEDATA_SIZE \
  sizeof(struct __CFMutableData) - sizeof(CFRuntimeBase)

static void
CFDataCheckCapacityAndGrow (CFMutableDataRef data, CFIndex capacity)
{
  struct __CFMutableData *d = (struct __CFMutableData*)data;
  
  if (capacity > d->_capacity)
    {
      d->_contents = CFAllocatorReallocate (d->_allocator, d->_contents,
        capacity, 0);
      d->_capacity = capacity;
    }
}

CFMutableDataRef
CFDataCreateMutable (CFAllocatorRef allocator, CFIndex capacity)
{
  struct __CFMutableData *newData;
  
  if (allocator == NULL)
    allocator = CFAllocatorGetDefault ();
  
  newData = (struct __CFMutableData*)_CFRuntimeCreateInstance (allocator,
    _kCFDataTypeID, CFMUTABLEDATA_SIZE, NULL);
  
  if (newData)
    {
      if (capacity < DEFAULT_CAPACITY)
        capacity = DEFAULT_CAPACITY;
      newData->_capacity = capacity;
      newData->_allocator = CFRetain(allocator);
      newData->_contents = CFAllocatorAllocate (allocator, capacity, 0);
      
      CFDataSetMutable ((CFDataRef)newData);
      CFDataSetFreeBytes((CFDataRef)newData);
    }
  
  return (CFMutableDataRef)newData;
}

CFMutableDataRef
CFDataCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFDataRef d)
{
  CFMutableDataRef newData;
  CFIndex length;
  
  length = CFDataGetLength (d);
  newData =
    CFDataCreateMutable (allocator, capacity > length ? capacity : length);
  
  memcpy ((UInt8*)newData->_contents, CFDataGetBytePtr(d), length);
  newData->_length = length;
  
  return newData;
}

void
CFDataAppendBytes (CFMutableDataRef d, const UInt8 *bytes, CFIndex length)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, void, d, "appendBytes:length:", bytes,
    length);
  
  CFDataReplaceBytes (d, CFRangeMake(d->_length, 0), bytes, length);
}

void
CFDataDeleteBytes (CFMutableDataRef d, CFRange range)
{
  CFDataReplaceBytes(d, range, NULL, 0);
}

UInt8 *
CFDataGetMutableBytePtr (CFMutableDataRef d)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, UInt8 *, d, "mutableBytes");
  
  if (!CFDataIsMutable(d))
    return NULL;
  
  return ((struct __CFMutableData*)d)->_contents;
}

void
CFDataIncreaseLength (CFMutableDataRef d, CFIndex length)
{
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, void, d, "increaseLengthBy:", length);
  
  CFDataSetLength (d, d->_length + length);
}

void
CFDataReplaceBytes (CFMutableDataRef d, CFRange range,
                    const UInt8 *newBytes, CFIndex newLength)
{
  struct __CFMutableData *md;
  CFIndex newBufLen;
  
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, void, d,
    "replaceBytesInRange:withBytes:length:", range, newBytes, newLength);
  
  if (!CFDataIsMutable(d))
    return;
  
  md = (struct __CFMutableData*)d;
  assert (range.location + range.length <= md->_capacity);
  
  newBufLen = range.location + newLength;
  CFDataCheckCapacityAndGrow (d, newBufLen);
  
  if (newLength != range.length && range.location + range.length < newBufLen)
    {
      UInt8 *moveFrom = md->_contents + range.location + range.length;
      UInt8 *moveTo = md->_contents + range.location + newLength;
      CFIndex moveLength = d->_length - (range.location + range.length);
      memmove (moveTo, moveFrom, moveLength);
    }
  if (newLength > 0)
    memmove (md->_contents + range.location, newBytes, newLength);
  
  md->_length = newBufLen;
  md->_hash = 0;
}

void
CFDataSetLength (CFMutableDataRef d, CFIndex length)
{
  struct __CFMutableData *md;
  
  CF_OBJC_FUNCDISPATCHV(_kCFDataTypeID, void, d, "setLength:", length);
  
  if (!CFDataIsMutable(d))
    return;
  
  CFDataCheckCapacityAndGrow (d, length);
  
  md = (struct __CFMutableData*)d;
  if (md->_length < length)
    memset (md->_contents + md->_length, 0, length - md->_length);
  
  md->_length = length;
}

