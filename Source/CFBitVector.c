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

#include "CoreFoundation/CFBitVector.h"

static CFTypeID _kCFBitVectorTypeID = 0;



CFTypeID
CFBitVectorGetTypeID (void)
{
  return _kCFBitVectorTypeID;
}

CFBitVectorRef
CFBitVectorCreate (CFAllocatorRef allocator, const UInt8 *bytes,
  CFIndex numBits)
{
  return NULL;
}

CFBitVectorRef
CFBitVectorCreateCopy (CFAllocatorRef allocator, CFBitVectorRef bv)
{
  return NULL;
}
Boolean
CFBitVectorContainsBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return false;
}

CFBit
CFBitVectorGetBitAtIndex (CFBitVectorRef bv, CFIndex idx)
{
  return 0;
}

void
CFBitVectorGetBits (CFBitVectorRef bv, CFRange range, UInt8 *bytes)
{
  
}

CFIndex
CFBitVectorGetCount (CFBitVectorRef bv)
{
  return -1;
}

CFIndex
CFBitVectorGetCountOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return 0;
}

CFIndex
CFBitVectorGetFirstIndexOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return 0;
}

CFIndex
CFBitVectorGetLastIndexOfBit (CFBitVectorRef bv, CFRange range, CFBit value)
{
  return 0;
}



CFMutableBitVectorRef
CFBitVectorCreateMutable (CFAllocatorRef allocator, CFIndex capacity)
{
  return NULL;
}

CFMutableBitVectorRef
CFBitVectorCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFBitVectorRef bv)
{
  return NULL;
}

void
CFBitVectorFlipBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx)
{
  
}

void
CFBitVectorFlipBits (CFMutableBitVectorRef bv, CFRange range)
{
  
}

void
CFBitVectorSetAllBits (CFMutableBitVectorRef bv, CFBit value)
{
  
}

void
CFBitVectorSetBitAtIndex (CFMutableBitVectorRef bv, CFIndex idx, CFBit value)
{
  
}

void
CFBitVectorSetBits (CFMutableBitVectorRef bv, CFRange range, CFBit value)
{
  
}

void
CFBitVectorSetCount (CFMutableBitVectorRef bv, CFIndex count)
{
  
}
