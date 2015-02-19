/* NSCFArray.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: November, 2011
   
   This file is part of GNUstep CoreBase Library.
   
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

#import <Foundation/NSArray.h>
#include <stdarg.h>

#include "NSCFType.h"
#include "CoreFoundation/CFArray.h"

@interface NSCFArray : NSMutableArray
NSCFTYPE_VARS
@end

@interface NSArray (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSCFArray
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithObjects:(id)firstObj, ...
{
  RELEASE(self);

  if (firstObj == nil)
    {
      return (NSCFArray*) CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
    }

  /* copy all arguments into a temporary mutable array */
  CFMutableArrayRef temp = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
  va_list ap;
  id obj;

  va_start(ap, firstObj);

  CFArrayAppendValue(temp, firstObj);

  while ((obj = va_arg(ap, id)) != nil)
	CFArrayAppendValue(temp, obj);

  va_end(ap);

  self = (NSCFArray*) CFArrayCreateCopy(NULL, temp);
  RELEASE((id)temp);

  return self;
}

- (NSUInteger) count
{
  return (NSUInteger)CFArrayGetCount ((CFArrayRef)self);
}

- (id) objectAtIndex: (NSUInteger) index
{
  return (id)CFArrayGetValueAtIndex ((CFArrayRef)self, (CFIndex)index);
}

-(void) addObject: (id) anObject
{
  CFArrayAppendValue ((CFMutableArrayRef)self, (const void*)anObject);
}

- (void) replaceObjectAtIndex: (NSUInteger) index withObject: (id) anObject
{
  CFArraySetValueAtIndex ((CFMutableArrayRef)self, (CFIndex)index,
                          (const void*)anObject);
}

- (void) insertObject: (id) anObject atIndex: (NSUInteger) index
{
  CFArrayInsertValueAtIndex ((CFMutableArrayRef)self, (CFIndex)index,
                             (const void*)anObject);
}

- (void) removeObjectAtIndex: (NSUInteger) index
{
  CFArrayRemoveValueAtIndex ((CFMutableArrayRef)self, (CFIndex)index);
}
@end

@implementation NSArray (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFArrayGetTypeID();
}
@end

