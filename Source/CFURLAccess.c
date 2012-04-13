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
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFURLAccess.h"
#include "GSPrivate.h"

CONST_STRING_DECL(kCFURLFileExists, "kCFURLFileExists");
CONST_STRING_DECL(kCFURLFileDirectoryContents, "kCFURLFileDirectoryContents");
CONST_STRING_DECL(kCFURLFileLength, "kCFURLFileLength");
CONST_STRING_DECL(kCFURLFileLastModificationTime, "kCFURLFileLastModificationTime");
CONST_STRING_DECL(kCFURLFilePOSIXMode, "kCFURLFilePOSIXMode");
CONST_STRING_DECL(kCFURLFileOwnerID, "kCFURLFileOwnerID");
CONST_STRING_DECL(kCFURLHTTPStatusCode, "kCFURLHTTPStatusCode");
CONST_STRING_DECL(kCFURLHTTPStatusLine, "kCFURLHTTPStatusLine");



Boolean
CFURLCreateDataAndPropertiesFromResource (CFAllocatorRef alloc, CFURLRef url,
  CFDataRef *resourceData, CFDictionaryRef *properties,
  CFArrayRef desiredProperties, SInt32 *errorCode)
{
  return false;
}

CFTypeRef
CFURLCreatePropertyFromResource (CFAllocatorRef alloc, CFURLRef url,
  CFStringRef property, SInt32 *errorCode)
{
  return NULL;
}

Boolean
CFURLDestroyResource (CFURLRef url, SInt32 *errorCode)
{
  return false;
}

Boolean
CFURLWriteDataAndPropertiesToResource (CFURLRef url, CFDataRef dataToWrite,
  CFDictionaryRef propertiesToWrite, SInt32 *errorCode)
{
  return false;
}
