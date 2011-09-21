/* CFURL.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Eric Wasylishen  <ewasylishen@gmail.com>
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

#import <Foundation/NSURL.h>

#include "CoreFoundation/CFURL.h"

//
// CFURL
//
CFTypeID
CFURLGetTypeID (void)
{
  return (CFTypeID)[NSURL class];
}

CFStringRef
CFURLCopyFileSystemPath (CFURLRef aURL, CFURLPathStyle style)
{
  // FIXME: Handle the style parameter
  return (CFStringRef)[(NSURL*)aURL path];
}

CFURLRef
CFURLCreateWithFileSystemPath (CFAllocatorRef allocator,
                               CFStringRef fileSystemPath,
                               CFURLPathStyle style, Boolean isDirectory)
{
  // FIXME: Handle the style parameter
  // FIXME: Should call initFileURLWithPath:isDirectory:
  return (CFURLRef)[[NSURL alloc] initFileURLWithPath:
    (NSString*)fileSystemPath];
}

CFURLRef
CFURLCreateWithString (CFAllocatorRef allocator, CFStringRef string,
                       CFURLRef baseURL)
{
  return (CFURLRef)[[NSURL alloc] initWithString: (NSString*)string
                                   relativeToURL: (NSURL*)baseURL];
}
