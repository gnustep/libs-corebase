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

#include <pthread.h>
#include <unicode/ucal.h>
#include <unicode/ures.h>
#include <unicode/ustring.h>

/* Only using ICU's zoneinfo64, right now.  If anyone is using the data
   related time zone functions, tzhead struct support can be added.
   
   ICU's zoneinfo64 table has the following format:
   "TZVersion"
   "Zones"
     {
       transPre32:intvector ( ) // zoneinfo64 only
       trans:intvector ( transition seconds )
       transPost32:intvector () // zoneinfo64 only
       typeOffsets:intvector ( GMT offset + DST offset pair )
       typeMap:bin ( binary map, which offset to use )
       finalRule ( rule name )
       finalRaw:int ( final seconds from GMT ) // finalRaw and finalYear combined in zoneinfo format
       finalYear:int ( last year the rule applies )
       links:intvector ( aliases )
     }
   "Names"
     {
       All known timezone names
     }
   "Rules"
     {
       intvector: ( savingsStartMonth, savingsStartDay, savingsStartDayOfWeek,
        savingsStartTime, savingsStartTimeMode, savingsEndMonth,
        savingsEndDay, savingsEndDayOfWeek, savingsEndTime,
        savingsEndTimeMode, savingsDST )
     }
   "Regions"
     {
       A mapping table between "Zones" and they're regions.
     }
*/

/*************************/
/* Time Zone Information */
/*************************/
#define	TZ_MAGIC	'TZif'
/* These structures is easy enough to reproduce. */
struct tzhead
{
  // All fields are in big-endian format.
  uint8_t magic[4];
  uint8_t reserved[16];
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



/* FIXME: Only valid for ICU version > 4.4 */
#define ICU_ZONEINFO    "zoneinfo64"
#define ICU_ZONENAMES   "Names"
#define ICU_ZONES       "Zones"
#define ICU_ZONERULES   "Rules"
#define ICU_ZONEREGIONS "Regions"
#define ICU_TRANSPRE32  "transPre32"
#define ICU_TRANS       "trans"
#define ICU_TRANSPOST32 "transPost32"
#define ICU_TYPEOFFSETS "typeOffsets"
#define ICU_TYPEMAP     "typeMap"
#define ICU_FINALRULE   "finalRule"
#define ICU_FINALRAW    "finalRaw"
#define ICU_FINALYEAR   "finalYear"
#define ICU_LINKS       "links"

struct __CFTimeZone
{
  CFRuntimeBase _parent;
  CFStringRef   _name;
  CFArrayRef    _aliases;
  CFDataRef     _data;
  struct
    {
      const int32_t *transPre32; // zoneinfo64 only
      int32_t transPre32Count;
      const int32_t *trans;
      int32_t transCount;
      const int32_t *transPost32; // zoneinfo64 only
      int32_t transPost32Count;
      const int32_t *typeOffsets;
      const uint8_t *typeMap;
      const int32_t *finalRule;
      int32_t finalRaw;
      int32_t finalYear;
    } _info;
};

static CFTypeID _kCFTimeZoneTypeID;

static pthread_mutex_t _kCFTimeZoneLock = PTHREAD_MUTEX_INITIALIZER;
static CFArrayRef _kCFTimeZoneBuiltinNames = NULL;
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
}



static CFArrayRef
CFTimeZoneGetBuiltinNames (void)
{
  pthread_mutex_lock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneBuiltinNames == NULL)
    {
      int32_t size;
      int32_t idx;
      int32_t len;
      CFMutableArrayRef array;
      CFStringRef name;
      const UChar *str;
      UResourceBundle *res;
      UResourceBundle *names;
      UErrorCode err = U_ZERO_ERROR;
      
      /* Open the "Names" resource in "zoneinfo"/"zoneinfo64" */
      res = ures_openDirect (NULL, ICU_ZONEINFO, &err);
      names = ures_getByKey (res, ICU_ZONENAMES, NULL, &err);
      
      array = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
      size = ures_getSize (names);
      idx = 0;
      while (idx < size)
        {
          str = ures_getStringByIndex (names, idx++, &len, &err);
          if (U_FAILURE(err))
            break;
          name = CFStringCreateWithCharactersNoCopy (NULL, str, len,
            kCFAllocatorNull);
          CFArrayAppendValue (array, name);
          CFRelease(name);
        }
      
      if (idx == size)
        _kCFTimeZoneBuiltinNames = CFArrayCreateCopy (NULL, array);
      
      CFRelease (array);
      ures_close (names);
      ures_close (res);
    }
  pthread_mutex_unlock (&_kCFTimeZoneLock);
  
  return _kCFTimeZoneBuiltinNames;
}

static CFArrayRef
CFTimeZoneGetCustomNames (void)
{
  return NULL;
}

