/* CFError.h
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2010
   
   This file is part of GNUstep CoreBase Library.
   
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

#ifndef __COREFOUNDATION_CFERROR_H__
#define __COREFOUNDATION_CFERROR_H__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>

#if MAC_OS_X_VERSION_10_5 <= MAC_OS_X_VERSION_MAX_ALLOWED

#if __OBJC__
@class NSError;
typedef NSError * CFErrorRef;
#else
typedef const struct __CFError * CFErrorRef;
#endif

const CFStringRef kCFErrorDomainPOSIX;
const CFStringRef kCFErrorDomainOSStatus;
const CFStringRef kCFErrorDomainMach;
const CFStringRef kCFErrorDomainCocoa;

const CFStringRef kCFErrorLocalizedDescriptionKey;
const CFStringRef kCFErrorLocalizedFailureReasonKey;
const CFStringRef kCFErrorLocalizedRecoverySuggestionKey;
const CFStringRef kCFErrorDescriptionKey;
const CFStringRef kCFErrorUnderlyingErrorKey;



/*
 * Creating a CFError
 */
CFErrorRef
CFErrorCreate (CFAllocatorRef allocator, CFStringRef domain, CFIndex code,
  CFDictionaryRef userInfo);

CFErrorRef
CFErrorCreateWithUserInfoKeysAndValues (CFAllocatorRef allocator,
  CFStringRef domain, CFIndex code, const void *const *userInfoKeys,
  const void *const *userInfoValues, CFIndex numUserInfoValues);

/*
 * Getting Information About an Error
 */
CFStringRef
CFErrorGetDomain (CFErrorRef err);

CFIndex
CFErrorGetCode (CFErrorRef err);

CFDictionaryRef
CFErrorCopyUserInfo (CFErrorRef err);

CFStringRef
CFErrorCopyDescription (CFErrorRef err);

CFStringRef
CFErrorCopyFailureReason (CFErrorRef err);

CFStringRef
CFErrorCopyRecoverySuggestion (CFErrorRef err);

/*
 * Getting the CFError Type ID
 */
CFTypeID
CFErrorGetTypeID (void);

#endif /* MAC_OS_X_VERSION_10_5 */

#endif /* __COREFOUNDATION_CFERROR_H__ */

