/* CFStringEncoding.c
   
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

#include <unicode/ucnv.h>
#include <unicode/ustring.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFByteOrder.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"

#include "CoreFoundation/ForFoundationOnly.h"

typedef struct
{
  CFStringEncoding enc;
  unsigned long nsEncoding;
  const char *converterName;
  UInt32 winCodepage;
} _str_encoding;

static const _str_encoding str_encoding_table[] =
{
  { kCFStringEncodingUTF8, 0, "UTF-8", 0 },
  { kCFStringEncodingUTF16, 0, "UTF-16", 0 },
  { kCFStringEncodingUTF16LE, 0, "UTF-16LE", 0 },
  { kCFStringEncodingUTF16BE, 0, "UTF-16BE", 0 },
  { kCFStringEncodingInvalidId, 0, NULL, 0 }
};

static const CFIndex str_encoding_table_size =
  sizeof(str_encoding_table) / sizeof(_str_encoding);

static CFIndex
CFStringEncodingTableIndex (CFStringEncoding encoding)
{
  CFIndex idx = 0;
  while (str_encoding_table[idx].enc != encoding
         && str_encoding_table[idx].enc != kCFStringEncodingInvalidId)
    idx++;
  return idx;
}

static inline const char *
CFStringICUConverterName (CFStringEncoding encoding)
{
  const char *name;
  CFIndex idx;
  
  idx = CFStringEncodingTableIndex (encoding);
  name = str_encoding_table[idx].converterName;
  
  return name;
}

static UConverter *
CFStringICUConverterOpen (CFStringEncoding encoding, char lossByte)
{
  const char *converterName;
  UConverter *cnv;
  UErrorCode err = U_ZERO_ERROR;
  
  converterName = CFStringICUConverterName (encoding);
  
  cnv = ucnv_open (converterName, &err);
  if (U_FAILURE(err))
    cnv = NULL;
  
  if (lossByte) // FIXME
    {
      // Use substitute functions...
    }
  else
    {
      // Use stop functions...
    }
  
  return cnv;
}

static void
CFStringICUConverterClose (UConverter *cnv)
{
  ucnv_close (cnv);
}



CFStringRef
CFStringConvertEncodingToIANACharSetName (CFStringEncoding encoding)
{
  const char *name;
  const char *cnvName;
  UErrorCode err = U_ZERO_ERROR;
  
  cnvName = CFStringICUConverterName (encoding);
  name = ucnv_getStandardName (cnvName, "IANA", &err);
  if (U_FAILURE(err))
    return NULL;
  /* Using this function here because we don't want to make multiple copies
     of this string. */
  return __CFStringMakeConstantString (name);
}

unsigned long
CFStringConvertEncodingToNSStringEncoding (CFStringEncoding encoding)
{
  CFIndex idx = CFStringEncodingTableIndex (encoding);
  return str_encoding_table[idx].nsEncoding;
}

UInt32
CFStringConvertEncodingToWindowsCodepage (CFStringEncoding encoding)
{
  CFIndex idx = CFStringEncodingTableIndex (encoding);
  return str_encoding_table[idx].winCodepage;
}

CFStringEncoding
CFStringConvertIANACharSetNameToEncoding (CFStringRef str)
{
  return kCFStringEncodingInvalidId;
}

CFStringEncoding
CFStringConvertNSStringEncodingToEncoding (unsigned long encoding)
{
  CFIndex idx = 0;
  CFStringEncoding enc;
  while ((enc = str_encoding_table[idx].enc) != kCFStringEncodingInvalidId)
    if (str_encoding_table[idx++].nsEncoding == encoding)
      return enc;
  return kCFStringEncodingInvalidId;
}

CFStringEncoding
CFStringConvertWindowsCodepageToEncoding (UInt32 codepage)
{
  CFIndex idx = 0;
  CFStringEncoding enc;
  while ((enc = str_encoding_table[idx].enc) != kCFStringEncodingInvalidId)
    if (str_encoding_table[idx++].winCodepage == codepage)
      return enc;
  return kCFStringEncodingInvalidId;
}

