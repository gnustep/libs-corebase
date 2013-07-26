/* CFNumber.c
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFNumber.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"

#include <string.h>
#include <math.h>

#ifndef INFINITY
# if defined(_MSC_VER)
#  include <float.h>
#  define INFINITY DBL_MAX + DBL_MAX
#  define NAN (INFINITY) - (INFINITY)
# else
#  define INFINITY 1.0 / 0.0
#  define NAN 0.0 / 0.0
# endif
#endif

struct __CFBoolean
{
  CFRuntimeBase  _parent;
};

static struct __CFBoolean _kCFBooleanTrue =
{
  INIT_CFRUNTIME_BASE()
};

static struct __CFBoolean _kCFBooleanFalse =
{
  INIT_CFRUNTIME_BASE()
};

const CFBooleanRef kCFBooleanTrue = &_kCFBooleanTrue;
const CFBooleanRef kCFBooleanFalse = &_kCFBooleanFalse;

static CFTypeID _kCFBooleanTypeID = 0;

static CFStringRef
CFBooleanCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return cf == kCFBooleanTrue ? CFSTR("true") : CFSTR("false");
}

static const CFRuntimeClass CFBooleanClass =
{
  0,
  "CFBoolean",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  CFBooleanCopyFormattingDesc,
  NULL
};

void CFBooleanInitialize (void)
{
  _kCFBooleanTypeID = _CFRuntimeRegisterClass(&CFBooleanClass);
  
  GSRuntimeConstantInit (kCFBooleanTrue, _kCFBooleanTypeID);
  GSRuntimeConstantInit (kCFBooleanFalse, _kCFBooleanTypeID);
}

CFTypeID
CFBooleanGetTypeID (void)
{
  return _kCFBooleanTypeID;
}

Boolean
CFBooleanGetValue (CFBooleanRef boolean)
{
  return (boolean == kCFBooleanTrue) ? true : false;
}



/* We do not include a pointer for the actual number because it will be
   right after the object itself and the CFNumberType will be stored in the
   info section of CFRuntimeBase.  This implementation of CFNumber will
   hold 3 possible types: 32-bit int, 64-bit int and 64-bit float. */
struct __CFNumber
{
  CFRuntimeBase _parent;
};

struct __CFNumber_static
{
  struct __CFNumber _cfnum;
  Float64 f64;
};

static struct __CFNumber_static _kCFNumberNaN =
{
  { INIT_CFRUNTIME_BASE() },
  ( NAN )
};

static struct __CFNumber_static _kCFNumberNegInf =
{
  { INIT_CFRUNTIME_BASE() },
  ( -INFINITY )
};

static struct __CFNumber_static _kCFNumberPosInf =
{
  { INIT_CFRUNTIME_BASE() },
  ( INFINITY )
};

const CFNumberRef kCFNumberNaN = (CFNumberRef)&_kCFNumberNaN;
const CFNumberRef kCFNumberNegativeInfinity = (CFNumberRef)&_kCFNumberNegInf;
const CFNumberRef kCFNumberPositiveInfinity = (CFNumberRef)&_kCFNumberPosInf;

static CFTypeID _kCFNumberTypeID = 0;

static CFTypeRef
CFNumberCopy (CFAllocatorRef alloc, CFTypeRef cf)
{
  CFNumberRef num;
  CFNumberType type;
  UInt8 bytes[sizeof(double)];
  
  num = (CFNumberRef)cf;
  type = CFNumberGetType (num);
  CFNumberGetValue (num, type, (void*)bytes);
  return CFNumberCreate (alloc, type, (void*)bytes);
}

static CFStringRef
CFNumberCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  CFNumberFormatterRef fmt;
  CFStringRef str;
  
  fmt = CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterNoStyle);
  str = CFNumberFormatterCreateStringWithNumber (NULL, NULL, cf);
  
  CFRelease (fmt);
  return str;
}

static CFRuntimeClass CFNumberClass =
{
  0,
  "CFNumber",
  NULL,
  CFNumberCopy,
  NULL,
  NULL,
  NULL,
  CFNumberCopyFormattingDesc,
  NULL
};

