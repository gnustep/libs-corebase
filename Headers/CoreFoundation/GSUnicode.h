/* GSUnicode.h

   Copyright (C) 2014 Free Software Foundation, Inc.

   Written by: Stefan Bidigaray
   Date: November, 2014

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
#define __GSUNICODE_H__ 1

#include <CoreFoundation/CFBase.h>
#include <stdarg.h>

/** @defgroup UnicodeUtils Unicode String Utilities
    @{
 */
/** @name Convert to/from a Unicode String
    @{
 */
/** @brief Convert a string in some external encoding to Unicode (UTF-16).
    @detail This function is used internally to convert to Unicode from
      the various supported encodings.

      The function performs checks on both the input and output
      to verify the results of the conversion is valid UTF-16 data.

    @note This function always attempts to consume the source buffer
      <b>s</b> completely. It will only stop if an invalid character
      and no <b>loss</b> character was provided. Certain encodings, like
      UTF-7, are stateful and cannot be converted recursively. This differs
      from the behavior of GSUnicodeToEncoding() and note must be taken.

    @param[in,out] d Pointer to the address the start of the destination
      buffer. If <code>NULL</code> or pointing to <code>NULL</code>, will
      cause the function to perform the conversion but not write any
      data out. On return, points to memory immediately after where the
      last byte of data was written out.
    @param[in] dLimit A pointer to memory immediately after the end of the
      destination buffer.
    @param[in] enc Encoding of the data in source buffer.
    @param[in,out] s Pointer to the first character of the source buffer. This
      value must not point to <code>NULL</code> or be <code>NULL</code>
      itself.
    @param[in] sLimit A pointer to memory immediate after the end of the
      source buffer.
    @param[in] loss A substitute character for invalid input. For example,
      if a UTF-8 input string encodes a surrogate without a pair. A typical
      character would be <b>U+FFFD</b> (replacement character). Specify a
      value of <code>0</code> if you do not want lossy conversion.
    @param[in] hasBOM If <code>true</code>, assumes the source includes a
      byte order mark.
    @return The amount of <code>UniChar</code> characters required to
      successfully complete the conversion. Will return <b>-1</b> if an
      error is encountered, such as an invalid character and no <b>loss</b>
      character was provided. If an error occurs, <b>dLen</b> and <b>sLen</b>
      are still updated and reflect where the error occurred.

    @see GSUnicodeToEncoding()
 */
CF_EXPORT CFIndex
GSUnicodeFromEncoding (UniChar ** d, const UniChar * const dLimit,
                       CFStringEncoding enc, const UInt8 ** s,
                       const UInt8 * const sLimit, const UTF16Char loss);

/** @brief Convert a Unicode string (UTF-16) to some external encoding.
    @detail This function is used internally to convert from Unicode to
      the various supported encodings.

      The function performs minimal checks on the input data and will only
      fail if a code point cannot be converted to the specified encoding and
      a <b>loss</b> character was not provided.

    @note This function only attempts to fill the destination buffer <b>d</b>.
      Only if <b>d</b> or <b>dLen</b> are <code>NULL</code> will this
      function attempt to consume the source buffer completely. Additionally,
      in the case when converting to UTF-16 this function does not perform
      any checks to ensure the input and output are correct. This differs
      from the behavior of GSUnicodeFromEncoding() and note must be taken.

    @param[in,out] d Pointer to the address the start of the destination
      buffer. If <code>NULL</code> or pointing to <code>NULL</code>, will
      cause the function to perform the conversion but not write any
      data out. On return, points to memory immediately after where the
      last byte of data was written out.
    @param[in] dLimit A pointer to memory immediately after the end of the
      destination buffer.
    @param[in] enc Encoding of the data in source buffer.
    @param[in,out] s Pointer to the first character of the source buffer. This
      value must not point to <code>NULL</code> or be <code>NULL</code>
      itself.
    @param[in] sLimit A pointer to memory immediate after the end of the
      source buffer.
    @param[in] loss A substitute character for invalid input.  For example,
      if a UTF-8 input string encodes a surrogate without a pair.  A typical
      character would be <b>'?'</b> (replacement character).  Specify a
      value of <b>0</b> if you do not want lossy conversion.
    @param[in] addBOM If <code>true</code>, adds a byte order mark to the
      start of the destination buffer.
    @return The number of successfully converted converted UTF-16 code points.
      May return <b>-1</b> if an error is encountered, such as an invalid
      character and no <b>loss</b> character was provided.

    @see GSUnicodeFromEncoding ()
 */
CF_EXPORT CFIndex
GSUnicodeToEncoding (UInt8 ** d, const UInt8 * const dLimit,
                     CFStringEncoding enc, const UniChar ** s,
                     const UniChar * const sLimit, const char loss,
                     Boolean addBOM);
/** @} */



CFIndex
GSUnicodeFormatWithArguments (UniChar * __restrict__ s, CFIndex n,
                              CFTypeRef locale,
                              const UniChar * __restrict__ format,
                              CFIndex fmtlen, va_list ap);

CFIndex
GSUnicodeFormat (UniChar * __restrict__ s, CFIndex n, CFTypeRef locale,
                 const UniChar * __restrict__ format, CFIndex fmtlen, ...);
#if 0
/** @name Printf-like Unicode Formatting
    @{
 */
/** @brief Creates an output according to a format per the printf family
      of functions.
    @param[in] buffer Output buffer.  If NULL, this function returns the number
      of characters needed.
    @param[in] size Maximum size of buffer.
    @param[in] locale This may be a CFDictionaryRef containing locale
      information or a CFLocaleRef object.  Pass NULL for POSIX locale.
    @param[in] format The formatted string with printf-style specifiers.
    @return On success, return the number of characters printed, excluding
      NULL byte.  If buffer is not long enough, returns how many
      characters would be required.
    @see GSUnicodeFormatWithArguments ()
 */
CF_EXPORT CFIndex
GSUnicodeFormat (UniChar * d, UniChar * const dLimit, CFTypeRef locale,
                 const UniChar * f, const UniChar * const fLimit, ...);

/** @brief Creates an output according to a format per the printf family
      of functions.
    @param[in] buffer Output buffer.  If NULL, this function returns the number
      of characters needed.
    @param[in] size Maximum size of buffer.
    @param[in] locale This may be a CFDictionaryRef containing locale
      information or a CFLocaleRef object.  Pass NULL for POSIX locale.
    @param[in] format The formatted string with printf-style directives.
    @param[in] arguments The variable argument list of values to be formatted.
    @return On success, return the number of characters printed, excluding
      NULL byte.  If buffer is not long enough, returns how many
      characters would be required.
    @see GSUnicodeFormat()
 */
CF_EXPORT CFIndex
GSUnicodeFormatWithArguments (UniChar * d, UniChar * const dLimit,
                              CFTypeRef locale, const UniChar * f,
                              const UniChar * fLimit, va_list ap);

/** @} */
#endif
/** @} */

#endif /* __GSUNICODE_H__ */
