/* GSCArray.h
   
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

#ifndef __GSCARRAY_H__
#define __GSCARRAY_H__

#include "config.h"

#include "CoreFoundation/CFBase.h"
#include "GSPrivate.h"

GS_PRIVATE void
GSCArrayQuickSort (const void **array, CFIndex length,
                   CFComparatorFunction comparator, void *context);

GS_PRIVATE void
GSCArrayInsertionSort (const void **array, CFIndex length,
                       CFComparatorFunction comparator, void *context);

GS_PRIVATE void
GSCArrayHeapify (const void **array, CFIndex length,
                 CFComparatorFunction comparator, void *context);

GS_PRIVATE CFIndex
GSCArrayBSearch (const void **array, const void *key, CFIndex length,
                 CFComparatorFunction comparator, void *context);

#endif /* __GSCARRAY_H__ */
