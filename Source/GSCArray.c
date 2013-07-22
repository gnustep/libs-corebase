/* GSCArray.c
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2013
   
   This file is part of GNUstep CoreBase library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#include "GSCArray.h"

#define GS_EXCHANGE_VALUES(_v1, _v2) do \
{ \
  const void *_tmp_; \
  _tmp_ = _v1; \
  _v1 = _v2; \
  _v2 = _tmp_; \
} while (0)

void
GSCArrayQuickSort (const void **array, CFIndex length,
                   CFComparatorFunction comparator, void *context)
{
  if (length > 16)              /* 16 is an arbritrary "small" number */
    {
      const void *value;
      CFIndex pivot;
      CFIndex stored;
      CFIndex idx;

      pivot = length / 2;
      value = array[pivot];
      GS_EXCHANGE_VALUES (array[pivot], array[length - 1]);
      for (idx = 0, stored = 0; idx < length; ++idx)
        {
          if ((*comparator) (array[idx], value, context) == kCFCompareLessThan)
            {
              GS_EXCHANGE_VALUES (array[idx], array[stored]);
              ++stored;
            }
        }
      GS_EXCHANGE_VALUES (array[stored], array[length - 1]);

      GSCArrayQuickSort (array, stored - 1, comparator, context);
      GSCArrayQuickSort (array + stored + 1, length - stored, comparator,
                         context);
    }
  GSCArrayInsertionSort (array, length, comparator, context);
}

void
GSCArrayInsertionSort (const void **array, CFIndex length,
                       CFComparatorFunction comparator, void *context)
{
  CFIndex idx;

  for (idx = 1; idx < length; ++idx)
    {
      int hole;
      const void *value;

      hole = idx;
      value = array[hole];
      while (hole > 0
             && (*comparator) (value, array[hole - 1],
                               context) == kCFCompareLessThan)
        {
          array[hole] = array[hole - 1];
          --hole;
        }
      array[hole] = value;
    }
}

CFIndex
GSCArrayBSearch (const void **array, const void *key, CFIndex length,
                 CFComparatorFunction comparator, void *context)
{
  CFIndex min;
  CFIndex mid;
  CFIndex max;

  min = 0;
  max = length;
  while (min < max)
    {
      CFComparisonResult r;

      mid = (min + max) / 2;
      r = (*comparator) (key, array[mid], context);
      if (r == kCFCompareLessThan)
        {
          max = mid - 1;
          break;
        }
      else if (r == kCFCompareGreaterThan)
        {
          max = mid - 1;
        }
      else
        {
          min = mid + 1;
        }
    }

  return max + 1;
}

void
GSCArrayHeapify (const void **array, CFIndex length,
                 CFComparatorFunction comparator, void *context)
{

}
