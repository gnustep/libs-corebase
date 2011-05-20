/* ForFoundationOnly.h
   
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

#ifndef __COREFOUNDATION_FORFOUNDATIONONLY__
#define __COREFOUNDATION_FORFOUNDATIONONLY__ 1

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>

CF_INLINE Boolean
__CFStringEncodingIsSupersetOfASCII (CFStringEncoding encoding)
{
  switch (encoding & 0xF00)
    {
      case 0x100:
        return encoding == kCFStringEncodingUTF8 ? true : false;
      case 0x000: // MacOS codepage
        if (encoding == kCFStringEncodingMacJapanese
            || encoding == kCFStringEncodingMacArabic
            || encoding == kCFStringEncodingMacHebrew
            || encoding == kCFStringEncodingMacUkrainian
            || encoding == kCFStringEncodingMacSymbol
            || encoding == kCFStringEncodingMacDingbats)
          return false;
      case 0x200: // ISO-8859-*
      case 0x400: // DOS codepage
      case 0x500: // Windows codepage
        return true;
      case 0x600:
        return encoding == kCFStringEncodingASCII ? true : false;
    }
  return false;
}

CFIndex
__CFStringEncodeByteStream (CFStringRef string, CFIndex rangeLoc,
  CFIndex rangeLen, Boolean generatingExternalFile, CFStringEncoding encoding,
  char lossByte, UInt8 *buffer, CFIndex max, CFIndex *usedBufLen);

#define __kCFVarWidthLocalBufferSize 1008
typedef struct
{
  union
    {
      UInt8   *c;
      UniChar *u;
    } chars;
  Boolean        isASCII;
  Boolean        shouldFreeChars;
  Boolean        _unused1;
  Boolean        _unused2;
  CFAllocatorRef allocator;
  CFIndex        numChars;
  UInt8          localBuffer[__kCFVarWidthLocalBufferSize];
} CFVarWidthCharBuffer;

Boolean
__CFStringDecodeByteStream3 (const UInt8 *bytes, CFIndex len,
  CFStringEncoding encoding, Boolean alwaysUnicode,
  CFVarWidthCharBuffer *buffer, Boolean *useClientsMemoryPtr,
  UInt32 converterFlags);

#endif /* __COREFOUNDATION_FORFOUNDATIONONLY__ */
