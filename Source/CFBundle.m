/* CFBundle.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: David Chisnall
   Date: April, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
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
#include "CoreFoundation/CFBundle.h"
#include <Foundation/NSBundle.h>
#include <Foundation/NSURL.h>

#if !defined(_WIN32)
#include <dlfcn.h>
#ifndef RTLD_DEFAULT
# define RTLD_DEFAULT   ((void *) 0)
#endif
#endif

CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL)
{
  NSString *path = [(NSURL*)bundleURL path];

  if (nil == path) { return 0; }

  return (CFBundleRef)[[NSBundle alloc] initWithPath: path];
}

void* CFBundleGetFunctionPointerForName(CFBundleRef bundle,
                                        CFStringRef functionName)
{
#if !defined(_WIN32)
  [bundle->bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
#else
  return NULL;
#endif
}

void* CFBundleGetDataPointerForName(CFBundleRef bundle,
                                    CFStringRef functionName)
{
#if !defined(_WIN32)
  [bundle->bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
#else
  return NULL;
#endif
}

