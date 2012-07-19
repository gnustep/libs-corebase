/* CFURLAccess.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: April, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFURLAccess.h"
#include "GSPrivate.h"

#if defined(_WIN32)

#include <windows.h>

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  define PATH_MAX 1024
# endif
#endif

#endif

CONST_STRING_DECL (kCFURLFileExists, "kCFURLFileExists");
CONST_STRING_DECL (kCFURLFileDirectoryContents, "kCFURLFileDirectoryContents");
CONST_STRING_DECL (kCFURLFileLength, "kCFURLFileLength");
CONST_STRING_DECL (kCFURLFileLastModificationTime,
                   "kCFURLFileLastModificationTime");
CONST_STRING_DECL (kCFURLFilePOSIXMode, "kCFURLFilePOSIXMode");
CONST_STRING_DECL (kCFURLFileOwnerID, "kCFURLFileOwnerID");
CONST_STRING_DECL (kCFURLHTTPStatusCode, "kCFURLHTTPStatusCode");
CONST_STRING_DECL (kCFURLHTTPStatusLine, "kCFURLHTTPStatusLine");



static Boolean
CFFileURLCreateDataAndPropertiesFromResource (CFAllocatorRef alloc,
                                              CFURLRef url,
                                              CFDataRef * resourceData,
                                              CFDictionaryRef * properties,
                                              CFArrayRef desiredProperties,
                                              SInt32 * errorCode)
{
  int fd;
  char path[PATH_MAX];
  SInt32  error;
  Boolean exists;
  CFIndex length;
  SInt64  modTime;
  SInt32  mode;
  SInt32  ownerID;
  
  error = 0;
  exists = false;
  length = 0;
  modTime = 0;
  mode = 0;
  ownerID = 0;
  
  if (!CFURLGetFileSystemRepresentation
      (url, true, (UInt8 *) path, PATH_MAX))
    {
      if (errorCode)
        *errorCode = kCFURLUnknownError;
      return false;
    }
  
  /* We'll check for data first */
  fd = open (path, O_RDONLY);
  if (fd >= 0)
    {
      ssize_t bytesRead;
      struct stat sb;
      void *bytes;
      
      fstat (fd, &sb);
      
      /* Might as well gather as much information as we can here */
      exists = true;
      mode = sb.st_mode;
      ownerID = sb.st_uid;
      length = sb.st_size;
      modTime = sb.st_mtime;
      
      if (resourceData)
        {
          bytes = CFAllocatorAllocate (alloc, length, 0);
          if ((bytesRead = read (fd, bytes, length)) < 0)
            {
              error = kCFURLUnknownError;
              CFAllocatorDeallocate (alloc, bytes);
            }
          else
            {
              *resourceData = CFDataCreateWithBytesNoCopy (alloc,
                                                           (const void *)bytes,
                                                           length,
                                                           alloc);
            }
        }
      
      close (fd);
    }
  else
    {
      switch (errno)
        {
          case EACCES:
            exists = true;
            error = kCFURLResourceAccessViolationError;
            break;
          case ENOENT:
            error = kCFURLResourceNotFoundError;
            break;
          default:
            error = kCFURLUnknownError;
        }
    }
  
  /* Now we worry about the properties */
  if (properties)
    {
      Boolean fetchAll;
      CFIndex count;
      CFMutableDictionaryRef props;
      
      if (desiredProperties)
        {
          fetchAll = false;
          count = CFArrayGetCount (desiredProperties);
        }
      else
        {
          fetchAll = true;
          count = 0;
        }
      
      /* We have a maximum of 6 properties */
      props = CFDictionaryCreateMutable (alloc, 6,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
      
      if (fetchAll
          || CFArrayContainsValue (desiredProperties, CFRangeMake(0, count),
          kCFURLFileExists))
        {
          CFDictionaryAddValue (props, kCFURLFileExists,
                                exists ? kCFBooleanTrue : kCFBooleanFalse);
        }
      if (exists)
        {
          if (CFURLHasDirectoryPath (url) && (fetchAll
              || CFArrayContainsValue (desiredProperties,
              CFRangeMake(0, count), kCFURLFileDirectoryContents)))
            {
              DIR *dir;
              struct dirent *entry;
              
              dir = opendir (path);
              if (dir)
                {
                  CFArrayRef array;
                  CFMutableArrayRef tmp;
                  
                  tmp = CFArrayCreateMutable (alloc, 0,&kCFTypeArrayCallBacks);
                  while ((entry = readdir (dir)) != NULL)
                    {
                      if (strncmp (entry->d_name, ".", 2) != 0
                          && strncmp (entry->d_name, "..", 3) != 0)
                        {
                          CFStringRef str;
                          
                          str = CFStringCreateWithFileSystemRepresentation (
                            alloc, entry->d_name);
                          CFArrayAppendValue (tmp, str);
                          CFRelease (str);
                        }
                    }
                  closedir (dir);
                  
                  array = CFArrayCreateCopy (alloc, tmp);
                  CFRelease (tmp);
                  CFDictionaryAddValue (props, kCFURLFileDirectoryContents,
                                        array);
                  CFRelease (array);
                }
              else
                {
                  error = kCFURLUnknownError;
                }
            }
          if (fetchAll
              || CFArrayContainsValue (desiredProperties,
              CFRangeMake(0, count), kCFURLFileLength))
            {
              CFNumberRef len;
              
              len = CFNumberCreate (alloc, kCFNumberCFIndexType, &length);
              CFDictionaryAddValue (props, kCFURLFileLength, len);
              CFRelease (len);
            }
          if (fetchAll
              || CFArrayContainsValue (desiredProperties,
              CFRangeMake(0, count), kCFURLFileLastModificationTime))
            {
              CFDateRef date;
              CFAbsoluteTime at;
              
              at = ((CFAbsoluteTime)modTime) - kCFAbsoluteTimeIntervalSince1970;
              date = CFDateCreate (alloc, at);
              CFDictionaryAddValue (props, kCFURLFileLastModificationTime, date);
              CFRelease (date);
            }
          if (fetchAll
              || CFArrayContainsValue (desiredProperties,
              CFRangeMake(0, count), kCFURLFilePOSIXMode))
            {
              CFNumberRef num;
              
              num = CFNumberCreate (alloc, kCFNumberSInt32Type, &mode);
              CFDictionaryAddValue (props, kCFURLFilePOSIXMode, num);
              CFRelease (num);
            }
          if (fetchAll
              || CFArrayContainsValue (desiredProperties,
              CFRangeMake(0, count), kCFURLFileOwnerID))
            {
              CFNumberRef num;
              
              num = CFNumberCreate (alloc, kCFNumberSInt32Type, &ownerID);
              CFDictionaryAddValue (props, kCFURLFileOwnerID, num);
              CFRelease (num);
            }
        }
      
      *properties = CFDictionaryCreateCopy (alloc, props);
      CFRelease (props);
    }
  
  if (error < 0)
    {
      if (errorCode)
        *errorCode = error;
      return false;
    }
  
  return true;
}

