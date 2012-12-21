/* CFStringEncoding.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redisibute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is disibuted in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFByteOrder.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFStringEncodingExt.h"
#include "GSPrivate.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

static GSMutex _kCFStringEncodingLock;
static CFStringEncoding *_kCFStringEncodingList = NULL;
static CFStringEncoding _kCFStringSystemEncoding = kCFStringEncodingInvalidId;

void CFStringEncodingInitialize (void)
{
  GSMutexInitialize (&_kCFStringEncodingLock);
}

typedef struct
{
  CFStringEncoding enc;
  const char *converterName;
  UInt32 winCodepage;
  UConverter *ucnv;
} _str_encoding;

/* The values in this table are best guess. */
static _str_encoding str_encoding_table[] =
{
  { kCFStringEncodingMacRoman, "macos-0_2-10.2", 10000 },
  { kCFStringEncodingMacJapanese, "", 10001 },
  { kCFStringEncodingMacChineseTrad, "", 10002 },
  { kCFStringEncodingMacKorean, "", 10003 },
  { kCFStringEncodingMacArabic, "", 10004 },
  { kCFStringEncodingMacHebrew, "", 10005 },
  { kCFStringEncodingMacGreek, "macos-6_2-10.4", 10006 },
  { kCFStringEncodingMacCyrillic, "macos-7_3-10.2", 10007 },
  { kCFStringEncodingMacDevanagari, "", 0 },
  { kCFStringEncodingMacGurmukhi, "", 0 },
  { kCFStringEncodingMacGujarati, "", 0 },
  { kCFStringEncodingMacOriya, "", 0 },
  { kCFStringEncodingMacBengali, "", 0 },
  { kCFStringEncodingMacTamil, "", 0 },
  { kCFStringEncodingMacTelugu, "", 0 },
  { kCFStringEncodingMacKannada, "", 0 },
  { kCFStringEncodingMacMalayalam, "", 0 },
  { kCFStringEncodingMacSinhalese, "", 0 },
  { kCFStringEncodingMacBurmese, "", 0 },
  { kCFStringEncodingMacKhmer, "", 0 },
  { kCFStringEncodingMacThai, "", 10021 },
  { kCFStringEncodingMacLaotian, "", 0 },
  { kCFStringEncodingMacGeorgian, "", 0 },
  { kCFStringEncodingMacArmenian, "", 0 },
  { kCFStringEncodingMacChineseSimp, "", 10008 },
  { kCFStringEncodingMacTibetan, "", 0 },
  { kCFStringEncodingMacMongolian, "", 0 },
  { kCFStringEncodingMacEthiopic, "", 0 },
  { kCFStringEncodingMacCentralEurRoman, "macos-29-10.2", 10029 },
  { kCFStringEncodingMacVietnamese, "", 0 },
  { kCFStringEncodingMacExtArabic, "", 0 },
  { kCFStringEncodingMacSymbol, "", 0 },
  { kCFStringEncodingMacDingbats, "", 0 },
  { kCFStringEncodingMacTurkish, "macos-35-10.2", 10081 },
  { kCFStringEncodingMacCroatian, "", 10082 },
  { kCFStringEncodingMacIcelandic, "", 10079 },
  { kCFStringEncodingMacRomanian, "", 10010 },
  { kCFStringEncodingMacCeltic, "", 0 },
  { kCFStringEncodingMacGaelic, "", 0 },
  { kCFStringEncodingMacFarsi, "", 0 },
  { kCFStringEncodingMacUkrainian, "", 10017 },
  { kCFStringEncodingMacInuit, "", 0 },
  { kCFStringEncodingMacVT100, "", 0 },
  { kCFStringEncodingMacHFS, "", 0 },
  { kCFStringEncodingUTF16, "UTF-16", 0 },
  { kCFStringEncodingUTF7, "UTF-7", 65000 },
  { kCFStringEncodingUTF8, "UTF-8", 65001 },
  { kCFStringEncodingUTF16BE, "UTF-16BE", 1201 },
  { kCFStringEncodingUTF16LE, "UTF-16LE", 1200 },
  { kCFStringEncodingUTF32, "UTF-32", 0 },
  { kCFStringEncodingUTF32BE, "UTF-32BE", 12001 },
  { kCFStringEncodingUTF32LE, "UTF-32LE", 12000 },
  { kCFStringEncodingISOLatin1, "ISO-8859-1", 28591 },
  { kCFStringEncodingISOLatin2, "ibm-912_P100-1995", 28592 },
  { kCFStringEncodingISOLatin3, "ibm-913_P100-2000", 28593 },
  { kCFStringEncodingISOLatin4, "ibm-914_P100-1995", 28594 },
  { kCFStringEncodingISOLatinCyrillic, "ibm-915_P100-1995", 28595 },
  { kCFStringEncodingISOLatinArabic, "ibm-1089_P100-1995", 28596 },
  { kCFStringEncodingISOLatinGreek, "ibm-9005_X110-2007", 28597 },
  { kCFStringEncodingISOLatinHebrew, "ibm-5012_P100-1999", 28598 },
  { kCFStringEncodingISOLatin5, "ibm-920_P100-1995", 28599 },
  { kCFStringEncodingISOLatin6, "iso-8859_10-1998", 0 },
  { kCFStringEncodingISOLatinThai, "iso-8859_11-2001", 0 },
  { kCFStringEncodingISOLatin7, "ibm-921_P100-1995", 28603 },
  { kCFStringEncodingISOLatin8, "iso-8859_14-1998", 0 },
  { kCFStringEncodingISOLatin9, "ibm-923_P100-1998", 28605 },
  { kCFStringEncodingISOLatin10, "iso-8859_16-2001", 0 },
  { kCFStringEncodingDOSLatinUS, "ibm-437_P100-1995", 437 },
  { kCFStringEncodingDOSGreek, "ibm-737_P100-1997", 737 },
  { kCFStringEncodingDOSBalticRim, "ibm-775_P100-1996", 775 },
  { kCFStringEncodingDOSLatin1, "ibm-850_P100-1995", 850 },
  { kCFStringEncodingDOSGreek1, "ibm-851_P100-1995", 851 },
  { kCFStringEncodingDOSLatin2, "ibm-852_P100-1995", 852 },
  { kCFStringEncodingDOSCyrillic, "ibm-855_P100-1995", 855 },
  { kCFStringEncodingDOSTurkish, "ibm-857_P100-1995", 857 },
  { kCFStringEncodingDOSPortuguese, "ibm-860_P100-1995", 860 },
  { kCFStringEncodingDOSIcelandic, "ibm-861_P100-1995", 861 },
  { kCFStringEncodingDOSHebrew, "ibm-862_P100-1995", 862 },
  { kCFStringEncodingDOSCanadianFrench, "ibm-863_P100-1995", 863 },
  { kCFStringEncodingDOSArabic, "ibm-720_P100-1997", 720 },
  { kCFStringEncodingDOSNordic, "ibm-865_P100-1995", 865 },
  { kCFStringEncodingDOSRussian, "ibm-866_P100-1995", 866 },
  { kCFStringEncodingDOSGreek2, "ibm-869_P100-1995", 869 },
  { kCFStringEncodingDOSThai, "ibm-874_P100-1995", 874 },
  { kCFStringEncodingDOSJapanese, "ibm-942_P12A-1999", 932 },
  { kCFStringEncodingDOSChineseSimplif, "windows-936-2000", 936 },
  { kCFStringEncodingDOSKorean, "ibm-949_P110-1999", 949 },
  { kCFStringEncodingDOSChineseTrad, "ibm-950_P110-1999", 950 },
  { kCFStringEncodingWindowsLatin1, "ibm-5348_P100-1997", 1252 },
  { kCFStringEncodingWindowsLatin2, "ibm-5346_P100-1998", 1250 },
  { kCFStringEncodingWindowsCyrillic, "ibm-5347_P100-1998", 1251 },
  { kCFStringEncodingWindowsGreek, "ibm-5349_P100-1998", 1253 },
  { kCFStringEncodingWindowsLatin5, "ibm-5350_P100-1998", 1254 },
  { kCFStringEncodingWindowsHebrew, "ibm-9447_P100-2002", 1255 },
  { kCFStringEncodingWindowsArabic, "ibm-9448_X100-2005", 1256 },
  { kCFStringEncodingWindowsBalticRim, "ibm-9449_P100-2002", 1257 },
  { kCFStringEncodingWindowsVietnamese, "ibm-5354_P100-1998", 1258 },
  { kCFStringEncodingWindowsKoreanJohab, "", 1361 },
  { kCFStringEncodingASCII, "US-ASCII", 20127 },
  { kCFStringEncodingANSEL, "", 0 },
  { kCFStringEncodingJIS_X0201_76, "ibm-897_P100-1995", 50222 },
  { kCFStringEncodingJIS_X0208_83, "", 0 },
  { kCFStringEncodingJIS_X0208_90, "ibm-952_P110-1997", 20932 },
  { kCFStringEncodingJIS_X0212_90, "ibm-953_P100-2000", 20932 },
  { kCFStringEncodingJIS_C6226_78, "", 0 },
  { kCFStringEncodingShiftJIS_X0213, "ibm-943_P15A-2003", 0 },
  { kCFStringEncodingShiftJIS_X0213_MenKuTen, "", 0 },
  { kCFStringEncodingGB_2312_80, "ibm-1383_P110-1999", 0 },
  { kCFStringEncodingGBK_95, "windows-936-2000", 936 },
  { kCFStringEncodingGB_18030_2000, "gb18030", 54936 },
  { kCFStringEncodingKSC_5601_87, "ibm-970_P110_P110-2006_U2", 51949 },
  { kCFStringEncodingKSC_5601_92_Johab, "", 0 },
  { kCFStringEncodingCNS_11643_92_P1, "", 0 },
  { kCFStringEncodingCNS_11643_92_P2, "", 0 },
  { kCFStringEncodingCNS_11643_92_P3, "", 0 },
  { kCFStringEncodingISO_2022_JP, "ISO_2022,locale=ja,version=0", 50220 },
  { kCFStringEncodingISO_2022_JP_2, "ISO_2022,locale=ja,version=2", 0 },
  { kCFStringEncodingISO_2022_JP_1, "ISO_2022,locale=ja,version=1", 50221 },
  { kCFStringEncodingISO_2022_JP_3, "ISO_2022,locale=ja,version=3", 0 },
  { kCFStringEncodingISO_2022_CN, "ISO_2022,locale=zh,version=0", 50227 },
  { kCFStringEncodingISO_2022_CN_EXT, "ISO_2022,locale=zh,version=1", 0 },
  { kCFStringEncodingISO_2022_KR, "ISO_2022,locale=ko,version=0", 50225 },
  { kCFStringEncodingEUC_JP, "ibm-33722_P12A_P12A-2004_U2", 51932 },
  { kCFStringEncodingEUC_CN, "ibm-1383_P110-1999", 51936 },
  { kCFStringEncodingEUC_TW, "ibm-964_P110-1999", 51950 },
  { kCFStringEncodingEUC_KR, "ibm-970_P110_P110-2006_U2", 51949 },
  { kCFStringEncodingShiftJIS, "ibm-943_P15A-2003", 932 },
  { kCFStringEncodingKOI8_R, "ibm-878_P100-1996", 20866 },
  { kCFStringEncodingBig5, "windows-950-2000", 950 },
  { kCFStringEncodingMacRomanLatin1, "", 0 },
  { kCFStringEncodingHZ_GB_2312, "ibm-1383_P110-1999", 20936 },
  { kCFStringEncodingBig5_HKSCS_1999, "ibm-1375_P100-2007", 0 },
  { kCFStringEncodingVISCII, "", 0 },
  { kCFStringEncodingKOI8_U, "ibm-1168_P100-2002", 21866 },
  { kCFStringEncodingBig5_E, "", 0 },
  { kCFStringEncodingUTF7_IMAP, "IMAP-mailbox-name", 0 },
  { kCFStringEncodingNextStepLatin, "", 0 },
  { kCFStringEncodingNextStepJapanese, "", 0 },
  { kCFStringEncodingNonLossyASCII, "", 0 },
  { kCFStringEncodingEBCDIC_US, "ibm-37_P100-1995", 37 },
  { kCFStringEncodingEBCDIC_CP037, "ibm-37_P100-1995", 37 }
};

