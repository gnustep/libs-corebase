/* GSHashTable.h
   
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

#include "CoreFoundation/CFBase.h"

/* GSHashTable is used internally by CoreBase to implement CFDictionary,
 * CFSet and CFBag.  The CF-types are reponsible for allocating memory and
 * managing the hash table.
 * 
 * The table uses open addressing + double hashing to resolve collisions.
 */

struct GSHashTable
{
  CFIndex       count;
  CFIndex       size;
  const void  **array;
};

/* This number is recommended by Thomas Wang if the expansion formula is
 * 2*n + 55 (http://www.cris.com/~ttwang/tech/mprime.htm).
 */
#define _kGSHashTableDefaultSize 23

CF_INLINE CFIndex
GSHashTableNextSize (CFIndex num)
{
  num += num + 55;
  return num;
}

CF_INLINE Boolean
GSHashTableIsAppropriateSize (CFIndex num, CFIndex max)
{
  return (((3 * num) / 4) >= max);
}

/* Returns the index of a slot matching value.  If value is not found,
 * returns the index of an empty slot.
 * Note: Table must not be full.
 * Warning: Do not try to find a NULL value.
 */
CFIndex
GSHashTableFind (struct GSHashTable *ht, const void *value,
  CFHashCode (*fHash)(const void *),
  Boolean (*fEqual)(const void*, const void*));