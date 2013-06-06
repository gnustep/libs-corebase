/* NSCFSet.m
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Lubos Dolezel
   Date: March, 2013
   
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

#import <Foundation/NSSet.h>
#import <Foundation/NSArray.h>

#include "NSCFType.h"
#include "CoreFoundation/CFSet.h"

@interface NSCFSet : NSMutableSet
NSCFTYPE_VARS
@end

@interface NSSet (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;

- (id) _cfGetValue: (id) value;
- (void) _cfGetValues: (id[]) values;
@end

@interface NSMutableSet (CoreBaseAdditions)
- (void) _cfReplaceValue: (id) value;
- (void) _cfSetValue: (id) value;
@end

@implementation NSCFSet
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithObjects: (const id[])objects
                 count: (NSUInteger)count
{
  RELEASE(self);
  
  self = (NSCFSet*) CFSetCreate(NULL, (const void**)objects, count,
    &kCFTypeSetCallBacks);
  
  return self;
}

- (NSUInteger) count
{
  return CFSetGetCount((CFSetRef)self);
}

- (id) member: (id)anObject
{
  id retval;
  
  if (CFSetGetValueIfPresent((CFSetRef)self, anObject,
                             (const void **)&retval))
    {
      return retval;
    }
  else
    {
      return NULL;
    }
}

- (NSEnumerator*) objectEnumerator
{
  NSArray *array;
  const void* values;
  const CFIndex count = CFSetGetCount((CFSetRef)self);
  
  values = malloc(sizeof(void*) * count);
  CFSetGetValues((CFSetRef)self, &values);
  
  array = [NSArray arrayWithObjects: (const id*)values
                              count: count];

  free((void*)values);
  return [array objectEnumerator];
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (id*)stackbuf
                                     count: (NSUInteger)len
{
  // TODO: inefficient
  NSEnumerator *enuM = [self objectEnumerator];
  
  return [enuM countByEnumeratingWithState: state
                                   objects: stackbuf
                                     count: len];
}

- (void) addObject: (id)anObject
{
  CFSetAddValue((CFMutableSetRef) self, anObject);
}

- (void) removeObject: (id)anObject
{
  CFSetRemoveValue((CFMutableSetRef) self, anObject);
}

- (void) removeAllObjects
{
  CFSetRemoveAllValues((CFMutableSetRef) self);
}
@end


@implementation NSSet (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFSetGetTypeID();
}

- (id) _cfGetValue: (id) value
{
  NSEnumerator *enuM = [self objectEnumerator];
  id elem;
  
  while ((elem = [enuM nextObject]))
    {
      if ([elem isEqual: value])
        return elem;
    }
    
  return NULL;
}

- (void) _cfGetValues: (id[]) values
{
  NSArray *array = [self allObjects];
  [array getObjects: values
              range: NSMakeRange(0, [self count])];
}

@end

@implementation NSMutableSet (CoreBaseAdditions)
- (void) _cfReplaceValue: (id) value
{
  if ([self containsObject: value])
    {
      [self removeObject: value];
      [self addObject: value];
    }
}

- (void) _cfSetValue: (id) value
{
  [self removeObject: value];
  [self addObject: value];
}
@end

