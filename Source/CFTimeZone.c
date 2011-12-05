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
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFData.h"
#include "CoreFoundation/CFDate.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFTimeZone.h"
#include "CoreFoundation/CFRuntime.h"
#include "GSPrivate.h"

/*************************/
/* Time Zone Information */
/*************************/
#define	TZ_MAGIC	'TZif'
/* These structures is easy enough to reproduce. */
struct tzhead
{
  // All fields are in big-endian format.
  uint8_t tzh_magic[4];
  uint8_t tzh_version[1];
  uint8_t tzh_reserved[15];
  uint8_t tzh_ttisgmtcnt[4];
  uint8_t tzh_ttisstdcnt[4];
  uint8_t tzh_leapcnt[4];
  uint8_t tzh_timecnt[4];
  uint8_t tzh_typecnt[4];
  uint8_t tzh_charcnt[4];
};

struct ttinfo
{
  uint8_t tt_gmtoff[4];
  uint8_t tt_isdst;
  uint8_t tt_abbrind;
};
/*************************/

#define BUFFER_SIZE 256

struct __CFTimeZone
{
  CFRuntimeBase _parent;
  CFStringRef   _name;
  CFDataRef     _data;
  CFStringRef   _abbrev;
  
};

static CFTypeID _kCFTimeZoneTypeID;

static GSMutex _kCFTimeZoneLock;
static CFTimeZoneRef _kCFTimeZoneDefault = NULL;
static CFTimeZoneRef _kCFTimeZoneSystem = NULL;
static CFDictionaryRef _kCFTimeZoneAbbreviations = NULL;

static CFRuntimeClass CFTimeZoneClass =
{
  0,
  "CFTimeZone",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFTimeZoneInitialize (void)
{
  _kCFTimeZoneTypeID = _CFRuntimeRegisterClass (&CFTimeZoneClass);
  GSMutexInitialize (&_kCFTimeZoneLock);
}



CFTimeZoneRef
CFTimeZoneCreateWithName (CFAllocatorRef alloc, CFStringRef name,
  Boolean tryAbbrev)
{
  return NULL;
}

CFTimeZoneRef
CFTimeZoneCreateWithTimeIntervalFromGMT (CFAllocatorRef alloc,
  CFTimeInterval ti)
{
  return NULL; // FIXME
}

CFTimeZoneRef
CFTimeZoneCreate (CFAllocatorRef alloc, CFStringRef name, CFDateRef data)
{
  return NULL; // FIXME
}

CFDictionaryRef
CFTimeZoneCopyAbbreviationDictionary (void)
{
  return NULL; // FIXME
}

CFStringRef
CFTimeZoneCopyAbbreviation (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return NULL; // FIXME
}

CFTimeZoneRef
CFTimeZoneCopyDefault (void)
{

  return NULL;
}

CFTimeZoneRef
CFTimeZoneCopySystem (void)
{
  return NULL; // FIXME
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
  return 0.0; // FIXME
}

Boolean
CFTimeZoneIsDaylightSavingTime (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return false; // FIXME
}

CFTimeInterval
CFTimeZoneGetDaylightSavingTimeOffset (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return 0.0; // FIXME
}

CFAbsoluteTime
CFTimeZoneGetNextDaylightSavingTimeTransition (CFTimeZoneRef tz,
  CFAbsoluteTime at)
{
  return 0.0; // FIXME
}

CFTypeID
CFTimeZoneGetTypeID (void)
{
  return _kCFTimeZoneTypeID;
}
