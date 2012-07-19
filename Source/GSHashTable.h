/* GSHashTable.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include "CoreFoundation/CFBase.h"

typedef CFStringRef (*GSHashTableCopyDescriptionCallBack)(const void *value);
typedef Boolean  (*GSHashTableEqualCallBack) (const void *value1,
  const void *value2);
typedef CFHashCode    (*GSHashTableHashCallBack) (const void *value);
typedef void (*GSHashTableReleaseCallBack) (CFAllocatorRef allocator,
  const void *value);
typedef const void *(*GSHashTableRetainCallBack) (CFAllocatorRef allocator,
  const void *value);

typedef struct GSHashTableKeyCallBacks GSHashTableKeyCallBacks;
struct GSHashTableKeyCallBacks
{
  CFIndex version;
  GSHashTableRetainCallBack retain;
  GSHashTableReleaseCallBack release;
  GSHashTableRetainCallBack copyDescrition;
  GSHashTableEqualCallBack equal;
  GSHashTableHashCallBack hash;
};

typedef struct GSHashTableValueCallBacks GSHashTableValueCallBacks;
struct GSHashTableValueCallBacks
{
  CFIndex version;
  GSHashTableRetainCallBack retain;
  GSHashTableReleaseCallBack release;
  GSHashTableRetainCallBack copyDescrition;
  GSHashTableEqualCallBack equal;
};

typedef struct GSHashTableBucket GSHashTableBucket;
struct GSHashTableBucket
{
  CFIndex count;
  const void *key;
  const void *value;
};

typedef struct GSHashTable *GSHashTableRef;
struct GSHashTable
{
  CFRuntimeBase  _parent;
  CFAllocatorRef _allocator;
  CFIndex        _capacity;
  CFIndex        _count;
  CFIndex        _total; /* Used for CFBagGetCount() */
  GSHashTableKeyCallBacks _keyCallBacks;
  GSHashTableValueCallBacks _valueCallBacks;
  struct GSHashTableBucket *_buckets;
};

void
GSHashTableFinalize (GSHashTableRef table);

Boolean
GSHashTableEqual (GSHashTableRef table1, GSHashTableRef table2);

CFHashCode
GSHashTableHash (GSHashTableRef table);

GSHashTableRef
GSHashTableCreate (CFAllocatorRef alloc, CFTypeID typeID,
  const void **keys, const void **values, CFIndex count,
  const GSHashTableKeyCallBacks *keyCallBacks,
  const GSHashTableValueCallBacks *valueCallBacks);

GSHashTableRef
GSHashTableCreateCopy (CFAllocatorRef alloc, GSHashTableRef table);

Boolean
GSHashTableContainsKey (GSHashTableRef table, const void *key);

Boolean
GSHashTableContainsValue (GSHashTableRef table, const void *value);

CFIndex
GSHashTableGetCount (GSHashTableRef table);

CFIndex
GSHashTableGetCountOfKey (GSHashTableRef table, const void *key);

CFIndex
GSHashTableGetCountOfValue (GSHashTableRef table, const void *value);

void
GSHashTableGetKeysAndValues (GSHashTableRef table, const void **keys,
  const void **values);

const void *
GSHashTableGetValue (GSHashTableRef table, const void *key);



GSHashTableRef
GSHashTableCreateMutable (CFAllocatorRef allocator,
  CFTypeID typeID, CFIndex capacity,
  const GSHashTableKeyCallBacks *keyCallBacks,
  const GSHashTableValueCallBacks *valueCallBacks);

GSHashTableRef
GSHashTableCreateMutableCopy (CFAllocatorRef alloc, GSHashTableRef table,
  CFIndex capacity);

void
GSHashTableAddValue (GSHashTableRef table, const void *key, const void *value);

void
GSHashTableReplaceValue (GSHashTableRef table, const void *key,
  const void *value);

void
GSHashTableSetValue (GSHashTableRef table, const void *key, const void *value);

void
GSHashTableRemoveAll (GSHashTableRef table);

void
GSHashTableRemoveValue (GSHashTableRef table, const void *key);