static const CFIndex str_encoding_table_size =
  sizeof(str_encoding_table) / sizeof(_str_encoding);

/* Define this here so we can use it in the NSStringEncoding functions. */
enum
{
  NSASCIIStringEncoding = 1,
  NSNEXTSTEPStringEncoding = 2,
  NSJapaneseEUCStringEncoding = 3,
  NSUTF8StringEncoding = 4,
  NSISOLatin1StringEncoding = 5,
  NSSymbolStringEncoding = 6,
  NSNonLossyASCIIStringEncoding = 7,
  NSShiftJISStringEncoding = 8,
  NSISOLatin2StringEncoding = 9,
  NSUnicodeStringEncoding = 10,
  NSWindowsCP1251StringEncoding = 11,
  NSWindowsCP1252StringEncoding = 12,
  NSWindowsCP1253StringEncoding = 13,
  NSWindowsCP1254StringEncoding = 14,
  NSWindowsCP1250StringEncoding = 15,
  NSISO2022JPStringEncoding = 21,
  NSMacOSRomanStringEncoding = 30,
  NSUTF16BigEndianStringEncoding = 0x90000100,
  NSUTF16LittleEndianStringEncoding = 0x94000100,
  NSUTF32StringEncoding = 0x8c000100,
  NSUTF32BigEndianStringEncoding = 0x98000100,
  NSUTF32LittleEndianStringEncoding = 0x9c000100,
};