const CFStringEncoding *
CFStringGetListOfAvailableEncodings (void)
{
  return NULL;
}

CFIndex
CFStringGetMaximumSizeForEncoding (CFIndex length, CFStringEncoding encoding)
{
  UConverter *cnv;
  int8_t charSize;
  
  cnv = CFStringICUConverterOpen (encoding, 0);
  charSize = ucnv_getMaxCharSize (cnv);
  CFStringICUConverterClose (cnv);
  return charSize * length;
}

CFStringEncoding
CFStringGetMostCompatibleMacStringEncoding (CFStringEncoding encoding)
{
  return kCFStringEncodingInvalidId;
}

CFStringEncoding
CFStringGetSystemEncoding (void)
{
  return kCFStringEncodingInvalidId;
}

Boolean
CFStringIsEncodingAvailable (CFStringEncoding encoding)
{
  const CFStringEncoding *encodings = CFStringGetListOfAvailableEncodings ();
  while (*encodings != kCFStringEncodingInvalidId)
    if (*(encodings++) == encoding)
      return true;
  return false;
}

CFStringEncoding
CFStringGetFastestEncoding (CFStringRef str)
{
  const UniChar *s = CFStringGetCharactersPtr (str);
  return s ? kCFStringEncodingUTF16 : kCFStringEncodingASCII;
}

CFStringEncoding
CFStringGetSmallestEncoding (CFStringRef str)
{
  return kCFStringEncodingInvalidId;
}

CFIndex
CFStringGetMaximumSizeOfFileSystemRepresentation (CFStringRef string)
{
  return 0;
}


CFStringRef
CFStringGetNameOfEncoding (CFStringEncoding encoding)
{
  return NULL;
}



