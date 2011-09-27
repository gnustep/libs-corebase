/* CFData.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFData.h"
#include "objc_interface.h"

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
  _kCFDataIsMutable = (1<<0)
};

CF_INLINE Boolean
CFDataIsMutable (CFDataRef d)
{
  return ((CFRuntimeBase *)d)->_flags.info & _kCFDataIsMutable ? true : false;
}

CF_INLINE void
CFDataSetMutable (CFDataRef d)
{
  ((CFRuntimeBase *)d)->_flags.info |= _kCFDataIsMutable;
}

static void
CFDataFinalize (CFTypeRef cf)
{
  CFDataRef d = (CFDataRef)cf;
  
  CFRelease (d->_deallocator);
}

static Boolean
CFDataEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFDataRef d1 = cf1;
  CFDataRef d2 = cf2;
  CFIndex length = CFDataGetLength (d1);
  
  if (length != CFDataGetLength(d2))
    return false;
  
  return memcmp (d1->_contents, d2->_contents, length);
}

static CFHashCode
CFDataHash (CFTypeRef cf)
{
  CFHashCode ret;
  CFIndex len;
  
  CFDataRef d = (CFDataRef)cf;
  if (d->_hash)
    return d->_hash;
  
  ret = 0;
  len = d->_length;
  if (len > 0)
    {
      register CFIndex idx = 0;
      register const UInt8 *p = d->_contents;
      while (idx < len)
        ret = (ret << 5) + ret + p[idx++];
      
      ret &= 0x0fffffff;
      if (ret == 0)
        ret = 0x0fffffff;
    }
  else
    {
      ret = 0x0ffffffe;
    }
  ((struct __CFData *)d)->_hash = ret;

  return ret;
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
  
  size = sizeof(struct __CFData) - sizeof(CFRuntimeBase);
  size += copy == true ? length : 0;
  
  if (bytesDealloc == NULL)
    bytesDealloc = CFAllocatorGetDefault ();
  
  newData = (struct __CFData*)_CFRuntimeCreateInstance (allocator,
    _kCFDataTypeID, size, NULL);
  
  newData->_length = length;
  
  if (copy)
    {
      newData->_deallocator = CFRetain(bytesDealloc);
      
      memcpy (&(newData[1]), bytes, length);
      bytes = (const UInt8*)&(newData[1]);
    }
  newData->_contents = bytes;
  
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
  CF_OBJC_FUNCDISPATCH0(_kCFDataTypeID, const UInt8 *, d, "bytes");
  
  return d->_contents;
}

void
CFDataGetBytes (CFDataRef d, CFRange range, UInt8 *buffer)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDataTypeID, void, d, "getBytes:range:", buffer,
    range);
  
  assert (range.location + range.length <= d->_length);
  memcpy (buffer, d->_contents + range.location, range.length);
}

CFIndex
CFDataGetLength (CFDataRef d)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDataTypeID, CFIndex, d, "length");
  
  return d->_length;
}



CFMutableDataRef
CFDataCreateMutable (CFAllocatorRef allocator, CFIndex capacity)
{
  struct __CFMutableData *newData;
  CFIndex size;
  
  size = sizeof(struct __CFMutableData) - sizeof(CFRuntimeBase);
  
  newData = (struct __CFMutableData*)_CFRuntimeCreateInstance (allocator,
    _kCFDataTypeID, size, NULL);
  
  newData->_capacity = capacity;
  newData->_allocator = CFRetain(allocator);
  newData->_contents = CFAllocatorAllocate (allocator, capacity, 0);
  
  return (CFMutableDataRef)newData;
}

CFMutableDataRef
CFDataCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFDataRef d)
{
  CFMutableDataRef newData;
  CFIndex length;
  
  assert (capacity > CFDataGetLength(d));
  
  newData = CFDataCreateMutable (allocator, capacity);
  
  length = CFDataGetLength (d);
  memcpy ((UInt8*)newData->_contents, CFDataGetBytePtr(d), length);
  newData->_length = length;
  
  return (CFMutableDataRef)newData;
}

void
CFDataAppendBytes (CFMutableDataRef d, const UInt8 *bytes, CFIndex length)
{
  CF_OBJC_FUNCDISPATCH2(_kCFDataTypeID, void, d, "appendBytes:length:", bytes,
    length);
  
  CFDataReplaceBytes (d, CFRangeMake(CFDataGetLength(d), 0), bytes, length);
}

void
CFDataDeleteBytes (CFMutableDataRef d, CFRange range)
{
  CFDataReplaceBytes(d, range, NULL, 0);
}

UInt8 *
CFDataGetMutableBytePtr (CFMutableDataRef d)
{
  CF_OBJC_FUNCDISPATCH0(_kCFDataTypeID, UInt8 *, d, "mutableBytes");
  
  return ((struct __CFMutableData*)d)->_contents;
}

void
CFDataIncreaseLength (CFMutableDataRef d, CFIndex length)
{
  CF_OBJC_FUNCDISPATCH1(_kCFDataTypeID, void, d, "increaseLengthBy:", length);
  
  if (!CFDataIsMutable(d))
    return;
  
  CFDataSetLength (d, CFDataGetLength(d) + length);
}

void
CFDataReplaceBytes (CFMutableDataRef d, CFRange range,
                    const UInt8 *newBytes, CFIndex newLength)
{
  struct __CFMutableData *md;
  CFIndex newBufLen;
  
  CF_OBJC_FUNCDISPATCH3(_kCFDataTypeID, void, d,
    "replaceBytesInRange:withBytes:length:", range, NULL, 0);
  
  assert (range.location + range.length <= CFDataGetLength(d));
  
  if (!CFDataIsMutable(d))
    return;
  
  md = (struct __CFMutableData*)d;
  
  newBufLen = range.location + newLength;
  if (newBufLen > md->_capacity)
    {
      md->_contents = CFAllocatorReallocate (md->_allocator, md->_contents,
        newBufLen, 0);
      md->_capacity = newBufLen;
    }
  
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
  
  CF_OBJC_FUNCDISPATCH1(_kCFDataTypeID, void, d, "setLength:", length);
  
  if (!CFDataIsMutable(d))
    return;
  
  md = (struct __CFMutableData*)d;
  if (md->_capacity < length)
    {
      md->_contents = CFAllocatorReallocate (md->_allocator, md->_contents,
        length, 0);
      md->_capacity = length;
      md->_length = length;
    }
  else
    {
      CFIndex oldLength = md->_length;
      if (oldLength != length)
        {
          memset (md->_contents + oldLength, 0, length - oldLength);
          md->_length = length;
        }
    }
}