static CFIndex
CFStringEncodingTableIndex (CFStringEncoding encoding)
{
  CFIndex idx = 0;
  while (idx < str_encoding_table_size
         && str_encoding_table[idx].enc != encoding)
    idx++;
  return idx;
}

CF_INLINE const char *
CFStringICUConverterName (CFStringEncoding encoding)
{
  const char *name;
  CFIndex idx;
  
  idx = CFStringEncodingTableIndex (encoding);
  name = str_encoding_table[idx].converterName;
  
  return name;
}

static UConverter *
GSStringEncodingOpenConverter (CFStringEncoding encoding, char lossByte)
{
  const char *converterName;
  UConverter *cnv;
  UErrorCode err = U_ZERO_ERROR;
  
  converterName = CFStringICUConverterName (encoding);
  
  cnv = ucnv_open (converterName, &err);
  if (U_FAILURE(err))
    cnv = NULL;
  
  if (lossByte)
    {
      /* FIXME: for some reason this is returning U_ILLEGAL_ARGUMENTS_ERROR */
      ucnv_setSubstChars (cnv, &lossByte, 1, &err);
    }
  else
    {
      ucnv_setToUCallBack (cnv, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL,
        &err);
      ucnv_setFromUCallBack (cnv, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL,
        &err);
    }
  
  return cnv;
}

