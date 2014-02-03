/* CFError.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: September, 2011
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFError.h"
#include "CoreFoundation/CFDictionary.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"



CONST_STRING_DECL(kCFErrorDomainPOSIX,
  "kCFErrorDomainPOSIX");
CONST_STRING_DECL(kCFErrorDomainOSStatus,
  "kCFErrorDomainOSStatus");
CONST_STRING_DECL(kCFErrorDomainMach,
  "kCFErrorDomainMach");
CONST_STRING_DECL(kCFErrorDomainCocoa,
  "kCFErrorDomainCocoa");
CONST_STRING_DECL(kCFErrorLocalizedDescriptionKey,
  "kCFErrorLocalizedDescriptionKey");
CONST_STRING_DECL(kCFErrorLocalizedFailureReasonKey,
  "kCFErrorLocalizedFailureReasonKey");
CONST_STRING_DECL(kCFErrorLocalizedRecoverySuggestionKey,
  "kCFErrorLocalizedRecoverySuggestionKey");
CONST_STRING_DECL(kCFErrorDescriptionKey,
  "kCFErrorDescriptionKey");
CONST_STRING_DECL(kCFErrorUnderlyingErrorKey,
  "kCFErrorUnderlyingErrorKey");

struct __CFError
{
  CFRuntimeBase   _parent;
  CFStringRef     _domain;
  CFIndex         _code;
  CFDictionaryRef _userInfo;
};

static CFTypeID _kCFErrorTypeID = 0;

static void
CFErrorFinalize (CFTypeRef cf)
{
  CFErrorRef err = (CFErrorRef)cf;
  
  CFRelease (err->_domain);
  CFRelease (err->_userInfo);
}

static Boolean
CFErrorEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFErrorRef err1 = (CFErrorRef)cf1;
  CFErrorRef err2 = (CFErrorRef)cf2;
  
  return err1->_code == err2->_code
    && CFEqual (err1->_domain, err2->_domain)
    && CFEqual (err1->_userInfo, err2->_userInfo);
}

static CFHashCode
CFErrorHash (CFTypeRef cf)
{
  return CFHash (((CFErrorRef)cf)->_domain);
}

static const CFRuntimeClass CFErrorClass =
{
  0,
  "CFError",
  NULL,
  NULL,
  CFErrorFinalize,
  CFErrorEqual,
  CFErrorHash,
  NULL,
  NULL
};

void CFErrorInitialize (void)
{
  _kCFErrorTypeID = _CFRuntimeRegisterClass (&CFErrorClass);
}

CFErrorRef
CFErrorCreate (CFAllocatorRef allocator, CFStringRef domain,
               CFIndex code, CFDictionaryRef userInfo)
{
  struct __CFError *new;
  
  if (domain == NULL)
    return NULL;
  
  new = (struct __CFError*)_CFRuntimeCreateInstance (allocator,
    _kCFErrorTypeID,
    sizeof(struct __CFError) - sizeof(CFRuntimeBase),
    0);
  
  if (new)
    {
      new->_domain = CFRetain (domain);
      new->_code = code;
      
      if (userInfo != NULL)
        new->_userInfo = (CFDictionaryRef)CFRetain (userInfo);
      else
        new->_userInfo = CFDictionaryCreate(allocator, NULL, NULL, 0, NULL, NULL);
    }
  
  return (CFErrorRef)new;
}

CFErrorRef
CFErrorCreateWithUserInfoKeysAndValues (CFAllocatorRef allocator,
                                        CFStringRef domain, CFIndex code,
                                        const void *const *userInfoKeys,
                                        const void *const *userInfoValues,
                                        CFIndex numUserInfoValues)
{
  CFDictionaryRef userInfo = CFDictionaryCreate (allocator, 
    (const void **)userInfoKeys, (const void **)userInfoValues, 
    numUserInfoValues, NULL, NULL);
  CFErrorRef ret = CFErrorCreate (allocator, domain, code, userInfo);
  
  CFRelease (userInfo);
  return ret;
}

CFStringRef
CFErrorCopyDescription (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFStringRef, err,
    "localizedDescription");
  
  return CFRetain(CFDictionaryGetValue (err->_userInfo,
    kCFErrorLocalizedDescriptionKey));
}

CFStringRef
CFErrorCopyFailureReason (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFStringRef, err,
    "localizedFailureReason");
  
  return CFRetain(CFDictionaryGetValue (err->_userInfo,
    kCFErrorLocalizedFailureReasonKey));
}

CFStringRef
CFErrorCopyRecoverySuggestion (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFStringRef, err,
    "localizedRecoverySuggestion");
  
  return CFRetain(CFDictionaryGetValue (err->_userInfo,
    kCFErrorLocalizedRecoverySuggestionKey));
}

CFDictionaryRef
CFErrorCopyUserInfo (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFDictionaryRef, err, "userInfo");
  
  return CFRetain(err->_userInfo);
}

CFIndex
CFErrorGetCode (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFIndex, err, "code");
  
  return err->_code;
}

CFStringRef
CFErrorGetDomain (CFErrorRef err)
{
  CF_OBJC_FUNCDISPATCHV(_kCFErrorTypeID, CFStringRef, err, "domain");
  
  return err->_domain;
}

CFTypeID
CFErrorGetTypeID (void)
{
  return _kCFErrorTypeID;
}