CFIndex
__CFStringEncodeByteStream (CFStringRef string, CFIndex rangeLoc,
  CFIndex rangeLen, Boolean generatingExternalFile, CFStringEncoding encoding,
  char lossByte, UInt8 *buffer, CFIndex max, CFIndex *usedBufLen)
{
  /* string = The string to convert
     rangeLoc = start
     rangeLen = end
     generatingExternalFile = include BOM
     encoding = the encoding to encode to
     lossByte = if a character can't be converted replace if with this
                if 0, then conversion stops
     buffer = the output buffer
              if buffer == NULL, then this function just checks if the string
              can be converted
     max = size of buffer
     usedBufLen = on return contains the number of bytes needed
     return -> number of characters converted (this is different from
               usedBufLen)
  */
  
  const UniChar *characters = CFStringGetCharactersPtr (string);
  CFIndex bytesNeeded = 0;
  CFIndex charactersConverted = 0;
  
  if (characters)
    {
      UConverter *ucnv;
      const UniChar *source = (const UniChar *)(characters + rangeLoc);
      const UniChar *sourceLimit = (const UniChar *)(source + rangeLen);
      char *target = (char *)buffer;
      const char *targetLimit = (const char *)(target + max);
      UErrorCode err = U_ZERO_ERROR;
      
      ucnv = CFStringICUConverterOpen (encoding, lossByte);
      
      ucnv_fromUnicode (ucnv, &target, targetLimit, &source, sourceLimit,
        NULL, true, &err);
      bytesNeeded = (CFIndex)(target - (char*)buffer);
      charactersConverted = bytesNeeded;
      if (err == U_BUFFER_OVERFLOW_ERROR)
        {
          char ibuffer[256]; // Arbitrary buffer size
          targetLimit = ibuffer + sizeof(ibuffer);
          do
            {
              target = ibuffer;
              ucnv_fromUnicode (ucnv, &target, targetLimit, &source,
                sourceLimit, NULL, true, &err);
              bytesNeeded += (CFIndex)(target - ibuffer);
            } while (err == U_BUFFER_OVERFLOW_ERROR);
        }
      
      CFStringICUConverterClose (ucnv);
    }
  else /* CFString is ASCII */
    {
      const char *str = CFStringGetCStringPtr (string, kCFStringEncodingASCII);
      if (__CFStringEncodingIsSupersetOfASCII(encoding))
        {
          char *target = (char *)buffer;
          const char *targetLimit = (const char *)(target + max);
          const char *source = (str + rangeLoc);
          const char *sourceLimit = (source + rangeLen);
          
          while (source < sourceLimit)
            {
              if (target < targetLimit)
                *(target++) = *(source++);
            }
          bytesNeeded = (CFIndex)(sourceLimit - str);
          charactersConverted = (CFIndex)(target - (char*)buffer);
        }
      else if (encoding == kCFStringEncodingUTF16)
        {
          UniChar *target = (UniChar *)buffer;
          const UniChar *targetLimit = (const UniChar *)(target + max);
          const char *source = str;
          const char *sourceLimit = (source + rangeLen);
          
          while (source < sourceLimit)
            {
              if (target < targetLimit)
                *(target++) = (UniChar)*(source++);
            }
          bytesNeeded = (CFIndex)(sourceLimit - str);
          charactersConverted = (CFIndex)(target - (UniChar*)buffer);
        }
      else /* Use UConverter */
        {
          CFIndex len;
          UConverter *ucnv;
          char *target = (char *)buffer;
          const char *source = (str + rangeLoc);
          CFIndex targetCapacity = max;
          CFIndex sourceLength = rangeLen;
          UErrorCode err = U_ZERO_ERROR;
          
          ucnv = CFStringICUConverterOpen (encoding, lossByte);
          
          len = ucnv_fromAlgorithmic (ucnv, UCNV_US_ASCII, target,
            targetCapacity, source, sourceLength, &err);
          
          CFStringICUConverterClose (ucnv);
          
          bytesNeeded = len;
          // FIXME: charactersConverted = ?
        }
    }
  
  if (usedBufLen)
    *usedBufLen = bytesNeeded;
  
  return charactersConverted;
}

#define CHECK_IS_ASCII(str, len, output) do \
{ \
  CFIndex i = 0; \
  output = true; \
  while (i < len) \
    if (str[i++] > 0x80) \
      { \
        output = false; \
        break; \
      } \
} while (0)

