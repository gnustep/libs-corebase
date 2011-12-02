/* GSHashTable.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include <CoreFoundation/CFBase.h>
#include "GSHashTable.h"

/* Both of these function are taken from Thomas Wang's website
 * (http://www.concentric.net/~Ttwang/tech/inthash.htm).  As far as I know,
 * these algorithm are in the public domain.
 */
CF_INLINE CFHashCode
GSHashInt64 (UInt64 value)
{
  register CFHashCode hash = (CFHashCode)value;
  hash = (~hash) + (hash << 21);
  hash = hash ^ (hash >> 24);
  hash = (hash + (hash << 3)) + (hash << 8);
  hash = hash ^ (hash >> 14);
  hash = (hash + (hash << 2)) + (hash << 4);
  hash = hash ^ (hash >> 28);
  hash = hash + (hash << 31);
  return hash;
}

CF_INLINE CFHashCode
GSHashInt32 (UInt32 value)
{
  register CFHashCode hash = (CFHashCode)value;
  hash = ~hash + (hash << 15);
  hash = hash ^ (hash >> 12);
  hash = hash + (hash << 2);
  hash = hash ^ (hash >> 4);
  hash = (hash + (hash << 3)) + (hash << 11);
  hash = hash ^ (hash >> 16);
  return hash;
}

CF_INLINE CFHashCode
GSHashTableHash (const void *value)
{
#if defined(__LP64__) || defined(_WIN64)
/* 64-bit operating systems */
  return GSHashInt64 ((Uint64)value);
#else
/* 32-bit operating systems */
  return GSHashInt32 ((UInt32)value);
#endif
}

CF_INLINE CFHashCode
GSHashTableHash2 (const void *value)
{
/* Knuth's hash function.  We add the 1 to make sure the answer is never
 * zero.
 */
#if defined(__LP64__) || defined(_WIN64)
/* 64-bit operating systems */
  return (CFHashCode)((((UInt64)value >> 3) + 1) * 2654435761UL);
#else
/* 32-bit operating systems */
  return (CFHashCode)((((UInt32)value >> 3) + 1) * 2654435761UL);
#endif
}

CFIndex
GSHashTableFind (struct GSHashTable *ht, const void *value,
  CFHashCode (*fHash)(const void *),
  Boolean (*fEqual)(const void*, const void*))
{
  const void **array;
  CFIndex size;
  CFIndex idx;
  CFHashCode hash;
  Boolean matched;
  
  array = ht->array;
  size = ht->size;
  hash = fHash ? fHash (value) : GSHashTableHash (value);
  idx = hash % size;
  matched = array[idx] == NULL || (fEqual ?
    fEqual (value, array[idx]) : value == array[idx]);
  
  if (!matched)
    {
      CFHashCode hash2 = GSHashTableHash2 (value);
      
      if (fEqual)
        {
          do
            {
              hash += hash2;
              idx = hash % size;
            } while (array[idx] != NULL && !fEqual (value, array[idx]));
        }
      else
        {
          do
            {
              hash += hash2;
              idx = hash % size;
            } while (array[idx] != NULL && value != array[idx]);
        }
    }
  
  return idx;
}

Boolean
CFHashTableNext (struct GSHashTable *ht, CFIndex *index)
{
  register const void **array;
  register CFIndex idx;
  register CFIndex size;
  
  for (array = ht->array, idx = *index, size = ht->size ; idx < size ; ++idx)
    {
      if (array[idx])
        {
          *index = idx;
          return true;
        }
    }
  
  return false;
}
