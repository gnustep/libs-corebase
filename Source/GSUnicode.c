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

#include "CoreFoundation/CFByteOrder.h"

#include "GSPrivate.h"
#include "GSUnicode.h"

#if defined(HAVE_UNICODE_CNV_H)
#include <unicode/ucnv.h>
#endif

CFIndex
GSUnicodeFromUTF8 (const UInt8 * s, CFIndex slen, UniChar lossChar, UniChar * d,
                   CFIndex dlen, CFIndex * usedLen)
{
  const UInt8 *sstart;
  const UInt8 *slimit;
  UniChar *dstart;
  UniChar *dlimit;

  sstart = s;
  slimit = sstart + slen;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      if (*s < 0x80)
        {
          if (dlen != 0)
            *d = *s;
          d++;
          s++;
        }
      else
        {
          UTF32Char u;
          CFIndex count;

          count = GS_UTF8_BYTE_COUNT (*s);
          if (count == 2 && GS_UTF8_IS_TRAIL (s[1]))
            {
              u = (s[0] & 0x1F) << 6;
              u |= s[1] & 0x3F;
            }
          else if (count == 3 && GS_UTF8_IS_TRAIL (s[1])
                   && GS_UTF8_IS_TRAIL (s[2]))
            {
              u = (s[0] & 0x0F) << 12;
              u |= (s[1] & 0x3F) << 6;
              u |= s[2] & 0x3F;
            }
          else if (count == 4 && GS_UTF8_IS_TRAIL (s[1])
                   && GS_UTF8_IS_TRAIL (s[2]) && GS_UTF8_IS_TRAIL (s[3]))
            {
              u = (s[0] & 0x07) << 18;
              u |= (s[1] & 0x3F) << 12;
              u |= (s[2] & 0x3F) << 6;
              u |= s[3] & 0x3F;
            }
          else
            {
              break;
            }
          s += count;
          if (u < 0x10000)
            {
              if (GS_UTF_IS_SURROGATE (u))
                break;
              if (dlen != 0)
                *d = u;
              d++;
            }
          else if (u < 0x10FFFF)
            {
              if (dlen != 0)
                {
                  d[0] = (u >> 10) + 0xD7C0;
                  d[1] = (u & 0x3FF) | 0xDC00;
                }
              d += 2;
            }
          else
            {
              break;
            }
        }
    }

  if (usedLen)
    *usedLen = ((UInt8 *) d) - ((UInt8 *) dstart);

  return s - sstart;
}

static UInt8 utf8[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };

CFIndex
GSUnicodeToUTF8 (const UniChar * s, CFIndex slen, UniChar lossChar, UInt8 * d,
                 CFIndex dlen, CFIndex * usedLen)
{
  const UniChar *sstart;
  const UniChar *slimit;
  UInt8 *dstart;
  UInt8 *dlimit;

  sstart = s;
  slimit = sstart + slen;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UTF32Char u;

      u = *s++;
      if (u < 0x80)
        {
          if (dlen != 0)
            *d = (UInt8) u;
          d++;
        }
      else
        {
          CFIndex count;

          if (GS_UTF_IS_SURROGATE (u) && GS_UTF_IS_LEAD_SURROGATE (u)
              && slimit - s > 0)
            {
              UTF16Char u16;

              u16 = *s++;
              if (GS_UTF_IS_TRAIL_SURROGATE (u16))
                u = GS_UTF16_GET_CHAR (u, u16);
              else if (lossChar)
                u = lossChar;
              else
                break;
            }
          else if (lossChar)
            {
              u = lossChar;
            }
          else
            {
              break;
            }
          count = GS_UTF8_LENGTH (u);
          if (count > 4)
            break;
          if (dlen != 0)
            {
              if (count < dlimit - d)
                break;
              switch (count)
                {
                case 4:
                  d[3] = (u & 0x3F) | 0x80;
                  u = u >> 6;
                case 3:
                  d[2] = (u & 0x3F) | 0x80;
                  u = u >> 6;
                case 2:
                  d[1] = (u & 0x3F) | 0x80;
                  u = u >> 6;
                case 1:
                  d[0] = (u & 0x3F) | utf8[count];
                }
            }
          d += count;
        }
    }

  if (usedLen)
    *usedLen = ((UInt8 *) d) - ((UInt8 *) dstart);

  return s - sstart;
}

