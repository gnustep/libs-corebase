/* CFURL.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2012
   
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
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"

static CFTypeID _kCFURLTypeID = 0;

struct __CFURL
{
  CFRuntimeBase _parent;
  CFStringRef   _urlString;
  CFURLRef      _baseURL;
};

static void
CFURLFinalize (CFTypeRef cf)
{
  CFURLRef url = (CFURLRef)cf;
  
  CFRelease (url->_urlString);
  if (url->_baseURL)
    CFRelease (url->_baseURL);
}

static const CFRuntimeClass CFURLClass =
{
  0,
  "CFURL",
  NULL,
  NULL,
  CFURLFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFURLInitialize (void)
{
  _kCFURLTypeID = _CFRuntimeRegisterClass (&CFURLClass);
}



CFTypeID
CFURLGetTypeID (void)
{
  return _kCFURLTypeID;
}

CFURLRef
CFURLCopyAbsoluteURL (CFURLRef relativeURL)
{
  return NULL;
}

CFURLRef
CFURLCreateAbsoluteURLWithBytes (CFAllocatorRef alloc,
  const UInt8 *relativeURLBytes, CFIndex length, CFStringEncoding encoding,
  CFURLRef baseURL, Boolean useCompatibilityMode)
{
  // FIXME: what to do with useCompatibilityMode?
  CFURLRef url;
  CFStringRef str;
  
  str = CFStringCreateWithBytes (alloc, relativeURLBytes, length,
    encoding, false);
  if (str == NULL)
    return NULL;
  
  url = CFURLCreateWithString (alloc, str, baseURL);
  if (url)
    {
      CFURLRef tmp = CFURLCopyAbsoluteURL (url);
      CFRelease (url);
      url = tmp;
    }
  
  return url;
}

CFURLRef
CFURLCreateByResolvingBookmarkData (CFAllocatorRef alloc, CFDataRef bookmark,
  CFURLBookmarkResolutionOptions options, CFURLRef relativeToURL,
  CFArrayRef resourcePropertiesToInclude, Boolean *isStale, CFErrorRef *error)
{
  return NULL; // FIXME: ???
}

CFURLRef
CFURLCreateCopyAppendingPathComponent (CFAllocatorRef alloc, CFURLRef url,
  CFStringRef pathComponent, Boolean isDirectory)
{
  return NULL;
}

CFURLRef
CFURLCreateCopyAppendingPathExtension (CFAllocatorRef alloc, CFURLRef url,
  CFStringRef extension)
{
  return NULL;
}

CFURLRef
CFURLCreateCopyDeletingLastPathComponent (CFAllocatorRef alloc, CFURLRef url)
{
  return NULL;
}

CFURLRef
CFURLCreateCopyDeletingPathExtension (CFAllocatorRef alloc, CFURLRef url)
{
  return NULL;
}

CFURLRef
CFURLCreateFilePathURL (CFAllocatorRef alloc, CFURLRef url,
  CFErrorRef *error)
{
  return NULL;
}

CFURLRef
CFURLCreateFileReferenceURL (CFAllocatorRef alloc, CFURLRef url,
  CFErrorRef *error)
{
  return NULL;
}

CFURLRef
CFURLCreateFromFileSystemRepresentation (CFAllocatorRef alloc,
  const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory)
{
  return CFURLCreateFromFileSystemRepresentationRelativeToBase (alloc,
    buffer, bufLen, isDirectory, NULL);
}

CFURLRef
CFURLCreateFromFileSystemRepresentationRelativeToBase (CFAllocatorRef alloc,
  const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL)
{
  return NULL;
}

CFURLRef
CFURLCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes, CFIndex length,
  CFStringEncoding encoding, CFURLRef baseURL)
{
  return NULL;
}

CFURLRef
CFURLCreateWithFileSystemPath (CFAllocatorRef alloc,
 CFStringRef fileSystemPath, CFURLPathStyle style, Boolean isDirectory)
{
  return CFURLCreateWithFileSystemPathRelativeToBase (alloc,
    fileSystemPath, style, isDirectory, NULL);
}

CFURLRef
CFURLCreateWithFileSystemPathRelativeToBase (CFAllocatorRef alloc,
  CFStringRef filePath, CFURLPathStyle style, Boolean isDirectory,
  CFURLRef baseURL)
{
  return NULL;
}

CFURLRef
CFURLCreateWithString (CFAllocatorRef alloc, CFStringRef string,
  CFURLRef baseURL)
{
  struct __CFURL *new;
  
  new = (struct __CFURL*)_CFRuntimeCreateInstance (alloc, _kCFURLTypeID,
    sizeof(struct __CFURL) - sizeof(CFRuntimeBase), 0);
  if (new)
    {
      new->_urlString = CFStringCreateCopy (alloc, string);
      new->_baseURL = baseURL ? CFURLCopyAbsoluteURL (baseURL) : NULL;
    }
  
  return new;
}

Boolean
CFURLCanBeDecomposed (CFURLRef url)
{
  return false;
}

CFStringRef
CFURLCopyFileSystemPath (CFURLRef aURL, CFURLPathStyle style)
{
  return NULL;
}

CFStringRef
CFURLCopyFragment (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
  return NULL;
}

CFStringRef
CFURLCopyHostName (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyLastPathComponent (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyNetLocation (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyParameterString (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
  return NULL;
}

CFStringRef
CFURLCopyPassword (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyPath (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyPathExtension (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyQueryString (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
  return NULL;
}

CFStringRef
CFURLCopyResourceSpecifier (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyScheme (CFURLRef url)
{
  return NULL;
}

CFStringRef
CFURLCopyStrictPath (CFURLRef url, Boolean *isAbsolute)
{
  return NULL;
}

CFStringRef
CFURLCopyUserName (CFURLRef url)
{
  return NULL;
}

SInt32
CFURLGetPortNumber (CFURLRef url)
{
  return 0;
}

Boolean
CFURLHasDirectoryPath (CFURLRef url)
{
  return false;
}

CFDataRef
CFURLCreateData (CFAllocatorRef alloc, CFURLRef url, CFStringEncoding encoding,
  Boolean escapeWhiteSpace)
{
  return NULL;
}

CFStringRef
CFURLCreateStringByAddingPercentEscapes (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef charactersToLeaveUnescaped,
  CFStringRef legalURLCharactersToBeEscaped, CFStringEncoding encoding)
{
  return NULL;
}

CFStringRef
CFURLCreateStringByReplacingPercentEscapes (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef charactersToLeaveEscaped)
{
  return NULL;
}

CFStringRef
CFURLCreateStringByReplacingPercentEscapesUsingEncoding (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef charactersToLeaveEscaped,
  CFStringEncoding encoding)
{
  return NULL;
}

Boolean
CFURLGetFileSystemRepresentation (CFURLRef url, Boolean resolveAgainstBase,
  UInt8 *buffer, CFIndex bufLen)
{
  return false;
}

CFStringRef
CFURLGetString (CFURLRef url)
{
  return url->_urlString;
}

CFURLRef
CFURLGetBaseURL (CFURLRef url)
{
  return url->_baseURL;
}

CFIndex
CFURLGetBytes (CFURLRef url, UInt8 *buffer, CFIndex bufLen)
{
  return 0;
}

CFRange
CFURLGetByteRangeForComponent (CFURLRef url, CFURLComponentType comp,
  CFRange *rangeIncludingSeparators)
{
  return CFRangeMake (kCFNotFound, 0);
}

Boolean
CFURLResourceIsReachable (CFURLRef url, CFErrorRef *error)
{
  return false;
}

void
CFURLClearResourcePropertyCache (CFURLRef url)
{
  
}

void
CFURLClearResourcePropertyCacheForKey (CFURLRef url, CFStringRef key)
{
  
}

CFDictionaryRef
CFURLCopyResourcePropertiesForKeys (CFURLRef url, CFArrayRef keys,
  CFErrorRef *error)
{
  return NULL;
}

Boolean
CFURLCopyResourcePropertyForKey (CFURLRef url, CFStringRef key,
  void *propertyValueTypeRefPtr, CFErrorRef *error)
{
  return false;
}

CFDictionaryRef
CFURLCreateResourcePropertiesForKeysFromBookmarkData (CFAllocatorRef alloc,
  CFArrayRef resourcePropertiesToReturn, CFDataRef bookmark)
{
  return NULL; // FIXME: ???
}

CFTypeRef
CFURLCreateResourcePropertyForKeyFromBookmarkData (CFAllocatorRef alloc,
  CFStringRef resourcePropertyKey, CFDataRef bookmark)
{
  return NULL; // FIXME: ???
}

Boolean
CFURLSetResourcePropertiesForKeys (CFURLRef url,
  CFDictionaryRef keyedPropertyValues, CFErrorRef *error)
{
  return false;
}

Boolean
CFURLSetResourcePropertyForKey (CFURLRef url, CFStringRef key,
  CFTypeRef propertValue, CFErrorRef *error)
{
  return false;
}

void
CFURLSetTemporaryResourcePropertyForKey (CFURLRef url, CFStringRef key,
  CFTypeRef propertyValue)
{
  
}

CFDataRef
CFURLCreateBookmarkData (CFAllocatorRef alloc, CFURLRef url,
  CFURLBookmarkCreationOptions options, CFArrayRef resourcePropertiesToInclude,
  CFURLRef relativeToURL, CFErrorRef *error)
{
  return NULL; // FIXME: ???
}

CFDataRef
CFURLCreateBookmarkDataFromAliasRecord (CFAllocatorRef alloc,
  CFDataRef aliasRecordDataRef)
{
  return NULL; // FIXME: ???
}

CFDataRef
CFURLCreateBookmarkDataFromFile (CFAllocatorRef alloc, CFURLRef fileURL,
  CFErrorRef *errorRef)
{
  return NULL; // FIXME: ???
}

Boolean
CFURLWriteBookmarkDataToFile (CFDataRef bookmarkRef, CFURLRef fileURL,
  CFURLBookmarkFileCreationOptions options, CFErrorRef *errorRef)
{
  return false; // FIXME: ???
}
