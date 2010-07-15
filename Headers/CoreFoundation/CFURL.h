/* CFURL.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Eric Wasylishen  <ewasylishen@gmail.com>
   Date: January, 2010
   
   This file is part of CoreBase.
   
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

#ifndef __COREFOUNDATION_CFURL_H__
#define __COREFOUNDATION_CFURL_H__

#include <CoreFoundation/CFBase.h>

#ifdef __OBJC__
@class NSURL;
typedef NSURL *CFURLRef;
#else
typedef struct CFURL *CFURLRef;
#endif

//
// Constants
//

typedef enum CFURLPathStyle {
  kCFURLPOSIXPathStyle = 0,
  kCFURLHFSPathStyle = 1,
  kCFURLWindowsPathStyle = 2
} CFURLPathStyle;

//
// Creating a CFURL Object
//

CFURLRef
CFURLCreateWithFileSystemPath (CFAllocatorRef allocator,
 CFStringRef fileSystemPath, CFURLPathStyle style, Boolean isDirectory);

CFURLRef
CFURLCreateWithString (CFAllocatorRef allocator, CFStringRef string,
  CFURLRef baseURL);

//
// Examining a CFURL Object
//

CFStringRef
CFURLCopyFileSystemPath (CFURLRef aURL, CFURLPathStyle style);

//
// Getting the CFURL Type ID
//
CFTypeID
CFURLGetTypeID (void);

#endif /* __COREFOUNDATION_CFURL_H__ */