CFIndex
GSUnicodeFromUTF32 (const UTF32Char * s, CFIndex slen, UniChar lossChar,
                    UniChar * d, CFIndex dlen, CFIndex * usedLen)
{
  const UTF32Char *sstart;
  const UTF32Char *slimit;
  UniChar *dstart;
  UniChar *dlimit;

  sstart = s;
  slimit = sstart + slen;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UTF32Char u;

      u = *s++;
      if (u < 0x10000)
        {
          if (GS_UTF_IS_SURROGATE (u))
            break;
          if (dlen != 0)
            *d = u;
          d++;
        }
      else
        {
          if (u >= 0x10FFFF)
            break;
          if (dlen != 0)
            {
              d[0] = (u >> 10) + 0xD7C0;
              d[1] = (u & 0x3FF) | 0xDC00;
            }
          d += 2;
        }
    }

  if (usedLen)
    *usedLen = ((UInt8 *) d) - ((UInt8 *) dstart);

  return s - sstart;
}

CFIndex
GSUnicodeToUTF32 (const UniChar * s, CFIndex slen, UniChar lossChar,
                  UTF32Char * d, CFIndex dlen, CFIndex * usedLen)
{
  const UniChar *sstart;
  const UniChar *slimit;
  UTF32Char *dstart;
  UTF32Char *dlimit;

  sstart = s;
  slimit = sstart + slen;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UTF32Char u;

      u = *s++;
      if (GS_UTF_IS_SURROGATE (u))
        {
          UTF16Char u16;

          if (slimit - s > 0 || !GS_UTF_IS_LEAD_SURROGATE (u))
            break;
          u16 = *s++;
          if (!GS_UTF_IS_TRAIL_SURROGATE (u))
            break;
          u = GS_UTF16_GET_CHAR (u, u16);
        }
      if (dlen != 0)
        *d = u;
      d++;
    }

  if (usedLen)
    *usedLen = ((UInt8 *) d) - ((UInt8 *) dstart);

  return s - sstart;
}

CFIndex
GSUnicodeFromNonLossyASCII (const char *s, CFIndex slen, UniChar lossChar,
                            UniChar * d, CFIndex dlen, CFIndex * usedLen)
{
  const char *sstart;
  const char *slimit;
  UniChar *dstart;
  UniChar *dlimit;

  sstart = s;
  slimit = sstart + slen;
  if (d == NULL)
    dlen = 0;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UniChar c;

      c = *s;
      if (c < 0x80 && c != '\\')
        {
          if (dlen != 0)
            *d = *s;
          d++;
          s++;
        }
      else if (c == '\\')
        {
          CFIndex convCount;

          c = s[1];
          if (c == '\\')
            {
              convCount = 2;
            }
          else if (CHAR_IS_OCTAL (c) && CHAR_IS_OCTAL (s[2])
                   && CHAR_IS_OCTAL (s[3]))
            {
              c = (s[1] - '0') << 6;
              c |= (s[2] - '0') << 3;
              c |= s[3] - '0';
              convCount = 4;
            }
          else if (c == 'u' && CHAR_IS_HEX (s[2]) && CHAR_IS_HEX (s[3])
                   && CHAR_IS_HEX (s[4]) && CHAR_IS_HEX (s[5]))
            {
              /* The first part of this equation gives us a value between
               * 0 and 9, the second part adds 9 to that if the char is
               * alphabetic.
               */
              c = ((s[5] & 0xF) + (s[5] >> 6) * 9) & 0x000F;
              c |= ((s[4] & 0xF) + (s[4] >> 6) * 9) << 4;
              c |= ((s[3] & 0xF) + (s[3] >> 6) * 9) << 8;
              c |= ((s[2] & 0xF) + (s[2] >> 6) * 9) << 12;
              convCount = 6;
            }
          else
            {
              break;
            }
          if (dlen != 0)
            *d = c;
          d++;
          s += convCount;
        }
      else
        {
          break;
        }
    }

  if (usedLen)
    *usedLen = d - dstart;

  return s - sstart;
}

static char base16[] = "0123456789ABCDEF";

