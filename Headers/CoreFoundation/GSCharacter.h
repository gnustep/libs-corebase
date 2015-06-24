/* GSCharacter.h

   Copyright (C) 2014 Free Software Foundation, Inc.

   Written by: Stefan Bidigaray
   Date: November, 2014

   This file is part of the GNUstep CoreBase Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

#ifndef __GSCHARACTER_H__
#define __GSCHARACTER_H__ 1

#include <CoreFoundation/CFBase.h>

/** @defgroup CharUtils Character Utilities
    @{
 */
/** @name Unicode Code Point Functions
    @{
 */
/** @brief Determine if a character is an ASCII character (less than 128).
    @param[in] c Character to test.
    @return Return true if character is an ASCII character.
 */
CF_INLINE Boolean
GSCharacterIsASCII (const UTF32Char c)
{
  return c < 128;
}

/** @brief Determine if a character is a whitespace character.
    @param[in] c Charater to test.
    @return True if character is whitespace.
 */
CF_INLINE Boolean
GSCharacterIsWhitespace (const UTF32Char c)
{
  return (0x0009 <= c && c <= 0x000D) || (c == 0x0020) || (c == 0x0085)
    || (c == 0x00A0) || (c == 0x1680) || (0x2000 <= c && c <= 0x200A)
    || (c == 0x2028) || (c == 0x2029) || (c == 0x202F) || (c == 0x205F)
    || (c == 0x3000);
}

/** @brief Determine if character is in one of the supplementary planes.
    @param[in] c Character to test.
    @return Returns true if character is in one of the supplementary planes
	    and false if in the Basic Multilingual plane.
 */
CF_INLINE Boolean
GSCharacterIsInSupplementaryPlane (const UTF32Char c)
{
  return c > 0xFFFF;
}

/** @brief Determine true if character is a surrogate code point.
    @param[in] c Character to test.
    @return Returns true if character is a surrogate and false, otherwise.
 */
CF_INLINE Boolean
GSCharacterIsSurrogate (const UTF32Char c)
{
  return (c & 0xFFFFF800) == 0xD800;
}

/** @brief Determine if character is a leading surrogate code point.
    @param[in] c Character to test.
    @return Returns true if character is leading and false, otherwise.
 */
CF_INLINE Boolean
GSCharacterIsLeadSurrogate (const UTF32Char c)
{
  return (c & 0xFFFFFC00) == 0xD800;
}

/** @brief Determine if character is a trailing surrogate code point.
    @param[in] c Character to test.
    @return Returns true if character is trailing and false, otherwise.
 */
CF_INLINE Boolean
GSCharacterIsTrailSurrogate (const UTF32Char c)
{
  return (c & 0xFFFFFC00) == 0xDC00;
}

/** @} */

/** @name UTF-8 Utilities
    @{
 */
/** @brief The maximum number of UTF-8 code units required to represent
      the highest Unicode code point.
 */
#define kGSUTF8CharacterMaximumLength 4

/** @brief Determine the number of trailing bytes for a UTF-8 character
      based on the leading code unit.
    @param[in] c Leading code unit to test.
    @return The number of trailing bytes.
 */
CF_INLINE CFIndex
GSUTF8CharacterTrailBytesCount (const UTF8Char c)
{
  return (c < 0xF5) ? (c >= 0xC0) + (c >= 0xE0) + (c >= 0xF0) : 0;
}

/** @brief Determines if the specified UTF-8 code unit is a trailing code unit.
    @param[in] c The code unit to test.
    @return Returns true if this UTF-8 code unit is a trailing code unit.
 */
CF_INLINE Boolean
GSUTF8CharacterIsTrailing (const UTF8Char c)
{
  return (c & 0xC0) == 0x80;
}

/** @brief Determine the number of UTF-8 code units required to represent
      the specified Unicode code point.
    @param[in] c The Unicode code point to test.
    @return The number of UTF-8 code units required.
 */
CF_INLINE CFIndex
GSUTF8CharacterLength (const UTF32Char c)
{
  return (c <= 0x10FFFF) ? 1 + (c >= 0x80) + (c >= 0x800) + (c >= 0x10000) : 0;
}

/** @brief Append the UTF-8 Byte Order Mark to the string buffer.
    @param[in,out] d A pointer to the current position of the string buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer.
    @return True if the function was successful and false, otherwise.
 */
CF_INLINE CFIndex
GSUTF8CharacterAppendByteOrderMark (UTF8Char * d, const UTF8Char * limit)
{
  if ((limit - d) > 3)
    {
      d[0] = 0xEF;
      d[1] = 0xBB;
      d[2] = 0xBF;
    }

  return 3;
}

/** @brief Determine if a UTF-8 string buffer has a Byte Order Mark.
    @param[in,out] s A pointer to the current position of the string buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer.  The caller
      must ensure this parameter is beyond the string buffer
      pointed to by <b>d</b>.
    @return True if a Byte Order Mark is found and false, otherwise.
 */
CF_INLINE Boolean
GSUTF8CharacterSkipByteOrderMark (const UTF8Char ** s, const UTF8Char * limit)
{
  const UTF8Char *p;

  p = *s;
  if ((limit - p) > 3)
    {
      if (*p++ == 0xEF && *p++ == 0xBB && *p++ == 0xBF)
        {
          *s = p;
          return true;
        }
    }

  return false;
}

/** @brief Append a character to a UTF-8 string buffer.
    @param[in] d A pointer to the current position of the string buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer.
    @param[in] c The Unicode code point to write.
    @return The amount of code units written to the destination buffer. Will
      return <code>0</code> if <b>c</b> is a surrogate or invalid code point.
 */
