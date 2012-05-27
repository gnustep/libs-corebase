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

#import <Foundation/NSObject.h>

#include "CoreFoundation/CoreFoundation.h"
#include "NSCFType.h"

extern void CFInitialize (void);
extern UInt32 __CFRuntimeClassTableSize;
extern UInt32 __CFRuntimeClassTableCount;
extern Class NSCFTypeClass;

extern void
GSRuntimeInitializeConstants (void);

void NSCFInitialize (void)
{
  static int requiredClasses = 5;
  --requiredClasses;
  
  if (requiredClasses == 0)
    {
      CFIndex i = 0;
      
      __CFRuntimeObjCClassTable = (Class *) calloc (__CFRuntimeClassTableSize,
                                    sizeof(Class));

      while (i < __CFRuntimeClassTableCount)
        __CFRuntimeObjCClassTable[i++] = NSCFTypeClass;
      
      CFRuntimeBridgeClass (CFNullGetTypeID(), "NSNull");
      CFRuntimeBridgeClass (CFArrayGetTypeID(), "NSCFArray");
      CFRuntimeBridgeClass (CFDataGetTypeID(), "NSCFData");
      CFRuntimeBridgeClass (CFErrorGetTypeID(), "NSCFError");
      CFRuntimeBridgeClass (CFStringGetTypeID(), "NSCFString");
      
      GSRuntimeInitializeConstants ();
    }
}

@interface NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID;
@end

@implementation NSObject (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return _kCFRuntimeNotATypeID;
}
@end

@implementation NSCFType

+ (void) load
{
  NSCFTypeClass = self;
  NSCFInitialize ();
}

- (id) retain
{
  return (id)CFRetain(self);
}

- (oneway void) release
{
  CFRelease(self);
}

- (NSUInteger) hash
{
  return (NSUInteger)CFHash (self);
}

- (BOOL) isEqual: (id) anObject
{
  return (BOOL)CFEqual (self, (CFTypeRef)anObject);
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