Boolean
__CFStringDecodeByteStream3 (const UInt8 *bytes, CFIndex len,
  CFStringEncoding encoding, Boolean alwaysUnicode,
  CFVarWidthCharBuffer *buffer, Boolean *useClientsMemoryPtr,
  UInt32 converterFlags)
{
  /* bytes = the input bytes
     len = number of bytes in bytes
     encoding = the encoding bytes are in
                if a BOM is not provided the function assumes big endian
     alwaysUnicode = convert to unicode
     buffer = a CFVarWidthCharBuffer structure:
              {
                chars = on return, a buffer with the characters
                isASCII = follow chars.c or chars.u
                shouldFreeChars = free chars after getting data
                allocator = allocator to use for chars
                numChars = number of characters written (allocated space may
                           be larger)
                localBuffer = for internal use
              }
     useClientsMemoryPtr = set to true if bytes did not need to be converted
     return -> false if an error occurred
  */
  
  if (len == 0)
    {
      buffer->numChars = 0;
      buffer->chars.c = "";
      buffer->isASCII = true;
      return true;
    }
  
  if (useClientsMemoryPtr)
    *useClientsMemoryPtr = true;
  buffer->isASCII = alwaysUnicode ? false : true;
  buffer->shouldFreeChars = false;
  if (buffer->allocator == NULL)
    buffer->allocator = CFAllocatorGetDefault ();
  
  if (encoding == kCFStringEncodingUTF16
      || encoding == kCFStringEncodingUTF16LE
      || encoding == kCFStringEncodingUTF16BE)
    {
      Boolean swap = false;
      const UniChar *source = (const UniChar *)bytes;
      const UniChar *sourceLimit  = (const UniChar *)(bytes + len);
      UniChar *characters = (UniChar *)source;
      
      if (encoding == kCFStringEncodingUTF16)
        {
          UniChar bom = *source;
          
          if (bom == 0xFFFE || bom == 0xFEFF)
            {
              ++source;
#if GS_WORDS_BIGENDIAN
              if (bom == 0xFFFE) // Little Endian
                swap = true;
#else
              if (bom == 0xFEFF) // Bid Endian
                swap = true;
#endif
            }
        }
#if GS_WORDS_BIGENDIAN
      else if (encoding == kCFStringEncodingUTF16LE)
        swap = true;
#else
      else if (encoding == kCFStringEncodingUTF16BE)
        swap = true;
#endif
      
      buffer->numChars = (CFIndex)(sourceLimit - source);
      buffer->isASCII = false;
      
      if (swap)
        {
          // Can we use the localBuffer?
          if (len < __kCFVarWidthLocalBufferSize)
            {
              characters = (UniChar *)buffer->localBuffer;
            }
          else
            {
              characters = CFAllocatorAllocate (buffer->allocator,
                len + sizeof(UniChar), 0);
              buffer->shouldFreeChars = true;
            }
          
          /* We'll swap the bytes if we need to */
          if (swap)
            {
              UniChar *in = (UniChar *)source;
              UniChar *out = characters;
              while (in < sourceLimit)
                *(out++) = CFSwapInt16 (*(in++));
            }
          
          if (useClientsMemoryPtr)
            *useClientsMemoryPtr = false;
        }
      
      buffer->chars.u = characters;
    }
  else
    {
      Boolean ascii;
      UniChar *dest;
      CFIndex destCapacity;
      CFIndex localBufferCapacity;
      const char *src = (const char *)bytes;
      UConverter *ucnv;
      UErrorCode err = U_ZERO_ERROR;
      
      /* Check to see if we can store this as ASCII */
      if (!alwaysUnicode && __CFStringEncodingIsSupersetOfASCII(encoding))
        {
          /* If all characters in this set are ASCII, treat them as such. */
          CHECK_IS_ASCII(bytes, len, ascii);
          if (ascii)
            {
              buffer->numChars = len;
              buffer->chars.c = (UInt8 *)bytes;
              return true;
            }
        }
      
      ucnv = CFStringICUConverterOpen (encoding, 0);
      if (ucnv == NULL)
        return false;
      
      dest = (UniChar *)buffer->localBuffer;
      localBufferCapacity = __kCFVarWidthLocalBufferSize / sizeof(UniChar);
      destCapacity = ucnv_toUChars (ucnv, dest, localBufferCapacity, src, len,
        &err);
      if (err == U_BUFFER_OVERFLOW_ERROR
          || err == U_STRING_NOT_TERMINATED_WARNING)
        {
          CFIndex newCapacity = destCapacity + 1;
          dest = CFAllocatorAllocate (buffer->allocator,
            newCapacity * sizeof(UniChar), 0);
          destCapacity = ucnv_toUChars (ucnv, dest, newCapacity, src, len,
            &err);
          buffer->shouldFreeChars = true;
        }
      
      CFStringICUConverterClose (ucnv);
      if (U_FAILURE(err))
        {
          if (dest != (UniChar *)buffer->localBuffer)
            CFAllocatorDeallocate (buffer->allocator, dest);
          return false;
        }
      
      if (useClientsMemoryPtr)
        *useClientsMemoryPtr = false;
      buffer->chars.u = dest;
      buffer->isASCII = false;
      buffer->numChars = destCapacity;
    }
  
  return true;
}