CF_INLINE CFIndex
GSUTF8CharacterAppend (UTF8Char * d, const UTF8Char * limit, UTF32Char c)
{
  static const UTF8Char utf8LeadHeader[4] = { 0x00, 0xC0, 0xE0, 0xF0 };
  CFIndex l;

  if (GSCharacterIsSurrogate (c))
    return 0;

  l = GSUTF8CharacterLength (c);
  if (l && limit - d > l)
    {
      switch (l)
        {
          case 4:
            d[3] = (c & 0x3F) | 0x80;
            c >>= 6;
          case 3:
            d[2] = (c & 0x3F) | 0x80;
            c >>= 6;
          case 2:
            d[1] = (c & 0x3F) | 0x80;
            c >>= 6;
          case 1:
            d[0] = c | utf8LeadHeader[l - 1];
        }
    }

  return l;
}

/** @brief Get a Unicode code unit from a UTF-8 string buffer.
    @param[in,out] s A pointer to the current position of the source buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer. Must
      be at least <code>*s + 1</code>.
    @param[in] loss If this value is non-zero, it is used in case an invalid
      character is found in the input buffer.  Must be a valid Unicode
      character.
    @return A valid Unicode code unit or a UTF-16 surrogate.
      Will return 0 if:
      -# The UTF-8 code unit is also a 0.
      -# An invalid code point or code unit is encountered and <b>loss</b>
      was not specified.
 */
CF_INLINE UTF32Char
GSUTF8CharacterGet (const UTF8Char ** s, const UTF8Char * limit,
                    const UTF32Char loss)
{
  UTF32Char c;
  const UTF8Char *p;
  static const UTF32Char utf8LeadMask[4] = { 0x0, 0x1F, 0x0F, 0x07 };

  p = *s;
  c = *p++;
  if (c > 0x7F)
    {
      CFIndex trail;

      trail = GSUTF8CharacterTrailBytesCount (c);
      if (limit - p < trail)
        return loss;
      c &= utf8LeadMask[trail];
      switch (trail)
        {
          case 3:
            if (!GSUTF8CharacterIsTrailing (*p))
              {
                c = loss;
                break;
              }
            c = (c << 6) | (*p++ & 0x3F);
          case 2:
            if (!GSUTF8CharacterIsTrailing (*p))
              {
                c = loss;
                break;
              }
            c = (c << 6) | (*p++ & 0x3F);
          case 1:
            if (!GSUTF8CharacterIsTrailing (*p))
              {
                c = loss;
                break;
              }
            c = (c << 6) | (*p++ & 0x3F);
            break;
          case 0:
            c = loss;
        }
      if (c > 0x10FFFF)
        c = loss;
    }
  *s = p;

  return c;
}

/** @} */

/** @name UTF-16 Utilities
    @{
 */
/** @brief The maximum number of UTF-16 code units required to represent the
      highest Unicode code point.
 */
#define kGSUTF16CharacterMaximumLength 2

/** @brief The Byte Order Mark for UTF-16 strings. */
#define kGSUTF16CharacterByteOrderMark 0xFEFF

/** @brief The swapped Byte Order Mark for UTF-16 strings. */
#define kGSUTF16CharacterSwappedByteOrderMark 0xFFFE

/** @brief Append a character to a UTF-16 string buffer.
    @param[in,out] d A pointer to the current position of the buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer.
    @param[in] c The Unicode code point to write.
    @return True if the functions was successful, and false if there is not
      enough space left in the string buffer or the code point is
      invalid.
 */
CF_INLINE CFIndex
GSUTF16CharacterAppend (UTF16Char * d, const UTF16Char * limit, UTF32Char c)
{
  if (c <= 0xFFFF && !GSCharacterIsSurrogate (c))
    {
      if (d < limit)
        *d = c;
      return 1;
    }
  else if (c <= 0x10FFFF)
    {
      if ((limit - d) > 2)
        {
          d[0] = (c >> 10) + 0xD7C0;
          d[1] = (c & 0x3FF) + 0xDC00;
        }
      return 2;
    }

  return 0;
}

/** @brief Get a Unicode code point from a UTF-16 string buffer.
    @param[in,out] s A pointer to the current position of the buffer.
      This value is updated after a call to the function.
    @param[in] limit The position just after the end of the buffer. Must
      be at least <code>*s + 1</code>.
    @param[in] loss If this value is non-zero, it is used in case an invalid
      character is found in the input.  Must be a valid Unicode character.
    @return A valid Unicode code point.  Will return 0 if:
      -# The UTF-16 code unit is also a 0.
      -# The UTF-16 code unit pointed to by <b>s</b> is not a leading code
      unit.
      -# The leading UTF-16 code unit does not have a trailing pair.
 */
CF_INLINE UTF32Char
GSUTF16CharacterGet (const UTF16Char ** s, const UTF16Char * limit,
                     const UTF32Char loss)
{
  UTF32Char c;
  const UTF16Char *p;

  p = *s;
  c = *p++;
  if (GSCharacterIsSurrogate (c))
    {
      if (GSCharacterIsLeadSurrogate (c) && p < limit
          && GSCharacterIsTrailSurrogate (*p))
        c = (c << 10) + (*p++) - ((0xD7C0 << 10) + 0xDC00);
      else
        c = loss;
    }
  *s = p;

  return c;
}

/** @} */

/** @name UTF-32 Utilities
    @{
 */
/** @brief The Byte Order Mark for UTF-32 strings. */
#define kGSUTF32CharacterByteOrderMark 0x0000FEFF

/** @brief The swapped Byte Order Mark for UTF-32 strings. */
#define kGSUTF32CharacterSwappedByteOrderMark 0xFFFE0000
/** @} */
/** @} */

#endif /* __GSCHARACTER_H__ */

