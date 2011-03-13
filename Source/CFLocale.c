/* CFLocale.c
   
   Copyright (C) 2010 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: March, 2011
   
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

#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFRuntime.h"

#include <pthread.h>
#include <unicode/uloc.h>
#include <unicode/ulocdata.h>

#define BUFFER_SIZE 1024

struct __CFLocale
{
  /* The ivar order is important... must be exactly like NSLocale's. */
  CFRuntimeBase          _parent;
  CFStringRef            _identifier;
  CFMutableDictionaryRef _components;
};

/* Use a spin lock because it's faster than a mutex.  Just make sure to not
   keep it locked for very long. */
static pthread_spinlock_t _kCFLocaleLock;

static CFTypeID _kCFLocaleTypeID;
static CFLocaleRef _kCFLocaleCurrent = NULL;
static CFLocaleRef _kCFLocaleSystem = NULL;

static const CFRuntimeClass CFLocaleClass =
{
  0,
  "CFLocale",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

void CFLocaleInitialize (void)
{
  pthread_spin_init (&_kCFLocaleLock, PTHREAD_PROCESS_SHARED);
  _kCFLocaleTypeID = _CFRuntimeRegisterClass(&CFLocaleClass);
}



CFLocaleRef
CFLocaleCopyCurrent (void)
{
  CFLocaleRef result;
  
  pthread_spin_lock (&_kCFLocaleLock);
  if (_kCFLocaleCurrent)
    {
      result = (CFLocaleRef)CFRetain ((CFTypeRef)_kCFLocaleCurrent);
      pthread_spin_unlock (&_kCFLocaleLock);
      return result;
    }
  pthread_spin_unlock (&_kCFLocaleLock);
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, NULL);
  
  pthread_spin_lock (&_kCFLocaleLock);
  _kCFLocaleCurrent = (CFLocaleRef)CFRetain ((CFTypeRef)result);
  pthread_spin_unlock (&_kCFLocaleLock);
  return result;
}

CFLocaleRef
CFLocaleCreate (CFAllocatorRef allocator,
                CFStringRef localeIdent)
{
  struct __CFLocale *new;
  
  new = (struct __CFLocale *)_CFRuntimeCreateInstance (allocator,
    CFLocaleGetTypeID(),
    sizeof(struct __CFLocale) - sizeof(CFRuntimeBase),
    NULL);
  
  new->_identifier =
    CFLocaleCreateCanonicalLocaleIdentifierFromString (allocator, localeIdent);
  new->_components = CFDictionaryCreateMutable (allocator,
    0,
    &kCFTypeDictionaryKeyCallBacks,
    &kCFTypeDictionaryValueCallBacks);
  
  return new;
}

CFLocaleRef
CFLocaleCreateCopy (CFAllocatorRef allocator,
                    CFLocaleRef locale)
{
  return (CFLocaleRef)CFRetain((CFTypeRef)locale);
}

CFLocaleRef
CFLocaleGetSystem (void)
{
  CFLocaleRef result;
  pthread_spin_lock (&_kCFLocaleLock);
  if (_kCFLocaleSystem)
    {
      result = (CFLocaleRef)CFRetain ((CFTypeRef)_kCFLocaleSystem);
      pthread_spin_unlock (&_kCFLocaleLock);
      return result;
    }
  pthread_spin_unlock (&_kCFLocaleLock);
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, CFSTR(""));
  
  pthread_spin_lock (&_kCFLocaleLock);
  _kCFLocaleSystem = (CFLocaleRef)CFRetain ((CFTypeRef)result);
  pthread_spin_unlock (&_kCFLocaleLock);
  return result;
}

CFArrayRef
CFLocaleCopyAvailableLocaleIdentifiers (void)
{
  int32_t count;
  int32_t idx;
  CFMutableArrayRef mArray;
  static CFArrayRef result = NULL;
  
  if (result)
    return (CFArrayRef)CFRetain (result);
  
  count = uloc_countAvailable ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  idx = 0;
  while (count > idx)
    {
      const char *str = uloc_getAvailable (idx);
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        str, kCFStringEncodingASCII);
      CFArrayAppendValue (mArray, cfStr);
      CFRelease ((CFTypeRef)cfStr);
      ++idx;
    }
  
  result = CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  CFRelease (mArray);
  return result;
}

CFArrayRef
CFLocaleCopyISOCountryCodes (void)
{
  const char *const *cCodes;
  CFMutableArrayRef mArray;
  int idx;
  static CFArrayRef result = NULL;
  
  if (result)
    return (CFArrayRef)CFRetain (result);
  
  cCodes = uloc_getISOCountries ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  for (idx = 0 ; cCodes[idx] ; ++idx)
    {
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        cCodes[idx], kCFStringEncodingASCII);
      CFArrayAppendValue (mArray, cfStr);
      CFRelease ((CFTypeRef)cfStr);
    }
  
  result = CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  CFRelease (mArray);
  return result;
}

