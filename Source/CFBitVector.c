/* CFBitVector.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFBitVector.h"

#include <string.h>

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

CF_INLINE CFIndex
CFBitVectorGetByteCount (CFIndex numBits)
{
  return (numBits + 7) >> 3; /* (numBits + 7) / 8 */
}

static void
CFBitVectorFinalize (CFTypeRef cf)
{
  CFBitVectorRef bv = (CFBitVectorRef)cf;
  if (CFBitVectorIsMutable(bv))
    CFAllocatorDeallocate (CFGetAllocator(cf), bv->_bytes);
}

static Boolean
CFBitVectorEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFBitVectorRef bv1 = (CFBitVectorRef)cf1;
  CFBitVectorRef bv2 = (CFBitVectorRef)cf2;
  
  if (bv1->_count == bv2->_count)
    {
      return memcmp (bv1->_bytes, bv2->_bytes,
        CFBitVectorGetByteCount(bv1->_count)) == 0 ? true : false;
    }
  return false;
}

static CFHashCode
CFBitVectorHash (CFTypeRef cf)
{
  CFBitVectorRef bv = (CFBitVectorRef)cf;
  return bv->_count;
}

static CFRuntimeClass CFBitVectorClass =
{
  0,
  "CFBitVector",
  NULL,
  NULL,
  CFBitVectorFinalize,
  CFBitVectorEqual,
  CFBitVectorHash,
  NULL,
  NULL
};

void CFBitVectorInitialize (void)
{
  _kCFBitVectorTypeID = _CFRuntimeRegisterClass (&CFBitVectorClass);
}



CF_INLINE CFIndex
CFBitVectorGetByte (CFIndex idx)
{
  return idx >> 3; /* idx / 8 */
}

CF_INLINE CFIndex
CFBitVectorGetBitIndex (CFIndex idx)
{
  return idx & 7; /* idx % 8 */
}

CF_INLINE UInt8
CFBitVectorBitMask (UInt8 mostSig, UInt8 leastSig)
{
  return ((0xFF << (7 - leastSig + mostSig)) >> mostSig);
}

static void
CFBitVectorOperation (CFBitVectorRef bv, CFRange range,
  UInt8 (*func)(UInt8, UInt8, void*), void *context)
{
  CFIndex curByte;
  CFIndex endByte;
  CFIndex startBit;
  CFIndex endBit;
  UInt8 mask;
  Boolean multiByte;
  
  curByte = CFBitVectorGetByte (range.location);
  endByte = CFBitVectorGetByte (range.location + range.length - 1);
  startBit = CFBitVectorGetBitIndex (range.location);
  endBit = CFBitVectorGetBitIndex (range.location + range.length - 1);
  
  /* First byte */
  if (curByte == endByte)
    {
      mask = CFBitVectorBitMask (startBit, startBit + endBit);
      multiByte = false;
    }
  else
    {
      mask = CFBitVectorBitMask (startBit, 7);
      multiByte = true;
    }
  bv->_bytes[curByte] = func (bv->_bytes[curByte], mask, context);
  
  /* Middle bytes */
  while (curByte < endByte)
    {
      bv->_bytes[curByte] = func (bv->_bytes[curByte], 0xFF, context);
      ++curByte;
    }
  
  /* Last byte */
  if (multiByte)
    {
      mask = CFBitVectorBitMask (0, endBit);
      bv->_bytes[curByte] = func (bv->_bytes[curByte], mask, context);
    }
}

CFTypeID
CFBitVectorGetTypeID (void)
{
  return _kCFBitVectorTypeID;
}

#define CFBITVECTOR_SIZE sizeof(struct __CFBitVector) - sizeof(CFRuntimeBase)

CFBitVectorRef
CFBitVectorCreate (CFAllocatorRef alloc, const UInt8 *bytes,
  CFIndex numBits)
{
  struct __CFBitVector *new;
  CFIndex byteCount;
  
  byteCount = CFBitVectorGetByteCount (numBits);
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
CFBitVectorCreateCopy (CFAllocatorRef alloc, CFBitVectorRef bv)
{
  return CFBitVectorCreate (alloc, bv->_bytes, bv->_count);
}
Boolean
CFBitVectorContainsBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return (CFBitVectorGetCountOfBit (bv, range, value) > 0) ? true : false;
}

