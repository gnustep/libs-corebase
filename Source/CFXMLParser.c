/* CFXMLParser.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFXMLParser.h"

static CFTypeID _kCFXMLParserTypeID = 0;



CFTypeID
CFXMLParserGetTypeID (void)
{
  return _kCFXMLParserTypeID;
}

void
CFXMLParserAbort (CFXMLParserRef parser, CFXMLParserStatusCode errorCode,
  CFStringRef errorDescription)
{
  
}

CFStringRef
CFXMLParserCopyErrorDescription (CFXMLParserRef parser)
{
  return NULL;
}

CFXMLParserRef
CFXMLParserCreate (CFAllocatorRef allocator, CFDataRef xmlData,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes,
  CFXMLParserCallBacks *callBacks, CFXMLParserContext *context)
{
  return NULL;
}

CFXMLParserRef
CFXMLParserCreateWithDataFromURL (CFAllocatorRef allocator,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes,
  CFXMLParserCallBacks *callBacks, CFXMLParserContext *context)
{
  return NULL;
}

void
CFXMLParserGetCallBacks (CFXMLParserRef parser,
  CFXMLParserCallBacks *callBacks)
{
  
}

void
CFXMLParserGetContext (CFXMLParserRef parser, CFXMLParserContext *context)
{
  
}

void *
CFXMLParserGetDocument (CFXMLParserRef parser)
{
  return NULL;
}

CFIndex
CFXMLParserGetLineNumber (CFXMLParserRef parser)
{
  return 0;
}

CFIndex
CFXMLParserGetLocation (CFXMLParserRef parser)
{
  return 0;
}

CFURLRef
CFXMLParserGetSourceURL (CFXMLParserRef parser)
{
  return NULL;
}

CFXMLParserStatusCode
CFXMLParserGetStatusCode (CFXMLParserRef parser)
{
  return 0;
}

Boolean
CFXMLParserParse (CFXMLParserRef parser)
{
  return false;
}



CFXMLTreeRef
CFXMLTreeCreateFromData (CFAllocatorRef allocator, CFDataRef xmlData,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes)
{
  return NULL;
}

CFXMLTreeRef
CFXMLTreeCreateWithDataFromURL (CFAllocatorRef allocator, CFURLRef dataSource,
  CFOptionFlags parseOptions, CFIndex versionOfNodes)
{
  return NULL;
}

CFXMLTreeRef
CFXMLTreeCreateWithNode (CFAllocatorRef allocator, CFXMLNodeRef node)
{
  return NULL;
}

CFDataRef
CFXMLTreeCreateXMLData (CFAllocatorRef allocator, CFXMLTreeRef xmlTree)
{
  return NULL;
}

CFXMLNodeRef
CFXMLTreeGetNode (CFXMLTreeRef xmlTree)
{
  return NULL;
}

CFStringRef
CFXMLCreateStringByEscapingEntities(CFAllocatorRef allocator,
  CFStringRef string, CFDictionaryRef entitiesDictionary)
{
  return NULL;
}

CFStringRef
CFXMLCreateStringByUnescapingEntities(CFAllocatorRef allocator,
  CFStringRef string, CFDictionaryRef entitiesDictionary)
{
  return NULL;
}

CFXMLTreeRef
CFXMLTreeCreateFromDataWithError (CFAllocatorRef allocator, CFDataRef xmlData,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes,
  CFDictionaryRef *errorDict)
{
  return NULL;
}
