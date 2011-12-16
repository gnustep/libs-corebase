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
#include "CoreFoundation/CFXMLNode.h"

static CFTypeID _kCFXMLNodeTypeID = 0;

CFTypeID
CFXMLNodeGetTypeID (void)
{
  return _kCFXMLNodeTypeID;
}

CFXMLNodeRef
CFXMLNodeCreate (CFAllocatorRef alloc, CFXMLNodeTypeCode xmlType,
  CFStringRef dataString, const void *additionalInfoPtr, CFIndex version)
{
  return NULL;
}

CFXMLNodeRef
CFXMLNodeCreateCopy (CFAllocatorRef alloc, CFXMLNodeRef origNode)
{
  return NULL;
}

const void *
CFXMLNodeGetInfoPtr (CFXMLNodeRef node)
{
  return NULL;
}

CFStringRef
CFXMLNodeGetString (CFXMLNodeRef node)
{
  return NULL;
}

CFXMLNodeTypeCode
CFXMLNodeGetTypeCode (CFXMLNodeRef node)
{
  return 0;
}

CFIndex
CFXMLNodeGetVersion (CFXMLNodeRef node)
{
  return 0;
}