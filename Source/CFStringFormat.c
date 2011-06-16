/* CFStringFormat.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: June, 2011
   
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

#include "CoreFoundation/CFString.h"

void
_CFStringAppendFormatAndArgumentsAux (CFMutableStringRef outputString,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef formatOptions, CFStringRef formatString, va_list args)
{
  // FIXME
}

CFStringRef
_CFStringCreateWithFormatAndArgumentsAux (CFAllocatorRef alloc,
  CFStringRef (*copyDescFunc)(void *, const void *loc),
  CFDictionaryRef formatOptions, CFStringRef formatString, va_list args)
{
  CFStringRef ret;
  CFMutableStringRef string;
  
  string = CFStringCreateMutable (alloc, 0);
  _CFStringAppendFormatAndArgumentsAux (string, copyDescFunc, formatOptions,
    formatString, args);
  
  ret = CFStringCreateCopy (alloc, string);
  CFRelease (string);
  return ret;
}


