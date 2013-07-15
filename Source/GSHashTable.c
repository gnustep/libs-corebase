/* GSHashTable.c
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "GSHashTable.h"
#include "GSPrivate.h"

#include <string.h>

/* READ THIS FIRST
 * 
 * GSHashTable uses an open-address scheme with double hashing for
 * collision resolution.  This method has a few drawbacks, such as much
 * higher memory usage on the worst case, but it is very simple to
 * implement.  This is because, as explained below, we try to keep the
 * load factor less than 80%, so at least 20% of the memory allocated is
 * not used.
 * 
 * One thing that needs to be kept in mind is table load, we
 * don't want it to go over 80%.  The collisions start to get out of
 * hand at that point.  Collisions might also be a problem if the hash
 * algorithm isn't very good.  Some of this is mitigated by using
 * prime numbers as the hash table size.  There are other open addressing
 * algorithms out there that can handle high load factors more gracefully
 * than double hashing, but they're also more complicated, like
 * Cuckoo hashing or Hopscotch hasing.
 * 
 * The implementation originally used an equation to calculate the next
 * array size.  This became a problem because we were calculating the
 * table size each time we add or removed an item.  Now a lookup table
 * is used, instead.
 * 
 * To be as easy as possible on system memory, we rehash not only when
 * the table load factor gets above 80% but also when it gets below 25%.
 * This is optimum, but we also want to keep memory usage at a manageable
 * level.  Rehashing to a lower level is only done when removing an object,
 * though.  So we can start with a really large table (high capacity) and
 * never shrink if we don't remove anything.
 */

