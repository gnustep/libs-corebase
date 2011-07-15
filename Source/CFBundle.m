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

#include <dlfcn.h>
#ifndef RTLD_DEFAULT
# define RTLD_DEFAULT   ((void *) 0)
#endif

struct __CFBundle
{
  /** Superclass. */
  CFRuntimeBase          _parent;
  NSBundle              *bundle;
};

static CFTypeID _kCFBundleTypeID;

static void CFBundleFinalize(CFBundleRef b)
{
  [b->bundle release];
}

static Boolean CFBundleEqual(CFBundleRef cf1, CFBundleRef cf2)
{
  return [cf1->bundle isEqual: cf2->bundle];
}

static CFHashCode CFBundleHash(CFBundleRef cf)
{
  return [cf->bundle hash];
}

static CFStringRef CFBundleCopyDebugDesc(CFBundleRef cf)
{
  return [NSString stringWithFormat: _(@"<CFBundle %p: %@>"), cf, [cf->bundle bundlePath]];
}

static CFStringRef CFBundleCopyFormattingDesc(CFBundleRef cf,
  CFDictionaryRef formatOptions)
{
  return CFBundleCopyDebugDesc(cf);
}

static const CFRuntimeClass CFBundleClass =
{
  0,
  "CFBundle",
  NULL,
  NULL,
  (void (*)(CFTypeRef))CFBundleFinalize,
  (Boolean (*)(CFTypeRef, CFTypeRef))CFBundleEqual,
  (CFHashCode (*)(CFTypeRef))CFBundleHash,
  (CFStringRef (*)(CFTypeRef, CFDictionaryRef))CFBundleCopyFormattingDesc,
  (CFStringRef (*)(CFTypeRef))CFBundleCopyDebugDesc
};

CFTypeID CFBundleGetTypeID(void)
{
  return _kCFBundleTypeID;
}

void CFBundleInitialize(void)
{
  _kCFBundleTypeID = _CFRuntimeRegisterClass(&CFBundleClass);
}

CFBundleRef CFBundleCreate(CFAllocatorRef allocator, CFURLRef bundleURL)
{
  NSString *path = [(NSURL*)bundleURL path];

  if (nil == path) { return 0; }

  struct __CFBundle *new;
  
  new = (struct __CFBundle *)_CFRuntimeCreateInstance(allocator,
    CFBundleGetTypeID(), sizeof(struct __CFBundle) - sizeof(CFRuntimeBase),
    NULL);
  new->bundle = [[NSBundle alloc] initWithPath: path];

  return new;
}

void* CFBundleGetFunctionPointerForName(CFBundleRef bundle,
                                        CFStringRef functionName)
{
  [bundle->bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
}

void* CFBundleGetDataPointerForName(CFBundleRef bundle,
                                    CFStringRef functionName)
{
  [bundle->bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
}