CFIndex
GSUnicodeToNonLossyASCII (const UniChar * s, CFIndex slen, UniChar lossChar,
                          char *d, CFIndex dlen, CFIndex * usedLen)
{
  const UniChar *sstart;
  const UniChar *slimit;
  char *dstart;
  char *dlimit;

  sstart = s;
  slimit = sstart + slen;
  if (d == NULL)
    dlen = 0;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UniChar c;

      c = *s;
      if (c < 0x80 && c != '\\')
        {
          if (dlen != 0)
            *d = c;
          d++;
          s++;
        }
      else
        {
          char conv[6];
          CFIndex convCount;

          conv[0] = '\\';
          if (c == '\\')
            {
              conv[1] = '\\';
              convCount = 2;
            }
          else if (c <= 0x00FF)
            {
              conv[3] = (c & 0x7) + '0';
              conv[2] = ((c >> 3) & 0x7) + '0';
              conv[1] = ((c >> 6) & 0x7) + '0';
              convCount = 4;
            }
          else
            {
              conv[5] = base16[(c & 0xF)];
              conv[4] = base16[((c >> 4) & 0xF)];
              conv[3] = base16[((c >> 8) & 0xF)];
              conv[2] = base16[((c >> 12) & 0xF)];
              conv[1] = 'u';
              convCount = 6;
            }
          if (dlen != 0 && convCount < dlimit - d)
            {
              CFIndex i;
              for (i = 0; i < convCount; ++i)
                d[i] = conv[i];
            }
          d += convCount;
        }
    }

  if (usedLen)
    *usedLen = d - dstart;

  return s - sstart;
}

CFIndex
GSUnicodeFromISOLatin1 (const UInt8 * s, CFIndex slen, UniChar lossChar,
                        UniChar * d, CFIndex dlen, CFIndex * usedLen)
{
  const UInt8 *sstart;
  const UInt8 *slimit;
  UniChar *dstart;
  UniChar *dlimit;

  sstart = s;
  slimit = sstart + slen;
  if (d == NULL)
    dlen = 0;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UInt8 c;

      c = *s++;
      if (dlen != 0)
        *d = c;
      d++;
    }

  if (usedLen)
    *usedLen = d - dstart;

  return s - sstart;
}

CFIndex
GSUnicodeToISOLatin1 (const UniChar * s, CFIndex slen, UniChar lossChar,
                      UInt8 * d, CFIndex dlen, CFIndex * usedLen)
{
  const UniChar *sstart;
  const UniChar *slimit;
  UInt8 *dstart;
  UInt8 *dlimit;

  sstart = s;
  slimit = sstart + slen;
  if (d == NULL)
    dlen = 0;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UniChar u;

      u = *s++;
      if (u > 0xFF)
        {
          if (lossChar)
            u = lossChar;
          else
            break;
        }
      if (dlen != 0)
        *d = u;
      d++;
    }

  if (usedLen)
    *usedLen = d - dstart;

  return s - sstart;
}

