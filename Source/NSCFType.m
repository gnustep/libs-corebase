/* NSCFType.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
   This file is part of GNUstep CoreBase library.
   
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

#include "CoreFoundation/CFRuntime.h"
#import "NSCFType.h"

extern void CFInitialize (void);
extern CFRuntimeClass **__CFRuntimeClassTable;

@implementation NSCFType

+ (void) load
{
  CFInitialize();
}

- (void) dealloc
{
  if (_flags.ro)
    return;
  
  CFRuntimeClass *cfclass = __CFRuntimeClassTable[([self _cfTypeID])];
  if (NULL != cfclass->finalize)
    return cfclass->finalize((CFTypeRef)self);
  
  [super dealloc];
}

- (id) retain
{
  if (_flags.ro)
    return self;
  
  return [super retain];
}

- (void) release
{
  if (_flags.ro)
    return;
  
  return [super release];
}

- (BOOL) isEqual: (id) anObject
{
  CFRuntimeClass *cfclass = __CFRuntimeClassTable[([self _cfTypeID])];
  if (NULL != cfclass->equal)
    return cfclass->equal((CFTypeRef)self, (CFTypeRef)anObject);
  else
    return [super isEqual: anObject];
}

- (CFTypeID) _cfTypeID
{
  /* This is an undocumented method.
     See: http://www.cocoadev.com/index.pl?HowToCreateTollFreeBridgedClass for
     more info.
  */
  return (CFTypeID)_typeID;
}

@end



@interface NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return _kCFRuntimeNotATypeID;
}
@end

