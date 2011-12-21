/* CFBitVector.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBitVector.h"

static CFTypeID _kCFBitVectorTypeID = 0;

struct __CFBitVector
{
  CFRuntimeBase _parent;
  CFIndex       _count;
  CFIndex       _byteCount;
  UInt8        *_bytes;
};

enum
{
  _kCFBitVectorIsMutable = (1<<0)
};

CF_INLINE Boolean
CFBitVectorIsMutable (CFBitVectorRef bv)
{
  return ((CFRuntimeBase *)bv)->_flags.info & _kCFBitVectorIsMutable ?
    true : false;
}

CF_INLINE void
CFBitVectorSetMutable (CFBitVectorRef bv)
{
  ((CFRuntimeBase *)bv)->_flags.info |= _kCFBitVectorIsMutable;
}

static CFRuntimeClass CFBitVectorClass =
{
  0,
  "CFBitVector",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFBitVectorInitialize (void)
{
  _kCFBitVectorTypeID = _CFRuntimeRegisterClass (&CFBitVectorClass);
}




#define _kCFBitVectorBitsPerByte 8

CF_INLINE CFIndex
CFBitVectorGetByteCount (CFIndex numBits)
{
  return (numBits + _kCFBitVectorBitsPerByte - 1) >> 3; // (numBits + 7) / 8
}

CFTypeID
CFBitVectorGetTypeID (void)
{
  return _kCFBitVectorTypeID;
}

#define CFBITVECTOR_SIZE sizeof(struct __CFBitVector) - sizeof(CFRuntimeBase)

static struct __CFBitVector *
CFBitVectorCreate_internal (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBits, CFIndex capacity)
{
  struct __CFBitVector *new;
  CFIndex byteCount;
  
  if (capacity < numBits)
    capacity = numBits;
  byteCount = CFBitVectorGetByteCount (capacity);
  new = (struct __CFBitVector*)_CFRuntimeCreateInstance (alloc,
    _kCFBitVectorTypeID, CFBITVECTOR_SIZE + byteCount, 0);
  if (new)
    {
      new->_count = numBits;
      new->_byteCount = byteCount;
      new->_bytes = (UInt8*)&new[1];
      
      memcpy (new->_bytes, bytes, byteCount);
    }
  
  return new;
}

CFBitVectorRef
CFBitVectorCreate (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBits)
{
  return (CFBitVectorRef)CFBitVectorCreate_internal (alloc, bytes, numBits,
    numBits);
}

CFBitVectorRef
CFBitVectorCreateCopy (CFAllocatorRef alloc, CFBitVectorRef bv)
{
  return (CFBitVectorRef)CFBitVectorCreate_internal (alloc, bv->_bytes,
    bv->_count, bv->_count);
}
Boolean
CFBitVectorContainsBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return (CFBitVectorGetCountOfBit (bv, range, value) > 0) ? true : false;
}

CFBit
CFBitVectorGetBitAtIndex (CFBitVectorRef bv, CFIndex idx)
{
  CFIndex byteIdx = idx >> 3;
  CFIndex bitIdx = idx & (_kCFBitVectorBitsPerByte - 1);
  return (bv->_bytes[byteIdx] >> (_kCFBitVectorBitsPerByte-1-bitIdx)) & 0x01;
}

void
CFBitVectorGetBits (CFBitVectorRef bv, CFRange range, UInt8 *bytes)
{
  
}

CFIndex
CFBitVectorGetCount (CFBitVectorRef bv)
{
  return bv->_count;
}

CFIndex
CFBitVectorGetCountOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return 0;
}

CFIndex
CFBitVectorGetFirstIndexOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  CFIndex idx;

  for (idx = range.location ; idx < range.length ; idx++)
    {
      if (value == CFBitVectorGetBitAtIndex (bv, idx))
        return idx;
    }
  
  return kCFNotFound;
}

CFIndex
CFBitVectorGetLastIndexOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  CFIndex idx;

  for (idx = range.location + range.length ; idx < range.location ; idx--)
    {
      if (value == CFBitVectorGetBitAtIndex (bv, idx))
        return idx;
    }
  
  return kCFNotFound;
}



CFMutableBitVectorRef
CFBitVectorCreateMutable (CFAllocatorRef alloc, CFIndex capacity)
{
  CFMutableBitVectorRef new;
  
  new = CFBitVectorCreate_internal (alloc, NULL, 0, capacity);
  if (new)
    CFBitVectorSetMutable (new);
  
  return new;
}

CFMutableBitVectorRef
CFBitVectorCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity,
  CFBitVectorRef bv)
{
  CFMutableBitVectorRef new;
  
  new = CFBitVectorCreate_internal (alloc, bv->_bytes, bv->_count, capacity);
  if (new)
    CFBitVectorSetMutable (new);
  
  return new;
}

void
CFBitVectorFlipBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx)
{
  CFBitVectorFlipBits (bv, CFRangeMake(idx, 1));
}

void
CFBitVectorFlipBits (CFMutableBitVectorRef bv, CFRange range)
{
  
}

void
CFBitVectorSetAllBits (CFMutableBitVectorRef bv, CFBit value)
{
  UInt8 bytes = value ? 0xFF : 0x00;
  memset (bv->_bytes, bytes, bv->_byteCount);
}

void
CFBitVectorSetBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx, CFBit value)
{
  CFBitVectorSetBits (bv, CFRangeMake(idx, 1), value);
}

void
CFBitVectorSetBits (CFMutableBitVectorRef bv, CFRange range, CFBit value)
{
  
}

void
CFBitVectorSetCount (CFMutableBitVectorRef bv, CFIndex count)
{
  
}