void CFNumberInitialize (void)
{
  _kCFNumberTypeID = _CFRuntimeRegisterClass (&CFNumberClass);
  
  GSRuntimeConstantInit (kCFNumberNaN, _kCFNumberTypeID);
  _kCFNumberNaN._cfnum._parent._flags.info = kCFNumberDoubleType;
  GSRuntimeConstantInit (kCFNumberNegativeInfinity, _kCFNumberTypeID);
  _kCFNumberNegInf._cfnum._parent._flags.info = kCFNumberDoubleType;
  GSRuntimeConstantInit (kCFNumberPositiveInfinity, _kCFNumberTypeID);
  _kCFNumberPosInf._cfnum._parent._flags.info = kCFNumberDoubleType;
}

CF_INLINE CFNumberType
CFNumberGetType_internal(CFNumberRef num)
{
  return (CFNumberType)num->_parent._flags.info;
}

CF_INLINE CFIndex
CFNumberByteSizeOfType (CFNumberType type)
{
  switch (type)
    {
      case kCFNumberSInt8Type:
        return sizeof(SInt8);
      case kCFNumberSInt16Type:
        return sizeof(SInt16);
      case kCFNumberSInt32Type:
        return sizeof(SInt32);
      case kCFNumberSInt64Type:
        return sizeof(SInt64);
      case kCFNumberFloat32Type:
        return sizeof(Float32);
      case kCFNumberFloat64Type:
        return sizeof(Float64);
      case kCFNumberCharType:
        return sizeof(char);
      case kCFNumberShortType:
        return sizeof(short);
      case kCFNumberIntType:
        return sizeof(int);
      case kCFNumberLongType:
        return sizeof(long);
      case kCFNumberCFIndexType:
        return sizeof(CFIndex);
      case kCFNumberNSIntegerType:
        return sizeof(uintptr_t);
      case kCFNumberLongLongType:
        return sizeof(long long);
      case kCFNumberFloatType:
        return sizeof(float);
      case kCFNumberDoubleType:
        return sizeof(double);
#if defined(__LP64__)
      case kCFNumberCGFloatType:
        return sizeof(double);
#else
      case kCFNumberCGFloatType:
        return sizeof(float);
#endif
    }
  return 0;
}

CF_INLINE CFNumberType
CFNumberBestType (CFNumberType type)
{
  switch (type)
    {
      case kCFNumberSInt8Type:
      case kCFNumberCharType:
      case kCFNumberSInt16Type:
      case kCFNumberShortType:
      case kCFNumberSInt32Type:
      case kCFNumberIntType:
#if !defined(__LP64__) && !defined(_WIN64)
      case kCFNumberLongType:
      case kCFNumberCFIndexType:
      case kCFNumberNSIntegerType:
#endif
        return kCFNumberSInt32Type;
        break;
      case kCFNumberSInt64Type:
      case kCFNumberLongLongType:
#if defined(__LP64__) || defined(_WIN64)
      case kCFNumberLongType:
      case kCFNumberCFIndexType:
      case kCFNumberNSIntegerType:
#endif
        return kCFNumberSInt64Type;
        break;
      case kCFNumberFloat32Type:
      case kCFNumberFloatType:
      case kCFNumberFloat64Type:
      case kCFNumberDoubleType:
      case kCFNumberCGFloatType:
        return kCFNumberFloat64Type;
        break;
    }
  return 0;
}

CF_INLINE Boolean
CFNumberTypeIsFloat (CFNumberType type)
{
  switch (type)
    {
      case kCFNumberFloat32Type:
      case kCFNumberFloat64Type:
      case kCFNumberFloatType:
      case kCFNumberDoubleType:
      case kCFNumberCGFloatType:
        return true;
      default:
        return false;
    }
}

CFComparisonResult
CFNumberCompare (CFNumberRef num, CFNumberRef oNum,
  void *context)
{
  CF_OBJC_FUNCDISPATCHV(_kCFNumberTypeID, CFComparisonResult, num,
    "compare:", oNum);
  CF_OBJC_FUNCDISPATCHV(_kCFNumberTypeID, CFComparisonResult, oNum,
    "compare:", num);
  
  return -1;
}

