/* CFTimeZone.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: April, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFByteOrder.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFURL.h"
#include "CoreFoundation/CFURLAccess.h"
#include "CoreFoundation/CFRuntime.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"
#include "tzfile.h"

#include <stdio.h>
#include <string.h>
#include <unicode/ucal.h>

struct _ttinfo
{
  SInt32 offset;
  UInt8  isdst;
  UInt8  abbr_idx;
} __attribute__((packed));

typedef struct
{
  SInt32 transition;
  SInt16 offset;
  UInt8  isDST;
  UInt8  abbrevIdx;
} TransInfo;

struct __CFTimeZone
{
  CFRuntimeBase _parent;
  CFStringRef   _name; /* The Olson name */
  CFDataRef     _data;
  CFIndex       _abbrevCount;
  CFStringRef  *_abbrevs;
  CFIndex       _transCount;
  TransInfo    *_transitions;
};

static CFTypeID _kCFTimeZoneTypeID = 0;

static GSMutex _kCFTimeZoneCacheLock;
static CFMutableDictionaryRef _kCFTimeZoneCache = NULL;
static CFTimeZoneRef _kCFTimeZoneDefault = NULL;
static CFTimeZoneRef _kCFTimeZoneSystem = NULL;
static CFDictionaryRef _kCFTimeZoneAbbreviationDictionary = NULL;

static void
CFTimeZoneFinalize (CFTypeRef cf)
{
  CFIndex i;
  CFAllocatorRef alloc;
  CFTimeZoneRef tz = (CFTimeZoneRef)cf;
  
  alloc = CFGetAllocator (tz);
  CFRelease (tz->_name);
  CFRelease (tz->_data);
  for (i = 0 ; i < tz->_abbrevCount ; ++i)
    CFRelease (tz->_abbrevs[i]);
  CFAllocatorDeallocate (alloc, tz->_transitions);
}

