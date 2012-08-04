/* CFXMLParser.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: December, 2011
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFNumber.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFXMLParser.h"
#include "GSPrivate.h"

#include <stdlib.h>
#include <string.h>

CONST_STRING_DECL(kCFXMLTreeErrorDescription,
  "kCFXMLTreeErrorDescription");
CONST_STRING_DECL(kCFXMLTreeErrorLineNumber,
  "kCFXMLTreeErrorLineNumber");
CONST_STRING_DECL(kCFXMLTreeErrorLocation,
  "kCFXMLTreeErrorLocation");
CONST_STRING_DECL(kCFXMLTreeErrorStatusCode,
  "kCFXMLTreeErrorStatusCode");

struct __CFXMLParser
{
  CFRuntimeBase         parent;
  CFXMLParserStatusCode _errorCode;
  CFStringRef           _errorDesc;
  CFDataRef             _data;
  CFURLRef              _dataSrc;
  CFIndex               _nodeVersion;
  CFXMLParserOptions    _options;
  CFXMLParserCallBacks  _callBacks;
  CFXMLParserContext    _context;
};

static CFTypeID _kCFXMLParserTypeID = 0;

static void
CFXMLParserFinalize (CFTypeRef cf)
{
  CFXMLParserRef parser = (CFXMLParserRef)cf;
  
  CFRelease (parser->_data);
  CFRelease (parser->_dataSrc);
  if (parser->_errorDesc)
    CFRelease (parser->_errorDesc);
}

static const CFRuntimeClass CFXMLParserClass =
{
  0,
  "CFXMLParser",
  NULL,
  NULL,
  CFXMLParserFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFXMLParserInitialize (void)
{
  _kCFXMLParserTypeID = _CFRuntimeRegisterClass (&CFXMLParserClass);
}



CFTypeID
CFXMLParserGetTypeID (void)
{
  return _kCFXMLParserTypeID;
}

void
CFXMLParserAbort (CFXMLParserRef parser, CFXMLParserStatusCode errorCode,
                  CFStringRef errorDesc)
{
  parser->_errorCode = errorCode;
  parser->_errorDesc = CFStringCreateCopy (CFGetAllocator(parser), errorDesc);
}

CFStringRef
CFXMLParserCopyErrorDescription (CFXMLParserRef parser)
{
  return CFRetain (parser->_errorDesc);
}

#define CFXMLPARSER_SIZE sizeof(struct __CFXMLParser) - sizeof(CFRuntimeBase)

CFXMLParserRef
CFXMLParserCreate (CFAllocatorRef alloc, CFDataRef xmlData,
                   CFURLRef dataSource, CFOptionFlags parseOptions,
                   CFIndex versionOfNodes, CFXMLParserCallBacks *callBacks,
                   CFXMLParserContext *context)
{
  struct __CFXMLParser *new;
  
  new = (struct __CFXMLParser*)_CFRuntimeCreateInstance (alloc,
                                                         _kCFXMLParserTypeID,
                                                         CFXMLPARSER_SIZE, 0);
  if (new)
    {
      new->_errorCode = kCFXMLStatusParseNotBegun;
      new->_data = CFDataCreateCopy (alloc, xmlData);
      if (dataSource)
        new->_dataSrc = CFURLCopyAbsoluteURL (dataSource);
      new->_nodeVersion = versionOfNodes;
      new->_options = parseOptions;
      if (callBacks)
        memcpy (&new->_callBacks, callBacks, sizeof(CFXMLParserCallBacks));
      if (context)
        {
          memcpy (&new->_context, context, sizeof(CFXMLParserContext));
          if (new->_context.retain)
            new->_context.retain (new->_context.info);
        }
    }
  
  return new;
}

CFXMLParserRef
CFXMLParserCreateWithDataFromURL (CFAllocatorRef alloc, CFURLRef dataSource,
                                  CFOptionFlags parseOptions,
                                  CFIndex versionOfNodes,
                                  CFXMLParserCallBacks *callBacks,
                                  CFXMLParserContext *context)
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



static void *
CFXMLTreeCreateXMLStructure (CFXMLParserRef parser,
  CFXMLNodeRef nodeDesc, void *info)
{
  return CFXMLTreeCreateWithNode (info, nodeDesc);
}

void
CFXMLTreeAddChild (CFXMLParserRef parser, void *parent, void *child,
  void *info)
{
  CFTreeAppendChild (parent, child);
}

void
CFXMLEndXMLStructure (CFXMLParserRef parser, void *xmlType, void *info)
{
  /* xmlType is of type CFXMLTree */
  if (CFTreeGetParent(xmlType)) /* Only release if not root */
    CFRelease ((CFTypeRef)xmlType);
}

