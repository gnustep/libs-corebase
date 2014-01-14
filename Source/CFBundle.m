/* CFBundle.m
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: David Chisnall
   Date: April, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
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

static CFTypeID _kCFBundleTypeID = 0;

@implementation NSBundle (CoreBaseAdditions)
- (CFTypeID) _cfTypeID
{
  return CFBundleGetTypeID();
}
@end

static const CFRuntimeClass CFBundleClass =
{
  0,
  "CFBundle",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFBundleInitialize (void)
{
  _kCFBundleTypeID = _CFRuntimeRegisterClass(&CFBundleClass);
}

CFTypeID
CFBundleGetTypeID (void)
{
  return _kCFBundleTypeID;
}

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
  [(NSBundle*)bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
#else
  return NULL;
#endif
}

void* CFBundleGetDataPointerForName(CFBundleRef bundle,
                                    CFStringRef functionName)
{
#if !defined(_WIN32)
  [(NSBundle*)bundle load];
  return dlsym(RTLD_DEFAULT, [functionName UTF8String]);
#else
  return NULL;
#endif
}

Boolean CFBundlePreflightExecutable(CFBundleRef bundle, CFErrorRef *error)
{
  NSBundle *ns = (NSBundle *) bundle;
  return [ns preflightAndReturnError: (NSError **)error];
}

Boolean CFBundleLoadExecutable(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  return [ns load];
}

Boolean CFBundleLoadExecutableAndReturnError(CFBundleRef bundle, CFErrorRef *error)
{
  NSBundle *ns = (NSBundle *) bundle;
  return [ns loadAndReturnError: (NSError**) error];
}

void CFBundleUnloadExecutable(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  [ns unload];
}

CFBundleRef CFBundleGetMainBundle(void)
{
  return (CFBundleRef) [NSBundle mainBundle];
}

CFBundleRef CFBundleGetBundleWithIdentifier(CFStringRef bundleID)
{
  return (CFBundleRef) [NSBundle bundleWithIdentifier: (NSString*)bundleID];
}

CFStringRef CFBundleGetIdentifier(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  return (CFStringRef) [ns bundleIdentifier];
}

CFURLRef CFBundleCopyBundleURL(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL *url = [ns bundleURL];
  [url retain];
  
  return (CFURLRef) url;
}

CFURLRef CFBundleCopyExecutableURL(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL* url;
  
  url = [ns executableURL];
  [url retain];
  
  return (CFURLRef) url;
}

CFURLRef CFBundleCopyBuiltInPlugInsURL(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL* url;
  
  url = [ns builtInPlugInsURL];
  [url retain];
  
  return (CFURLRef) url;
}

CFURLRef CFBundleCopyResourcesDirectoryURL(CFBundleRef bundle)
{
	NSBundle *ns = (NSBundle *) bundle;
	NSURL* url;

	url = [ns resourceURL];
	[url retain];

	return (CFURLRef) url;
}

CFURLRef CFBundleCopyResourceURL(CFBundleRef bundle, CFStringRef resourceName,
                                 CFStringRef resourceType,
                                 CFStringRef subDirName)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL *url;
  
  url = [ns URLForResource: (NSString *) resourceName
             withExtension: (NSString *) resourceType
              subdirectory: (NSString *) subDirName];

  [url retain];
  return (CFURLRef) url;
}

CFURLRef CFBundleCopyResourceURLForLocalization(CFBundleRef bundle,
   CFStringRef resourceName, CFStringRef resourceType,
   CFStringRef subDirName, CFStringRef localizationName)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL *url;
  
  url = [ns URLForResource: (NSString *) resourceName
             withExtension: (NSString *) resourceType
              subdirectory: (NSString *) subDirName
              localization: (NSString *) localizationName];

  [url retain];
  return (CFURLRef) url;
}


CFURLRef CFBundleCopyPrivateFrameworksURL(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL *url;
  
  url = [ns privateFrameworksURL];
  [url retain];
  
  return (CFURLRef) url;
}

CFURLRef CFBundleCopyAuxiliaryExecutableURL(CFBundleRef bundle,
                                            CFStringRef executableName)
{
  NSBundle *ns = (NSBundle *) bundle;
  NSURL *url;
  
  url = [ns URLForAuxiliaryExecutable: (NSString *) executableName];
  [url retain];
  
  return (CFURLRef) url;
}


CFDictionaryRef CFBundleGetInfoDictionary(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  return (CFDictionaryRef) [ns infoDictionary];
}

CFDictionaryRef CFBundleGetLocalInfoDictionary(CFBundleRef bundle)
{
  NSBundle *ns = (NSBundle *) bundle;
  return (CFDictionaryRef) [ns localizedInfoDictionary];
}

CFTypeRef CFBundleGetValueForInfoDictionaryKey(CFBundleRef bundle,
                                               CFStringRef key)
{
  NSBundle *ns = (NSBundle *) bundle;
  return [ns objectForInfoDictionaryKey: (NSString *)key];
}