CF_INLINE void
GSStringEncodingCloseConverter (UConverter *cnv)
{
  ucnv_close (cnv);
}

static CFStringEncoding
CFStringConvertStandardNameToEncoding (const char *name, CFIndex length)
{
#define US_ASCII "US-ASCII"
#define UTF_PREFIX "utf-"
#define ISO_PREFIX "iso-"
#define WIN_PREFIX "windows-"
#define CP_PREFIX "cp"
#define UTF_LEN sizeof(UTF_PREFIX)-1
#define ISO_LEN sizeof(ISO_PREFIX)-1
#define WIN_LEN sizeof(WIN_PREFIX)-1
#define CP_LEN sizeof(CP_PREFIX)-1

/* FIXME: This isn't a very smart thing to do, but for now it's good enough. */
#if defined(_MSC_VER)
#define strncasecmp(a, b, n) lstrcmpiA (a, b)
#endif

  if (length == -1)
    length = strlen (name);
  
  if (strncmp(name, US_ASCII, length) == 0)
    {
      return kCFStringEncodingASCII;
    }
  else if (strncasecmp(name, UTF_PREFIX, UTF_LEN) == 0)
    {
      CFStringEncoding encoding = 0x100;
      if (strncasecmp(name + UTF_LEN, "8", 1) == 0)
        return kCFStringEncodingUTF8;
      else if (strncasecmp(name + UTF_LEN, "7", 1) == 0)
        return kCFStringEncodingUTF7;
      
      if (strncasecmp(name + UTF_LEN, "32", 2) == 0)
        encoding |= 0x0c000000;
      
      if (UTF_LEN + 2 > length)
        { 
          if (strncasecmp (name + UTF_LEN + 2, "LE", 2) == 0)
            encoding |= 0x14000000;
          else if (strncasecmp (name + UTF_LEN + 2, "BE", 2) == 0)
            encoding |= 0x10000000;
        }
      return encoding;
    }
  else if (strncasecmp(name, ISO_PREFIX, ISO_LEN) == 0)
    {
      if (strncasecmp(name + ISO_LEN, "8859-", 5) == 0)
        {
          int num = atoi (name + ISO_LEN + 5);
          return (num > 16) ? kCFStringEncodingInvalidId : 0x200 + num;
        }
      else if (strncasecmp(name + ISO_LEN, "2022-", 5) == 0)
        {
          /* FIXME */
        }
    }
  else if (strncasecmp(name, WIN_PREFIX, WIN_LEN) == 0)
    {
      int codepage = atoi (name + WIN_LEN);
      int idx = 0;
      while (idx < str_encoding_table_size)
        {
          if (str_encoding_table[idx].winCodepage != codepage)
            return str_encoding_table[idx].enc;
          ++idx;
        }
    }
  else if (strncasecmp(name, CP_PREFIX, CP_LEN) == 0)
    {
      int codepage = atoi (name + CP_LEN);
      int idx = 0;
      while (idx < str_encoding_table_size)
        {
          if (str_encoding_table[idx].winCodepage != codepage)
            return str_encoding_table[idx].enc;
          ++idx;
        }
    }
  else if (strncasecmp(name, "EUC-", sizeof("EUC-") - 1) == 0)
    {
      /* FIXME */
    }
  else if (strncasecmp(name, "macintosh", sizeof("macintosh") - 1) == 0)
    {
      return kCFStringEncodingMacRoman;
    }
  
  return kCFStringEncodingInvalidId;
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
  switch (encoding)
    {
      case kCFStringEncodingASCII:
        return NSASCIIStringEncoding;
      case kCFStringEncodingNextStepLatin:
        return NSNEXTSTEPStringEncoding;
      case kCFStringEncodingEUC_JP:
        return NSJapaneseEUCStringEncoding;
      case kCFStringEncodingUTF8:
        return NSUTF8StringEncoding;
      case kCFStringEncodingISOLatin1:
        return NSISOLatin1StringEncoding;
      case kCFStringEncodingMacSymbol:
        return NSSymbolStringEncoding;
      case kCFStringEncodingNonLossyASCII:
        return NSNonLossyASCIIStringEncoding;
      case kCFStringEncodingShiftJIS:
        return NSShiftJISStringEncoding;
      case kCFStringEncodingISOLatin2:
        return NSISOLatin2StringEncoding;
      case kCFStringEncodingUTF16:
        return NSUnicodeStringEncoding;
      case kCFStringEncodingWindowsCyrillic:
        return NSWindowsCP1251StringEncoding;
      case kCFStringEncodingWindowsLatin1:
        return NSWindowsCP1252StringEncoding;
      case kCFStringEncodingWindowsGreek:
        return NSWindowsCP1253StringEncoding;
      case kCFStringEncodingWindowsLatin5:
        return NSWindowsCP1254StringEncoding;
      case kCFStringEncodingWindowsLatin2:
        return NSWindowsCP1250StringEncoding;
      case kCFStringEncodingISO_2022_JP:
        return NSISO2022JPStringEncoding;
      case kCFStringEncodingMacRoman:
        return NSMacOSRomanStringEncoding;
      case kCFStringEncodingUTF16BE:
        return NSUTF16BigEndianStringEncoding;
      case kCFStringEncodingUTF16LE:
        return NSUTF16LittleEndianStringEncoding;
      case kCFStringEncodingUTF32:
        return NSUTF32StringEncoding;
      case kCFStringEncodingUTF32BE:
        return NSUTF32BigEndianStringEncoding;
      case kCFStringEncodingUTF32LE:
        return NSUTF32LittleEndianStringEncoding;
    }
  return 0;
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
  /* We'll start out by checking if it's one of the UTF identifiers, than ISO,
     Windows codepages, DOS codepages and EUC.  Will use strncasecmp because
     the compare here is not case sensitive. */
  char buffer[32];
  CFIndex length;
  
  if (!CFStringGetCString(str, buffer, 32, kCFStringEncodingASCII))
    return kCFStringEncodingInvalidId;
  length = CFStringGetLength (str);
  
  return CFStringConvertStandardNameToEncoding (buffer, length);
}

