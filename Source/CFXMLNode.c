/* CFXMLNode.c
   
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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFXMLNode.h"

struct __CFXMLNode
{
  CFRuntimeBase _parent;
  CFStringRef   _string;
  const void   *_info;
  CFIndex       _version;
};

static CFTypeID _kCFXMLNodeTypeID = 0;

static const CFRuntimeClass CFXMLNodeClass =
{
  0,
  "CFXMLNode",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFXMLNodeInitialize (void)
{
  _kCFXMLNodeTypeID = _CFRuntimeRegisterClass (&CFXMLNodeClass);
}



CFTypeID
CFXMLNodeGetTypeID (void)
{
  return _kCFXMLNodeTypeID;
}

#define CFXMLNODE_SIZE (sizeof(struct __CFXMLNode) - sizeof(CFRuntimeBase))

CFXMLNodeRef
CFXMLNodeCreate (CFAllocatorRef alloc, CFXMLNodeTypeCode xmlType,
  CFStringRef dataString, const void *additionalInfoPtr, CFIndex version)
{
  struct __CFXMLNode *new;
  CFIndex additionalInfoSize;
  
  switch (xmlType)
    {
      case kCFXMLNodeTypeDocument:
        additionalInfoSize = sizeof(struct CFXMLDocumentInfo);
        break;
      case kCFXMLNodeTypeElement:
        additionalInfoSize = sizeof(struct CFXMLElementInfo);
        break;
      case kCFXMLNodeTypeProcessingInstruction:
        additionalInfoSize = sizeof(struct CFXMLProcessingInstructionInfo);
        break;
      case kCFXMLNodeTypeEntity:
        additionalInfoSize = sizeof(struct CFXMLEntityInfo);
        break;
      case kCFXMLNodeTypeEntityReference:
        additionalInfoSize = sizeof(struct CFXMLEntityReferenceInfo);
        break;
      case kCFXMLNodeTypeDocumentType:
        additionalInfoSize = sizeof(struct CFXMLDocumentTypeInfo);
        break;
      case kCFXMLNodeTypeNotation:
        additionalInfoSize = sizeof(struct CFXMLNotationInfo);
        break;
      case kCFXMLNodeTypeElementTypeDeclaration:
        additionalInfoSize = sizeof(struct CFXMLElementTypeDeclarationInfo);
        break;
      case kCFXMLNodeTypeAttributeListDeclaration:
        additionalInfoSize = sizeof(struct CFXMLAttributeListDeclarationInfo);
        break;
      default:
        additionalInfoSize = 0;
    }
  
  new = (struct __CFXMLNode*)_CFRuntimeCreateInstance (alloc, _kCFXMLNodeTypeID,
    CFXMLNODE_SIZE + additionalInfoSize, 0);
  if (new)
    {
      new->_string = CFStringCreateCopy (alloc, dataString);
      new->_version = version;
      if (additionalInfoPtr)
        {
          new->_info = (const void*)&(new[1]);
          memcpy ((void*)new->_info, additionalInfoPtr, additionalInfoSize);
        }
    }
  
  return new;
}

CFXMLNodeRef
CFXMLNodeCreateCopy (CFAllocatorRef alloc, CFXMLNodeRef origNode)
{
  return NULL;
}

const void *
CFXMLNodeGetInfoPtr (CFXMLNodeRef node)
{
  return node->_info;
}

CFStringRef
CFXMLNodeGetString (CFXMLNodeRef node)
{
  return node->_string;
}

CFXMLNodeTypeCode
CFXMLNodeGetTypeCode (CFXMLNodeRef node)
{
  return ((CFRuntimeBase*)node)->_flags.info;
}

CFIndex
CFXMLNodeGetVersion (CFXMLNodeRef node)
{
  return node->_version;
}