static CFRuntimeClass CFTimeZoneClass =
{
  0,
  "CFTimeZone",
  NULL,
  NULL,
  CFTimeZoneFinalize,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFTimeZoneInitialize (void)
{
  _kCFTimeZoneTypeID = _CFRuntimeRegisterClass (&CFTimeZoneClass);
  GSMutexInitialize (&_kCFTimeZoneCacheLock);
}



CFTypeID
CFTimeZoneGetTypeID (void)
{
  return _kCFTimeZoneTypeID;
}

#define CFTIMEZONE_SIZE sizeof(struct __CFTimeZone) - sizeof(CFRuntimeBase)

CFTimeZoneRef
CFTimeZoneCreate (CFAllocatorRef alloc, CFStringRef name, CFDataRef data)
{
  const UInt8 *bytes;
  const UInt8 *endOfBytes;
  const struct tzhead *header;
  SInt32 tzh_timecnt;
  SInt32 tzh_typecnt;
  SInt32 tzh_charcnt;
  UInt32 *tmp;
  struct __CFTimeZone *new;
  CFTimeZoneRef old;
  
  if (_kCFTimeZoneCache == NULL)
    {
      GSMutexLock(&_kCFTimeZoneCacheLock);
      /* Double check the cache is still NULL. */
      if (_kCFTimeZoneCache == NULL)
        _kCFTimeZoneCache = CFDictionaryCreateMutable (
          kCFAllocatorSystemDefault, 0,
          &kCFCopyStringDictionaryKeyCallBacks, NULL);
      GSMutexUnlock(&_kCFTimeZoneCacheLock);
    }
  /* Verify we haven't created a timezone with this name already. */
  old = (CFTimeZoneRef)CFDictionaryGetValue (_kCFTimeZoneCache, name);
  if (old != NULL)
    return CFRetain (old);
  
  /* Do some basic checks before we try anything else. */
  bytes = CFDataGetBytePtr (data);
  endOfBytes = bytes + CFDataGetLength (data);
  header = (const struct tzhead*)bytes;
  bytes += sizeof(struct tzhead);
  
  if (bytes > endOfBytes)
    return NULL;
  
  if (memcmp(header->tzh_magic, TZ_MAGIC, 4) != 0)
    return NULL;
  tmp = (UInt32*)header->tzh_timecnt;
  tzh_timecnt = (SInt32)CFSwapInt32BigToHost (*tmp);
  tmp = (UInt32*)header->tzh_typecnt;
  tzh_typecnt = (SInt32)CFSwapInt32BigToHost (*tmp);
  tmp = (UInt32*)header->tzh_charcnt;
  tzh_charcnt = (SInt32)CFSwapInt32BigToHost (*tmp);
  /* Make sure we're not above any of the maximums. */
  if (tzh_timecnt > TZ_MAX_TIMES || tzh_typecnt > TZ_MAX_TYPES
      || tzh_charcnt > TZ_MAX_CHARS)
    return NULL;
  
  new = (struct __CFTimeZone*)_CFRuntimeCreateInstance (alloc,
    _kCFTimeZoneTypeID, CFTIMEZONE_SIZE, 0);
  if (new)
    {
      CFIndex idx;
      TransInfo *trans;
      SInt32 *times;
      UInt8 *typeIdx;
      struct _ttinfo *types;
      char *chars;
      char *charsEnd;
      CFStringRef abbrevs[TZ_MAX_CHARS / 4];
      
      new->_name = CFStringCreateCopy (alloc, name);
      new->_data = CFDataCreateCopy (alloc, data);
      
      trans = CFAllocatorAllocate (alloc, sizeof(TransInfo) * tzh_timecnt, 0);
      new->_transitions = trans;
      
      times = (SInt32*)bytes;
      typeIdx = (UInt8*)((sizeof(SInt32) * tzh_timecnt) + bytes);
      types = (struct _ttinfo*)(typeIdx + (sizeof(UInt8) * tzh_timecnt));
      chars = (char*)(((UInt8*)types) + (sizeof(struct _ttinfo) * tzh_typecnt));
      charsEnd = chars + tzh_charcnt;
      
      /* Transition times and information */
      new->_transCount = tzh_timecnt;
      for (idx = 0 ; idx < tzh_timecnt ; ++idx)
        {
          trans->transition = (SInt32)CFSwapInt32BigToHost(*(times++));
          trans->offset =
            (SInt16)((SInt32)CFSwapInt32BigToHost(types[*typeIdx].offset));
          trans->isDST = types[*typeIdx].isdst;
          trans->abbrevIdx = types[*typeIdx].abbr_idx;
          
          trans++;
          typeIdx++;
        }
      
      /* Abbreviations */
      idx = 0;
      while (chars < charsEnd)
        {
          abbrevs[idx++] =
            CFStringCreateWithCString (alloc, chars, kCFStringEncodingASCII);
          while (*chars != '\0')
            chars++;
          chars++; /* Skip '\0' */
        }
      
      new->_abbrevs = CFAllocatorAllocate (alloc, sizeof(void*) * idx, 0);
      new->_abbrevCount = idx;
      for (idx = 0 ; idx < new->_abbrevCount ; ++idx)
        new->_abbrevs[idx] = abbrevs[idx];
      
      if (header->tzh_version[0] == '2')
        {
          /* FIXME: Code for version '2' TZif files */
        }
      
      GSMutexLock(&_kCFTimeZoneCacheLock);
      /* Double check another an entry wasn't inserted before we could act. */
      old = (CFTimeZoneRef)CFDictionaryGetValue (_kCFTimeZoneCache, name);
      if (old != NULL)
        {
          GSMutexUnlock(&_kCFTimeZoneCacheLock);
          CFRelease ((CFTimeZoneRef)new);
          return CFRetain (old);
        }
      CFDictionaryAddValue (_kCFTimeZoneCache, name, (const void *)new);
      old = (CFTimeZoneRef)new;
      GSMutexUnlock(&_kCFTimeZoneCacheLock);
    }
  
  return old;
}

struct TZFile
{
  struct tzhead header;
  SInt32 time;
  UInt8  type;
  struct _ttinfo ttinfo;
  char   abbrev[10]; /* 10 = max number of characters, ie 'GMT+01:00\0'. */
} __attribute__((packed));

CFTimeZoneRef
CFTimeZoneCreateWithTimeIntervalFromGMT (CFAllocatorRef alloc,
  CFTimeInterval ti)
{
  CFTimeZoneRef new;
  struct TZFile tzfile;
  CFIndex numChars;
  CFStringRef name;
  CFDataRef data;
  char sign;
  SInt32 sec;
  SInt32 hour;
  SInt32 min;
  
  sign = (ti < 0) ? '-' : '+';
  sec = (SInt32)ti;
  hour = (ti < 0) ? -sec / 3600 : sec / 3600;
  sec -= ((ti < 0) ? -hour : hour) * 3600;
  min = (ti < 0) ? -sec / 60 : sec / 60;
  
  memset (&tzfile, 0, sizeof(struct TZFile));
  memcpy (tzfile.header.tzh_magic, "TZif", 4);
  tzfile.header.tzh_timecnt[3] = 1;
  tzfile.header.tzh_typecnt[3] = 1;
  tzfile.ttinfo.offset = CFSwapInt32HostToBig((SInt32)ti);
  numChars = snprintf (tzfile.abbrev, 10, "GMT%c%02d:%02d", sign, hour, min);
  tzfile.header.tzh_charcnt[3] = numChars;
  
  name = CFStringCreateWithCString (alloc, tzfile.abbrev,
                                    kCFStringEncodingASCII);
  data = CFDataCreateWithBytesNoCopy (alloc, (UInt8*)&tzfile,
    sizeof(struct TZFile) - (10 - numChars), kCFAllocatorNull);
  new = CFTimeZoneCreate (alloc, name, data);
  CFRelease (name);
  CFRelease (data);
  
  return new;
}

CFTimeZoneRef
CFTimeZoneCreateWithName (CFAllocatorRef alloc, CFStringRef name,
  Boolean tryAbbrev)
{
  CFURLRef tzdir;
  CFURLRef path;
  CFDataRef data;
  CFTimeZoneRef new;
  
  if (tryAbbrev)
    {
      CFDictionaryRef abbrevs;
      CFStringRef nonAbbrev;
      
      abbrevs = CFTimeZoneCopyAbbreviationDictionary ();
      if (CFDictionaryGetValueIfPresent (abbrevs, name, (const void**)&nonAbbrev))
        {
          name = nonAbbrev;
        }
      
      CFRelease (abbrevs);
    }
  
  tzdir = CFURLCreateWithFileSystemPathRelativeToBase (alloc, CFSTR(TZDIR),
                                                       kCFURLPOSIXPathStyle,
                                                       true, NULL);
  path = CFURLCreateWithFileSystemPathRelativeToBase (alloc, name,
                                                      kCFURLPOSIXPathStyle,
                                                      false, tzdir);
  CFRelease (tzdir);
  
  if (CFURLCreateDataAndPropertiesFromResource (alloc, path, &data, NULL,
                                                NULL, NULL))
    {
      new = CFTimeZoneCreate (alloc, name, data);
      CFRelease (data);
    }
  else
    {
      new = NULL;
    }
  
  CFRelease (path);
  
  return new;
}

CFStringRef
CFTimeZoneGetName (CFTimeZoneRef tz)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFStringRef, tz, "name");
  return tz->_name;
}

