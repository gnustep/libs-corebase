/* CFByteOrder.m
   
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

#include "CoreFoundation/CFByteOrder.h"
#include <Foundation/NSByteOrder.h>

CFByteOrder
CFByteOrderGetCurrent()
{
  return (CFByteOrder)NSHostByteOrder();
}

CFSwappedFloat64
CFConvertDoubleHostToSwapped(double in)
{
  return (CFSwappedFloat64)NSConvertHostDoubleToSwapped(in);
}

double
CFConvertDoubleSwappedToHost(CFSwappedFloat64 in)
{
  return NSConvertSwappedDoubleToHost((NSSwappedDouble)in);
}

CFSwappedFloat32
CFConvertFloat32HostToSwapped(Float32 in)
{
  return (CFSwappedFloat32)NSConvertHostFloatToSwapped((float)in);
}

Float32
CFConvertFloat32SwappedToHost(CFSwappedFloat32 in)
{
  return NSConvertSwappedFloatToHost((NSSwappedFloat)in);
}

CFSwappedFloat64
CFConvertFloat64HostToSwapped(Float64 in)
{
  return (CFSwappedFloat64)NSConvertHostDoubleToSwapped((double)in);
}

Float64
CFConvertFloat64SwappedToHost(CFSwappedFloat64 in)
{
  return (Float64)NSConvertSwappedDoubleToHost((NSSwappedDouble)in);
}

CFSwappedFloat32
CFConvertFloatHostToSwapped(float in)
{
  return (CFSwappedFloat32)NSConvertHostFloatToSwapped((float)in);
}

float
CFConvertFloatSwappedToHost(CFSwappedFloat32 in)
{
  return NSConvertSwappedFloatToHost((NSSwappedFloat)in);
}

uint16_t
CFSwapInt16(uint16_t in)
{
  return GSSwapI16(in);
}

uint16_t
CFSwapInt16BigToHost(uint16_t in)
{
  return GSSwapBigI16ToHost(in);
}

uint16_t
CFSwapInt16HostToBig(uint16_t in)
{
  return GSSwapHostI16ToBig(in);
}

uint16_t
CFSwapInt16HostToLittle(uint16_t in)
{
  return GSSwapHostI16ToLittle(in);
}

uint16_t
CFSwapInt16LittleToHost(uint16_t in)
{
  return GSSwapLittleI16ToHost(in);
}

uint32_t
CFSwapInt32(uint32_t in)
{
  return GSSwapI32(in);
}

uint32_t
CFSwapInt32BigToHost(uint32_t in)
{
  return GSSwapBigI32ToHost(in);
}

uint32_t
CFSwapInt32HostToBig(uint32_t in)
{
  return GSSwapHostI32ToBig(in);
}

uint32_t
CFSwapInt32HostToLittle(uint32_t in)
{
  return GSSwapHostI32ToLittle(in);
}

uint32_t
CFSwapInt32LittleToHost(uint32_t in)
{
  return GSSwapLittleI32ToHost(in);
}

uint64_t
CFSwapInt64(uint64_t in)
{
  return GSSwapI64(in);
}

uint64_t
CFSwapInt64BigToHost(uint64_t in)
{
  return GSSwapBigI64ToHost(in);
}

uint64_t
CFSwapInt64HostToBig(uint64_t in)
{
  return GSSwapHostI64ToBig(in);
}

uint64_t
CFSwapInt64HostToLittle(uint64_t in)
{
  return GSSwapHostI64ToLittle(in);
}

uint64_t
CFSwapInt64LittleToHost(uint64_t in)
{
  return GSSwapLittleI64ToHost(in);
}
