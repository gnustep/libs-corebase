/* CFByteOrder.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Eric Wasylishen
   Date: June, 2010
   
   Most of the code here was copied from NSByteOrder.h in GNUstep-base
   written by Richard Frith-Macdonald.
   
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

#ifndef __COREFOUNDATION_CFBYTEORDER_H__
#define __COREFOUNDATION_CFBYTEORDER_H__

#include <CoreFoundation/CFBase.h>

typedef enum
{
  CFByteOrderUnknown,
  CFByteOrderLittleEndian,
  CFByteOrderBigEndian
} CFByteOrder;

typedef uint32_t CFSwappedFloat32;  /* Same as GNUstep NSSwappedFloat */
typedef uint64_t CFSwappedFloat64;  /* Same as GNUstep NSSwappedDouble */

static inline CFByteOrder
CFByteOrderGetCurrent()
{
#if GS_WORDS_BIGENDIAN
  return CFByteOrderBigEndian;
#else
  return CFByteOrderLittleEndian;
#endif
}

static inline uint16_t
CFSwapInt16(uint16_t in)
{
  union swap
    {
      uint16_t num;
      uint8_t  byt[2];
    } dst;
  union swap *src = (union swap*)&in;
  dst.byt[0] = src->byt[1];
  dst.byt[1] = src->byt[0];
  return dst.num;
}

static inline uint32_t
CFSwapInt32(uint32_t in)
{
  union swap
    {
      uint32_t num;
      uint8_t  byt[4];
    } dst;
  union swap *src = (union swap*)&in;
  dst.byt[0] = src->byt[3];
  dst.byt[1] = src->byt[2];
  dst.byt[2] = src->byt[1];
  dst.byt[3] = src->byt[0];
  return dst.num;
}

static inline uint64_t
CFSwapInt64(uint64_t in)
{
  union swap
    {
      uint64_t num;
      uint8_t  byt[8];
    } dst;
  union swap *src = (union swap*)&in;
  dst.byt[0] = src->byt[7];
  dst.byt[1] = src->byt[6];
  dst.byt[2] = src->byt[5];
  dst.byt[3] = src->byt[4];
  dst.byt[4] = src->byt[3];
  dst.byt[5] = src->byt[2];
  dst.byt[6] = src->byt[1];
  dst.byt[7] = src->byt[0];
  return dst.num;
}



#if GS_WORDS_BIGENDIAN

static inline uint16_t
CFSwapInt16BigToHost(uint16_t in)
{
  return in;
}

static inline uint16_t
CFSwapInt16HostToBig(uint16_t in)
{
  return in;
}
static inline uint16_t
CFSwapInt16HostToLittle(uint16_t in)
{
  return CFSwapInt16(in);
}

static inline uint16_t
CFSwapInt16LittleToHost(uint16_t in)
{
  return CFSwapInt16(in);
}

static inline uint32_t
CFSwapInt32BigToHost(uint32_t in)
{
  return in;
}

static inline uint32_t
CFSwapInt32HostToBig(uint32_t in)
{
  return in;
}

static inline uint32_t
CFSwapInt32HostToLittle(uint32_t in)
{
  return CFSwapInt32(in);
}

static inline uint32_t
CFSwapInt32LittleToHost(uint32_t in)
{
  return CFSwapInt32(in);
}

static inline uint64_t
CFSwapInt64BigToHost(uint64_t in)
{
  return in;
}

static inline uint64_t
CFSwapInt64HostToBig(uint64_t in)
{
  return in;
}

static inline uint64_t
CFSwapInt64HostToLittle(uint64_t in)
{
  return CFSwapInt64(in);
}

static inline uint64_t
CFSwapInt64LittleToHost(uint64_t in)
{
  return CFSwapInt64(in);
}

#else

static inline uint16_t
CFSwapInt16BigToHost(uint16_t in)
{
  return CFSwapInt16(in);
}

static inline uint16_t
CFSwapInt16HostToBig(uint16_t in)
{
  return CFSwapInt16(in);
}
static inline uint16_t
CFSwapInt16HostToLittle(uint16_t in)
{
  return in;
}

static inline uint16_t
CFSwapInt16LittleToHost(uint16_t in)
{
  return in;
}

static inline uint32_t
CFSwapInt32BigToHost(uint32_t in)
{
  return CFSwapInt32(in);
}

static inline uint32_t
CFSwapInt32HostToBig(uint32_t in)
{
  return CFSwapInt32(in);
}

static inline uint32_t
CFSwapInt32HostToLittle(uint32_t in)
{
  return in;
}

static inline uint32_t
CFSwapInt32LittleToHost(uint32_t in)
{
  return in;
}

static inline uint64_t
CFSwapInt64BigToHost(uint64_t in)
{
  return CFSwapInt64(in);
}

static inline uint64_t
CFSwapInt64HostToBig(uint64_t in)
{
  return CFSwapInt64(in);
}

static inline uint64_t
CFSwapInt64HostToLittle(uint64_t in)
{
  return in;
}

static inline uint64_t
CFSwapInt64LittleToHost(uint64_t in)
{
  return in;
}

#endif



union dconv
{
  double           d;
  Float64          num;
  CFSwappedFloat64 sf;
};

union fconv
{
  float            f;
  Float32          num;
  CFSwappedFloat32 sf;
};

static inline CFSwappedFloat64
CFConvertFloat64HostToSwapped(Float64 in)
{
  union dconv conv.num = in;
  return CFSwapInt64 (conv.sf);
}

static inline Float64
CFConvertFloat64SwappedToHost(CFSwappedFloat64 in)
{
  union dconv conv.sf = CFSwapInt64 (in);
  return conv.num;
}

static inline CFSwappedFloat64
CFConvertDoubleHostToSwapped(double in)
{
  union dconv conv.d = in;
  return CFSwapInt64 (conv.sf);
}

static inline double
CFConvertDoubleSwappedToHost(CFSwappedFloat64 in)
{
  union dconv conv.sf = CFSwapInt64 (in);
  return conv.d;
}

static inline CFSwappedFloat32
CFConvertFloat32HostToSwapped(Float32 in)
{
  union fconv conv.num = in;
  return CFSwapInt32 (conv.sf);
}

static inline Float32
CFConvertFloat32SwappedToHost(CFSwappedFloat32 in)
{
  union fconv conv.sf = CFSwapInt32 (in);
  return conv.num;
}

static inline CFSwappedFloat32
CFConvertFloatHostToSwapped(float in)
{
  union fconv conv.f = in;
  return CFSwapInt32 (conv.sf);
}

static inline float
CFConvertFloatSwappedToHost(CFSwappedFloat32 in)
{
  union fconv conv.sf = CFSwapInt32 (in);
  return conv.f;
}

#endif /* __COREFOUNDATION_CFBYTEORDER_H__ */