CFDataRef
CFTimeZoneGetData (CFTimeZoneRef tz)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFDataRef, tz, "data");
  return tz->_data;
}

static CFComparisonResult
CFTimeZoneComparator (const void *v1, const void *v2, void *ctxt)
{
  TransInfo *ti1 = (TransInfo*)v1;
  TransInfo *ti2 = (TransInfo*)v2;
  
  return (ti1->transition < ti2->transition) ? kCFCompareLessThan :
    ((ti1->transition == ti2->transition) ? kCFCompareEqualTo :
    kCFCompareGreaterThan);
}

CFStringRef
CFTimeZoneCopyAbbreviation (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFStringRef, tz,
    "_cfCopyAbbreviation:", at);
  
  TransInfo tmp;
  CFIndex idx;
  
  tmp.transition = (SInt32)(at + kCFAbsoluteTimeIntervalSince1970);
  idx = GSBSearch (tz->_transitions, &tmp, CFRangeMake(0, tz->_transCount),
    sizeof(TransInfo), CFTimeZoneComparator, NULL);
  
  return CFRetain (tz->_abbrevs[tz->_transitions[idx].abbrevIdx]);
}

CFTimeInterval
CFTimeZoneGetDaylightSavingTimeOffset (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFTimeInterval, tz,
    "_cfGetDaylightSavingTimeOffset:", at);
  
  TransInfo tmp;
  TransInfo *cur;
  TransInfo *prev;
  CFIndex idx;
  CFTimeInterval ret;
  
  if (tz->_transCount <= 1)
    return 0.0; /* No DST, so no DST offset. */
  
  tmp.transition = (SInt32)(at + kCFAbsoluteTimeIntervalSince1970);
  idx = GSBSearch (tz->_transitions, &tmp, CFRangeMake(0, tz->_transCount),
    sizeof(TransInfo), CFTimeZoneComparator, NULL);
  
  cur = &(tz->_transitions[idx]);
  idx = idx > 0 ? idx - 1 : idx + 1;
  prev = &(tz->_transitions[idx]);
  if (cur->isDST && cur->offset > prev->offset)
    ret = (CFTimeInterval)(cur->offset - prev->offset);
  else
    ret = 0.0;
  
  return ret;
}