CFNumberRef
CFNumberCreate (CFAllocatorRef alloc, CFNumberType type,
  const void *valuePtr)
{
  struct __CFNumber *new;
  CFIndex size;
  CFIndex byteSize;
  CFNumberType bestType;
  SInt32 value32;
  Boolean hasValue32;
  
  switch (type)
    {
      case kCFNumberSInt8Type:
      case kCFNumberCharType:
        value32 = *(const SInt8*)valuePtr;
        hasValue32 = true;
        break;
      case kCFNumberSInt16Type:
      case kCFNumberShortType:
        value32 = *(const SInt16*)valuePtr;
        hasValue32 = true;
        break;
      case kCFNumberSInt32Type:
      case kCFNumberIntType:
        value32 = *(const SInt32*)valuePtr;
        hasValue32 = true;
        break;
      default:
        value32 = 0;
        hasValue32 = false;
    }
  bestType = CFNumberBestType (type);
  byteSize = CFNumberByteSizeOfType(bestType);
  
  size = sizeof(struct __CFNumber) - sizeof(CFRuntimeBase) + byteSize;
  new = (struct __CFNumber*)_CFRuntimeCreateInstance (alloc, _kCFNumberTypeID,
    size, 0);
  
  new->_parent._flags.info = bestType;
  
  if (hasValue32)
    {
      memcpy ((void*)&new[1], &value32, sizeof(SInt32));
    }
  else if (type == kCFNumberFloat32Type
          || type == kCFNumberFloatType
#if !defined(__LP64__) && !defined(_WIN64)
          || type == kCFNumberCGFloatType
#endif
          )
    {
      Float64 d = *(const float*)valuePtr;
      memcpy ((void*)&new[1], &d, sizeof(Float64));
    }
  else
    {
      memcpy ((void*)&new[1], valuePtr, byteSize);
    }
  
  return new;
}

CFIndex
CFNumberGetByteSize (CFNumberRef num)
{
  return CFNumberByteSizeOfType (CFNumberGetType_internal(num));
}

CFNumberType
CFNumberGetType (CFNumberRef num)
{
  return CFNumberGetType_internal (num);
}

CFTypeID
CFNumberGetTypeID (void)
{
  return _kCFNumberTypeID;
}

#define CFNumberConvert(srcType, src, dstType, dst, success) do \
{ \
  srcType source; \
  dstType destination; \
  memcpy (&source, src, sizeof(srcType)); \
  destination = (dstType)source; \
  memcpy (dst, &destination, sizeof(dstType)); \
  success = (*(dstType*)dst == *(srcType*)src); \
} while(0)

Boolean
CFNumberGetValue (CFNumberRef num, CFNumberType type, void *valuePtr)
{
  CFNumberType numType = CFNumberGetType_internal (num);
  Boolean success;
  
  switch (type)
    {
      case kCFNumberSInt8Type:
      case kCFNumberCharType:
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), SInt8, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), SInt8, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), SInt8, valuePtr, success);
        return success;
      case kCFNumberSInt16Type:
      case kCFNumberShortType:
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), SInt16, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), SInt16, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), SInt16, valuePtr, success);
        return success;
      case kCFNumberSInt32Type:
      case kCFNumberIntType:
#if !defined(__LP64__) && !defined(_WIN64)
      case kCFNumberLongType:
      case kCFNumberCFIndexType:
      case kCFNumberNSIntegerType:
#endif
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), SInt32, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), SInt32, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), SInt32, valuePtr, success);
        return success;
      case kCFNumberSInt64Type:
      case kCFNumberLongLongType:
#if defined(__LP64__) || defined(_WIN64)
      case kCFNumberLongType:
      case kCFNumberCFIndexType:
      case kCFNumberNSIntegerType:
#endif
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), SInt64, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), SInt64, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), SInt64, valuePtr, success);
        return success;
      case kCFNumberFloat32Type:
      case kCFNumberFloatType:
#if !defined(__LP64__) && !defined(_WIN64)
      case kCFNumberCGFloatType:
#endif
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), Float32, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), Float32, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), Float32, valuePtr, success);
        return success;
      case kCFNumberFloat64Type:
      case kCFNumberDoubleType:
#if defined(__LP64__) || defined(_WIN64)
      case kCFNumberCGFloatType:
#endif
        if (numType == kCFNumberSInt32Type)
          CFNumberConvert (SInt32, &(num[1]), Float64, valuePtr, success);
        else if (numType == kCFNumberSInt64Type)
          CFNumberConvert (SInt64, &(num[1]), Float64, valuePtr, success);
        else
          CFNumberConvert (Float64, &(num[1]), Float64, valuePtr, success);
        return success;
    }
  
  return false;
}

Boolean
CFNumberIsFloatType (CFNumberRef num)
{
  CFNumberType type = CFNumberGetType_internal (num);
  
  return CFNumberTypeIsFloat (type);
}