CFIndex
CFTimeZoneFindName (CFStringRef value, CFArrayRef names)
{
  CFIndex min, max, mid, oldMid = INT32_MAX;
  
  min = 0;
  max = CFArrayGetCount (names);
  
  while (min <= max)
    {
      CFStringRef midValue;
      CFComparisonResult res;
      
      mid = (min + max) / 2;
      if (oldMid == mid)
        break;
      
      oldMid = mid;
      
      midValue = CFArrayGetValueAtIndex (names, mid);
      res = CFStringCompare (value, midValue, 0);
      if (res == kCFCompareEqualTo)
        {
          return mid;
        }
      if (res == kCFCompareGreaterThan)
        {
          max = mid;
        }
      else
        {
          min = mid;
        }
    }
  
  return -1;
}

CFTimeZoneRef
CFTimeZoneCreateWithName (CFAllocatorRef alloc, CFStringRef name,
  Boolean tryAbbrev)
{
  UniChar c;
  CFIndex idx;
  CFArrayRef builtins;
  struct __CFTimeZone *new;
  UResourceBundle *top;
  UResourceBundle *res;
  UErrorCode err = U_ZERO_ERROR;
  
  /* FIXME: what to do with tryAbbrev? */
  
  builtins = CFTimeZoneGetBuiltinNames ();
  idx = CFTimeZoneFindName (name, builtins);
  if (idx < 0)
    {
      /* Most of this was lifted from NSTimeZone.m.  Don't have to worry
         about GMT, UTC, etc because ICU already includes those timezones. */
      CFIndex ti = 0.0;
      CFIndex length = CFStringGetLength (name);
      
      if (length == 5 && CFStringHasPrefix(name, CFSTR("GMT"))
          && ((c = CFStringGetCharacterAtIndex(name, 3)) == '+' || c == '-')
          && ((c = CFStringGetCharacterAtIndex(name, 4)) >= '0' && c <= '9'))
        {
          // GMT-9 to GMT+9
          ti = (c - '0') * 60 * 60;
          if (CFStringGetCharacterAtIndex(name, 3) == '-')
            ti = -ti;
        }
      else if (length == 6 && CFStringHasPrefix(name, CFSTR("GMT"))
          && ((c = CFStringGetCharacterAtIndex(name, 3)) == '+' || c == '-')
          && ((c = CFStringGetCharacterAtIndex(name, 4)) == '0' || c == '1')
          && ((c = CFStringGetCharacterAtIndex(name, 4)) >= '0' && c <= '4'))
        {
          // GMT-14 to GMT-10 and GMT+10 to GMT+14
          ti = (c - '0') * 60 * 60;
          if (CFStringGetCharacterAtIndex(name, 4) == '1')
            ti += 60 * 60 * 10;
          if (CFStringGetCharacterAtIndex(name, 3) == '-')
            ti = -ti;
        }
      else if (length == 0 && CFStringHasPrefix(name, CFSTR("GMT"))
          && ((c = CFStringGetCharacterAtIndex(name, 3)) == '+' || c == '-'))
        {
          // GMT+NNNN and GMT-NNNN
          c = CFStringGetCharacterAtIndex(name, 4);
          if (c >= '0' && c <= '9')
            {
              ti = c - '0';
              c = CFStringGetCharacterAtIndex(name, 5);
              if (c >= '0' && c <= '9')
                {
                ti = ti * 10 + (c - '0');
                c = CFStringGetCharacterAtIndex(name, 6);
                if (c >= '0' && c <= '9')
                  {
                  ti = ti * 6 + (c - '0');
                  c = CFStringGetCharacterAtIndex(name, 7);
                  if (c >= '0' && c <= '9')
                    {
                      ti = ti * 10 + (c - '0');
                      ti = ti * 60;
                      if (CFStringGetCharacterAtIndex(name, 3) == '-')
                        ti = -ti;              
                    }
                  }
                }
            }
        }
      else
        {
          return NULL;
        }
      
      return CFTimeZoneCreateWithTimeIntervalFromGMT (alloc,
        (CFTimeInterval)ti);
    }
  
  top = ures_openDirect (NULL, ICU_ZONEINFO, &err);
  ures_getByKey (top, ICU_ZONES, top, &err);
  res = ures_getByIndex (top, idx, NULL, &err);
  if (ures_getType(res) == URES_INT) // This is an alias
    {
      idx = ures_getInt (res, &err);
      ures_getByIndex (top, idx, res, &err);
    }
  
  if (U_FAILURE(err))
    return NULL;
  
  new = (struct __CFTimeZone*)_CFRuntimeCreateInstance (alloc,
    _kCFTimeZoneTypeID, sizeof(struct __CFTimeZone) - sizeof(CFRuntimeBase),
    0);
  if (new == NULL)
    return NULL;
  
  new->_name = CFStringCreateCopy (alloc,
    CFArrayGetValueAtIndex(builtins, idx));
  
  if (U_SUCCESS(err))
    {
      UResourceBundle *info;
      const UChar *ruleName;
      int32_t len;
      
      info = ures_getByKey (res, ICU_TRANSPRE32, NULL, &err);
      new->_info.transPre32 = ures_getIntVector (info, &len, &err);
      new->_info.transPre32Count = len / 2;
      if (err == U_MISSING_RESOURCE_ERROR)
        {
          new->_info.transPre32 = NULL;
          new->_info.transPre32Count = 0;
          err = U_ZERO_ERROR;
        }
      
      ures_getByKey (res, ICU_TRANS, info, &err);
      new->_info.trans = ures_getIntVector (info, &len, &err);
      new->_info.transCount = len;
      if (err == U_MISSING_RESOURCE_ERROR)
        {
          new->_info.trans = NULL;
          new->_info.transCount = 0;
          err = U_ZERO_ERROR;
        }
      
      ures_getByKey (res, ICU_TRANSPOST32, info, &err);
      new->_info.transPost32 = ures_getIntVector (info, &len, &err);
      new->_info.transPost32Count = len * 2;
      if (err == U_MISSING_RESOURCE_ERROR)
        {
          new->_info.transPost32 = NULL;
          new->_info.transPost32Count = 0;
          err = U_ZERO_ERROR;
        }
      
      ures_getByKey (res, ICU_TYPEOFFSETS, info, &err);
      new->_info.typeOffsets = ures_getIntVector (info, NULL, &err);
      if (U_FAILURE(err))
        {
          ures_close (info);
          CFRelease (new);
          return NULL;
        }
      
      ures_getByKey (res, ICU_TYPEMAP, info, &err);
      new->_info.typeMap = ures_getBinary (info, &len, &err);
      if (U_FAILURE(err))
        {
          ures_close (info);
          CFRelease (new);
          return NULL;
        }
      
      ruleName = ures_getStringByKey (info, ICU_FINALRULE, &len, &err);
      ures_getByKey (res, ICU_FINALRAW, info, &err);
      new->_info.finalRaw = ures_getInt (info, &err);
      ures_getByKey (res, ICU_FINALYEAR, info, &err);
      new->_info.finalYear = ures_getInt (info, &err);
      if (U_SUCCESS(err))
        {
          char key[32];
          UResourceBundle *rule;
          u_strToUTF8 (key, 32, &len, ruleName, len, &err);
          rule = ures_getByKey (top, ICU_ZONERULES, NULL, &err);
          ures_getByKey (rule, key, rule, &err);
          if (ures_getType(rule) == URES_INT_VECTOR)
            {
              new->_info.finalRule = ures_getIntVector (rule, NULL, &err);
              if (U_FAILURE(err)) // Shouldn't ever happen...
                {
                  ures_close (rule);
                  CFRelease (new);
                  return NULL;
                }
            }
          
          ures_close (rule);
        }
      else
        {
          new->_info.finalRule = NULL;
          new->_info.finalRaw = 0;
          new->_info.finalYear = 0;
        }
      
      ures_close (info);
    }
  
  ures_close (res);
  ures_close (top);
  
  return (CFTimeZoneRef)new;
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
  pthread_mutex_lock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneDefault != NULL)
    CFRelease (_kCFTimeZoneDefault);
  _kCFTimeZoneDefault = CFRetain (tz);
  pthread_mutex_unlock (&_kCFTimeZoneLock);
}