CFIndex
GSFromUnicode (const UniChar * s, CFIndex slen,
               CFStringEncoding enc, UniChar lossChar,
               Boolean isExtRep, UInt8 * d, CFIndex dlen, CFIndex * usedDstLen)
{
  CFIndex converted;

  converted = 0;
  if (d == NULL)
    dlen = 0;

  if (enc == kCFStringEncodingUTF8)
    {
      if (isExtRep && dlen > 3)
        {
          *d++ = 0xEF;
          *d++ = 0xBB;
          *d++ = 0xBF;
          dlen -= 3;
        }
      converted = GSUnicodeToUTF8 (s, slen, lossChar, d, dlen, usedDstLen);
    }
  else if (enc == kCFStringEncodingUTF16
           || enc == kCFStringEncodingUTF16BE
           || enc == kCFStringEncodingUTF16LE)
    {
      UniChar *dst;
      CFIndex copyLength;

      dst = (UniChar *) d;
      if (isExtRep && enc == kCFStringEncodingUTF16 && dlen >= 2)
        {
          *dst = GS_UTF16_BOM;
          dst++;
          dlen -= 2;
        }

      copyLength =
        (dlen <= slen * sizeof (UniChar)) ? dlen : (slen * sizeof (UniChar));
      memcpy (dst, s, copyLength);
      if (enc == GS_UTF16_ENCODING_TO_SWAP)
        {
          UniChar *end;

          end = dst + converted;
          while (dst < end)
            {
              *dst = CFSwapInt16 (*dst);
              ++dst;
            }
        }
      if (usedDstLen)
        *usedDstLen = slen + (isExtRep ? 2 : 0);
      converted = copyLength / sizeof (UniChar);
    }
  else if (enc == kCFStringEncodingUTF32
           || enc == kCFStringEncodingUTF32BE
           || enc == kCFStringEncodingUTF32LE)
    {
      UTF32Char *dst;

      dst = (UTF32Char *) d;
      if (isExtRep && enc == kCFStringEncodingUTF32 && dlen >= 4)
        {
          *dst++ = GS_UTF32_BOM;
          dlen -= 4;
        }
      /* round to the nearest multiple of 4 */
      dlen &= ~0x3;
      converted =
        GSUnicodeToUTF32 (s, slen / sizeof (UTF32Char), lossChar,
                          (UTF32Char *) d, dlen, usedDstLen);
      if (enc == GS_UTF32_ENCODING_TO_SWAP && dlen != 0)
        {
          UTF32Char *cur;
          UTF32Char *end;

          cur = (UTF32Char *) d;
          end = (UTF32Char *) (d + dlen);
          while (cur < end)
            {
              *cur = CFSwapInt32 (*cur);
              cur++;
            }
        }
    }
  else if (enc == kCFStringEncodingASCII)
    {
      UInt8 *dstart;
      const UInt8 *dlimit;
      const UniChar *sstart;
      const UniChar *slimit;

      sstart = s;
      slimit = s + slen;
      dstart = (UInt8 *) d;
      dlimit = dstart + dlen;
      while (s < slimit && (dlen == 0 || d < dlimit))
        {
          if (*s < 0x80)
            {
              *d++ = (UInt8) * s++;
            }
          else
            {
              if (!lossChar)
                break;
              *d++ = (UInt8) lossChar;
              s++;
            }
        }
      if (usedDstLen)
        *usedDstLen = (d - dstart);

      converted = (CFIndex) (s - sstart);
    }
  else if (enc == kCFStringEncodingNonLossyASCII)
    {
      converted = GSUnicodeToNonLossyASCII (s, slen, lossChar, (char *) d, dlen,
                                            usedDstLen);
    }
  else if (enc == kCFStringEncodingISOLatin1)
    {
      converted = GSUnicodeToISOLatin1 (s, slen, lossChar, d, dlen, usedDstLen);
    }
#if 0
  else
    {
      UConverter *ucnv;

      ucnv = GSStringOpenConverter (enc, lossChar);
      if (ucnv)
        {
          char *target;
          const char *targetLimit;
          const UniChar *source;
          const UniChar *sourceLimit;
          UErrorCode err = U_ZERO_ERROR;

          target = (char *) d;
          targetLimit = target + dlen;
          source = s;
          sourceLimit = source + slen;

          ucnv_fromUnicode (ucnv, &target, targetLimit, &source, sourceLimit,
                            NULL, true, &err);
          converted = (CFIndex) (target - (char *) d);
          if (usedDstLen)
            {
              *usedDstLen = converted;
              if (err == U_BUFFER_OVERFLOW_ERROR)
                {
                  char ibuffer[256];    /* Arbitrary buffer size */

                  targetLimit = ibuffer + 255;
                  do
                    {
                      target = ibuffer;
                      err = U_ZERO_ERROR;
                      ucnv_fromUnicode (ucnv, &target, targetLimit, &source,
                                        sourceLimit, NULL, true, &err);
                      *usedDstLen += (CFIndex) (target - ibuffer);
                    }
                  while (err == U_BUFFER_OVERFLOW_ERROR);
                }
            }

          GSStringCloseConverter (ucnv);
        }
    }
#endif

  return converted;
}