Boolean
CFURLCreateDataAndPropertiesFromResource (CFAllocatorRef alloc, CFURLRef url,
                                          CFDataRef * resourceData,
                                          CFDictionaryRef * properties,
                                          CFArrayRef desiredProperties,
                                          SInt32 * errorCode)
{
  CFStringRef scheme;
  SInt32 error;
  
  scheme = CFURLCopyScheme (url);
  if (scheme == NULL)
    {
      error = kCFURLImproperArgumentsError;
    }
  else if (CFStringCompare (scheme, CFSTR ("file"), 0) == kCFCompareEqualTo)
    {
      CFRelease (scheme);
      return CFFileURLCreateDataAndPropertiesFromResource (alloc, url,
                                                           resourceData,
                                                           properties,
                                                           desiredProperties,
                                                           errorCode);
    }
  else if (CFStringCompare (scheme, CFSTR ("http"), 0) == kCFCompareEqualTo)
    {
      /* FIXME */
      error = kCFURLUnknownSchemeError;
    }
  else
    {
      error = kCFURLUnknownSchemeError;
    }
  
  if (scheme)
    CFRelease (scheme);
  if (errorCode)
    *errorCode = error;
  
  return false;
}

CFTypeRef
CFURLCreatePropertyFromResource (CFAllocatorRef alloc, CFURLRef url,
                                 CFStringRef property, SInt32 * errorCode)
{
  CFDictionaryRef dict;
  CFArrayRef array;
  CFTypeRef ret = NULL;
  
  array = CFArrayCreate (alloc, (const void **) &property, 1, NULL);
  if (CFURLCreateDataAndPropertiesFromResource (alloc, url, NULL, &dict,
      array, errorCode) == true)
    {
      ret = CFRetain (CFDictionaryGetValue (dict, property));
      CFRelease (dict);
    }
  CFRelease (array);
  
  return ret;
}

