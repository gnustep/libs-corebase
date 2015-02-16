/* NSCFDictionary.m
   
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

#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>

#include "NSCFType.h"
#include "CoreFoundation/CFDictionary.h"

@interface NSCFDictionary : NSMutableDictionary
NSCFTYPE_VARS
@end

@interface NSDictionary (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;

- (CFIndex) _cfCountOfValue: (id)value;
@end

@interface NSMutableDictionary (CoreBaseAdditions)
- (void) _cfSetValue: (id)key
                    : (id)value;

- (void) _cfReplaceValue: (id)key
                        : (id)value;
@end

@implementation NSCFDictionary
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (id) initWithObjects: (const id[])objects
               forKeys: (const id<NSCopying>[])keys
                 count: (NSUInteger)count
{
  RELEASE(self);
  
  self = (NSCFDictionary*) CFDictionaryCreate(kCFAllocatorDefault,
    (const void **) keys, (const void **) objects, count,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  return self;
}

- (id) initWithCapacity: (NSUInteger)numItems
{
  RELEASE(self);
  
  self = (NSCFDictionary*) CFDictionaryCreateMutable(kCFAllocatorDefault,
    numItems, &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);
  
  return self;
}

- (NSUInteger) count
{
  return CFDictionaryGetCount((CFDictionaryRef) self);
}

- (NSEnumerator*) keyEnumerator
{
  CFIndex count;
  const void **keys;
  NSArray *array;
  
  count = CFDictionaryGetCount((CFDictionaryRef) self);
  keys = (const void**) malloc(sizeof(void*) * count);
  
  CFDictionaryGetKeysAndValues((CFDictionaryRef) self,
    keys, NULL);
  
  array = [NSArray arrayWithObjects: (const id*)keys
                              count: count];

  free((void*)keys);
  return [array objectEnumerator];
}

- (id) objectForKey: (id)aKey
{
  return (id) CFDictionaryGetValue((CFDictionaryRef) self, aKey);
}

- (NSEnumerator*) objectEnumerator
{
  CFIndex count;
  const void **values;
  NSArray *array;
  
  count = CFDictionaryGetCount((CFDictionaryRef) self);
  values = (const void**) malloc(sizeof(void*) * count);
  
  CFDictionaryGetKeysAndValues((CFDictionaryRef) self,
    NULL, values);
  
  array = [NSArray arrayWithObjects: (const id*)values
                              count: count];

  free((void*)values);
  return [array objectEnumerator];

}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (id[])stackbuf
                                     count: (NSUInteger)len
{
  NSEnumerator *enuM = [self keyEnumerator];
  
  return [enuM countByEnumeratingWithState: state
                                   objects: stackbuf
                                     count: len];
}

- (void) setObject: anObject forKey: (id)aKey
{
  CFDictionarySetValue((CFMutableDictionaryRef) self, aKey, anObject);
}

- (void) removeObjectForKey: (id)aKey
{
  CFDictionaryRemoveValue((CFMutableDictionaryRef) self, aKey);
}

- (void) removeAllObjects
{
  CFDictionaryRemoveAllValues((CFMutableDictionaryRef) self);
}
@end

@implementation NSDictionary (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFDictionaryGetTypeID();
}

- (CFIndex) _cfCountOfValue: (id)value
{
  CFIndex countOfValue = 0;
  CFIndex i;
  NSUInteger count;
  NSArray* array;
  
  // TODO: getObjects:andKeys: could be faster (less calls)
  
  array = [self allValues];
  count = [self count];
  
  for (i = 0; i < count; i++)
    {
      if ([[array objectAtIndex: i] isEqual: value])
        countOfValue++;
    }
  
  return countOfValue;
}

@end

@implementation NSMutableDictionary (CoreBaseAdditions)

- (void) _cfSetValue: (id)key
                    : (id)value
{
  [self removeObjectForKey: key];
  [self setObject: value
           forKey: key];
}

- (void) _cfReplaceValue: (id)key
                        : (id)value
{
  if ([self objectForKey: key] != NULL)
    {
      [self removeObjectForKey: key];
      [self setObject: value
               forKey: key];
    }
}

@end