Boolean
CFTimeZoneIsDaylightSavingTime (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, Boolean, tz,
    "_cfIsDaylightSavingTime:", at);
  
  TransInfo tmp;
  CFIndex idx;
  
  tmp.transition = (SInt32)(at + kCFAbsoluteTimeIntervalSince1970);
  idx = GSBSearch (tz->_transitions, &tmp, CFRangeMake(0, tz->_transCount),
    sizeof(TransInfo), CFTimeZoneComparator, NULL);
  
  return (Boolean)(tz->_transitions[idx].isDST);
}

CFTimeInterval
CFTimeZoneGetSecondsFromGMT (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFTimeInterval, tz,
    "_cfGetSecondsFromGMT:", at);
  
  TransInfo tmp;
  CFIndex idx;
  
  tmp.transition = (SInt32)(at + kCFAbsoluteTimeIntervalSince1970);
  idx = GSBSearch (tz->_transitions, &tmp, CFRangeMake(0, tz->_transCount),
    sizeof(TransInfo), CFTimeZoneComparator, NULL);
  
  return (CFTimeInterval)(tz->_transitions[idx].offset);
}

CFAbsoluteTime
CFTimeZoneGetNextDaylightSavingTimeTransition (CFTimeZoneRef tz,
  CFAbsoluteTime at)
{
  CF_OBJC_FUNCDISPATCHV(_kCFTimeZoneTypeID, CFAbsoluteTime, tz,
    "_cfGetNextDaylightSavingTimeTransition:", at);
  
  TransInfo tmp;
  CFIndex idx;
  
  tmp.transition = (SInt32)(at + kCFAbsoluteTimeIntervalSince1970);
  idx = GSBSearch (tz->_transitions, &tmp, CFRangeMake(0, tz->_transCount),
    sizeof(TransInfo), CFTimeZoneComparator, NULL);
  
  return (idx + 1 >= tz->_transCount) ? 0.0 :
    tz->_transitions[idx + 1].transition - kCFAbsoluteTimeIntervalSince1970;
}

#define BUFFER_SIZE 256