CFBit
CFBitVectorGetBitAtIndex (CFBitVectorRef bv, CFIndex idx)
{
  CFIndex byteIdx = CFBitVectorGetByte (idx);
  CFIndex bitIdx = CFBitVectorGetBitIndex (idx);
  return (bv->_bytes[byteIdx] >> (7 - bitIdx)) & 0x01;
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

#if defined(__GNUC__) || defined(__llvm__)
#define POPCOUNT(u8) __builtin_popcount(u8)
#else
static UInt8 mu0 = 0x55;
static UInt8 mu1 = 0x33;
static UInt8 mu2 = 0x0F;
CF_INLINE CFIndex POPCOUNT(UInt8 u8)
{
  UInt8 x = u8 - ((u8>>1)&mu0);
  x = (x & mu1) + ((x>>2)&mu1);
  x = (x + (x>>4)) & mu2;
  return x & 0xFF;
}
#endif

static UInt8
CountOne (UInt8 byte, UInt8 mask, void *context)
{
  CFIndex *count = (CFIndex*)context;
  
  *count += POPCOUNT(byte & mask);
  return byte;
}

static UInt8
CountZero (UInt8 byte, UInt8 mask, void *context)
{
  CFIndex *count = (CFIndex*)context;
  
  *count += POPCOUNT(~byte & mask);
  return byte;
}

CFIndex
CFBitVectorGetCountOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  CFIndex count;
  CFBitVectorOperation (bv, range, value ? CountOne : CountZero, &count);
  return count;
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
  
  new = (CFMutableBitVectorRef)CFBitVectorCreate (alloc, NULL, 0);
  if (new)
    {
      CFIndex byteCount;
      
      CFBitVectorSetMutable (new);
      
      byteCount = CFBitVectorGetByteCount(capacity);
      new->_bytes = CFAllocatorAllocate (alloc, byteCount, 0);
    }
  
  return new;
}

CFMutableBitVectorRef
CFBitVectorCreateMutableCopy (CFAllocatorRef alloc, CFIndex capacity,
  CFBitVectorRef bv)
{
  CFMutableBitVectorRef new;
  
  if (capacity < bv->_count)
    capacity = bv->_count;
  new = CFBitVectorCreateMutable (alloc, capacity);
  if (new)
    {
      memcpy (new->_bytes, bv->_bytes, CFBitVectorGetByteCount(bv->_count));
      new->_count = bv->_count;
    }
  
  return new;
}

static UInt8
FlipBits (UInt8 byte, UInt8 mask, void *context)
{
  return byte ^ mask;
}

void
CFBitVectorFlipBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx)
{
  CFBitVectorOperation (bv, CFRangeMake(idx, 1), FlipBits, NULL);
}

void
CFBitVectorFlipBits (CFMutableBitVectorRef bv, CFRange range)
{
  CFBitVectorOperation (bv, range, FlipBits, NULL);
}

void
CFBitVectorSetAllBits (CFMutableBitVectorRef bv, CFBit value)
{
  UInt8 bytes = value ? 0xFF : 0x00;
  memset (bv->_bytes, bytes, bv->_byteCount);
}

static UInt8
SetOne (UInt8 byte, UInt8 mask, void *context)
{
  return byte | mask;
}

static UInt8
SetZero (UInt8 byte, UInt8 mask, void *context)
{
  return byte & (~mask);
}

void
CFBitVectorSetBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx, CFBit value)
{
  CFBitVectorOperation (bv, CFRangeMake(idx, 1), value ? SetOne : SetZero, NULL);
}

void
CFBitVectorSetBits (CFMutableBitVectorRef bv, CFRange range, CFBit value)
{
  CFBitVectorOperation (bv, range, value ? SetOne : SetZero, NULL);
}

void
CFBitVectorSetCount (CFMutableBitVectorRef bv, CFIndex count)
{
  if (count != bv->_count)
    {
      CFIndex newByteCount = CFBitVectorGetByteCount (count);
      
      if (newByteCount > bv->_byteCount)
        {
          UInt8 *newBytes;
          
          newBytes = CFAllocatorAllocate (CFGetAllocator(bv), newByteCount, 0);
          memcpy (newBytes, bv->_bytes, bv->_byteCount);
          
          CFAllocatorDeallocate (CFGetAllocator(bv), bv->_bytes);
          
          bv->_bytes = newBytes;
          bv->_count = count;
        }
      else
        {
          bv->_count = count;
        }
    }
}