CFArrayRef
CFLocaleCopyISOLanguageCodes (void)
{
  const char *const *cCodes;
  CFMutableArrayRef mArray;
  int idx;
  static CFArrayRef result = NULL;
  
  if (result)
    return (CFArrayRef)CFRetain (result);
  
  cCodes = uloc_getISOLanguages ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  for (idx = 0 ; cCodes[idx] ; ++idx)
    {
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        cCodes[idx], kCFStringEncodingASCII);
      CFArrayAppendValue (mArray, cfStr);
      CFRelease ((CFTypeRef)cfStr);
    }
  
  result = CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  CFRelease (mArray);
  return result;
}

CFArrayRef
CFLocaleCopyISOCurrencyCodes (void);

CFArrayRef
CFLocaleCopyCommonISOCurrencyCodes (void);

CFArrayRef
CFLocaleCopyPreferredLanguages (void);

CFStringRef
CFLocaleCopyDisplayNameForPropertyValue (CFLocaleRef displayLocale,
                                         CFStringRef key,
                                         CFStringRef value)
{
/*  char kw[ULOC_KEYWORDS_CAPACITY];
  UniChar buffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  if (CFStringGetCString (key, buffer, BUFFER_SIZE-1, kCFStringEncodingASCII)
      && CFStringGetCString ()
  
  length = uloc_getDisplayKeywordValue (cLocale, kw, dLocale, buffer,
    BUFFER_SIZE, &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR)
    return NULL;
  if (length > BUFFER_SIZE)
    length = BUFFER_SIZE;
  
  return CFStringCreateWithCharacters (NULL, buffer, length);*/
  return NULL;
}

CFTypeRef
CFLocaleGetValue (CFLocaleRef locale,
                  CFStringRef key)
{
  CFTypeRef result;
  int32_t length;
  char cLocale[ULOC_FULLNAME_CAPACITY];
  char kw[ULOC_KEYWORDS_CAPACITY];
  char buffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (locale->_identifier, cLocale,
         ULOC_FULLNAME_CAPACITY-1, CFStringGetSystemEncoding())
      && !CFStringGetCString (key, kw, ULOC_KEYWORDS_CAPACITY-1,
            CFStringGetSystemEncoding()))
    return NULL;
  
  length = uloc_getKeywordValue (cLocale, kw, buffer, BUFFER_SIZE-1, &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR)
    return NULL;
  
  result =
    CFStringCreateWithCString (NULL, buffer, CFStringGetSystemEncoding());
  
  return result;
}

CFStringRef
CFLocaleGetIdentifier (CFLocaleRef locale)
{
  return locale->_identifier;
}

CFStringRef
CFLocaleCreateCanonicalLocaleIdentifierFromString (CFAllocatorRef allocator,
                                                   CFStringRef localeIdent)
{
  CFStringRef result;
  char cLocale[ULOC_FULLNAME_CAPACITY];
  char buffer[BUFFER_SIZE];
  int32_t length;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString(localeIdent, cLocale, ULOC_FULLNAME_CAPACITY,
         CFStringGetSystemEncoding()))
    return NULL;
  
  length = uloc_canonicalize (cLocale, buffer, BUFFER_SIZE-1, &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR)
    return NULL;
  
  result =
    CFStringCreateWithCString (allocator, buffer, CFStringGetSystemEncoding());
  
  return result;
}

CFStringRef
CFLocaleCreateCanonicalLanguageIdentifierFromString (CFAllocatorRef allocator,
                                                     CFStringRef localeIdent);

CFDictionaryRef
CFLocaleCreateComponentsFromLocaleIdentifier (CFAllocatorRef allocator,
                                              CFStringRef localeIdent);

CFStringRef
CFLocaleCreateLocaleIdentifierFromComponents (CFAllocatorRef allocator,
                                              CFDictionaryRef dictionary);

CFTypeID
CFLocaleGetTypeID (void)
{
  return _kCFLocaleTypeID;
}

CFStringRef
CFLocaleCreateLocaleIdentifierFromWindowsLocaleCode (CFAllocatorRef allocator,
                                                     uint32_t lcid);

CFLocaleLanguageDirection
CFLocaleGetLanguageCharacterDirection (CFStringRef isoLangCode);

CFLocaleLanguageDirection
CFLocaleGetLanguageLineDirection (CFStringRef isoLangCode);

uint32_t
CFLocaleGetWindowsLocaleCodeFromLocaleIdentifier (CFStringRef localeIdent)
{
  char buffer[BUFFER_SIZE];
  if (CFStringGetCString (localeIdent, buffer, BUFFER_SIZE - 1,
      kCFStringEncodingASCII))
    return uloc_getLCID (buffer);
  return 0;
}
