/* CFXMLNode.c
   
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
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFXMLNode.h"

#include <stdlib.h>

struct __CFXMLNode
{
  CFRuntimeBase _parent;
  CFXMLNodeTypeCode _type;
  CFStringRef   _string;
  CFIndex       _version;
  void         *_info;
};

static CFTypeID _kCFXMLNodeTypeID = 0;

static void
CFXMLNodeFinalize (CFTypeRef cf)
{
  CFXMLNodeRef node = (CFXMLNodeRef)cf;
  
  switch (node->_type)
    {
      case kCFXMLNodeTypeDocument:
        {
          CFXMLDocumentInfo *info = (CFXMLDocumentInfo*)node->_info;
          CFRelease (info->sourceURL);
        }
        break;
      case kCFXMLNodeTypeElement:
        {
          CFXMLElementInfo *info = (CFXMLElementInfo*)node->_info;
          CFRelease (info->attributes);
          CFRelease (info->attributeOrder);
        }
        break;
      case kCFXMLNodeTypeProcessingInstruction:
        {
          CFXMLProcessingInstructionInfo *info =
            (CFXMLProcessingInstructionInfo*)node->_info;
          CFRelease (info->dataString);
        }
        break;
      case kCFXMLNodeTypeEntity:
        {
          CFXMLEntityInfo *info = (CFXMLEntityInfo*)node->_info;
          CFRelease (info->replacementText);
          CFRelease (info->entityID.systemID);
          CFRelease (info->entityID.publicID);
          CFRelease (info->notationName);
        }
        break;
      case kCFXMLNodeTypeDocumentType: /* These two have the same structure. */
      case kCFXMLNodeTypeNotation:
        {
          CFXMLDocumentTypeInfo *info =
            (CFXMLDocumentTypeInfo*)node->_info;
          CFRelease (info->externalID.systemID);
          CFRelease (info->externalID.publicID);
        }
        break;
      case kCFXMLNodeTypeElementTypeDeclaration:
        {
          CFXMLElementTypeDeclarationInfo *info =
            (CFXMLElementTypeDeclarationInfo*)node->_info;
          CFRelease (info->contentDescription);
        }
        break;
      case kCFXMLNodeTypeAttributeListDeclaration:
        {
          CFXMLAttributeListDeclarationInfo *info =
            (CFXMLAttributeListDeclarationInfo*)node->_info;
          CFIndex idx;
          CFIndex num = info->numberOfAttributes;
          
          for (idx = 0 ; idx < num ; ++idx)
            {
              CFRelease (info->attributes[idx].attributeName);
              CFRelease (info->attributes[idx].typeString);
              CFRelease (info->attributes[idx].defaultString);
            }
          CFAllocatorDeallocate (CFGetAllocator (node), info->attributes);
        }
        break;
      default: /* Do nothing for everything else */
        break;
    }
  
  CFRelease (node->_string);
}

