/* CFNumber.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
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

#import <Foundation/NSValue.h>

#include "CoreFoundation/CFNumber.h"

// FIXME:
const CFNumberRef kCFNumberNaN = (CFNumberRef)nil;
const CFNumberRef kCFNumberNegativeInfinity = (CFNumberRef)nil;
const CFNumberRef kCFNumberPositiveInfinity = (CFNumberRef)nil;



CFComparisonResult CFNumberCompare (CFNumberRef number,
  CFNumberRef otherNumber, void *context)
{
  return [(NSNumber *)number compare: (NSNumber *)otherNumber];
}

CFNumberRef CFNumberCreate (CFAllocatorRef allocator, CFNumberType theType,
  const void *valuePtr)
{
  char *type;
  switch (theType)
    {
      case kCFNumberSInt8Type:
        type = @encode(SInt8);
        break;
      case kCFNumberSInt16Type:
        type = @encode(SInt16);
        break;
      case kCFNumberSInt32Type:
        type = @encode(SInt32);
        break;
      case kCFNumberSInt64Type:
        type = @encode(SInt64);
        break;
      case kCFNumberFloat32Type:
        type = @encode(Float32);
        break;
      case kCFNumberFloat64Type:
        type = @encode(Float64);
        break;
      case kCFNumberCharType:
        type = @encode(char);
        break;
      case kCFNumberShortType:
        type = @encode(short);
        break;
      case kCFNumberIntType:
        type = @encode(int);
        break;
      case kCFNumberLongType:
        type = @encode(long);
        break;
      case kCFNumberLongLongType:
        type = @encode(long long);
        break;
      case kCFNumberFloatType:
        type = @encode(float);
        break;
      case kCFNumberDoubleType:
        type = @encode(double);
        break;
      case kCFNumberCFIndexType:
        type = @encode(CFIndex);
        break;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
      case kCFNumberNSIntegerType:
        type = @encode(NSInteger);
        break;
      case kCFNumberCGFloatType:
        type = @encode(CGFloat);
        break;
#endif
      default:
        type = NULL;
        break;
    }
  
  return (CFNumberRef)[[NSNumber allocWithZone: allocator]
                        initWithBytes: valuePtr objCType: type];
}

CFIndex CFNumberGetByteSize (CFNumberRef number)
{
  // FIXME: Is this the correct way of doing this operation?
  return (CFIndex)objc_sizeof_type([(NSNumber *)number objCType]);
}

CFNumberType CFNumberGetType (CFNumberRef number)
{
  GSNumberInfo *info = GSNumberInfoFromObject((NSNumber *)number);
  
  // FIXME: is this everything?
  switch (info->typeLevel)
    {
      case 1:
        return kCFNumberCharType;
      case 3:
        return kCFNumberShortType;
      case 5:
        return kCFNumberIntType;
      case 7:
        return kCFNumberLongType;
      case 9:
        return kCFNumberLongLongType;
      case 11:
        return kCFNumberFloatType;
      case 12:
        return kCFNumberDoubleType;
      defualt:
        return 0;
    }
}

CFTypeID CFNumberGetTypeID (void)
{
  return (CFTypeID)[NSNumber class];
}

Boolean CFNumberGetValue (CFNumberRef number, CFNumberType theType,
  void *valuePtr)
{
  // FIXME
  if (CFNumberGetType(number) == theType)
    {
      [(NSNumber *)number getValue: valuePtr];
      return true;
    }
  return false;
}

Boolean CFNumberIsFloatType (CFNumberRef number)
{
  const char *type = [(NSNumber *)number objCType];
  
  // Did I miss anything?
  if (strcmp (@encode(float), type) == 0
      || strcmp (@encode(double), type) == 0
      || strcmp (@encode(Float32), type) == 0
      || strcmp (@encode(Float64), type) == 0)
    return true;
  
  return false;
}