CFStringRef
CFTimeZoneCopyLocalizedName (CFTimeZoneRef tz, CFTimeZoneNameStyle style,
  CFLocaleRef locale)
{
  CF_OBJC_FUNCDISPATCHV_RETAINED(_kCFTimeZoneTypeID, CFStringRef, tz,
    "localizedName:locale:", style, locale);
  
#if HAVE_ICU
  UniChar localizedName[BUFFER_SIZE];
  UniChar zoneID[BUFFER_SIZE];
  char cLocale[ULOC_FULLNAME_CAPACITY];
  CFIndex len;
  UCalendarDisplayNameType ucaltype;
  UCalendar *ucal;
  UErrorCode err;
  CFStringRef ret;
  
  len = CFStringGetLength (tz->_name);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  CFStringGetCharacters (tz->_name, CFRangeMake(0, len), zoneID);
  
  err = U_ZERO_ERROR;
  ucal = ucal_open (zoneID, len, NULL, UCAL_TRADITIONAL, &err);
  if (U_FAILURE(err))
    return NULL;
  
  switch (style)
    {
      case kCFTimeZoneNameStyleShortStandard:
        ucaltype = UCAL_SHORT_STANDARD;
        break;
      case kCFTimeZoneNameStyleDaylightSaving:
        ucaltype = UCAL_DST;
        break;
      case kCFTimeZoneNameStyleShortDaylightSaving:
        ucaltype = UCAL_SHORT_DST;
        break;
      default: /* Covers kCFTimeZoneNameStyleStandard */
        ucaltype = UCAL_STANDARD;
    }
  CFStringGetCString (CFLocaleGetIdentifier(locale), cLocale,
                      ULOC_FULLNAME_CAPACITY, kCFStringEncodingASCII);
  len = ucal_getTimeZoneDisplayName (ucal, ucaltype, cLocale,
    localizedName, BUFFER_SIZE, &err);
  if (len > BUFFER_SIZE)
    len = BUFFER_SIZE;
  
  ret = CFStringCreateWithCharacters (CFGetAllocator(tz), localizedName, len);
  ucal_close (ucal);
  
  return ret;
#else
  return NULL;
#endif
}

CFTimeZoneRef
CFTimeZoneCopyDefault (void)
{
  if (_kCFTimeZoneDefault == NULL)
    {
      CFTimeZoneRef new;
      new = CFTimeZoneCopySystem(); /* FIXME */
      if (GSAtomicCompareAndSwapPointer(&_kCFTimeZoneDefault, NULL, new) != NULL)
        CFRelease (new);
    }
  
  return CFRetain (_kCFTimeZoneDefault);
}

void
CFTimeZoneSetDefault (CFTimeZoneRef tz)
{
  CFTimeZoneRef old;
  old = GSAtomicCompareAndSwapPointer(&_kCFTimeZoneDefault,
                                      _kCFTimeZoneDefault, CFRetain (tz));
  if (old != NULL)
    CFRelease (old);
}

CFTimeZoneRef
CFTimeZoneCopySystem (void)
{
  if (_kCFTimeZoneSystem == NULL)
    {
      CFTimeZoneRef new;
      new = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0); /* FIXME */
      if (GSAtomicCompareAndSwapPointer(&_kCFTimeZoneSystem, NULL, new) != NULL)
        CFRelease (new);
    }
  
  return CFRetain (_kCFTimeZoneSystem);
}

void
CFTimeZoneResetSystem (void)
{
  if (_kCFTimeZoneSystem != NULL)
    {
      CFTimeZoneRef old;
      old = GSAtomicCompareAndSwapPointer(&_kCFTimeZoneSystem,
                                          _kCFTimeZoneSystem, NULL);
      if (old != NULL)
        CFRelease (old);
    }
}

CFArrayRef
CFTimeZoneCopyKnownNames (void)
{
  return NULL; /* FIXME */
}