CFXMLTreeRef
CFXMLTreeCreateFromDataWithError (CFAllocatorRef allocator, CFDataRef xmlData,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes,
  CFDictionaryRef *errorDict)
{
  CFXMLTreeRef new;
  CFXMLParserRef parser;
  CFXMLParserCallBacks callBacks;
  CFXMLParserContext context;
  
  callBacks.version = 0;
  callBacks.createXMLStructure = CFXMLTreeCreateXMLStructure;
  callBacks.addChild = CFXMLTreeAddChild;
  callBacks.endXMLStructure = CFXMLEndXMLStructure;
  callBacks.resolveExternalEntity = NULL;
  callBacks.handleError = NULL;
  
  context.version = 0;
  context.info = (void*)allocator;
  context.retain = NULL;
  context.release = NULL;
  context.copyDescription = NULL;
  
  parser = CFXMLParserCreate (allocator, xmlData, dataSource, parseOptions,
    versionOfNodes, &callBacks, &context);
  if (CFXMLParserParse(parser))
    {
      new = (CFXMLTreeRef)CFRetain (CFXMLParserGetDocument (parser));
    }
  else
    {
      if (errorDict)
        {
          /* Handle error */
          CFIndex num;
          CFTypeRef obj;
          CFMutableDictionaryRef dict = CFDictionaryCreateMutable (allocator,
            4, &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
          
          obj = CFXMLParserCopyErrorDescription (parser);
          if (obj)
            {
              CFDictionaryAddValue (dict, kCFXMLTreeErrorDescription, obj);
              CFRelease (obj);
            }
          
          num =  CFXMLParserGetLineNumber (parser);
          obj = CFNumberCreate (allocator, kCFNumberCFIndexType, &num);
          if (obj)
            {
              CFDictionaryAddValue (dict, kCFXMLTreeErrorLineNumber, obj);
              CFRelease (obj);
            }
          
          num = CFXMLParserGetLocation (parser);
          obj = CFNumberCreate (allocator, kCFNumberCFIndexType, &num);
          if (obj)
            {
              CFDictionaryAddValue (dict, kCFXMLTreeErrorLocation, obj);
              CFRelease (obj);
            }
          
          num = CFXMLParserGetStatusCode (parser);
          obj = CFNumberCreate (allocator, kCFNumberCFIndexType, &num);
          if (obj)
            {
              CFDictionaryAddValue (dict, kCFXMLTreeErrorStatusCode, obj);
              CFRelease (obj);
            }
          
          *errorDict = dict;
        }
      
      new = NULL;
    }
  CFRelease (parser);
  
  return new;
}

CFXMLTreeRef
CFXMLTreeCreateFromData (CFAllocatorRef allocator, CFDataRef xmlData,
  CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes)
{
  return CFXMLTreeCreateFromDataWithError (allocator, xmlData, dataSource,
    parseOptions, versionOfNodes, NULL);
}

CFXMLTreeRef
CFXMLTreeCreateWithDataFromURL (CFAllocatorRef allocator, CFURLRef dataSource,
  CFOptionFlags parseOptions, CFIndex versionOfNodes)
{
  CFXMLTreeRef new;
  CFXMLParserRef parser;
  CFXMLParserCallBacks callBacks;
  CFXMLParserContext context;
  
  callBacks.version = 0;
  callBacks.createXMLStructure = CFXMLTreeCreateXMLStructure;
  callBacks.addChild = CFXMLTreeAddChild;
  callBacks.endXMLStructure = CFXMLEndXMLStructure;
  callBacks.resolveExternalEntity = NULL;
  callBacks.handleError = NULL;
  
  context.version = 0;
  context.info = (void*)allocator;
  context.retain = NULL;
  context.release = NULL;
  context.copyDescription = NULL;
  
  parser = CFXMLParserCreateWithDataFromURL (allocator, dataSource,
    parseOptions, versionOfNodes, &callBacks, &context);
  if (CFXMLParserParse(parser))
    new = (CFXMLTreeRef)CFRetain (CFXMLParserGetDocument (parser));
  else
    new = NULL;
  CFRelease (parser);
  
  return new;
}

CFXMLTreeRef
CFXMLTreeCreateWithNode (CFAllocatorRef allocator, CFXMLNodeRef node)
{
  CFTreeContext context;
  context.version = 0;
  context.info = (void*)node;
  context.retain = CFRetain;
  context.release = CFRelease;
  context.copyDescription = CFCopyDescription;
  
  return CFTreeCreate (allocator, &context);
}

CFXMLNodeRef
CFXMLTreeGetNode (CFXMLTreeRef xmlTree)
{
  CFTreeContext context;
  CFTreeGetContext (xmlTree, &context);
  return context.info;
}

CFDataRef
CFXMLTreeCreateXMLData (CFAllocatorRef allocator, CFXMLTreeRef xmlTree)
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