CFIndex
GSToUnicode (const UInt8 * s, CFIndex slen, CFStringEncoding enc,
             UniChar lossChar, Boolean isExtRep, UniChar * d, CFIndex dlen,
             CFIndex * usedDstLen)
{
  CFIndex converted;

  converted = 0;
  if (d == NULL)
    dlen = 0;

  if (enc == kCFStringEncodingUTF8)
    {
      if (isExtRep && slen > 3
          && (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF))
        s += 3;

      converted = GSUnicodeFromUTF8 (s, slen, lossChar, d, dlen, usedDstLen);
    }
  else if (enc == kCFStringEncodingUTF16
           || enc == kCFStringEncodingUTF16BE
           || enc == kCFStringEncodingUTF16LE)
    {
      const UniChar *src;
      CFIndex copyLength;
      Boolean swap;

      src = (const UniChar *) s;
      swap = false;

      if (enc == kCFStringEncodingUTF16)
        {
          UniChar bom;

          bom = (*src == 0xFEFF || *src == 0xFFFE) ? *src++ : 0;
#if WORDS_BIGENDIAN
          if (bom == 0xFFFE)
#else
          if (bom == 0xFEFF)
#endif
            swap = true;
        }
      else if (enc == GS_UTF16_ENCODING_TO_SWAP)
        {
          swap = true;
        }

      if (swap && slen != 0)
        {
          UniChar *cur;
          UniChar *end;

          cur = (UniChar *) s;
          end = (UniChar *) (s + slen);
          while (cur < end)
            {
              *cur = CFSwapInt16 (*cur);
              ++cur;
            }
        }
      copyLength =
        (dlen * sizeof (UniChar) <= slen) ? (dlen * sizeof (UniChar)) : slen;
      memcpy (d, s, copyLength);
      if (usedDstLen)
        *usedDstLen = slen;
      converted = copyLength / sizeof (UniChar);
    }
  else if (enc == kCFStringEncodingUTF32
           || enc == kCFStringEncodingUTF32BE
           || enc == kCFStringEncodingUTF32LE)
    {
      const UTF32Char *src;
      Boolean swap;

      src = (const UTF32Char *) s;
      swap = false;
      if (enc == kCFStringEncodingUTF32)
        {
          UTF32Char bom;

          bom = (*src == 0x0000FEFF || *src == 0xFFFE0000) ? *src++ : 0;
#if WORDS_BIGENDIAN
          if (bom == 0xFFFE0000)
#else
          if (bom == 0x0000FEFF)
#endif
            swap = true;
        }
      else if (enc == GS_UTF32_ENCODING_TO_SWAP)
        {
          swap = true;
        }
      /* round to the nearest multiple of 4 */
      slen &= ~0x3;
      if (swap && slen != 0)
        {
          UTF32Char *cur;
          UTF32Char *end;

          cur = (UTF32Char *) s;
          end = (UTF32Char *) (s + slen);
          while (cur < end)
            {
              *cur = CFSwapInt32 (*cur);
              cur++;
            }
        }
      converted =
        GSUnicodeFromUTF32 ((const UTF32Char *) s, slen / sizeof (UTF32Char),
                            lossChar, d, dlen, usedDstLen);
    }
  else if (enc == kCFStringEncodingASCII)
    {
      UniChar *dstart;
      const UniChar *dlimit;

      if (dlen > slen)
        dlen = slen;
      dstart = d;
      dlimit = d + dlen;
      while (d < dlimit)
        *d++ = *s++;
      if (usedDstLen)
        *usedDstLen = slen;
      converted = d - dstart;
    }
  else if (enc == kCFStringEncodingNonLossyASCII)
    {
      converted =
        GSUnicodeFromNonLossyASCII ((const char *) s, slen, lossChar, d, dlen,
                                    usedDstLen);
    }
  else if (enc == kCFStringEncodingISOLatin1)
    {
      converted = GSUnicodeFromISOLatin1 (s, slen, lossChar, d, dlen,
                                          usedDstLen);
    }
#if 0
  else
    {
      UConverter *ucnv;

      ucnv = GSStringOpenConverter (enc, 0);
      converted = 0;
      if (ucnv)
        {
          UniChar *target;
          const UniChar *targetLimit;
          const char *source;
          const char *sourceLimit;
          UErrorCode err = U_ZERO_ERROR;

          target = d;
          targetLimit = target + dlen;
          source = (char *) s;
          sourceLimit = source + slen;

          ucnv_toUnicode (ucnv, &target, targetLimit, &source, sourceLimit,
                          NULL, true, &err);
          converted = (CFIndex) (target - d);
          if (usedDstLen)
            {
              *usedDstLen = converted;
              if (err == U_BUFFER_OVERFLOW_ERROR)
                {
                  UniChar ibuffer[256]; /* Arbitrary buffer size */

                  targetLimit = ibuffer + 255;
                  do
                    {
                      target = ibuffer;
                      err = U_ZERO_ERROR;
                      ucnv_toUnicode (ucnv, &target, targetLimit, &source,
                                      sourceLimit, NULL, true, &err);
                      *usedDstLen +=
                        (CFIndex) ((char *) target - (char *) ibuffer);
                    }
                  while (err == U_BUFFER_OVERFLOW_ERROR);
                }
            }

          GSStringCloseConverter (ucnv);
        }
    }
#endif

  return converted;
}
