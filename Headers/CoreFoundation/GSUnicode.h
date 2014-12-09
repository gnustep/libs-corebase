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

/** \defgroup UnicodeUtils Unicode String Utilities
    \{
 */
/** \name Converter to/from a Unicode String
    \{
 */
/** \brief Convert a buffer with bytes from the specified encoding to an
      array of UTF-16 characters.
    \details The intended use of this function is to convert a continuous
      stream of data to a fixed-size buffer.  Because it updates the
      position of the source buffer, it can be called repeatedly with a
      small, fixed-size buffer and the updated source buffer.
    \param[in] enc Encoding of the source buffer.
    \param[in,out] d A UTF-16 buffer to which converted characters are written.
    \param[in] d_end Pointer to memory just after the end of the destination
      buffer such that the buffer capacity is <code>d_end - d</code>.
    \param[in,out] s A pointer to a source byte buffer.  After a
      successful return, points to the location immediately after the
      last byte consumed.
    \param[in] s_end Pointer to memory just after the end of the source buffer.
    \param[in] loss A substitute character for invalid input.  For example,
      if a UTF-8 input string encodes a surrogate without a pair.  A typical
      character would be U+FFFD (replacement character).  Specify a value
      of 0 if you do not want lossy conversion.
    \param[in] bom If <code>true</code>, assumes the source includes a
      byte order mark.
    \return Returns <code>true</code> if:
      -# The source buffer was completely consumed.
      -# The destination buffer is full.
 */
CF_EXPORT Boolean
GSUnicodeFromEncoding (CFStringEncoding enc, UniChar ** d,
                       const UniChar * d_end, const UInt8 ** s,
                       const UInt8 * s_end, const UTF16Char loss, Boolean bom);

/** \brief Convert a UTF-16 character buffer to the specified encoding.
    \details The intended use of this function is to convert a continuous
      stream of data to a fixed-size buffer.  Because it updates the
      position of the source buffer, it can be called repeatedly with a
      small, fixed-size buffer and the updated source buffer.
    \param[in] enc Encoding of the destination buffer.
    \param[in,out] d A byte buffer to which converted characters are written.
    \param[in] d_end Pointer to memory just after the end of the destination
      buffer such that the buffer capacity is <code>d_end - d</code>.
    \param[in,out] s A pointer to a UTF-16 source buffer.  After a
      successful return, points to the location immediately after the last
      character consumed.
    \param[in] s_end Pointer to memory just after the end of the source buffer.
    \param[in] loss A substitute characters that cannot be converted to
      the specified encoding.
    \param[in] bom If <code>true</code>, will try to add a byter order mark
      to the output.
    \return Returns <code>true</code> if:
      -# The source buffer was completely consumed.
      -# The destination buffer is full.
 */
CF_EXPORT Boolean
GSUnicodeToEncoding (CFStringEncoding enc, UInt8 ** d, const UInt8 * d_end,
                     const UniChar ** s, const UniChar * s_end,
                     const char loss, Boolean bom);
/** \} */

/** \name Unicode Formatting
    \{
 */
/** \brief Creates an output according to a format per the printf family
      of functions.
    \param[in] buffer Output buffer.  If NULL, this function returns the number
      of characters needed.
    \param[in] size Maximum size of buffer.
    \param[in] locale This may be a CFDictionaryRef containing locale
      information or a CFLocaleRef object.  Pass NULL for POSIX locale.
    \param[in] format The formatted string with printf-style specifiers.
    \return On success, return the number of characters printed, excluding
      NULL byte.  If buffer is not long enough, returns how many
      characters would be required.
    \see GSUnicodeFormatWithArguments ()
 */
CF_EXPORT Boolean
GSUnicodeFormat (UniChar ** d, const UniChar * d_end, CFTypeRef locale,
                 const UniChar ** f, const UniChar * f_end, ...);

/** \brief Creates an output according to a format per the printf family
      of functions.
    \param[in] buffer Output buffer.  If NULL, this function returns the number
      of characters needed.
    \param[in] size Maximum size of buffer.
    \param[in] locale This may be a CFDictionaryRef containing locale
      information or a CFLocaleRef object.  Pass NULL for POSIX locale.
    \param[in] format The formatted string with printf-style directives.
    \param[in] arguments The variable argument list of values to be formatted.
    \return On success, return the number of characters printed, excluding
      NULL byte.  If buffer is not long enough, returns how many
      characters would be required.
    \see GSUnicodeFormat()
 */
CF_EXPORT Boolean
GSUnicodeFormatWithArguments (UniChar ** d, const UniChar * d_end,
                              CFTypeRef locale, const UniChar ** f,
                              const UniChar * f_end, va_list ap);

/** \} */
/** \} */

#endif /* __GSUNICODE_H__ */
