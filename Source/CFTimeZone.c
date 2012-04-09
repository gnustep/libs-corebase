/* CFTimeZone.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: July, 2011
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

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

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFByteOrder.h"
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"
#include "GSPrivate.h"
#include "tzfile.h"

#include <stdio.h>
#include <string.h>

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

static GSMutex _kCFTimeZoneLock;
static GSMutex _kCFTimeZoneCacheLock;
static CFMutableDictionaryRef _kCFTimeZoneCache = NULL;
static CFTimeZoneRef _kCFTimeZoneDefault = NULL;
static CFTimeZoneRef _kCFTimeZoneSystem = NULL;
static CFDictionaryRef _kCFTimeZoneAbbreviations = NULL;

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
  GSMutexInitialize (&_kCFTimeZoneLock);
  GSMutexInitialize (&_kCFTimeZoneCacheLock);
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
    return old;
  
  /* Do some basic checks before we try anything else. */
  bytes = CFDataGetBytePtr (data);
  endOfBytes = bytes + CFDataGetLength (data);
  header = (const struct tzhead*)bytes;
  bytes += sizeof(struct tzhead);
  
  if (bytes > endOfBytes)
    return NULL;
  
  if (memcmp(header->tzh_magic, TZ_MAGIC, sizeof(TZ_MAGIC)) != 0)
    return NULL;
  tzh_timecnt = (SInt32)CFSwapInt32BigToHost ((UInt32)header->tzh_timecnt);
  tzh_typecnt = (SInt32)CFSwapInt32BigToHost ((UInt32)header->tzh_typecnt);
  tzh_charcnt = (SInt32)CFSwapInt32BigToHost ((UInt32)header->tzh_charcnt);
  /* Make sure we're not above any of the maximums. */
  if (tzh_timecnt > TZ_MAX_TIMES)
    return NULL;
  if (tzh_typecnt > TZ_MAX_TYPES)
    return NULL;
  if (tzh_charcnt > TZ_MAX_CHARS)
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
      new->_transitions = trans;
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
          chars++; // Skip '\0'
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
          return old;
        }
      CFDictionaryAddValue (_kCFTimeZoneCache, name, (const void *)new);
      old = (CFTimeZoneRef)new;
      GSMutexUnlock(&_kCFTimeZoneCacheLock);
    }
  
  return old;
}

CFTimeZoneRef
CFTimeZoneCreateWithName (CFAllocatorRef alloc, CFStringRef name,
  Boolean tryAbbrev)
{
  return NULL;
}

struct TZFile
{
  struct tzhead header;
  SInt32 time;
  UInt8  type;
  struct _ttinfo ttinfo;
  char   abbrev[9]; // 9 = max number of characters, ie 'GMT+0100\0'.
};

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
  min = (ti < 0) ? -sec / 60 : sec / 60;
  
  memset (&tzfile, 0, sizeof(struct TZFile));
  memcpy (tzfile.header.tzh_magic, "TZif", 4);
  tzfile.header.tzh_timecnt[3] = 1;
  tzfile.header.tzh_typecnt[3] = 1;
  numChars = snprintf (tzfile.abbrev, 9, "GMT%c%02d%02d", sign, hour, min);
  tzfile.header.tzh_charcnt[3] = numChars;
  
  name = CFStringCreateWithCString (alloc, tzfile.abbrev,
    kCFStringEncodingASCII);
  data = CFDataCreateWithBytesNoCopy (alloc, (UInt8*)&tzfile,
    sizeof(struct TZFile) - (9 - numChars), kCFAllocatorNull);
  new = CFTimeZoneCreate (alloc, name, data);
  CFRelease (name);
  CFRelease (data);
  
  return new;
}

CFDictionaryRef
CFTimeZoneCopyAbbreviationDictionary (void)
{
  return NULL; /* FIXME */
}

CFStringRef
CFTimeZoneCopyAbbreviation (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return NULL; /* FIXME */
}

CFTimeZoneRef
CFTimeZoneCopyDefault (void)
{

  return NULL;
}

CFTimeZoneRef
CFTimeZoneCopySystem (void)
{
  return NULL; /* FIXME */
}

void
CFTimeZoneSetDefault (CFTimeZoneRef tz)
{
  GSMutexLock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneDefault != NULL)
    CFRelease (_kCFTimeZoneDefault);
  _kCFTimeZoneDefault = CFRetain (tz);
  GSMutexUnlock (&_kCFTimeZoneLock);
}

CFArrayRef
CFTimeZoneCopyKnownNames (void)
{
  return NULL;
}

void
CFTimeZoneResetSystem (void)
{
  GSMutexLock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneSystem != NULL)
    {
      CFRelease (_kCFTimeZoneSystem);
      _kCFTimeZoneSystem = NULL;
    }
  GSMutexUnlock (&_kCFTimeZoneLock);
}

void
CFTimeZoneSetAbbreviationDictionary (CFDictionaryRef dict)
{
  GSMutexLock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneAbbreviations != NULL)
    CFRelease (_kCFTimeZoneAbbreviations);
  _kCFTimeZoneAbbreviations = (CFDictionaryRef)CFRetain (dict);
  GSMutexUnlock (&_kCFTimeZoneLock);
}

CFStringRef
CFTimeZoneGetName (CFTimeZoneRef tz)
{
  return tz->_name;
}

CFStringRef
CFTimeZoneCopyLocalizedName (CFTimeZoneRef tz, CFTimeZoneNameStyle style,
  CFLocaleRef locale)
{
  return NULL;
}

CFDataRef
CFTimeZoneGetData (CFTimeZoneRef tz)
{
  return tz->_data;
}

CFTimeInterval
CFTimeZoneGetSecondsFromGMT (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return 0.0; /* FIXME */
}

Boolean
CFTimeZoneIsDaylightSavingTime (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return false; /* FIXME */
}

CFTimeInterval
CFTimeZoneGetDaylightSavingTimeOffset (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return 0.0; /* FIXME */
}

CFAbsoluteTime
CFTimeZoneGetNextDaylightSavingTimeTransition (CFTimeZoneRef tz,
  CFAbsoluteTime at)
{
  return 0.0; /* FIXME */
}

CFTypeID
CFTimeZoneGetTypeID (void)
{
  return _kCFTimeZoneTypeID;
}