CFArrayRef
CFTimeZoneCopyKnownNames (void)
{
  CFIndex builtinLen;
  CFIndex customLen;
  CFArrayRef builtins;
  CFArrayRef customs;
  CFArrayRef allNames;
  CFArrayRef ret;
  
  builtins = CFTimeZoneGetBuiltinNames ();
  builtinLen = CFArrayGetCount (builtins);
  customs = CFTimeZoneGetCustomNames ();
  customLen = CFArrayGetCount (customs);
  
  allNames = CFArrayCreateMutable (NULL, builtinLen + customLen,
    &kCFTypeArrayCallBacks);
  CFArrayAppendArray (allNames, builtins, CFRangeMake(0, builtinLen));
  CFArrayAppendArray (allNames, customs, CFRangeMake(0, customLen));
  
  ret = CFArrayCreateCopy (NULL, allNames);
  CFRelease (allNames);
  
  return ret;
}

void
CFTimeZoneResetSystem (void)
{
  pthread_mutex_lock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneSystem != NULL)
    {
      CFRelease (_kCFTimeZoneSystem);
      _kCFTimeZoneSystem = NULL;
    }
  pthread_mutex_unlock (&_kCFTimeZoneLock);
}

void
CFTimeZoneSetAbbreviationDictionary (CFDictionaryRef dict)
{
  pthread_mutex_lock (&_kCFTimeZoneLock);
  if (_kCFTimeZoneAbbreviations != NULL)
    CFRelease (_kCFTimeZoneAbbreviations);
  _kCFTimeZoneAbbreviations = CFRetain (dict);
  pthread_mutex_unlock (&_kCFTimeZoneLock);
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
  return NULL; // FIXME
}

CFTimeInterval
CFTimeZoneGetSecondsFromGMT (CFTimeZoneRef tz, CFAbsoluteTime at)
{
  return 0.0; // FIXME
}

CFDataRef
CFTimeZoneGetData (CFTimeZoneRef tz)
{
  return tz->_data;
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
