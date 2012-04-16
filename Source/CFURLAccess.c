/* CFURLAccess.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: April, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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
  
  if (!CFURLGetFileSystemRepresentation
      (url, true, (UInt8 *) path, PATH_MAX))
    {
      if (errorCode)
        *errorCode = kCFURLUnknownError;
      return false;
    }
  
  /* We'll check for data first */
  if (resourceData)
    {
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
          
          bytes = CFAllocatorAllocate (alloc, length, 0);
          if ((bytesRead = read (fd, bytes, length)) < 0)
            {
              error = kCFURLUnknownError;
              CFAllocatorDeallocate (alloc, bytes);
            }
          else
            {
              *resourceData = CFDataCreateWithBytesNoCopy (alloc,
                                                           (const void *)
                                                           bytes, length,
                                                           alloc);
            }
          
          close (fd);
        }
      else
        {
          switch (errno)
            {
              case ENOENT:
                exists = false;
                error = kCFURLResourceNotFoundError;
                break;
              default:
                error = kCFURLUnknownError;
            }
        }
    }
  
  /* Now we worry about the properties */
  if (exists && desiredProperties && properties)
    {
      CFIndex idx;
      CFIndex count;
      
      count = CFArrayGetCount (desiredProperties);
      for (idx = 0; idx < count; ++idx)
        {
          CFTypeRef obj = CFArrayGetValueAtIndex (desiredProperties, idx);
          if (kCFURLFileExists == obj
              || CFStringCompare (kCFURLFileExists, obj, 0)
              == kCFCompareEqualTo)
            {
              
            }
          else if (kCFURLFileDirectoryContents == obj
                   || CFStringCompare (kCFURLFileDirectoryContents, obj, 0)
                   == kCFCompareEqualTo)
            {
              
            }
          else if (kCFURLFileLength == obj
                   || CFStringCompare (kCFURLFileLength, obj, 0)
                   == kCFCompareEqualTo)
            {
              
            }
          else if (kCFURLFileLastModificationTime == obj
                   || CFStringCompare (kCFURLFileLastModificationTime, obj, 0)
                   == kCFCompareEqualTo)
            {
              
            }
          else if (kCFURLFilePOSIXMode == obj
                   || CFStringCompare (kCFURLFilePOSIXMode, obj, 0)
                   == kCFCompareEqualTo)
            {
              
            }
          else if (kCFURLFileOwnerID == obj
                   || CFStringCompare (kCFURLFileOwnerID, obj, 0)
                   == kCFCompareEqualTo)
            {
              
            }
        }
    }
    /*{
      struct stat sb;

      if (stat (path, &sb) < 0)
        error = kCFURLUnknownError;

      if (desiredProperties)
        {
          DIR *dir;
          struct dirent *entry;

          dir = opendir (path);
          while ((entry = readdir (dir)) != NULL)
            {
              if (strncmp (entry->d_name, ".", 2) != 0
                  && strncmp (entry->d_name, "..", 3) != 0)
                {
                  printf ("%s\n", entry->d_name);
                }
            }
          closedir (dir);
        }
    }*/
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
  error = 0;
  if (scheme == NULL)
    {
      error = kCFURLImproperArgumentsError;
    }
  else if (CFStringCompare (scheme, CFSTR ("file"), 0) == kCFCompareEqualTo)
    {
      CFFileURLCreateDataAndPropertiesFromResource (alloc, url, resourceData,
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
  
  CFRelease (scheme);
  if (error < 0 && errorCode)
    {
      *errorCode = error;
      return false;
    }
  
  return true;
}

CFTypeRef
CFURLCreatePropertyFromResource (CFAllocatorRef alloc,
                                 CFURLRef url,
                                 CFStringRef property, SInt32 * errorCode)
{
  CFDictionaryRef dict;
  CFArrayRef array;
  CFTypeRef ret;
  
  array = CFArrayCreate (alloc, (const void **) &property, 1, NULL);
  if (CFURLCreateDataAndPropertiesFromResource
      (alloc, url, NULL, &dict, array, errorCode) == false)
    return NULL;
  ret = CFRetain (CFDictionaryGetValue (dict, property));
  CFRelease (array);
  CFRelease (dict);
  
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
  
  CFRelease (scheme);
  if (error < 0 && errorCode)
    {
      *errorCode = error;
      return false;
    }
  
  return true;
}

Boolean
CFURLWriteDataAndPropertiesToResource (CFURLRef url,
                                       CFDataRef
                                       dataToWrite,
                                       CFDictionaryRef
                                       propertiesToWrite, SInt32 * errorCode)
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
      if (!CFURLGetFileSystemRepresentation
          (url, true, (UInt8 *) path, PATH_MAX))
        {
          if (errorCode)
            *errorCode = kCFURLUnknownError;
          return false;
        }
      
      mode = 0644;
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
  
  CFRelease (scheme);
  if (error < 0 && errorCode)
    {
      *errorCode = error;
      return false;
    }
  
  return true;
}
