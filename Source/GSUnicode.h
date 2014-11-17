/* GSUnicode.h
   
   Copyright (C) 2013 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2013
   
   This file is part of GNUstep CoreBase library.
   
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

#ifndef __GSUNICODE_H__
#define __GSUNICODE_H__

#include "config.h"
#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFString.h"

CF_EXTERN_C_BEGIN

/*
 * Printf style functions that output to a UniChar buffer.
 */
/** @internal
 * Creates an output according to a format per the printf family of functions.
 * @return On success, return the number of characters printed, excluding
 *         NULL byte.  If buffer is not long enough, returns how many
 *         characters would be required.
 *
 * @param buffer Output buffer.  If NULL, this function returns the number
 *               of characters needed.
 * @param size Maximum size of buffer.
 * @param locale This may be a CFDictionaryRef containing locale information
 *               or a CFLocaleRef object.  Pass NULL for POSIX locale.
 * @param format The formatted string with printf-style specifiers.
 *
 * @see GSUnicodeFormatWithArguments ()
 */ 
GS_PRIVATE CFIndex
GSUnicodeFormat (UniChar *__restrict__ s, CFIndex n, CFTypeRef locale,
                 const UniChar *__restrict__ format, CFIndex fmtlen, ...);

/** @internal
 * Creates an output according to a format per the printf family of functions.
 * @return On success, return the number of characters printed, excluding
 *         NULL byte.  If buffer is not long enough, returns how many
 *         characters would be required.
 *
 * @param buffer Output buffer.  If NULL, this function returns the number
 *               of characters needed.
 * @param size Maximum size of buffer.
 * @param locale This may be a CFDictionaryRef containing locale information
 *               or a CFLocaleRef object.  Pass NULL for POSIX locale.
 * @param format The formatted string with printf-style directives.
 * @param arguments The variable argument list of values to be formatted. 
 *
 * @see GSUnicodeFormat()
 */ 
GS_PRIVATE CFIndex
GSUnicodeFormatWithArguments (UniChar *__restrict__ s, CFIndex n,
                              CFTypeRef locale,
                              const UniChar *__restrict__ format,
                              CFIndex fmtlen,
                              va_list ap);

/*
 * General Unicode Conversion Macros and Functions
 */
/** @internal
 * Convert from Unicode to a specified encoding.
 * @return Number of characters converted.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param encoding Convert to encoding.
 * @param lossChar Character to be used if a character cannot be converted.
 *                 This is a Unicode character that can be converted to the
 *                 specified encoding.
 * @param isExternalRepresentation Specify if a BOM should be added.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of characters
 *            consumed from src.
 * @param dstLen Length of buffer (disregarded if buffer is NULL).
 * @param usedDstLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSFromUnicode (const UniChar * src, CFIndex srcLen, CFStringEncoding encoding,
               UniChar lossChar, Boolean isExternalRepresentation,
               UInt8 * buffer, CFIndex bufLen, CFIndex * usedDstLen);

/** @internal
 * Convert from a specified encoding to Unicode.
 * @return Number of bytes read.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param encoding Encoding to convert from.
 * @param lossChar Character to be used if a character cannot be converted.
 * @param isExternalRepresentation Specify if a BOM should be added.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of bytes
 *            consumed from source buffer.
 * @param dstLen Length of dst (disregarded if dst is NULL).
 * @param usedDstLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSToUnicode (const UInt8 * src, CFIndex srcLen, CFStringEncoding encoding,
             UniChar lossChar, Boolean isExternalRepresentation, UniChar * dst,
             CFIndex dstLen, CFIndex * usedDstLen);



/*
 * UTF-8 Conversion Macros and Functions
 */
/** @internal
 * Convert from UTF-8 to Unicode
 * @return Number of bytes read.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 * @param dst The destination buffer.  If NULL, conversion is attempted,
 *            usedLen will be est to the number of bytes required, and returns
 *            the number of bytes actually consumed form source buffer.
 * @param dstLen Destination buffer length.
 * @param usedLen If non-NULL, contains the number of characters written to dst.
 */
GS_PRIVATE CFIndex
GSUnicodeFromUTF8 (const UInt8 * src, CFIndex srcLen, UniChar lossChar,
                   UniChar * dst, CFIndex dstLen, CFIndex * usedLen);