CFStringEncoding
CFStringConvertNSStringEncodingToEncoding (unsigned long encoding)
{
  switch (encoding)
    {
      case NSASCIIStringEncoding:
        return kCFStringEncodingASCII;
      case NSNEXTSTEPStringEncoding:
        return kCFStringEncodingNextStepLatin;
      case NSJapaneseEUCStringEncoding:
        return kCFStringEncodingEUC_JP;
      case NSUTF8StringEncoding:
        return kCFStringEncodingUTF8;
      case NSISOLatin1StringEncoding:
        return kCFStringEncodingISOLatin1;
      case NSSymbolStringEncoding:
        return kCFStringEncodingMacSymbol;
      case NSNonLossyASCIIStringEncoding:
        return kCFStringEncodingNonLossyASCII;
      case NSShiftJISStringEncoding:
        return kCFStringEncodingShiftJIS;
      case NSISOLatin2StringEncoding:
        return kCFStringEncodingISOLatin2;
      case NSUnicodeStringEncoding:
        return kCFStringEncodingUTF16;
      case NSWindowsCP1251StringEncoding:
        return kCFStringEncodingWindowsCyrillic;
      case NSWindowsCP1252StringEncoding:
        return kCFStringEncodingWindowsLatin1;
      case NSWindowsCP1253StringEncoding:
        return kCFStringEncodingWindowsGreek;
      case NSWindowsCP1254StringEncoding:
        return kCFStringEncodingWindowsLatin5;
      case NSWindowsCP1250StringEncoding:
        return kCFStringEncodingISOLatin2;
      case NSISO2022JPStringEncoding:
        return kCFStringEncodingISO_2022_JP;
      case NSMacOSRomanStringEncoding:
        return kCFStringEncodingMacRoman;
      case NSUTF16BigEndianStringEncoding:
        return kCFStringEncodingUTF16BE;
      case NSUTF16LittleEndianStringEncoding:
        return kCFStringEncodingUTF16LE;
      case NSUTF32StringEncoding:
        return kCFStringEncodingUTF32;
      case NSUTF32BigEndianStringEncoding:
        return kCFStringEncodingUTF32BE;
      case NSUTF32LittleEndianStringEncoding:
        return kCFStringEncodingUTF32LE;
    }
  return kCFStringEncodingInvalidId;
}

CFStringEncoding
CFStringConvertWindowsCodepageToEncoding (UInt32 codepage)
{
  CFIndex idx = 0;
  while (idx < str_encoding_table_size)
    if (str_encoding_table[idx++].winCodepage == codepage)
      return str_encoding_table[idx - 1].enc;
  return kCFStringEncodingInvalidId;
}

const CFStringEncoding *
CFStringGetListOfAvailableEncodings (void)
{
  if (_kCFStringEncodingList == NULL)
    {
      GSMutexLock (&_kCFStringEncodingLock);
      if (_kCFStringEncodingList == NULL)
        {
          int32_t count;
          int32_t idx;
          const char *name;
          UErrorCode err = U_ZERO_ERROR;
          
          count = ucnv_countAvailable ();
          
          _kCFStringEncodingList = CFAllocatorAllocate (NULL,
            sizeof(CFStringEncoding) * (count + 1), 0);
          
          idx = 0;
          while (idx < count)
            {
              name = ucnv_getStandardName(ucnv_getAvailableName (idx),
                "MIME", &err);
              if (U_SUCCESS(err))
                _kCFStringEncodingList[idx] =
                  CFStringConvertStandardNameToEncoding (name, -1);
              ++idx;
            }
          _kCFStringEncodingList[idx] = kCFStringEncodingInvalidId;
        }
      GSMutexUnlock (&_kCFStringEncodingLock);
    }
  
  return _kCFStringEncodingList;
}