Boolean
CFURLDestroyResource (CFURLRef url, SInt32 * errorCode)
{
  CFStringRef scheme;
  SInt32 error;
  
  scheme = CFURLCopyScheme (url);
  error = 0;
  if (scheme == NULL)
    {
      error = kCFURLImproperArgumentsError;
    }
  else if (CFStringCompare (scheme, CFSTR ("file"), 0) == kCFCompareEqualTo)
    {
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation
          (url, true, (UInt8 *) path, PATH_MAX))
        {
          CFRelease (scheme);
          if (errorCode)
            *errorCode = kCFURLUnknownError;
          return false;
        }
      
      if (CFURLHasDirectoryPath (url))
        {
          if (rmdir (path) < 0)
            error = kCFURLUnknownError;
        }
      else
        {
          if (unlink (path) < 0)
            error = kCFURLUnknownError;
        }
    }
  else if (CFStringCompare (scheme, CFSTR ("http"), 0) == kCFCompareEqualTo)
    {
      /* FIXME */
      error = kCFURLUnknownSchemeError;
    }
  else
    {
      error = kCFURLUnknownSchemeError;
    }
  
  if (scheme)
    CFRelease (scheme);
  
  if (error < 0)
    {
      if (errorCode)
        *errorCode = error;
      return false;
    }
  
  return true;
}

Boolean
CFURLWriteDataAndPropertiesToResource (CFURLRef url,
                                       CFDataRef dataToWrite,
                                       CFDictionaryRef propertiesToWrite,
                                       SInt32 * errorCode)
{
  CFStringRef scheme;
  SInt32 error;
  
  scheme = CFURLCopyScheme (url);
  error = 0;
  if (scheme == NULL)
    {
      error = kCFURLImproperArgumentsError;
    }
  else if (CFStringCompare (scheme, CFSTR ("file"), 0) == kCFCompareEqualTo)
    {
      char path[PATH_MAX];
      int mode;
      if (!CFURLGetFileSystemRepresentation (url, true, (UInt8*)path, PATH_MAX))
        {
          CFRelease (scheme);
          if (errorCode)
            *errorCode = kCFURLUnknownError;
          return false;
        }
      
      mode = CFURLHasDirectoryPath (url) ? 0755 : 0644;
      if (propertiesToWrite)
        {
          /* All other properties are going to be disregarded */
          CFTypeRef num;
          if (CFDictionaryGetValueIfPresent (propertiesToWrite,
                                             kCFURLFilePOSIXMode, &num))
            CFNumberGetValue (num, kCFNumberIntType, &mode);
        }
      
      if (CFURLHasDirectoryPath (url))
        {
          if (mkdir (path, mode) < 0)
            error = kCFURLUnknownError;
        }
      else
        {
          int fd;
          fd = open (path, O_WRONLY | O_TRUNC | O_CREAT, mode);
          if (fd >= 0)
            {
              if (dataToWrite)
                {
                  CFIndex length;
                  const void *buf;
                  
                  length = CFDataGetLength (dataToWrite);
                  buf = CFDataGetBytePtr (dataToWrite);
                  if (length > 0 && write (fd, buf, length) != length)
                    error = kCFURLUnknownError;
                }
              
              close (fd);
            }
          else
            {
              error = kCFURLUnknownError;
            }
        }
    }
  else if (CFStringCompare (scheme, CFSTR ("http"), 0) == kCFCompareEqualTo)
    {
      /* FIXME */
      error = kCFURLUnknownSchemeError;
    }
  else
    {
      error = kCFURLUnknownSchemeError;
    }
  
  if (scheme)
    CFRelease (scheme);
  if (error < 0)
    {
      if (errorCode)
        *errorCode = error;
      return false;
    }
  
  return true;
}

