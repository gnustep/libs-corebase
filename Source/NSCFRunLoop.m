/* NSCFRunLoop.m

   Copyright (C) 2026 Free Software Foundation, Inc.

   Written by: Frederik Seiffert
   Date: October, 2026

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

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

#include "NSCFType.h"
#include "GSPrivate.h"
#include "CoreFoundation/CFRunLoop.h"

GS_PRIVATE void
_CFRunLoopAddCommonMode (CFRunLoopRef rl, CFStringRef mode);
GS_PRIVATE CFArrayRef
_CFRunLoopCopyAllModes (CFRunLoopRef rl);
GS_PRIVATE CFStringRef
_CFRunLoopCopyCurrentMode (CFRunLoopRef rl);

/* Implements the NSRunLoop methods used by the CFRunLoop mode functions
 * in GNUstep Base, so that those functions also work on CFRunLoopRefs.
 */
@interface NSCFRunLoop : NSObject
NSCFTYPE_VARS
@end

@implementation NSCFRunLoop
+ (void) load
{
  NSCFInitialize ();
}

+ (void) initialize
{
  GSObjCAddClassBehavior (self, [NSCFType class]);
}

- (void) addCommonMode: (NSString*)mode
{
  _CFRunLoopAddCommonMode ((CFRunLoopRef)self, (CFStringRef)mode);
}

/* These return Base objects rather than autoreleasing the CF objects, as
 * autoreleasing bridged CF objects crashes with the gnustep-1.9 runtime.
 */
- (NSArray*) allModes
{
  CFArrayRef modes = _CFRunLoopCopyAllModes ((CFRunLoopRef)self);
  NSArray *result = [NSArray arrayWithArray: (NSArray*)modes];

  CFRelease (modes);
  return result;
}

- (NSString*) currentMode
{
  CFStringRef mode = _CFRunLoopCopyCurrentMode ((CFRunLoopRef)self);
  NSString *result;

  if (mode == NULL)
    return nil;
  result = [NSString stringWithString: (NSString*)mode];
  CFRelease (mode);
  return result;
}
@end