static const char *_kCFTimeZoneAbbreviationKeys[] =
{
  "ACDT",
  "ACST",
  "ADT",
  "AEDT",
  "AEST",
  "AFT",
  "AKDT",
  "AKST",
  "AMT",
  "ART",
  "AST",
  "AWDT",
  "AWST",
  "AZOST",
  "AZT",
  "BDT",
  "BIOT",
  "BOT",
  "BRT",
  "BST",
  "BTT",
  "CAT",
  "CCT",
  "CDT",
  "CEDT",
  "CEST",
  "CET",
  "CHADT",
  "CHAST",
  "CLST",
  "CLT",
  "COT",
  "CST",
  "CT",
  "CVT",
  "CXT",
  "EAST",
  "EAT",
  "ECT",
  "EDT",
  "EEDT",
  "EEST",
  "EET",
  "EST",
  "FET",
  "FJT",
  "FKST",
  "FKT",
  "GALT",
  "GET",
  "GFT",
  "GIT",
  "GST",
  "GYT",
  "HADT",
  "HAST",
  "HKT",
  "HST",
  "ICT",
  "IRKT",
  "IRST",
  "IST",
  "JST",
  "KRAT",
  "KST",
  "MAGT",
  "MDT",
  "MSD",
  "MSK",
  "MST",
  "MYT",
  "NDT",
  "NPT",
  "NST",
  "NZDT",
  "NZST",
  "PDT",
  "PETT",
  "PKT",
  "PST",
  "SAMT",
  "SGT",
  "SST",
  "TAHT",
  "THA",
  "UYST",
  "UYT",
  "VET",
  "VLAT",
  "WAT",
  "WEDT",
  "WEST",
  "WET",
  "YAKT",
  "YEKT"
};

static const char *_kCFTimeZoneAbbreviationValues[] =
{
  /* ACDT */  "Australia/Adelaide",
  /* ACST */  "Australia/Adelaide",
  /* ADT */   "America/Halifax",
  /* AEDT */  "Australia/Sydney",
  /* AEST */  "Australia/Sydney",
  /* AFT */   "Asia/Kabul",
  /* AKDT */  "America/Juneau",
  /* AKST */  "America/Juneau",
  /* AMT */   "Asia/Yakutsk",
  /* ART */   "America/Argentina/Buenos_Aires",
  /* AST */   "America/Halifax",
  /* AWDT */  "Australia/Perth",
  /* AWST */  "Australia/Perth",
  /* AZOST */ "Atlantic/Azores",
  /* AZT */   "Asia/Baku",
  /* BDT */   "Asia/Dhaka",
  /* BIOT */  "Indian/Chagos",
  /* BOT */   "America/La_Paz",
  /* BRT */   "America/Sao_Paulo",
  /* BST */   "Europe/London",
  /* BTT */   "Asia/Thimphu",
  /* CAT */   "Africa/Harare",
  /* CCT */   "Indian/Cocos",
  /* CDT */   "America/Chicago",
  /* CEDT */  "Europe/Paris",
  /* CEST */  "Europe/Paris",
  /* CET */   "Europe/Paris",
  /* CHADT */ "Pacific/Chatham",
  /* CHAST */ "Pacific/Chatham",
  /* CLST */  "America/Santiago",
  /* CLT */   "America/Santiago",
  /* COT */   "America/Bogota",
  /* CST */   "America/Chicago",
  /* CT */    "Asia/Shanghai",
  /* CVT */   "Atlantic/Cape_Verde",
  /* CXT */   "Indian/Christmas",
  /* EAST */  "Pacific/Easter",
  /* EAT */   "Africa/Addis_Ababa",
  /* ECT */   "America/Guayaquil",
  /* EDT */   "America/New_York",
  /* EEDT */  "Europe/Istanbul",
  /* EEST */  "Europe/Istanbul",
  /* EET */   "Europe/Istanbul",
  /* EST */   "America/New_York",
  /* FET */   "Europe/Kaliningrad",
  /* FJT */   "Pacific/Fiji",
  /* FKST */  "Atlantic/Stanley",
  /* FKT */   "Atlantic/Stanley",
  /* GALT */  "Pacific/Galapagos",
  /* GET */   "Asia/Tbilisi",
  /* GFT */   "America/Cayenne",
  /* GIT */   "Pacific/Gambier",
  /* GST */   "Asia/Dubai",
  /* GYT */   "America/Guyana",
  /* HADT */  "America/Adak",
  /* HAST */  "America/Adak",
  /* HKT */   "Asia/Hong_Kong",
  /* HST */   "Pacific/Honolulu",
  /* ICT */   "Asia/Bangkok",
  /* IRKT */  " Asia/Irkutsk",
  /* IRST */  "Asia/Tehran",
  /* IST */   "Asia/Calcutta",
  /* JST */   "Asia/Tokyo",
  /* KRAT */  "Asia/Krasnoyarsk",
  /* KST */   "Asia/Seoul",
  /* MAGT */  "Asia/Magadan",
  /* MDT */   "America/Denver",
  /* MSD */   "Europe/Moscow",
  /* MSK */   "Europe/Moscow",
  /* MST */   "America/Denver",
  /* MYT */   "Asia/Kuala_Lumpur",
  /* NDT */   "America/St_Johns",
  /* NPT */   "Asia/Kathmandu",
  /* NST */   "America/St_Johns",
  /* NZDT */  "Pacific/Auckland",
  /* NZST */  "Pacific/Auckland",
  /* PDT */   "America/Los_Angeles",
  /* PETT */  "Asia/Kamchatka",
  /* PKT */   "Asia/Karachi",
  /* PST */   "America/Los_Angeles",
  /* SAMT */  "Europe/Samara",
  /* SGT */   "Asia/Singapore",
  /* SST */   "Pacific/Pago_Pago",
  /* TAHT */  "Pacific/Tahiti",
  /* THA */   "Asia/Bangkok",
  /* UYST */  "America/Montevideo",
  /* UYT */   "America/Montevideo",
  /* VET */   "America/Caracas",
  /* VLAT */  "Asia/Vladivostok",
  /* WAT */   "Africa/Lagos",
  /* WEDT */  "Europe/Lisbon",
  /* WEST */  "Europe/Lisbon",
  /* WET */   "Europe/Lisbon",
  /* YAKT */  "Asia/Yakutsk",
  /* YEKT */  "Asia/Yekaterinburg"
};

