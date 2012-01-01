/* CFCharacterSet.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
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

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFCharacterSet.h"

#include <unicode/uset.h>

struct __CFCharacterSet
{
  CFRuntimeBase _parent;
  USet         *_uset;
};

static CFTypeID _kCFCharacterSetTypeID = 0;

static void
CFCharacterSetFinalize (CFTypeRef cf)
{
  CFCharacterSetRef cs = (CFCharacterSetRef)cf;
  uset_close (cs->_uset);
}

static const CFRuntimeClass CFCharacterSetClass =
{
  0,
  "CFCharacterSet",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFCharacterSetCreateCopy,
  CFCharacterSetFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFCharacterSetInitialize (void)
{
  _kCFCharacterSetTypeID = _CFRuntimeRegisterClass (&CFCharacterSetClass);
}

CFTypeID
CFCharacterSetGetTypeID (void)
{
  return _kCFCharacterSetTypeID;
}

CFCharacterSetRef
CFCharacterSetCreateCopy (CFAllocatorRef alloc, CFCharacterSetRef set)
{
  return NULL;
}

CFCharacterSetRef
CFCharacterSetCreateInvertedSet (CFAllocatorRef alloc, CFCharacterSetRef set)
{
  return NULL;
}

CFCharacterSetRef
CFCharacterSetCreateWithCharactersInRange (CFAllocatorRef alloc,
  CFRange range)
{
  return NULL;
}

CFCharacterSetRef
CFCharacterSetCreateWithCharactersInString (CFAllocatorRef alloc,
  CFStringRef string)
{
  return NULL;
}

CFCharacterSetRef
CFCharacterSetCreateWithBitmapRepresentation (CFAllocatorRef alloc,
  CFDataRef data)
{
  return NULL;
}

CFCharacterSetRef
CFCharacterSetGetPredefined (CFCharacterSetPredefinedSet setIdentifier)
{
  return NULL;
}

CFDataRef
CFCharacterSetCreateBitmapRepresentation (CFAllocatorRef alloc,
  CFCharacterSetRef set)
{
  return NULL;
}

Boolean
CFCharacterSetIsCharacterMember (CFCharacterSetRef set, UniChar c)
{
  return false;
}

Boolean
CFCharacterSetHasMemberInPlane (CFCharacterSetRef set, CFIndex plane)
{
  return false;
}

Boolean
CFCharacterSetIsLongCharacterMember (CFCharacterSetRef set, UTF32Char c)
{
  return false;
}

Boolean
CFCharacterSetIsSupersetOfSet (CFCharacterSetRef set,
  CFCharacterSetRef otherSet)
{
  return false;
}



CFMutableCharacterSetRef
CFCharacterSetCreateMutable (CFAllocatorRef alloc)
{
  return NULL;
}

CFMutableCharacterSetRef
CFCharacterSetCreateMutableCopy (CFAllocatorRef alloc, CFCharacterSetRef set)
{
  return NULL;
}

void
CFCharacterSetAddCharactersInRange (CFMutableCharacterSetRef set,
  CFRange range)
{
  
}

void
CFCharacterSetAddCharactersInString (CFMutableCharacterSetRef set,
  CFStringRef string)
{
  
}

void
CFCharacterSetRemoveCharactersInRange (CFMutableCharacterSetRef set,
  CFRange range)
{
  
}

void
CFCharacterSetRemoveCharactersInString (CFMutableCharacterSetRef set,
  CFStringRef string)
{
  
}

void
CFCharacterSetIntersect (CFMutableCharacterSetRef set,
  CFCharacterSetRef otherSet)
{
  
}

void
CFCharacterSetInvert (CFMutableCharacterSetRef set)
{
  
}

void
CFCharacterSetUnion (CFMutableCharacterSetRef set, CFCharacterSetRef otherSet)
{
  
}
