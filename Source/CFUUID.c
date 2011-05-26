/* CFUUID.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is disibuted in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <uuid/uuid.h>

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFUUID.h"



struct __CFUUID
{
  CFRuntimeBase _parent;
  CFUUIDBytes   _bytes;
};

static CFTypeID _kCFUUIDTypeID = 0;

#define UUID_FROM_UUIDBYTES(uuid, uuidBytes) do\
{ \
  uuid[0] = (uuidBytes).byte0; \
  uuid[1] = (uuidBytes).byte1; \
  uuid[2] = (uuidBytes).byte2; \
  uuid[3] = (uuidBytes).byte3; \
  uuid[4] = (uuidBytes).byte4; \
  uuid[5] = (uuidBytes).byte5; \
  uuid[6] = (uuidBytes).byte6; \
  uuid[7] = (uuidBytes).byte7; \
  uuid[8] = (uuidBytes).byte8; \
  uuid[9] = (uuidBytes).byte9; \
  uuid[10] = (uuidBytes).byte10; \
  uuid[11] = (uuidBytes).byte11; \
  uuid[12] = (uuidBytes).byte12; \
  uuid[13] = (uuidBytes).byte13; \
  uuid[14] = (uuidBytes).byte14; \
  uuid[15] = (uuidBytes).byte15; \
} while (0)

static Boolean
CFUUIDEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  uuid_t uu1, uu2;
  CFUUIDBytes bytes1 = ((CFUUIDRef)cf1)->_bytes;
  CFUUIDBytes bytes2 = ((CFUUIDRef)cf2)->_bytes;
  
  UUID_FROM_UUIDBYTES(uu1, bytes1);
  UUID_FROM_UUIDBYTES(uu2, bytes2);
  
  return uuid_compare (uu1, uu2) == 0 ? true : false;
}

static CFStringRef
CFUUIDCopyFormattingDescription (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  return CFUUIDCreateString (NULL, cf);
}

const CFRuntimeClass CFUUIDClass =
{
  0,
  "CFUUID",
  NULL, // init
  NULL, // copy
  NULL, // dealloc
  CFUUIDEqual, // equal
  NULL, // hash
  CFUUIDCopyFormattingDescription, // copyFormattingDesc
  NULL  // copyDebugDesc
};

void CFUUIDInitialize (void)
{
  _kCFUUIDTypeID = _CFRuntimeRegisterClass (&CFUUIDClass);
}



static inline CFUUIDRef
CFUUIDCreateFromUUID (CFAllocatorRef alloc, uuid_t uuid)
{
  CFUUIDBytes uuidBytes;
  
  uuidBytes.byte0 = uuid[0];
  uuidBytes.byte1 = uuid[1];
  uuidBytes.byte2 = uuid[2];
  uuidBytes.byte3 = uuid[3];
  uuidBytes.byte4 = uuid[4];
  uuidBytes.byte5 = uuid[5];
  uuidBytes.byte6 = uuid[6];
  uuidBytes.byte7 = uuid[7];
  uuidBytes.byte8 = uuid[8];
  uuidBytes.byte9 = uuid[9];
  uuidBytes.byte10 = uuid[10];
  uuidBytes.byte11 = uuid[11];
  uuidBytes.byte12 = uuid[12];
  uuidBytes.byte13 = uuid[13];
  uuidBytes.byte14 = uuid[14];
  uuidBytes.byte15 = uuid[15];
  
  return CFUUIDCreateFromUUIDBytes (alloc, uuidBytes);
}

CFUUIDRef
CFUUIDCreate (CFAllocatorRef alloc)
{
  uuid_t uuid;
  uuid_generate (uuid);
  return CFUUIDCreateFromUUID (alloc, uuid);
}

CFUUIDRef
CFUUIDCreateFromString (CFAllocatorRef alloc, CFStringRef uuidStr)
{
  uuid_t uuid;
  const char *data = CFStringGetCStringPtr (uuidStr, kCFStringEncodingASCII);
  if (data == NULL)
    return NULL; // FIXME
	uuid_parse (data, uuid);
  
  return CFUUIDCreateFromUUID (alloc, uuid);
}

CFUUIDRef
CFUUIDCreateFromUUIDBytes (CFAllocatorRef alloc, CFUUIDBytes bytes)
{
  struct __CFUUID *new;
  
  new = (struct __CFUUID *)_CFRuntimeCreateInstance (alloc, _kCFUUIDTypeID,
    sizeof(struct __CFUUID) - sizeof(CFRuntimeBase), NULL);
  if (new)
    new->_bytes = bytes;
  
  return (CFUUIDRef)new;
}

CFUUIDRef
CFUUIDCreateWithBytes (CFAllocatorRef alloc, UInt8 byte0, UInt8 byte1,
  UInt8 byte2, UInt8 byte3, UInt8 byte4, UInt8 byte5, UInt8 byte6, UInt8 byte7,
  UInt8 byte8, UInt8 byte9, UInt8 byte10, UInt8 byte11, UInt8 byte12,
  UInt8 byte13, UInt8 byte14, UInt8 byte15)
{
  CFUUIDBytes uuidBytes;
  uuidBytes.byte0 = byte0;
  uuidBytes.byte1 = byte1;
  uuidBytes.byte2 = byte2;
  uuidBytes.byte3 = byte3;
  uuidBytes.byte4 = byte4;
  uuidBytes.byte5 = byte5;
  uuidBytes.byte6 = byte6;
  uuidBytes.byte7 = byte7;
  uuidBytes.byte8 = byte8;
  uuidBytes.byte9 = byte9;
  uuidBytes.byte10 = byte10;
  uuidBytes.byte11 = byte11;
  uuidBytes.byte12 = byte12;
  uuidBytes.byte13 = byte13;
  uuidBytes.byte14 = byte14;
  uuidBytes.byte15 = byte15;
  
  return CFUUIDCreateFromUUIDBytes (alloc, uuidBytes);
}

CFStringRef
CFUUIDCreateString (CFAllocatorRef alloc, CFUUIDRef uuid)
{
  char out[37]; // 36 + null byte
  uuid_t uu;
  CFUUIDBytes uuidBytes = uuid->_bytes;
  
  UUID_FROM_UUIDBYTES(uu, uuidBytes);
  
  uuid_unparse (uu, out);
  
  return CFStringCreateWithCString (alloc, out, kCFStringEncodingASCII);
}

CFUUIDRef
CFUUIDGetConstantUUIDWithBytes (CFAllocatorRef alloc, UInt8 byte0, UInt8 byte1,
  UInt8 byte2, UInt8 byte3, UInt8 byte4, UInt8 byte5, UInt8 byte6, UInt8 byte7,
  UInt8 byte8, UInt8 byte9, UInt8 byte10, UInt8 byte11, UInt8 byte12,
  UInt8 byte13, UInt8 byte14, UInt8 byte15)
{
  return NULL;
}

CFUUIDBytes
CFUUIDGetUUIDBytes (CFUUIDRef uuid)
{
  return uuid->_bytes;
}

CFTypeID
CFUUIDGetTypeID (void)
{
  return _kCFUUIDTypeID;
}