/** @internal
 * Convert from Unicode to UTF-8
 * @return Number of characters converted.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 *                 This is a Unicode character that can be converted to the
 *                 specified encoding.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of characters
 *            consumed from src.
 * @param dstLen Length of buffer (disregarded if buffer is NULL).
 * @param usedLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSUnicodeToUTF8 (const UniChar * src, CFIndex srcLen, UniChar lossChar,
                 UInt8 * dst, CFIndex dstLen, CFIndex * usedLen);



/*
 * UTF-32 Conversion Macros and Functions
 */
/** @internal
 * Convert from UTF-32 to Unicode
 * @return Number of bytes read.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 * @param dst The destination buffer.  If NULL, conversion is attempted,
 *            usedLen will be est to the number of bytes required, and returns
 *            the number of bytes actually consumed form source buffer.
 * @param dstLen Destination buffer length.
 * @param usedLen If non-NULL, contains the number of characters written to dst.
 */
GS_PRIVATE CFIndex
GSUnicodeFromUTF32 (const UTF32Char * src, CFIndex srcLen, UniChar lossChar,
                    UniChar * dst, CFIndex dstLen, CFIndex * usedLen);

/** @internal
 * Convert from Unicode to UTF-32
 * @return Number of characters converted.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 *                 This is a Unicode character that can be converted to the
 *                 specified encoding.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of characters
 *            consumed from src.
 * @param dstLen Length of buffer (disregarded if buffer is NULL).
 * @param usedLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSUnicodeToUTF32 (const UniChar * src, CFIndex srcLen, UniChar lossChar,
                  UTF32Char * dst, CFIndex dstLen, CFIndex * usedLen);


/*
 * Macros and Functions for conversion of other Encodings
 */
/** @internal
 * Convert from Non-Lossy ASCII to Unicode
 * @return Number of bytes read.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 * @param dst The destination buffer.  If NULL, conversion is attempted,
 *            usedLen will be est to the number of bytes required, and returns
 *            the number of bytes actually consumed form source buffer.
 * @param dstLen Destination buffer length.
 * @param usedLen If non-NULL, contains the number of characters written to dst.
 */
GS_PRIVATE CFIndex
GSUnicodeFromNonLossyASCII (const char *src, CFIndex srcLen,
                            UniChar lossChar, UniChar * dst, CFIndex dstLen,
                            CFIndex * usedLen);

/** @internal
 * Convert from ISO-8859-1 (Latin1) to Unicode
 * @return Number of bytes read.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 * @param dst The destination buffer.  If NULL, conversion is attempted,
 *            usedLen will be est to the number of bytes required, and returns
 *            the number of bytes actually consumed form source buffer.
 * @param dstLen Destination buffer length.
 * @param usedLen If non-NULL, contains the number of characters written to dst.
 */
GS_PRIVATE CFIndex
GSUnicodeFromLatin1 (const UInt8 * src, CFIndex srcLen, UniChar lossChar,
                     UniChar * dst, CFIndex dstLen, CFIndex * usedLen);

/** @internal
 * Convert from Unicode to Non-Lossy ASCII
 * @return Number of characters converted.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 *                 This is a Unicode character that can be converted to the
 *                 specified encoding.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of characters
 *            consumed from src.
 * @param dstLen Length of buffer (disregarded if buffer is NULL).
 * @param usedLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSUnicodeToNonLossyASCII (const UniChar * src, CFIndex srcLen, UniChar lossChar,
                          char *dst, CFIndex dstLen, CFIndex * usedLen);

/** @internal
 * Convert from Unicode to ISO-8859-1 (Latin1)
 * @return Number of characters converted.
 * @param src Source buffer.
 * @param srcLen Source buffer length.
 * @param lossChar Character to be used if a character cannot be converted.
 *                 This is a Unicode character that can be converted to the
 *                 specified encoding.
 * @param dst The destination buffer which characters are converted to.
 *            If NULL, conversion is attempted, usedBufLen will be set to the
 *            number of bytes required, and returns the number of characters
 *            consumed from src.
 * @param dstLen Length of buffer (disregarded if buffer is NULL).
 * @param usedLen If non-NULL, the number of bytes used in destination buffer.
 */
GS_PRIVATE CFIndex
GSUnicodeToLatin1 (const UniChar * src, CFIndex srcLen, UniChar lossChar,
                   UInt8 * dst, CFIndex dstLen, CFIndex * usedLen);

CF_EXTERN_C_END

#endif /* __GSUNICODE_H__ */
