/* NSCFType.h
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
   This file is part of GNUstep CoreBase library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#ifndef __NSCFTYPE_H__
#define __NSCFTYPE_H__

#import <Foundation/NSObject.h>
#import <GNUstepBase/GSObjCRuntime.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFRuntime.h"

extern void **__CFRuntimeObjCClassTable;

void NSCFInitialize (void);

CF_INLINE void
CFRuntimeBridgeClass (CFTypeID typeID, const char *cls)
{
  __CFRuntimeObjCClassTable[typeID] = (Class)objc_getClass (cls);
}

CF_INLINE void
CFRuntimeSetInstanceISA (CFTypeRef cf, Class cls)
{
  ((CFRuntimeBase *)cf)->_isa = cls;
}

/* This is NSCFType, the ObjC class that all non-bridged CF types belong to.
 */
#define NSCFTYPE_VARS { \
  /* NSCFType's ivar layout must match CFRuntimeBase. */ \
  int16_t _typeID; \
  struct \
    { \
      int16_t ro:       1; \
      int16_t unused:   7; \
      int16_t reserved: 8; \
    } _flags; \
}
@interface NSCFType : NSObject
NSCFTYPE_VARS
- (CFTypeID) _cfTypeID;
@end

#endif /* __NSCFTYPE_H__ */