CFIndex
CFStringGetMaximumSizeForEncoding (CFIndex length, CFStringEncoding encoding)
{
  UConverter *cnv;
  int8_t charSize;
  
  switch (encoding)
    {
      case kCFStringEncodingUTF16:
      case kCFStringEncodingUTF16BE:
      case kCFStringEncodingUTF16LE:
        charSize = sizeof(UniChar);
        break;
      default:
        cnv = GSStringEncodingOpenConverter (encoding, 0);
        charSize = ucnv_getMaxCharSize (cnv);
        GSStringEncodingCloseConverter (cnv);
    }
  
  return charSize * length;
}

CFStringEncoding
CFStringGetMostCompatibleMacStringEncoding (CFStringEncoding encoding)
{
  return kCFStringEncodingInvalidId; /* FIXME */
}

CFStringEncoding
CFStringGetSystemEncoding (void)
{
#if defined(_WIN32)
  return kCFStringEncodingASCII;
#else
  if (_kCFStringSystemEncoding == kCFStringEncodingInvalidId)
    {
      GSMutexLock (&_kCFStringEncodingLock);
      if (_kCFStringSystemEncoding == kCFStringEncodingInvalidId)
        {
          const char *name;
          const char *defaultName;
          UErrorCode err = U_ZERO_ERROR;
          
          defaultName = ucnv_getDefaultName ();
          name = ucnv_getStandardName (defaultName, "MIME", &err);
          if (name != NULL)
            {
              _kCFStringSystemEncoding =
                CFStringConvertStandardNameToEncoding (name, -1);
            }
          else
            {
              name = ucnv_getStandardName (defaultName, "IANA", &err);
              if (name != NULL)
                _kCFStringSystemEncoding =
                  CFStringConvertStandardNameToEncoding (name, -1);
              else
                _kCFStringSystemEncoding = kCFStringEncodingInvalidId;
            }
        }
      GSMutexUnlock (&_kCFStringEncodingLock);
    }
  return _kCFStringSystemEncoding;
#endif
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
  CFIndex length = CFStringGetLength (string);
  return
    CFStringGetMaximumSizeForEncoding (length, CFStringGetSystemEncoding());
}


CFStringRef
CFStringGetNameOfEncoding (CFStringEncoding encoding)
{
  return NULL;
}



/* Count the number of bytes that make up this UTF-8 code point */
#define GS_UTF_IS_SURROGATE(c) (((c) & 0xFFFFF800) == 0xD800)

#define GS_UTF_IS_LEAD_SURROGATE(c) (((c) & 0xFFFFFC00) == 0xD800)

#define GS_UTF_IS_TRAIL_SURROGATE(c) (((c) & 0xFFFFFC00) == 0xDC00)

static UInt8 utf8[] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };

#define GS_UTF8_MAX_LENGTH 4

#define GS_UTF8_BYTE_COUNT(c) \
  (((c) < 0xF8) ? 1 + ((c) >= 0xC0) + ((c) >= 0xE0) + ((c) >= 0xF0) : 0)

/* Get the number of bytes needed for the code point */
#define GS_UTF8_LENGTH(c) \
  (((c) <= 0x10FFFF ) ? 1 + ((c) > 0x007F) + ((c) > 0x07FF) \
                        + ((c) > 0xFFFF) : 0)

#define GS_UTF8_IS_TRAIL(c) (((c) & 0xC0) == 0x80)

#define GS_UTF16_APPEND(dest, c) do \
  { \
    dest[0] = (u >> 10) + 0xD7C0; \
    dest[1] = (u & 0x3FF) | 0xDC00; \
  } while(0)

#define GS_UTF16_GET_CHAR(c_lead, c_trail) \
  ((UTF32Char)c_lead << 10) + (UTF32Char)c_trail - ((0xD7C0 << 10) + 0xDC00);

#define GS_ASCII_IS_OCTAL(c) (((c) & 0xF8) == 0)

#define GS_ASCII_IS_HEX(c) (((c) >= '0' && (c) <= '9') \
                            || ((c) >= 'A' && (c) <= 'Z') \
                            || ((c) >= 'a' && (c) <= 'z'))

/* Convert from UTF-8 to UTF-16
 * Return:
 *      Number of bytes converted.
 */
