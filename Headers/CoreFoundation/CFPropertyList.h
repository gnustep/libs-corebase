/* CFPropertyList.h
   
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

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFStream.h>

#ifndef __COREFOUNDATION_CFPROPERTYLIST_H__
#define __COREFOUNDATION_CFPROPERTYLIST_H__

typedef enum CFPropertyListFormat CFPropertyListFormat;
enum CFPropertyListFormat
{
   kCFPropertyListOpenStepFormat = 1,
   kCFPropertyListXMLFormat_v1_0 = 100,
   kCFPropertyListBinaryFormat_v1_0 = 200
};

#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
typedef enum CFPropertyListMutabilityOptions CFPropertyListMutabilityOptions;
enum CFPropertyListMutabilityOptions
{
   kCFPropertyListImmutable = 0,
   kCFPropertyListMutableContainers = 1,
   kCFPropertyListMutableContainersAndLeaves = 2
};
#endif

#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum
{
   kCFPropertyListReadCorruptError = 3840,
   kCFPropertyListReadUnknownVersionError = 3841,
   kCFPropertyListReadStreamError = 3842,
   kCFPropertyListWriteStreamError = 3851,
};
#endif



CFPropertyListRef
CFPropertyListCreateDeepCopy (CFAllocatorRef allocator,
                              CFPropertyListRef propertyList,
                              CFOptionFlags mutabilityOption);

// Function marked as obsolete as of 10.6
CFPropertyListRef
CFPropertyListCreateFromXMLData (CFAllocatorRef allocator, CFDataRef xmlData,
                                 CFOptionFlags mutabilityOption,
                                 CFStringRef *errorString);

// Function marked as obsolete as of 10.6
CFDataRef
CFPropertyListCreateXMLData (CFAllocatorRef allocator,
                             CFPropertyListRef propertyList);

#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
// Function is marked as obsolete as of 10.6
CFPropertyListRef
CFPropertyListCreateFromStream (CFAllocatorRef allocator,
                                CFReadStreamRef stream,
                                CFIndex streamLength,
                                CFOptionFlags mutabilityOption,
                                CFPropertyListFormat *format,
                                CFStringRef *errorString);

Boolean
CFPropertyListIsValid (CFPropertyListRef plist, CFPropertyListFormat format);

// Function is marked as obsolete as of 10.6
CFIndex
CFPropertyListWriteToStream (CFPropertyListRef propertyList,
                             CFWriteStreamRef stream,
                             CFPropertyListFormat format,
                             CFStringRef *errorString);
#endif

#if MAC_OS_X_VERSION_10_6 <= MAC_OS_X_VERSION_MAX_ALLOWED
CFDataRef
CFPropertyListCreateData (CFAllocatorRef allocator,
                          CFPropertyListRef propertyList,
                          CFPropertyListFormat format, CFOptionFlags options,
                          CFErrorRef *error);

CFPropertyListRef
CFPropertyListCreateWithData (CFAllocatorRef allocator, CFDataRef data,
                              CFOptionFlags options,
                              CFPropertyListFormat *format,
                              CFErrorRef *error);

CFPropertyListRef
CFPropertyListCreateWithStream (CFAllocatorRef allocator,
                                CFReadStreamRef stream,
                                CFIndex streamLength, CFOptionFlags options,
                                CFPropertyListFormat *format,
                                CFErrorRef *error);

CFIndex
CFPropertyListWrite (CFPropertyListRef propertyList, CFWriteStreamRef stream,
                     CFPropertyListFormat format, CFOptionFlags options,
                     CFErrorRef *error);
#endif

#endif /* __COREFOUNDATION_CFPROPERTYLIST_H__ */
