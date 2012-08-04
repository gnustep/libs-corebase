/* CFPropertyList.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: August, 2012
   
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

#include "CoreFoundation/CFPropertyList.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
//#include "CoreFoundation/CFStream.h"

CFPropertyListRef
CFPropertyListCreateDeepCopy (CFAllocatorRef alloc, CFPropertyListRef pList,
                              CFOptionFlags opts)
{
  return NULL;
}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef alloc, CFDataRef data,
                              CFOptionFlags opts, CFPropertyListFormat *fmt,
                              CFErrorRef *error)
{
  return NULL;
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat *fmt, CFErrorRef *error)
{
  return NULL;
}

CFDataRef
CFPropertyListCreateData (CFAllocatorRef alloc, CFPropertyListRef pList,
                          CFPropertyListFormat fmt, CFOptionFlags opts,
                          CFErrorRef *error)
{
  return NULL;
}

CFIndex
CFPropertyListWrite (CFPropertyListRef pList, CFWriteStreamRef stream,
                     CFPropertyListFormat fmt, CFOptionFlags opts,
                     CFErrorRef *error)
{
  return 0;
}

Boolean
CFPropertyListIsValid (CFPropertyListRef plist, CFPropertyListFormat fmt)
{
  return false;
}

/* The following functions are marked as obsolete as of Mac OS X 10.6.  They
 * will be implemented here as wrappers around the new functions.
 */
CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef alloc, CFPropertyListRef pList)
{
  return CFPropertyListCreateData(alloc, pList, kCFPropertyListXMLFormat_v1_0,
                                  0, NULL);
}

CFIndex
CFPropertyListWriteToStream (CFPropertyListRef pList, CFWriteStreamRef stream,
                             CFPropertyListFormat fmt, CFStringRef *errStr)
{
  CFIndex ret;
  CFErrorRef err = NULL;
  
  ret = CFPropertyListWrite (pList, stream, fmt, 0, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }

  return ret;
}

CFPropertyListRef
CFPropertyListCreateFromXMLData (CFAllocatorRef alloc, CFDataRef data,
                                 CFOptionFlags opts, CFStringRef *errStr)
{
  CFPropertyListRef plist;
  CFErrorRef err = NULL;
  
  plist = CFPropertyListCreateWithData (alloc, data, opts, NULL, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }
  
  return plist;
}

CFPropertyListRef
CFPropertyListCreateFromStream (CFAllocatorRef alloc, CFReadStreamRef stream,
                                CFIndex len, CFOptionFlags opts,
                                CFPropertyListFormat *fmt, CFStringRef *errStr)
{
  CFPropertyListRef plist;
  CFErrorRef err = NULL;
  
  plist = CFPropertyListCreateWithStream (alloc, stream, len, opts, fmt, &err);
  if (err)
    {
      if (errStr)
        *errStr = CFErrorCopyDescription (err);
      CFRelease (err);
    }
  
  return plist;
}

