/* GSFunctions.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: April, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include "GSPrivate.h"

CFIndex
GSBSearch (const void *array, const void *key, CFRange range, CFIndex size,
  CFComparatorFunction comp, void *ctxt)
{
  CFIndex min;
  CFIndex mid;
  CFIndex max;
  const void *cur;
  
  min = range.location;
  max = min + range.length;
  
  while (max > min)
    {
      CFComparisonResult r;
      
      mid = (min + max) >> 1;
      cur = ((const UInt8*)array) + (mid * size);
      r = comp (key, cur, ctxt);
      if (r == kCFCompareLessThan)
        {
          max = mid - 1;
        }
      else if (r == kCFCompareGreaterThan)
        {
          min = mid + 1;
        }
      else
        {
          min = mid + 1;
          break;
        }
    }
  
  return min - 1;
}