static Boolean
CFXMLNodeEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  CFXMLNodeRef node1 = (CFXMLNodeRef)cf1;
  CFXMLNodeRef node2 = (CFXMLNodeRef)cf2;
  
  if (node1->_type == node2->_type
      && CFEqual (node1->_string, node2->_string)
      && node1->_version == node2->_version)
    {
      switch (node1->_type)
        {
          case kCFXMLNodeTypeDocument:
            {
              CFXMLDocumentInfo *doc1 = (CFXMLDocumentInfo*)node1->_info;
              CFXMLDocumentInfo *doc2 = (CFXMLDocumentInfo*)node2->_info;
              return CFEqual (doc1->sourceURL, doc2->sourceURL);
            }
          case kCFXMLNodeTypeElement:
            {
              CFXMLElementInfo *elem1 = (CFXMLElementInfo*)node1->_info;
              CFXMLElementInfo *elem2 = (CFXMLElementInfo*)node2->_info;
              return elem1->isEmpty == elem2->isEmpty
                && CFEqual (elem1->attributes, elem2->attributes);
            }
          case kCFXMLNodeTypeProcessingInstruction:
            {
              CFXMLProcessingInstructionInfo *proc1 =
                (CFXMLProcessingInstructionInfo*)node1->_info;
              CFXMLProcessingInstructionInfo *proc2 =
                (CFXMLProcessingInstructionInfo*)node2->_info;
              return CFEqual (proc1->dataString, proc2->dataString);
            }
          case kCFXMLNodeTypeEntity:
            {
              CFXMLEntityInfo *ent1 = (CFXMLEntityInfo*)node1->_info;
              CFXMLEntityInfo *ent2 = (CFXMLEntityInfo*)node2->_info;
              return ent1->entityType == ent2->entityType
                && CFEqual (ent1->replacementText, ent2->replacementText)
                && CFEqual (ent1->notationName, ent2->notationName)
                && CFEqual (ent1->entityID.systemID, ent2->entityID.systemID)
                && CFEqual (ent1->entityID.publicID, ent2->entityID.publicID);
            }
          case kCFXMLNodeTypeEntityReference:
            {
              CFXMLEntityReferenceInfo *entRef1 =
                (CFXMLEntityReferenceInfo*)node1->_info;
              CFXMLEntityReferenceInfo *entRef2 =
                (CFXMLEntityReferenceInfo*)node2->_info;
              return entRef1->entityType == entRef2->entityType;
            }
          case kCFXMLNodeTypeDocumentType: /* These two have the same structure. */
          case kCFXMLNodeTypeNotation:
            {
              CFXMLDocumentTypeInfo *info1 =
                (CFXMLDocumentTypeInfo*)node1->_info;
              CFXMLDocumentTypeInfo *info2 =
                (CFXMLDocumentTypeInfo*)node2->_info;
              return CFEqual (info1->externalID.systemID,
                  info2->externalID.systemID)
                && CFEqual (info1->externalID.publicID,
                  info2->externalID.publicID);
            }
          case kCFXMLNodeTypeElementTypeDeclaration:
            {
              CFXMLElementTypeDeclarationInfo *decl1 =
                (CFXMLElementTypeDeclarationInfo*)node1->_info;
              CFXMLElementTypeDeclarationInfo *decl2 =
                (CFXMLElementTypeDeclarationInfo*)node2->_info;
              return
                CFEqual (decl1->contentDescription, decl2->contentDescription);
            }
          case kCFXMLNodeTypeAttributeListDeclaration:
            {
              CFXMLAttributeListDeclarationInfo *decl1 =
                (CFXMLAttributeListDeclarationInfo*)node1->_info;
              CFXMLAttributeListDeclarationInfo *decl2 =
                (CFXMLAttributeListDeclarationInfo*)node2->_info;
              CFIndex idx;
              
              if (decl1->numberOfAttributes != decl2->numberOfAttributes)
                return false;
              
              for (idx = 0 ; idx < decl1->numberOfAttributes ; ++idx)
                {
                  if (!CFEqual(decl1->attributes[idx].attributeName,
                        decl2->attributes[idx].attributeName)
                      || !CFEqual(decl1->attributes[idx].typeString,
                        decl2->attributes[idx].typeString)
                      || !CFEqual(decl1->attributes[idx].defaultString,
                        decl2->attributes[idx].defaultString))
                    return false;
                }
            }
          default:
            break;
        }
    }
  
  return false;
}

static CFHashCode
CFXMLNodeHash (CFTypeRef cf)
{
  CFXMLNodeRef node = (CFXMLNodeRef)cf;
  return (node->_string ? CFHash(node->_string) : 0) +
    node->_type + node->_version;
}

static const CFRuntimeClass CFXMLNodeClass =
{
  0,
  "CFXMLNode",
  NULL,
  NULL,
  CFXMLNodeFinalize,
  CFXMLNodeEqual,
  CFXMLNodeHash,
  NULL,
  NULL
};

void CFXMLNodeInitialize (void)
{
  _kCFXMLNodeTypeID = _CFRuntimeRegisterClass (&CFXMLNodeClass);
}

