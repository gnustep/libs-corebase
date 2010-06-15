/* CFByteOrder.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Eric Wasylishen
   Date: June, 2010
   
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

#ifndef __COREFOUNDATION_CFBYTEORDER_H__
#define __COREFOUNDATION_CFBYTEORDER_H__

#include <CoreFoundation/CFBase.h>

typedef enum {
  CFByteOrderUnknown,
  CFByteOrderLittleEndian,
  CFByteOrderBigEndian
} CFByteOrder;

typedef uint32_t CFSwappedFloat32;  /* Same as GNUstep NSSwappedFloat */
typedef uint64_t CFSwappedFloat64;  /* Same as GNUstep NSSwappedDouble */

CFByteOrder
CFByteOrderGetCurrent();

CFSwappedFloat64
CFConvertDoubleHostToSwapped(double in);

double
CFConvertDoubleSwappedToHost(CFSwappedFloat64 in);

CFSwappedFloat32
CFConvertFloat32HostToSwapped(Float32 in);

Float32
CFConvertFloat32SwappedToHost(CFSwappedFloat32 in);

CFSwappedFloat64
CFConvertFloat64HostToSwapped(Float64 in);

Float64
CFConvertFloat64SwappedToHost(CFSwappedFloat64 in);

CFSwappedFloat32
CFConvertFloatHostToSwapped(float in);

float
CFConvertFloatSwappedToHost(CFSwappedFloat32 in);

uint16_t
CFSwapInt16(uint16_t in);

uint16_t
CFSwapInt16BigToHost(uint16_t in);

uint16_t
CFSwapInt16HostToBig(uint16_t in);

uint16_t
CFSwapInt16HostToLittle(uint16_t in);

uint16_t
CFSwapInt16LittleToHost(uint16_t in);

uint32_t
CFSwapInt32(uint32_t in);

uint32_t
CFSwapInt32BigToHost(uint32_t in);

uint32_t
CFSwapInt32HostToBig(uint32_t in);

uint32_t
CFSwapInt32HostToLittle(uint32_t in);

uint32_t
CFSwapInt32LittleToHost(uint32_t in);

uint64_t
CFSwapInt64(uint64_t in);

uint64_t
CFSwapInt64BigToHost(uint64_t in);

uint64_t
CFSwapInt64HostToBig(uint64_t in);

uint64_t
CFSwapInt64HostToLittle(uint64_t in);

uint64_t
CFSwapInt64LittleToHost(uint64_t in);

#endif /* __COREFOUNDATION_CFBYTEORDER_H__ */