/* NSCFArray.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#import <Foundation/NSArray.h>

#include "NSCFType.h"
#include "CoreFoundation/CFArray.h"

@interface NSCFArray : NSMutableArray
@end

@implementation NSCFArray
+ (void) load
{
  CFRuntimeBridgeClass (CFArrayGetTypeID(), "NSCFArray");
}

- (NSUInteger) count
{
  return (NSUInteger)CFArrayGetCount (self);
}

- (id) objectAtIndex: (NSUInteger) index
{
  return (id)CFArrayGetValueAtIndex (self, (CFIndex)index);
}

-(void) addObject: (id) anObject
{
  CFArrayAppendValue (self, (const void*)anObject);
}

- (void) replaceObjectAtIndex: (NSUInteger) index withObject: (id) anObject
{
  CFArraySetValueAtIndex (self, (CFIndex)index, (const void*)anObject);
}

- (void) insertObject: (id) anObject atIndex: (NSUInteger) index
{
  CFArrayInsertValueAtIndex (self, (CFIndex)index, (const void*)anObject);
}

- (void) removeObjectAtIndex: (NSUInteger) index
{
  CFArrayRemoveValueAtIndex (self, (CFIndex)index);
}
@end