CF_INLINE void
CFXMLNodeCopyAdditionalInfo (CFAllocatorRef alloc, void *info,
  const void *additionalInfoPtr, CFXMLNodeTypeCode xmlType)
{
  switch (xmlType)
    {
      case kCFXMLNodeTypeDocument:
        {
          CFXMLDocumentInfo *src = (CFXMLDocumentInfo*)additionalInfoPtr;
          CFXMLDocumentInfo *dest = (CFXMLDocumentInfo*)info;
          dest->sourceURL = CFRetain (src->sourceURL);
          dest->encoding = src->encoding;
        }
        break;
      case kCFXMLNodeTypeElement:
        {
          CFXMLElementInfo *src = (CFXMLElementInfo*)additionalInfoPtr;
          CFXMLElementInfo *dest = (CFXMLElementInfo*)info;
          dest->attributes = src->attributes ?
            CFDictionaryCreateCopy (alloc, src->attributes) : NULL;
          dest->attributeOrder = src->attributeOrder ?
            CFArrayCreateCopy (alloc, src->attributeOrder) : NULL;
          dest->isEmpty = src->isEmpty;
        }
        break;
      case kCFXMLNodeTypeProcessingInstruction:
        {
          CFXMLProcessingInstructionInfo *src =
            (CFXMLProcessingInstructionInfo*)additionalInfoPtr;
          CFXMLProcessingInstructionInfo *dest =
            (CFXMLProcessingInstructionInfo*)info;
          dest->dataString = src->dataString ?
            CFStringCreateCopy (alloc, src->dataString) : NULL;
        }
        break;
      case kCFXMLNodeTypeEntity:
        {
          CFXMLEntityInfo *src = (CFXMLEntityInfo*)additionalInfoPtr;
          CFXMLEntityInfo *dest = (CFXMLEntityInfo*)info;
          dest->entityType = src->entityType;
          dest->replacementText = src->replacementText ?
            CFStringCreateCopy (alloc, src->replacementText) : NULL;
          dest->entityID.systemID = src->entityID.systemID ?
            CFRetain(src->entityID.systemID) : NULL;
          dest->entityID.publicID = src->entityID.publicID ?
            CFStringCreateCopy (alloc, src->entityID.publicID) : NULL;
          dest->notationName = src->notationName ?
            CFStringCreateCopy (alloc, src->notationName) : NULL;
        }
        break;
      case kCFXMLNodeTypeEntityReference:
        {
          CFXMLEntityReferenceInfo *src =
            (CFXMLEntityReferenceInfo*)additionalInfoPtr;
          CFXMLEntityReferenceInfo *dest = (CFXMLEntityReferenceInfo*)info;
          dest->entityType = src->entityType;
        }
        break;
      case kCFXMLNodeTypeDocumentType: /* These two have the same structure. */
      case kCFXMLNodeTypeNotation:
        {
          CFXMLDocumentTypeInfo *src =
            (CFXMLDocumentTypeInfo*)additionalInfoPtr;
          CFXMLDocumentTypeInfo *dest = (CFXMLDocumentTypeInfo*)info;
          dest->externalID.systemID = src->externalID.systemID ?
            CFRetain(src->externalID.systemID) : NULL;
          dest->externalID.publicID = src->externalID.publicID ?
            CFStringCreateCopy (alloc, src->externalID.publicID) : NULL;
        }
        break;
      case kCFXMLNodeTypeElementTypeDeclaration:
        {
          CFXMLElementTypeDeclarationInfo *src =
            (CFXMLElementTypeDeclarationInfo*)additionalInfoPtr;
          CFXMLElementTypeDeclarationInfo *dest =
            (CFXMLElementTypeDeclarationInfo*)info;
          dest->contentDescription = src->contentDescription ?
            CFStringCreateCopy (alloc, src->contentDescription) : NULL;
        }
        break;
      case kCFXMLNodeTypeAttributeListDeclaration:
        {
          CFXMLAttributeListDeclarationInfo *src =
            (CFXMLAttributeListDeclarationInfo*)additionalInfoPtr;
          CFXMLAttributeListDeclarationInfo *dest =
            (CFXMLAttributeListDeclarationInfo*)info;
          CFIndex idx;
          CFIndex num = src->numberOfAttributes;
          
          dest->numberOfAttributes = num;
          dest->attributes = num > 0 ? CFAllocatorAllocate (alloc,
            sizeof(CFXMLAttributeDeclarationInfo) * num, 0) : NULL;
          
          for (idx = 0 ; idx < num ; ++idx)
            {
              dest->attributes[idx].attributeName =
                CFStringCreateCopy (alloc, src->attributes[idx].attributeName);
              dest->attributes[idx].typeString =
                CFStringCreateCopy (alloc, src->attributes[idx].typeString);
              dest->attributes[idx].defaultString =
                CFStringCreateCopy (alloc, src->attributes[idx].defaultString);
            }
        }
        break;
      default: /* Do nothing for everything else */
        break;
    }
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
      new->_type = xmlType;
      new->_string =
        dataString ? CFStringCreateCopy (alloc, dataString) : dataString;
      new->_version = version;
      if (additionalInfoPtr)
        {
          new->_info = (void*)&(new[1]);
          CFXMLNodeCopyAdditionalInfo (alloc, new->_info, additionalInfoPtr,
            xmlType);
        }
    }
  
  return new;
}

CFXMLNodeRef
CFXMLNodeCreateCopy (CFAllocatorRef alloc, CFXMLNodeRef origNode)
{
  return CFXMLNodeCreate (alloc, origNode->_type, origNode->_string,
    origNode->_info, origNode->_version);
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