static CFIndex
GSStringEncodingFromUTF8 (UniChar *d, CFIndex dlen, const UInt8 *s,
                          CFIndex slen, CFIndex *needed)
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
          
          count = GS_UTF8_BYTE_COUNT(*s);
          if (count < 2)
            break;
          if (count == 2 && GS_UTF8_IS_TRAIL(s[1]))
            {
              u = (s[0] & 0x1F) << 6;
              u |= s[1] & 0x3F;
            }
          else if (count == 3 && GS_UTF8_IS_TRAIL(s[1])
                   && GS_UTF8_IS_TRAIL(s[2]))
            {
              u = (s[0] & 0x0F) << 12;
              u |= (s[1] & 0x3F) << 6;
              u |= s[2] & 0x3F;
            }
          else if (count == 4 && GS_UTF8_IS_TRAIL(s[1])
                   && GS_UTF8_IS_TRAIL(s[2]) && GS_UTF8_IS_TRAIL(s[3]))
            {
              u = (s[0] & 0x07) << 18;
              u |= (s[1] & 0x3F) << 12;
              u |= (s[2] & 0x3F) << 6;
              u |= s[3] & 0x3F;
            }
          else
            break;
          s += count;
          if (u < 0x10000)
            {
              if (GS_UTF_IS_SURROGATE(u))
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
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

/* Convert from UTF-16 to UTF-8 */
static CFIndex
GSStringEncodingToUTF8 (UInt8 *d, CFIndex dlen, const UniChar *s, CFIndex slen,
                        UniChar lossByte, CFIndex *needed)
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
            *d = (UInt8)u;
          d++;
        }
      else
        {
          CFIndex count;
          
          if (GS_UTF_IS_SURROGATE(u))
            {
              UTF16Char u16;
              
              if (slimit - s > 0 || !GS_UTF_IS_LEAD_SURROGATE(u))
                break;
              u16 = *s++;
              if (!GS_UTF_IS_TRAIL_SURROGATE(u16))
                break;
              u = GS_UTF16_GET_CHAR(u, u16);
            }
          count = GS_UTF8_LENGTH(u);
          if (count < 2)
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
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

static CFIndex
GSStringEncodingFromUTF32 (UniChar *d, CFIndex dlen, const UTF32Char *s,
                           CFIndex slen, CFIndex *needed)
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
          if (GS_UTF_IS_SURROGATE(u))
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
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

static CFIndex
GSStringEncodingToUTF32 (UTF32Char *d, CFIndex dlen, const UniChar *s,
                         CFIndex slen, UniChar lossByte, CFIndex *needed)
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
      if (GS_UTF_IS_SURROGATE(u))
        {
          UTF16Char u16;
          
          if (slimit - s > 0 || !GS_UTF_IS_LEAD_SURROGATE(u))
            break;
          u16 = *s++;
          if (!GS_UTF_IS_TRAIL_SURROGATE(u))
            break;
          u = GS_UTF16_GET_CHAR(u, u16);
        }
      if (dlen != 0)
        *d = u;
      d++;
    }
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

static CFIndex
GSStringEncodingToNonLossyASCII (char *d, CFIndex dlen, const UniChar *s,
                                 CFIndex slen, UInt8 lossByte, CFIndex *needed)
{
  const UniChar *sstart;
  const UniChar *slimit;
  char *dstart;
  char *dlimit;
  
  sstart = s;
  slimit = sstart + slen;
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
              conv[5] = 0;
              conv[4] = 0;
              conv[3] = 0;
              conv[2] = 0;
              conv[1] = 'u';
              convCount = 6;
            }
          if (dlen != 0 && convCount < dlimit - d)
            {
              CFIndex i;
              for (i = 0 ; i < convCount ; ++i)
                d[i] = conv[i];
            }
          d += convCount;
        }
    }
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

