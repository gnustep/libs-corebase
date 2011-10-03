/* CFArray.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
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


#ifndef __COREFOUNDATION_CFARRAY_H__
#define __COREFOUNDATION_CFARRAY_H__

#include <CoreFoundation/CFBase.h>

CF_EXTERN_C_BEGIN

#ifdef __OBJC__
@class NSArray;
@class NSMutableArray;
typedef NSArray* CFArrayRef;
typedef NSMutableArray* CFMutableArrayRef;
#else
typedef const struct __CFArray * CFArrayRef;
typedef struct __CFArray * CFMutableArrayRef;
#endif

typedef void (*CFArrayApplierFunction) (const void *value, void *context);
typedef CFStringRef (*CFArrayCopyDescriptionCallBack) (const void *value);
typedef void (*CFArrayReleaseCallBack) (CFAllocatorRef allocator,
  const void *value);
typedef const void *(*CFArrayRetainCallBack) (CFAllocatorRef allocator,
  const void *value);
typedef Boolean (*CFArrayEqualCallBack) (const void *value1,
  const void *value2);

typedef struct _CFArrayCallBacks CFArrayCallBacks;
struct _CFArrayCallBacks
{
  CFIndex version;
  CFArrayRetainCallBack retain;
  CFArrayReleaseCallBack release;
  CFArrayCopyDescriptionCallBack copyDescription;
  CFArrayEqualCallBack equal;
};

CF_EXPORT const CFArrayCallBacks kCFTypeArrayCallBacks;



//
// Creating an Array
//
CFArrayRef
CFArrayCreate (CFAllocatorRef allocator, const void **values,
  CFIndex numValues, const CFArrayCallBacks *callBacks);

CFArrayRef
CFArrayCreateCopy (CFAllocatorRef allocator, CFArrayRef theArray);

//
// Examining an Array
//
CFIndex
CFArrayBSearchValues (CFArrayRef theArray, CFRange range, const void *value,
  CFComparatorFunction comparator, void *context);

Boolean
CFArrayContainsValue (CFArrayRef theArray, CFRange range, const void *value);

CFIndex
CFArrayGetCount (CFArrayRef theArray);

CFIndex
CFArrayGetCountOfValue (CFArrayRef theArray, CFRange range, const void *value);

CFIndex
CFArrayGetFirstIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value);

CFIndex
CFArrayGetLastIndexOfValue (CFArrayRef theArray, CFRange range,
  const void *value);

void
CFArrayGetValues (CFArrayRef theArray, CFRange range, const void **values);

const void *
CFArrayGetValueAtIndex (CFArrayRef theArray, CFIndex idx);

//
// Applying a Function to Elements
//
void
CFArrayApplyFunction (CFArrayRef theArray, CFRange range,
  CFArrayApplierFunction applier, void *context);

//
// Getting the CFArray Type ID
//
CFTypeID
CFArrayGetTypeID (void);

//
// CFMutableArray
//
void
CFArrayAppendArray (CFMutableArrayRef theArray, CFArrayRef otherArray,
  CFRange otherRange);

void
CFArrayAppendValue (CFMutableArrayRef theArray, const void *value);

CFMutableArrayRef
CFArrayCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
  const CFArrayCallBacks *callBacks);

CFMutableArrayRef
CFArrayCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
  CFArrayRef theArray);

void
CFArrayExchangeValuesAtIndices (CFMutableArrayRef theArray, CFIndex idx1,
  CFIndex idx2);

void
CFArrayInsertValueAtIndex (CFMutableArrayRef theArray, CFIndex idx,
  const void *value);

void
CFArrayRemoveAllValues (CFMutableArrayRef theArray);

void
CFArrayRemoveValueAtIndex (CFMutableArrayRef theArray, CFIndex idx);

void
CFArrayReplaceValues (CFMutableArrayRef theArray, CFRange range,
  const void **newValues, CFIndex newCount);

void
CFArraySetValueAtIndex (CFMutableArrayRef theArray, CFIndex idx,
  const void *value);

void
CFArraySortValues (CFMutableArrayRef theArray, CFRange range,
  CFComparatorFunction comparator, void *context);

CF_EXTERN_C_END

#endif /* __COREFOUNDATION_CFARRAY_H__ */
