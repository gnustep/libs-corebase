/* CFPropertyList.m
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
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


#import <Foundation/NSPropertyList.h>

#include "CoreFoundation/CFPropertyList.h"

CFDataRef
CFPropertyListCreateData (CFAllocatorRef allocator,
                          CFPropertyListRef propertyList,
                          CFPropertyListFormat format, CFOptionFlags options,
                          CFErrorRef *error)
{
  // FIXME
}

CFPropertyListRef
CFPropertyListCreateDeepCopy (CFAllocatorRef allocator,
                              CFPropertyListRef propertyList,
                              CFOptionFlags mutabilityOption)
{
  // FIXME
}

// Function is marked as obsolete as of 10.6
CFPropertyListRef
CFPropertyListCreateFromStream (CFAllocatorRef allocator,
                                CFReadStreamRef stream,
                                CFIndex streamLength,
                                CFOptionFlags mutabilityOption,
                                CFPropertyListFormat *format,
                                CFStringRef *errorString)
{
/* FIXME
  CFErrorRef err = NULL;
  
  CFPropertyListRef ret =
    CFPropertyListCreateWithStream (allocator, stream, streamLength,
      mutabilityOption, format, &err);
  if (err != NULL)
    {
      if (errorString != NULL)
        {
          *errorString = CFErrorCopyDescription (err);
        }
      CFRelease (err);
    }
  
  return ret;
*/
}

CFPropertyListRef
CFPropertyListCreateFromXMLData (CFAllocatorRef allocator, CFDataRef xmlData,
                                 CFOptionFlags mutabilityOption,
                                 CFStringRef *errorString)
{
  return (CFPropertyListRef)CFRetain([NSPropertyListSerialization
                              propertyListFromData: (NSData *)xmlData
                              mutabilityOption: mutabilityOption
                              format: NULL
                              errorDescription: (NSString **)errorString]);
/* FIXME
  CFErrorRef err = NULL;
  
  CFPropertyListRef ret =
    CFPropertyListCreateWithData (allocator, xmlData, mutabilityOption,
      NULL, &err);
  if (err != NULL)
    {
      if (errorString != NULL)
        {
          *errorString = CFErrorCopyDescription (err);
        }
      CFRelease (err);
    }
  
  return ret;
*/
}

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef allocator, CFDataRef data,
                              CFOptionFlags options,
                              CFPropertyListFormat *format,
                              CFErrorRef *error)
{
  // FIXME: GNUstep does not have the necessary method to implement this.
}

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef allocator,
                                CFReadStreamRef stream,
                                CFIndex streamLength, CFOptionFlags options,
                                CFPropertyListFormat *format,
                                CFErrorRef *error)
{
  // FIXME: See above.
}

CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef allocator,
                             CFPropertyListRef propertyList)
{
  return (CFDataRef)[NSPropertyListSerialization
                      dataFromPropertyList: (id)propertyList
                                    format: kCFPropertyListXMLFormat_v1_0
                          errorDescription: NULL];
/* FIXME
  return CFPropertyListCreateData (allocator, propertyList, 
    kCFPropertyListXMLFormat_v1_0, 0, NULL)
*/
}

Boolean
CFPropertyListIsValid (CFPropertyListRef plist, CFPropertyListFormat format)
{
  return (Boolean)[NSPropertyListSerialization propertyList: (id)plist
                                           isValidForFormat: format];
}

CFIndex
CFPropertyListWrite (CFPropertyListRef propertyList, CFWriteStreamRef stream,
                     CFPropertyListFormat format, CFOptionFlags options,
                     CFErrorRef *error)
{
  // FIXME: See above
}

CFIndex
CFPropertyListWriteToStream (CFPropertyListRef propertyList,
                             CFWriteStreamRef stream,
                             CFPropertyListFormat format,
                             CFStringRef *errorString)
{
/* FIXME
  CFErrorRef err = NULL;
  
  CFPropertyListRef ret =
    CFPropertyListWrite (propertyList, stream, format, 0, &err);
  if (err != NULL)
    {
      if (errorString != NULL)
        {
          *errorString = CFErrorCopyDescription (err);
        }
      CFRelease (err);
    }
  
  return ret;
*/
}