static const GSHashTableKeyCallBacks _kGSNullHashTableKeyCallBacks = {
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static const GSHashTableValueCallBacks _kGSNullHashTableValueCallBacks = {
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

enum
{
  _kGSHashTableMutable = (1 << 0),
  _kGSHashTableShouldCount = (1 << 1)
};

CF_INLINE Boolean
GSHashTableIsMutable (GSHashTableRef table)
{
  return ((CFRuntimeBase *) table)->_flags.info & _kGSHashTableMutable ?
    true : false;
}

CF_INLINE Boolean
GSHashTableShouldCount (GSHashTableRef table)
{
  return ((CFRuntimeBase *) table)->_flags.info & _kGSHashTableShouldCount ?
    true : false;
}

CF_INLINE void
GSHashTableSetMutable (GSHashTableRef table)
{
  ((CFRuntimeBase *) table)->_flags.info |= _kGSHashTableMutable;
}

CF_INLINE void
GSHashTableSetShouldCount (GSHashTableRef table)
{
  ((CFRuntimeBase *) table)->_flags.info |= _kGSHashTableShouldCount;
}



CF_INLINE void
GSHashTableAddKeyValuePair (GSHashTableRef table,
                            GSHashTableBucket * bucket, const void *key,
                            const void *value)
{
  GSHashTableRetainCallBack keyRetain = table->_keyCallBacks.retain;
  GSHashTableRetainCallBack valueRetain = table->_valueCallBacks.retain;

  bucket->count++;
  bucket->key = keyRetain ? keyRetain (table->_allocator, key) : key;
  bucket->value = valueRetain ? valueRetain (table->_allocator, value) : value;
}

CF_INLINE void
GSHashTableReplaceKeyValuePair (GSHashTableRef table,
                                GSHashTableBucket * bucket, const void *key,
                                const void *value)
{
  GSHashTableReleaseCallBack release = table->_valueCallBacks.release;
  GSHashTableRetainCallBack retain = table->_valueCallBacks.retain;

  if (release)
    release (table->_allocator, bucket->value);
  bucket->value = retain ? retain (table->_allocator, value) : value;
}

CF_INLINE void
GSHashTableRemoveKeyValuePair (GSHashTableRef table, GSHashTableBucket * bucket)
{
  GSHashTableReleaseCallBack keyRelease = table->_keyCallBacks.release;
  GSHashTableReleaseCallBack valueRelease = table->_valueCallBacks.release;

  if (keyRelease)
    keyRelease (table->_allocator, bucket->key);
  if (valueRelease)
    valueRelease (table->_allocator, bucket->value);

  bucket->count = 0;
  bucket->key = NULL;
  bucket->value = NULL;
}

CF_INLINE CFHashCode
GSHash2 (CFHashCode value)
{
/* Knuth's hash function.  We add the 1 to make sure the answer is never
 * zero.
 */
#if defined(__LP64__) || defined(_WIN64)
/* 64-bit operating systems */
  return (CFHashCode) (((UInt64) value + 1) * 2654435761UL);
#else
/* 32-bit operating systems */
  return (CFHashCode) (((UInt32) value + 1) * 2654435761UL);
#endif
}

static GSHashTableBucket *
GSHashTableFindBucket (GSHashTableRef table, const void *key)
{
  GSHashTableBucket *buckets;
  CFIndex capacity;
  CFIndex idx;
  CFHashCode hash;
  Boolean matched;
  GSHashTableHashCallBack fHash = table->_keyCallBacks.hash;
  GSHashTableEqualCallBack fEqual = table->_keyCallBacks.equal;

  buckets = table->_buckets;
  capacity = table->_capacity;
  hash = fHash ? fHash (key) : GSHashPointer (key);
  idx = hash % capacity;
  matched = buckets[idx].key == NULL || (fEqual ?
                                         fEqual (key,
                                                 buckets[idx].key) : key ==
                                         buckets[idx].key);

  if (!matched)
    {
      CFHashCode hash2 = GSHash2 (hash);

      if (fEqual)
        {
          do
            {
              hash += hash2;
              idx = hash % capacity;
            }
          while (buckets[idx].key != NULL && !fEqual (key, buckets[idx].key));
        }
      else
        {
          do
            {
              hash += hash2;
              idx = hash % capacity;
            }
          while (buckets[idx].key != NULL && key != buckets[idx].key);
        }
    }

  return &buckets[idx];
}



/* Go as close to INT_MAX as we can. */
static CFIndex _kGSHashTableSizes[] = {
  7, 13, 29, 59, 127, 257, 521, 1049, 2099, 4201,
  8419, 16843, 33703, 67409, 134837, 269683, 539389,
  1078787, 2157587, 4315183, 8630387, 17260781, 34521589,
  69043189, 138086407, 276172823, 552345671, 1104691373
};

static CFIndex _kGSHashTableSizesCount =
  sizeof (_kGSHashTableSizes) / sizeof (CFIndex);

/* Calculted 80% of values above. */
static CFIndex _kGSHashTableFilled[] = {
  5, 10, 23, 47, 101, 205, 416, 839, 2099, 3360,
  6735, 13474, 26962, 53927, 107869, 215746, 431511,
  863029, 1726069, 3452146, 6904309, 13808624, 27617271,
  55234551, 110469125, 220938258, 441876536, 883753098
};

CF_INLINE CFIndex
GSHashTableGetSize (CFIndex min)
{
  CFIndex idx = 0;
  while (min > _kGSHashTableFilled[idx] && idx < _kGSHashTableSizesCount)
    ++idx;
  return _kGSHashTableSizes[idx];
}

#define GSHASHTABLE_EXTRA (sizeof(struct GSHashTable) - sizeof(CFRuntimeBase))
#define GET_ARRAY_SIZE(s) ((s) * sizeof(GSHashTableBucket))

GSHashTableRef
GSHashTableCreate (CFAllocatorRef alloc, CFTypeID typeID,
                   const void **keys, const void **values, CFIndex numValues,
                   const GSHashTableKeyCallBacks * keyCallBacks,
                   const GSHashTableValueCallBacks * valueCallBacks)
{
  CFIndex arraySize;
  CFIndex capacity;
  GSHashTableRef new;

  capacity = GSHashTableGetSize (numValues);
  arraySize = GET_ARRAY_SIZE (capacity);

  new = (GSHashTableRef) _CFRuntimeCreateInstance (alloc, typeID,
                                                   GSHASHTABLE_EXTRA +
                                                   arraySize, NULL);
  if (new)
    {
      CFIndex idx;
      GSHashTableBucket *bucket;

      new->_allocator = alloc;
      new->_buckets = (GSHashTableBucket *) & (new[1]);

      new->_capacity = capacity;

      if (keyCallBacks == NULL)
        keyCallBacks = &_kGSNullHashTableKeyCallBacks;
      if (valueCallBacks == NULL)
        valueCallBacks = &_kGSNullHashTableValueCallBacks;

      memcpy (&new->_keyCallBacks, keyCallBacks,
              sizeof (GSHashTableKeyCallBacks));
      memcpy (&new->_valueCallBacks, valueCallBacks,
              sizeof (GSHashTableValueCallBacks));

      if (keys != NULL)
        {
          for (idx = 0; idx < numValues; ++idx)
            {
              bucket = GSHashTableFindBucket (new, keys[idx]);
              GSHashTableAddKeyValuePair (new, bucket, keys[idx], values[idx]);
              new->_count += 1;
            }
        }
    }

  return new;
}

GSHashTableRef
GSHashTableCreateCopy (CFAllocatorRef alloc, GSHashTableRef table)
{
  CFIndex count;
  GSHashTableRef new;

  count = GSHashTableGetCount (table);
  new = GSHashTableCreate (alloc, CFGetTypeID (table), NULL, NULL,
                           count, &table->_keyCallBacks,
                           &table->_valueCallBacks);
  if (new)
    {
      CFIndex idx;
      GSHashTableBucket *bucket;
      GSHashTableBucket *buckets = table->_buckets;
      for (idx = 0; idx < table->_capacity; ++idx)
        {
          if (buckets[idx].key)
            {
              bucket = GSHashTableFindBucket (new, buckets[idx].key);
              GSHashTableAddKeyValuePair (new, bucket, buckets[idx].key,
                                          buckets[idx].value);
              new->_count += 1;
            }
        }
    }

  return new;
}

void
GSHashTableFinalize (GSHashTableRef table)
{
  GSHashTableRemoveAll (table);
  if (GSHashTableIsMutable (table))
    CFAllocatorDeallocate (table->_allocator, table->_buckets);
}

Boolean
GSHashTableEqual (GSHashTableRef table1, GSHashTableRef table2)
{
  if (table1->_count == table2->_count)
    {
      GSHashTableBucket *current;
      GSHashTableBucket *end;
      GSHashTableBucket *other;
      GSHashTableEqualCallBack keyEqual;
      GSHashTableEqualCallBack valueEqual;

      current = table1->_buckets;
      end = current + table1->_capacity;
      keyEqual = table1->_keyCallBacks.equal;
      valueEqual = table1->_valueCallBacks.equal;
      while (current < end)
        {
          if (current->count > 0)
            {
              other = GSHashTableFindBucket (table2, current->key);
              if (current->count != other->count
                  || keyEqual ? !keyEqual (current->key, other->key) :
                  current->key != other->key
                  || valueEqual ? !valueEqual (current->value, other->value) :
                  current->value != other->value)
                return false;
            }
          ++current;
        }

      return true;
    }

  return false;
}

CFHashCode
GSHashTableHash (GSHashTableRef table)
{
  return table->_count;
}

Boolean
GSHashTableContainsKey (GSHashTableRef table, const void *key)
{
  GSHashTableBucket *bucket;
  bucket = GSHashTableFindBucket (table, key);

  return bucket->count > 0 ? true : false;
}

Boolean
GSHashTableContainsValue (GSHashTableRef table, const void *value)
{
  CFIndex idx;
  GSHashTableBucket *buckets = table->_buckets;
  GSHashTableEqualCallBack equal = table->_valueCallBacks.equal;

  for (idx = 0; idx < table->_capacity; ++idx)
    {
      if (buckets[idx].key)
        {
          if (equal ? equal (value, buckets[idx].value) :
              value == buckets[idx].value)
            return true;
        }
    }
  return false;
}

CFIndex
GSHashTableGetCount (GSHashTableRef table)
{
  return table->_count;
}

CFIndex
GSHashTableGetCountOfKey (GSHashTableRef table, const void *key)
{
  GSHashTableBucket *bucket = GSHashTableFindBucket (table, key);
  return bucket->count;;
}

CFIndex
GSHashTableGetCountOfValue (GSHashTableRef table, const void *value)
{
  CFIndex idx;
  CFIndex count = 0;
  GSHashTableBucket *buckets = table->_buckets;
  GSHashTableEqualCallBack equal = table->_valueCallBacks.equal;

  for (idx = 0; idx < table->_capacity; ++idx)
    {
      if (buckets[idx].key)
        {
          if (equal ? equal (value, buckets[idx].value) :
              value == buckets[idx].value)
            count += buckets[idx].count;
        }
    }
  return count;
}

void
GSHashTableGetKeysAndValues (GSHashTableRef table, const void **keys,
                             const void **values)
{
  CFIndex idx;
  CFIndex j = 0;
  GSHashTableBucket *buckets = table->_buckets;

  for (idx = 0; idx < table->_capacity; ++idx)
    {
      if (buckets[idx].count > 0)
        {
          if (keys)
            keys[j] = buckets[idx].key;
          if (values)
            values[j] = buckets[idx].value;
          ++j;
        }
    }
}

const void *
GSHashTableGetValue (GSHashTableRef table, const void *key)
{
  GSHashTableBucket *bucket;
  bucket = GSHashTableFindBucket (table, key);

  return bucket->value;
}



static void
GSHashTableRehash (GSHashTableRef table, CFIndex newCapacity)
{
  CFIndex idx;
  CFIndex oldSize;
  GSHashTableBucket *bucket;
  GSHashTableBucket *oldBuckets;

  oldSize = table->_capacity;
  oldBuckets = table->_buckets;

  table->_capacity = newCapacity;
  table->_buckets = CFAllocatorAllocate (table->_allocator,
                                         GET_ARRAY_SIZE (newCapacity), 0);
  memset (table->_buckets, 0, GET_ARRAY_SIZE (newCapacity));

  for (idx = 0; idx < oldSize; ++idx)
    {
      if (oldBuckets[idx].key)
        {
          bucket = GSHashTableFindBucket (table, oldBuckets[idx].key);
          GSHashTableAddKeyValuePair (table, bucket, oldBuckets[idx].key,
                                      oldBuckets[idx].value);
        }
    }

  CFAllocatorDeallocate (table->_allocator, oldBuckets);
}

CF_INLINE void
GSHashTableGrowIfNeeded (GSHashTableRef table)
{
  CFIndex newSize;
  if ((newSize = GSHashTableGetSize (table->_count + 1)) > table->_capacity)
    GSHashTableRehash (table, newSize);
}

CF_INLINE void
GSHashTableShrinkIfNeeded (GSHashTableRef table)
{
  /* Shrink if count is less than half of capacity) */
  if (table->_count < (table->_capacity >> 2))
    GSHashTableRehash (table, GSHashTableGetSize (table->_count));
}

GSHashTableRef
GSHashTableCreateMutable (CFAllocatorRef allocator,
                          CFTypeID typeID, CFIndex capacity,
                          const GSHashTableKeyCallBacks * keyCallBacks,
                          const GSHashTableValueCallBacks * valueCallBacks)
{
  GSHashTableRef new;

  new = (GSHashTableRef) _CFRuntimeCreateInstance (allocator, typeID,
                                                   GSHASHTABLE_EXTRA, NULL);
  if (new)
    {
      CFIndex arraySize;

      capacity = GSHashTableGetSize (capacity);
      arraySize = GET_ARRAY_SIZE (capacity);

      new->_allocator = allocator;
      new->_buckets = CFAllocatorAllocate (allocator, arraySize, 0);
      memset (new->_buckets, 0, arraySize);

      new->_capacity = capacity;

      if (keyCallBacks == NULL)
        keyCallBacks = &_kGSNullHashTableKeyCallBacks;
      if (valueCallBacks == NULL)
        valueCallBacks = &_kGSNullHashTableValueCallBacks;

      memcpy (&new->_keyCallBacks, keyCallBacks,
              sizeof (GSHashTableKeyCallBacks));
      memcpy (&new->_valueCallBacks, valueCallBacks,
              sizeof (GSHashTableValueCallBacks));

      GSHashTableSetMutable (new);
    }

  return new;
}

GSHashTableRef
GSHashTableCreateMutableCopy (CFAllocatorRef alloc, GSHashTableRef table,
                              CFIndex capacity)
{
  GSHashTableRef new;

  if (capacity < table->_count)
    capacity = table->_count;
  new = GSHashTableCreateMutable (alloc, CFGetTypeID (table), capacity,
                                  &table->_keyCallBacks,
                                  &table->_valueCallBacks);
  if (new)
    {
      CFIndex idx;
      GSHashTableBucket *bucket;
      GSHashTableBucket *buckets = table->_buckets;
      for (idx = 0; idx < table->_capacity; ++idx)
        {
          if (buckets[idx].key)
            {
              bucket = GSHashTableFindBucket (new, buckets[idx].key);
              GSHashTableAddKeyValuePair (new, bucket, buckets[idx].key,
                                          buckets[idx].value);
              new->_count += 1;
            }
        }
    }

  return new;
}

void
GSHashTableAddValue (GSHashTableRef table, const void *key, const void *value)
{
  GSHashTableBucket *bucket;

  GSHashTableGrowIfNeeded (table);

  bucket = GSHashTableFindBucket (table, key);
  if (bucket->count == 0)
    {
      GSHashTableAddKeyValuePair (table, bucket, key, value);
      table->_count += 1;
    }
}

void
GSHashTableReplaceValue (GSHashTableRef table, const void *key,
                         const void *value)
{
  GSHashTableBucket *bucket;

  bucket = GSHashTableFindBucket (table, key);
  if (bucket->count > 0)
    GSHashTableReplaceKeyValuePair (table, bucket, key, value);
}

void
GSHashTableSetValue (GSHashTableRef table, const void *key, const void *value)
{
  GSHashTableBucket *bucket;

  GSHashTableGrowIfNeeded (table);

  bucket = GSHashTableFindBucket (table, key);
  if (bucket->count > 0)
    {
      GSHashTableReplaceKeyValuePair (table, bucket, key, value);
    }
  else
    {
      GSHashTableAddKeyValuePair (table, bucket, key, value);
      table->_count += 1;
    }
}

void
GSHashTableRemoveAll (GSHashTableRef table)
{
  CFIndex idx;
  GSHashTableBucket *buckets = table->_buckets;

  for (idx = 0; idx < table->_capacity; ++idx)
    {
      if (buckets[idx].count > 0)
        GSHashTableRemoveKeyValuePair (table, &buckets[idx]);
    }
  table->_count = 0;
}

void
GSHashTableRemoveValue (GSHashTableRef table, const void *key)
{
  GSHashTableBucket *bucket;

  GSHashTableShrinkIfNeeded (table);

  bucket = GSHashTableFindBucket (table, key);
  if (bucket->count > 1)
    {
      bucket->count -= 1;
    }
  else if (bucket->count == 1)
    {
      GSHashTableRemoveKeyValuePair (table, bucket);
      table->_count -= 1;
    }
}