static CFIndex
GSStringEncodingFromNonLossyASCII (UniChar *d, CFIndex dlen, const char *s,
                                   CFIndex slen, CFIndex *needed)
{
  const char *sstart;
  const char *slimit;
  UniChar *dstart;
  UniChar *dlimit;
  
  sstart = s;
  slimit = sstart + slen;
  dstart = d;
  dlimit = dstart + dlen;
  while (s < slimit && (dlen == 0 || d < dlimit))
    {
      UniChar c;
      
      c = *s;
      if (c == '\\')
        {
          CFIndex convCount;
          
          c = s[1];
          if (c == '\\')
            {
              convCount = 2;
            }
          else if (GS_ASCII_IS_OCTAL(c) && GS_ASCII_IS_OCTAL(s[2])
                   && GS_ASCII_IS_OCTAL(s[3]))
            {
              c = (s[1] - '0') << 6;
              c |= (s[2] - '0') << 3;
              c |= s[3] - '0';
              convCount = 4;
            }
          else if (c == 'u' && GS_ASCII_IS_HEX(s[2]) && GS_ASCII_IS_HEX(s[3])
                   && GS_ASCII_IS_HEX(s[4]) && GS_ASCII_IS_HEX(s[5]))
            {
              
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
      else if (c < 0x80)
        {
          if (dlen != 0)
            *d = *s;
          d++;
          s++;
        }
      else
        {
          break;
        }
    }
  
  if (needed)
    *needed = ((UInt8 *)d) - ((UInt8 *)dstart);
  
  return s - sstart;
}

CFIndex
GSStringEncodingFromUnicode (CFStringEncoding enc, UInt8 *d, CFIndex dlen,
                             const UniChar *s, CFIndex slen, UniChar lossByte,
                             Boolean isExtRep, CFIndex *needed)
{
  CFIndex converted;
  
  if (d == NULL)
    dlen = 0;
  
  if (enc == kCFStringEncodingUTF8)
    {
      if (isExtRep && dlen > 3)
        {
          *d++ = 0xEF;
          *d++ = 0xBB;
          *d++ = 0xBF;
        }
      converted = GSStringEncodingToUTF8 (d, dlen, s, slen, lossByte, needed);
    }
  else if (enc == kCFStringEncodingUTF32
           || enc == kCFStringEncodingUTF32BE
           || enc == kCFStringEncodingUTF32LE)
    {
      UTF32Char *dst;
      
      dst = (UTF32Char *)d;
      if (isExtRep && enc == kCFStringEncodingUTF32 && dlen >= 4)
        {
          *dst++ = 0x0000FEFF;
          dlen -= 4;
        }
      /* round to the nearest multiple of 4 */
      dlen &= ~0x3;
      converted = GSStringEncodingToUTF32 (dst, dlen, s,
                                           slen / sizeof(UTF32Char),
                                           lossByte, needed);
      if (enc == UTF32_ENCODING_TO_SWAP && dlen != 0)
        {
          UTF32Char *cur;
          UTF32Char *end;
          
          cur = (UTF32Char *)d;
          end = (UTF32Char *)(d + dlen);
          while (cur < end)
            {
              *cur = CFSwapInt32 (*cur);
              cur++;
            }
        }
    }
  else
    {
      CFIndex used;
      UConverter *ucnv;
      
      ucnv = GSStringEncodingOpenConverter (enc, lossByte);
      used = 0;
      if (ucnv)
        {
          char *target;
          const char *targetLimit;
          const UniChar *source;
          const UniChar *sourceLimit;
          UErrorCode err = U_ZERO_ERROR;
          
          target = (char*)d;
          targetLimit = target + dlen;
          source = s;
          sourceLimit = source + slen;
          
          ucnv_fromUnicode (ucnv, &target, targetLimit, &source, sourceLimit,
                            NULL, true, &err);
          used = (CFIndex)(target - (char*)d);
          if (needed)
            {
              *needed = used;
              if (needed && err == U_BUFFER_OVERFLOW_ERROR)
                {
                  char ibuffer[256]; /* Arbitrary buffer size */
                  
                  targetLimit = ibuffer + 255;
                  do
                    {
                      target = ibuffer;
                      err = U_ZERO_ERROR;
                      ucnv_fromUnicode (ucnv, &target, targetLimit, &source,
                        sourceLimit, NULL, true, &err);
                      *needed += (CFIndex)(target - ibuffer);
                    } while (err == U_BUFFER_OVERFLOW_ERROR);
                }
            }
          
          GSStringEncodingCloseConverter (ucnv);
        }
      
      return used;
    }
  
  return converted;
}

CFIndex
GSStringEncodingToUnicode (CFStringEncoding enc, UniChar *d, CFIndex dlen,
                           const UInt8 *s, CFIndex slen,
                           Boolean isExtRep, CFIndex *needed)
{
  CFIndex converted;
  
  if (d == NULL)
    dlen = 0;
  
  if (enc == kCFStringEncodingUTF8)
    {
      if (isExtRep && slen > 3
          && (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF))
        s += 3;
        
      converted = GSStringEncodingFromUTF8 (d, dlen, s, slen, needed);
    }
  else if (enc == kCFStringEncodingUTF32
           || enc == kCFStringEncodingUTF32BE
           || enc == kCFStringEncodingUTF32LE)
    {
      const UTF32Char *src;
      Boolean swap;
      
      src = (const UTF32Char *)s;
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
      else if (enc == UTF32_ENCODING_TO_SWAP)
        {
          swap = true;
        }
      /* round to the nearest multiple of 4 */
      dlen &= ~0x3;
      if (swap && dlen != 0)
        {
          UTF32Char *cur;
          UTF32Char *end;
          
          cur = (UTF32Char *)d;
          end = (UTF32Char *)(d + dlen);
          while (cur < end)
            {
              *cur = CFSwapInt32 (*cur);
              cur++;
            }
        }
      converted = GSStringEncodingFromUTF32 (d, dlen, src,
                                             slen / sizeof(UTF32Char), needed);
    }
  else
    {
      UConverter *ucnv;
      
      ucnv = GSStringEncodingOpenConverter (enc, 0);
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
          source = (char*)s;
          sourceLimit = source + slen;
          
          ucnv_toUnicode (ucnv, &target, targetLimit, &source, sourceLimit, NULL,
            true, &err);
          converted = (CFIndex)(target - d);
          if (needed)
            {
              *needed = converted;
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
                      *needed += (CFIndex)((char*)target - (char*)ibuffer);
                    } while (err == U_BUFFER_OVERFLOW_ERROR);
                }
            }
          
          GSStringEncodingCloseConverter (ucnv);
        }
    }
  
  return converted;
}

