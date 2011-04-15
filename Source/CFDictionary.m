/* CFDictionary.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
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

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFDictionary.h"



const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};
const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};
const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks =
{
  0,
  NULL,
  NULL,
  NULL,
  NULL
};

//
// CFDictionary
//
void
CFDictionaryApplyFunction (CFDictionaryRef theDict,
                           CFDictionaryApplierFunction applier, void *context)
{
  // FIXME
}

Boolean
CFDictionaryContainsKey (CFDictionaryRef theDict, const void *key)
{
  return ([(NSDictionary*)theDict objectForKey: (id)key] != nil);
}

Boolean
CFDictionaryContainsValue (CFDictionaryRef theDict, const void *value)
{
  return ([(NSDictionary*)theDict allKeysForObject: (id)value] != nil);
}

CFDictionaryRef
CFDictionaryCreate (CFAllocatorRef allocator, const void **keys,
                    const void **values, CFIndex numValues,
                    const CFDictionaryKeyCallBacks *keyCallBacks,
                    const CFDictionaryValueCallBacks *valueCallBacks)
{
  /* FIXME: will this even work if keys and values aren't objects? */
  return (CFDictionaryRef)[[NSDictionary allocWithZone: allocator] initWithObjects: (id *)keys
                                                          forKeys: (id *)values
                                                            count: numValues];
}

CFDictionaryRef
CFDictionaryCreateCopy (CFAllocatorRef allocator, CFDictionaryRef theDict)
{
  return (CFDictionaryRef)[(NSDictionary*)theDict copyWithZone: allocator];
}

CFIndex
CFDictionaryGetCount (CFDictionaryRef theDict)
{
  return [(NSDictionary*)theDict count];
}

CFIndex
CFDictionaryGetCountOfKey (CFDictionaryRef theDict, const void *key)
{
  if (CFDictionaryGetValueIfPresent (theDict, key, NULL))
    return 1;
  else
    return 0;
}

CFIndex
CFDictionaryGetCountOfValue (CFDictionaryRef theDict, const void *value)
{
  return [[(NSDictionary*)theDict allKeysForObject: (id)value] count];
}

void
CFDictionaryGetKeysAndValues (CFDictionaryRef theDict, const void **keys,
                              const void **values)
{
  if (keys != NULL)
    {
      NSArray *allKeys;

      allKeys = [(NSDictionary*)theDict allKeys];
      [allKeys getObjects: (id*)keys];
    }

  if (values != NULL)
    {
      NSArray *allValues;

      allValues = [(NSDictionary*)theDict allValues];
      [allValues getObjects: (id*)values];
    }
}

CFTypeID
CFDictionaryGetTypeID (void)
{
  return (CFTypeID)[NSDictionary class];
}

const void *
CFDictionaryGetValue (CFDictionaryRef theDict, const void *key)
{
  return (void *)[(NSDictionary*)theDict objectForKey: (id)key];
}

Boolean
CFDictionaryGetValueIfPresent (CFDictionaryRef theDict,
  const void *key, const void **value)
{
  // FIXME
  void *val = [(NSDictionary*)theDict objectForKey: (id)key];

  if (value != NULL)
    *value = val;
  return (val != nil);
}



//
// CFMutableDictionary
//
void
CFDictionaryAddValue (CFMutableDictionaryRef theDict, const void *key,
                      const void *value)
{
  if (!CFDictionaryGetValueIfPresent((CFDictionaryRef)theDict, key, NULL))
    CFDictionarySetValue(theDict, key, value);
}

CFMutableDictionaryRef
CFDictionaryCreateMutable (CFAllocatorRef allocator, CFIndex capacity,
                           const CFDictionaryKeyCallBacks *keyCallBacks,
                           const CFDictionaryValueCallBacks *valueCallBacks)
{
  // FIXME
  return (CFMutableDictionaryRef)[[NSMutableDictionary allocWithZone: allocator]
    initWithCapacity: capacity];
}

CFMutableDictionaryRef
CFDictionaryCreateMutableCopy (CFAllocatorRef allocator, CFIndex capacity,
                               CFDictionaryRef theDict)
{
  // FIXME
  return (CFMutableDictionaryRef)[(NSMutableDictionary*)theDict mutableCopyWithZone: allocator];
}

void
CFDictionaryRemoveAllValues (CFMutableDictionaryRef theDict)
{
  [(NSMutableDictionary*)theDict removeAllObjects];
}

void
CFDictionaryRemoveValue (CFMutableDictionaryRef theDict, const void *key)
{
  [(NSMutableDictionary*)theDict removeObjectForKey: (id)key];
}

void
CFDictionaryReplaceValue (CFMutableDictionaryRef theDict, const void *key,
                          const void *value)
{
  if (CFDictionaryGetValueIfPresent((CFDictionaryRef)theDict, key, NULL))
    CFDictionarySetValue(theDict, key, value);
}

void
CFDictionarySetValue (CFMutableDictionaryRef theDict, const void *key,
                      const void *value)
{
  [(NSMutableDictionary*)theDict setObject: (id)value forKey: (id)key];
}