static const CFIndex _kCFTimeZoneAbbreviationsSize =
  sizeof(_kCFTimeZoneAbbreviationKeys) / sizeof(void*);

CFDictionaryRef
CFTimeZoneCopyAbbreviationDictionary (void)
{
  if (_kCFTimeZoneAbbreviationDictionary == NULL)
    {
      CFIndex i;
      CFMutableDictionaryRef dict;
      CFDictionaryRef new;
      
      dict = CFDictionaryCreateMutable (NULL, _kCFTimeZoneAbbreviationsSize,
                                        &kCFCopyStringDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
      i = 0;
      while (i < _kCFTimeZoneAbbreviationsSize)
        {
          CFStringRef abbrev;
          CFStringRef fullname;
          
          abbrev =
            __CFStringMakeConstantString (_kCFTimeZoneAbbreviationKeys[i]);
          fullname =
            __CFStringMakeConstantString (_kCFTimeZoneAbbreviationValues[i]);
          CFDictionaryAddValue (dict, abbrev, fullname);
          i++;
        }
      new = CFDictionaryCreateCopy (NULL, dict);
      CFRelease (dict);
      
      if (GSAtomicCompareAndSwapPointer(&_kCFTimeZoneAbbreviationDictionary,
          NULL, new) != NULL)
        CFRelease (new);
    }
  
  return CFRetain (_kCFTimeZoneAbbreviationDictionary);
}

void
CFTimeZoneSetAbbreviationDictionary (CFDictionaryRef dict)
{
  CFDictionaryRef old;
  old = GSAtomicCompareAndSwapPointer(&_kCFTimeZoneAbbreviationDictionary,
                                      _kCFTimeZoneAbbreviationDictionary,
                                      CFDictionaryCreateCopy (NULL, dict));
  if (old != NULL)
    CFRelease (old);
}

