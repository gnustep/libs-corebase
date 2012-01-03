/* CFLocale.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
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

#include <stdio.h>
#include <string.h>
#include <unicode/uloc.h>
#include <unicode/ulocdata.h>
#include <unicode/ucurr.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFLocale.h"
#include "GSPrivate.h"

#define BUFFER_SIZE 256

#define ICU_CALENDAR_KEY "calendar"
#define ICU_COLLATION_KEY "collation"
#define ICU_CURRENCY_KEY "currency"

struct __CFLocale
{
  /* The ivar order is important... must be exactly like NSLocale's. */
  CFRuntimeBase          _parent;
  CFStringRef            _identifier;
  CFMutableDictionaryRef _components;
};

static GSMutex _kCFLocaleLock;
static CFTypeID _kCFLocaleTypeID = 0;
static CFLocaleRef _kCFLocaleCurrent = NULL;
static CFLocaleRef _kCFLocaleSystem = NULL;
static CFArrayRef _kCFLocaleAvailableLocaleIdentifiers = NULL;
static CFArrayRef _kCFLocaleISOCountryCodes = NULL;
static CFArrayRef _kCFLocaleISOLanguageCodes = NULL;

static void CFLocaleFinalize (CFTypeRef cf)
{
  struct __CFLocale *o = (struct __CFLocale *)cf;
  CFRelease (o->_identifier);
  CFRelease (o->_components);
}

static Boolean CFLocaleEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  return CFEqual (((CFLocaleRef)cf1)->_identifier,
    ((CFLocaleRef)cf2)->_identifier);
}

static CFHashCode CFLocaleHash (CFTypeRef cf)
{
  return CFHash (((CFLocaleRef)cf)->_identifier);
}

static CFStringRef CFLocaleCopyFormattingDesc (CFTypeRef cf,
  CFDictionaryRef formatOptions)
{
  return CFRetain (((CFLocaleRef)cf)->_identifier);
}

static const CFRuntimeClass CFLocaleClass =
{
  0,
  "CFLocale",
  NULL,
  (CFTypeRef (*)(CFAllocatorRef, CFTypeRef))CFLocaleCreateCopy,
  CFLocaleFinalize,
  CFLocaleEqual,
  CFLocaleHash,
  CFLocaleCopyFormattingDesc,
  NULL
};

void CFLocaleInitialize (void)
{
  _kCFLocaleTypeID = _CFRuntimeRegisterClass(&CFLocaleClass);
  GSMutexInitialize (&_kCFLocaleLock);
}

static inline CFLocaleLanguageDirection
ICUToCFLocaleOrientation (ULayoutType layout)
{
  switch (layout)
    {
      case ULOC_LAYOUT_LTR:
        return kCFLocaleLanguageDirectionLeftToRight;
      case ULOC_LAYOUT_RTL:
        return kCFLocaleLanguageDirectionRightToLeft;
      case ULOC_LAYOUT_TTB:
        return kCFLocaleLanguageDirectionTopToBottom;
      case ULOC_LAYOUT_BTT:
        return kCFLocaleLanguageDirectionBottomToTop;
      default:
        return kCFLocaleLanguageDirectionUnknown;
    }
}

CF_INLINE const char *
CFLocaleGetCStringIdentifier_inline (CFLocaleRef locale)
{
  // This works because CFLocaleCreateCanonicalIdentifierFromString()
  // outputs ASCII characters.
  return CFStringGetCStringPtr (locale->_identifier, kCFStringEncodingASCII);
}

const char *
CFLocaleGetCStringIdentifier (CFLocaleRef locale)
{
  return CFLocaleGetCStringIdentifier_inline (locale);
}

static CFArrayRef
CFArrayCreateArrayWithUEnumeration (UEnumeration *en)
{
  CFMutableArrayRef mArray;
  CFArrayRef result;
  int32_t count;
  int32_t len;
  const UChar *current;
  UErrorCode err = U_ZERO_ERROR;
  
  count = uenum_count (en, &err);
  if (U_FAILURE(err))
    return NULL;
  mArray = CFArrayCreateMutable (NULL, (CFIndex)count, &kCFTypeArrayCallBacks);
  while ((current = uenum_unext(en, &len, &err)))
    {
      if (U_FAILURE(err))
        continue;
      CFStringRef string =
        CFStringCreateWithCharacters (NULL, current, (CFIndex)len);
      CFArrayAppendValue (mArray, string);
    }
  
  // Close it UEnumeration here so it doesn't get leaked.
  uenum_close (en);
  
  result = CFArrayCreateCopy (NULL, mArray);
  CFRelease (mArray);
  return result;
}

static CFTypeRef
CFLocaleCopyMeasurementSystem (CFLocaleRef loc, const void *context)
{
  CFTypeRef result;
  UMeasurementSystem ums;
  const char *cLocale;
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier_inline (loc);
  ums = ulocdata_getMeasurementSystem (cLocale, &err);
  if (CFEqual(*((CFTypeRef*)context), kCFLocaleMeasurementSystem))
    {
      if (ums == UMS_SI)
        result = CFSTR("Metric");
      else
        result = CFSTR("U.S.");
    }
  else
    {
      if (ums == UMS_SI)
        result = kCFBooleanTrue;
      else
        result = kCFBooleanFalse;
    }
  
  return result;
}

struct _kCFNumberFormatterProperty
{
  const CFStringRef *key;
  CFNumberFormatterStyle style;
};

struct _kCFNumberFormatterProperty decimalSep =
  { &kCFNumberFormatterDecimalSeparator, kCFNumberFormatterDecimalStyle };
struct _kCFNumberFormatterProperty groupingSep =
  { &kCFNumberFormatterGroupingSeparator, kCFNumberFormatterDecimalStyle };
struct _kCFNumberFormatterProperty currencySym =
  { &kCFNumberFormatterCurrencySymbol, kCFNumberFormatterCurrencyStyle };
struct _kCFNumberFormatterProperty currecyCode =
  { &kCFNumberFormatterCurrencyCode, kCFNumberFormatterCurrencyStyle };

static CFTypeRef
CFLocaleCopyNumberFormatterProperty (CFLocaleRef loc, const void *context)
{
  CFTypeRef result;
  CFNumberFormatterRef fmt;
  struct _kCFNumberFormatterProperty *c =
    (struct _kCFNumberFormatterProperty*)context;
  
  fmt = CFNumberFormatterCreate (NULL, loc, c->style); 
  result = CFNumberFormatterCopyProperty (fmt, *(c->key));
  
  CFRelease (fmt);
  return result;
}

static CFTypeRef
CFLocaleCopyIdentifierProperty (CFLocaleRef loc, const void *context)
{
  CFTypeRef result;
  const char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  int32_t length;
  int32_t (*func)(const char*, char*, int32_t, UErrorCode*) = context;
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier_inline (loc);
  length = (*func)(cLocale, buffer, ULOC_FULLNAME_CAPACITY, &err);
  if (U_FAILURE(err) || length <= 0)
    result = NULL;
  else
    result = CFStringCreateWithCString (NULL, buffer, kCFStringEncodingUTF8);
  
  return result;
}

static CFTypeRef
CFLocaleCopyKeyword (CFLocaleRef loc, const void *context)
{
  CFTypeRef result = NULL;
  const char *cLocale;
  char buffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier_inline (loc);
  if (uloc_getKeywordValue (cLocale, context, buffer,
      BUFFER_SIZE, &err) > 0 && U_SUCCESS(err))
    {
      if (context == (const void*)ICU_CALENDAR_KEY)
        {
          char *calIdent = buffer;
          if (strncmp(calIdent, "gregorian", sizeof("gregorian")-1) == 0)
            result = kCFGregorianCalendar;
          else
            result = NULL;
        }
      else
        {
          result = CFStringCreateWithCString (NULL, buffer,
            kCFStringEncodingUTF8);
        }
    
    }
  else
    {
      if (context == (const void*)ICU_CALENDAR_KEY)
        result = kCFGregorianCalendar;
    }
  
  return result;
}

static CFTypeRef
CFLocaleCopyCalendar (CFLocaleRef locale, const void *context)
{
  CFTypeRef result;
  CFStringRef calId;
  CFAllocatorRef allocator = CFGetAllocator (locale);
  int len;
  const char *cLocale;
  char buffer[ULOC_KEYWORDS_CAPACITY];
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier_inline (locale);
  len = uloc_getKeywordValue (cLocale, ICU_CALENDAR_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  if (U_SUCCESS(err) && len > 0)
    calId = CFStringCreateWithCString (allocator, buffer,
      kCFStringEncodingUTF8);
  else
    calId = kCFGregorianCalendar;
  
  result = CFCalendarCreateWithIdentifier (allocator, calId);
  CFRelease (calId);
  
  return result;
}

static CFTypeRef
CFLocaleCopyDelimiter (CFLocaleRef loc, const void *context)
{
  const char *cLocale;
  UniChar ubuffer[BUFFER_SIZE];
  CFIndex length;
  CFTypeRef result;
  ULocaleData *uld;
  UErrorCode err = U_ZERO_ERROR;
  
  cLocale = CFLocaleGetCStringIdentifier_inline (loc);
  uld = ulocdata_open (cLocale, &err);
  
  length = ulocdata_getDelimiter (uld, (ULocaleDataDelimiterType)context,
    ubuffer, BUFFER_SIZE, &err);
  
  if (U_FAILURE(err))
    return NULL;
  
  result = CFStringCreateWithCharacters (NULL, ubuffer, length);
  return result;
}

static CFTypeRef
CFLocaleReturnNull (CFLocaleRef loc, const void *context)
{
  return NULL;
}

static struct _kCFLocaleValues
{
  const CFStringRef *value;
  const void *context;
  CFTypeRef (*copy)(CFLocaleRef loc, const void *context);
} _kCFLocaleValues[] =
{
  { &kCFLocaleMeasurementSystem, &kCFLocaleMeasurementSystem,
      CFLocaleCopyMeasurementSystem },
  { &kCFLocaleDecimalSeparator, &decimalSep,
      CFLocaleCopyNumberFormatterProperty },
  { &kCFLocaleGroupingSeparator, &groupingSep,
      CFLocaleCopyNumberFormatterProperty },
  { &kCFLocaleCurrencySymbol, &currencySym,
      CFLocaleCopyNumberFormatterProperty },
  { &kCFLocaleCurrencyCode, &currecyCode,
      CFLocaleCopyNumberFormatterProperty },
  { &kCFLocaleLanguageCode, (const void*)&uloc_getLanguage,
      CFLocaleCopyIdentifierProperty },
  { &kCFLocaleCountryCode, (const void*)&uloc_getCountry,
      CFLocaleCopyIdentifierProperty },
  { &kCFLocaleScriptCode, (const void*)&uloc_getScript,
      CFLocaleCopyIdentifierProperty },
  { &kCFLocaleVariantCode, (const void*)&uloc_getVariant, 
      CFLocaleCopyIdentifierProperty },
  { &kCFLocaleExemplarCharacterSet, NULL, CFLocaleReturnNull },
  { &kCFLocaleCalendarIdentifier, ICU_CALENDAR_KEY,
      CFLocaleCopyKeyword },
  { &kCFLocaleCalendar, NULL, CFLocaleCopyCalendar },
  { &kCFLocaleCollationIdentifier, ICU_COLLATION_KEY,
      CFLocaleCopyKeyword },
  { &kCFLocaleUsesMetricSystem, &kCFLocaleUsesMetricSystem,
      CFLocaleCopyMeasurementSystem },
  { &kCFLocaleCollatorIdentifier, NULL, CFLocaleReturnNull },
  { &kCFLocaleQuotationBeginDelimiterKey,
      (const void*)ULOCDATA_QUOTATION_START, CFLocaleCopyDelimiter },
  { &kCFLocaleQuotationEndDelimiterKey,
      (const void*)ULOCDATA_QUOTATION_END, CFLocaleCopyDelimiter },
  { &kCFLocaleAlternateQuotationBeginDelimiterKey,
      (const void*)ULOCDATA_ALT_QUOTATION_START, CFLocaleCopyDelimiter },
  { &kCFLocaleAlternateQuotationEndDelimiterKey,
      (const void*)ULOCDATA_ALT_QUOTATION_END, CFLocaleCopyDelimiter }
};
static const CFIndex
_kCFLocaleValuesSize = sizeof(_kCFLocaleValues) /
  sizeof(struct _kCFLocaleValues);



CFLocaleRef
CFLocaleCopyCurrent (void)
{
  CFLocaleRef result;
  
  GSMutexLock (&_kCFLocaleLock);
  if (_kCFLocaleCurrent)
    {
      result = (CFLocaleRef)CFRetain (_kCFLocaleCurrent);
      GSMutexUnlock (&_kCFLocaleLock);
      return result;
    }
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, NULL);
  
  _kCFLocaleCurrent = (CFLocaleRef)CFRetain (result);
  GSMutexUnlock (&_kCFLocaleLock);
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
  return (CFLocaleRef)CFRetain(locale);
}

CFLocaleRef
CFLocaleGetSystem (void)
{
  CFLocaleRef result;
  GSMutexLock (&_kCFLocaleLock);
  if (_kCFLocaleSystem)
    {
      result = (CFLocaleRef)CFRetain (_kCFLocaleSystem);
      GSMutexUnlock (&_kCFLocaleLock);
      return result;
    }
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, CFSTR(""));
  
  _kCFLocaleSystem = (CFLocaleRef)CFRetain (result);
  GSMutexUnlock (&_kCFLocaleLock);
  return result;
}

CFArrayRef
CFLocaleCopyAvailableLocaleIdentifiers (void)
{
  int32_t count;
  int32_t idx;
  CFMutableArrayRef mArray;
  
  GSMutexLock (&_kCFLocaleLock);
  if (_kCFLocaleAvailableLocaleIdentifiers)
    {
      GSMutexUnlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleAvailableLocaleIdentifiers);
    }
  
  count = uloc_countAvailable ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0, &kCFTypeArrayCallBacks);
  
  for (idx = 0 ; idx < count ; ++idx)
    {
      const char *str = uloc_getAvailable (idx);
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        str, kCFStringEncodingUTF8);
      CFArrayAppendValue (mArray, cfStr);
      CFRelease (cfStr);
    }
  
  _kCFLocaleAvailableLocaleIdentifiers =
    CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  GSMutexUnlock (&_kCFLocaleLock);
  CFRelease (mArray);
  return (CFArrayRef)CFRetain (_kCFLocaleAvailableLocaleIdentifiers);
}

static CFArrayRef
CFLocaleCreateArrayWithCodes (const char *const *codes)
{
  CFArrayRef array;
  CFMutableArrayRef mArray;
  CFStringRef str;
  CFIndex idx;
  
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0, &kCFTypeArrayCallBacks);
  
  for (idx = 0 ; codes[idx] ; ++idx)
    {
      str = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        codes[idx], kCFStringEncodingUTF8);
      CFArrayAppendValue (mArray, str);
      CFRelease (str);
    }
  
  array = CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  CFRelease (mArray);
  return array;
}

CFArrayRef
CFLocaleCopyISOCountryCodes (void)
{
  const char *const *cCodes;
  
  GSMutexLock (&_kCFLocaleLock);
  if (_kCFLocaleISOCountryCodes)
    {
      GSMutexUnlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleISOCountryCodes);
    }
  
  cCodes = uloc_getISOCountries ();
  _kCFLocaleISOCountryCodes = CFLocaleCreateArrayWithCodes (cCodes);
  GSMutexUnlock (&_kCFLocaleLock);
  
  return (CFArrayRef)CFRetain (_kCFLocaleISOCountryCodes);
}

CFArrayRef
CFLocaleCopyISOLanguageCodes (void)
{
  const char *const *cCodes;
  
  GSMutexLock (&_kCFLocaleLock);
  if (_kCFLocaleISOLanguageCodes)
    {
      GSMutexUnlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleISOLanguageCodes);
    }
  
  cCodes = uloc_getISOLanguages ();
  _kCFLocaleISOLanguageCodes = CFLocaleCreateArrayWithCodes (cCodes);
  GSMutexUnlock (&_kCFLocaleLock);
  
  return (CFArrayRef)CFRetain (_kCFLocaleISOLanguageCodes);
}

CFArrayRef
CFLocaleCopyISOCurrencyCodes (void)
{
  UEnumeration *en;
  UErrorCode err = U_ZERO_ERROR;
  
  en = ucurr_openISOCurrencies (UCURR_ALL, &err);
  if (U_FAILURE(err))
    return NULL;
  return CFArrayCreateArrayWithUEnumeration (en);
}

CFArrayRef
CFLocaleCopyCommonISOCurrencyCodes (void)
{
  UEnumeration *en;
  UErrorCode err = U_ZERO_ERROR;
  
  en = ucurr_openISOCurrencies (UCURR_COMMON, &err);
  if (U_FAILURE(err))
    return NULL;
  return CFArrayCreateArrayWithUEnumeration (en);
}

CFArrayRef
CFLocaleCopyPreferredLanguages (void);

CFStringRef
CFLocaleCopyDisplayNameForPropertyValue (CFLocaleRef displayLocale,
                                         CFStringRef key,
                                         CFStringRef value)
{
  char locale[ULOC_FULLNAME_CAPACITY];
  char valueBuffer[BUFFER_SIZE];
  UChar buffer[BUFFER_SIZE];
  int32_t len;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (displayLocale->_identifier, locale,
      ULOC_FULLNAME_CAPACITY, kCFStringEncodingUTF8))
    return NULL;
  
  if (!CFStringGetCString (value, valueBuffer, BUFFER_SIZE,
      kCFStringEncodingUTF8))
    return NULL;
  
#define GET_DISPLAY_VALUE(value, func) \
  len = func (value, locale, buffer, BUFFER_SIZE, &err)
  if (key == kCFLocaleIdentifier)
    GET_DISPLAY_VALUE(valueBuffer, uloc_getDisplayName);
  else if (key == kCFLocaleLanguageCode)
    GET_DISPLAY_VALUE(valueBuffer, uloc_getDisplayLanguage);
  else if (key == kCFLocaleCountryCode)
    GET_DISPLAY_VALUE(valueBuffer, uloc_getDisplayCountry);
  else if (key == kCFLocaleScriptCode)
    GET_DISPLAY_VALUE(valueBuffer, uloc_getDisplayScript);
  else if (key == kCFLocaleVariantCode)
    GET_DISPLAY_VALUE(valueBuffer, uloc_getDisplayVariant);
  else if (key == kCFLocaleCalendarIdentifier)
    len = uloc_getDisplayKeywordValue (valueBuffer, ICU_CALENDAR_KEY, locale,
      buffer, BUFFER_SIZE, &err);
  else if (key == kCFLocaleCollationIdentifier)
    len = uloc_getDisplayKeywordValue (valueBuffer, ICU_COLLATION_KEY, locale,
      buffer, BUFFER_SIZE, &err);
  else if (key == kCFLocaleCurrencyCode)
    len = uloc_getDisplayKeywordValue (valueBuffer, ICU_CURRENCY_KEY, locale,
      buffer, BUFFER_SIZE, &err);
  else
    len = 0;
  
  if (U_SUCCESS(err) && len > 0)
    return CFStringCreateWithCharacters (NULL, buffer, len);
  
  return NULL;
}

CFTypeRef
CFLocaleGetValue (CFLocaleRef locale,
                  CFStringRef key)
{
  CFTypeRef result = NULL;
  CFIndex idx;
  Boolean found = false;
  
  // Don't waste any time.
  if (locale == NULL || key == NULL)
    return NULL;
  if (key == kCFLocaleIdentifier)
    return locale->_identifier;
  
  // Make sure we haven't been through this already.
  if (CFDictionaryGetValueIfPresent(locale->_components, key, &result))
    return result;
  
  for (idx = 0 ; idx < _kCFLocaleValuesSize && !found ; ++idx)
    {
      if (key == *(_kCFLocaleValues[idx].value))
        {
          result = (_kCFLocaleValues[idx].copy)(locale,
            _kCFLocaleValues[idx].context);
          found = true;
        }
    }
  
  if (found == false)
    {
      for (idx = 0 ; idx < _kCFLocaleValuesSize && !found ; ++idx)
        {
          if (CFEqual(key, *(_kCFLocaleValues[idx].value)))
            {
              result = (_kCFLocaleValues[idx].copy)(locale,
                _kCFLocaleValues[idx].context);
              found = true;
            }
        }
    }
  
  if (result != NULL)
    CFDictionaryAddValue (locale->_components, key, result);
  CFRelease (result);
  
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
  char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  char canonical[ULOC_FULLNAME_CAPACITY];
  UErrorCode err = U_ZERO_ERROR;
  
  if (localeIdent == NULL)
    {
      cLocale = (char *)uloc_getDefault ();
    }
  else
    {
      if (!CFStringGetCString(localeIdent, buffer, ULOC_FULLNAME_CAPACITY,
             kCFStringEncodingUTF8))
        return NULL;
      cLocale = buffer;
    }
  
  uloc_canonicalize (cLocale, canonical, ULOC_FULLNAME_CAPACITY, &err);
  if (U_FAILURE(err))
    return NULL;
  
  return CFStringCreateWithCString (allocator, canonical,
    kCFStringEncodingUTF8);
}

CFStringRef
CFLocaleCreateCanonicalLanguageIdentifierFromString (CFAllocatorRef allocator,
                                                     CFStringRef localeIdent)
{
  CFStringRef result;
  char cLocale[ULOC_FULLNAME_CAPACITY];
  char canonical[ULOC_FULLNAME_CAPACITY];
  char lang[ULOC_LANG_CAPACITY];
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString(localeIdent, cLocale, ULOC_FULLNAME_CAPACITY,
         kCFStringEncodingUTF8))
    return NULL;
  
  uloc_canonicalize (cLocale, canonical, BUFFER_SIZE, &err);
  uloc_getLanguage (canonical, lang, ULOC_LANG_CAPACITY, &err);
  if (U_FAILURE(err))
    return NULL;
  
  result =
    CFStringCreateWithCString (allocator, lang, kCFStringEncodingUTF8);
  
  return result;
}

static void
CFLocaleAddKeyValuePairToDictionary (CFMutableDictionaryRef dict,
  CFStringRef key, char *str, int32_t length, UErrorCode error)
{
  if (U_SUCCESS(error) && length > 0)
    {
      CFStringRef value = CFStringCreateWithCString (NULL, str,
        kCFStringEncodingUTF8);
      CFDictionaryAddValue (dict, key, value);
      CFRelease (value);
    }
  error = U_ZERO_ERROR;
}

CFDictionaryRef
CFLocaleCreateComponentsFromLocaleIdentifier (CFAllocatorRef allocator,
                                              CFStringRef localeIdent)
{
  char locale[ULOC_FULLNAME_CAPACITY];
  char buffer[ULOC_KEYWORDS_CAPACITY];
  CFDictionaryRef result;
  CFMutableDictionaryRef dict;
  int32_t len;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (localeIdent, locale, ULOC_FULLNAME_CAPACITY,
      kCFStringEncodingUTF8))
    return NULL;
  
  /* Using capacity = 7 because that's the most amount of keys we'll have:
      language_script_country_variant@calendar;collation;currency.
  */
  dict = CFDictionaryCreateMutable (NULL, 7,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  len = uloc_getLanguage (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleLanguageCode, buffer,
    len, err);

  len = uloc_getCountry (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleCountryCode, buffer,
    len, err);

  len = uloc_getScript (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleScriptCode, buffer,
    len, err);

  len = uloc_getVariant (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleVariantCode, buffer,
    len, err);
  
  len = uloc_getKeywordValue (locale, ICU_CALENDAR_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleCalendarIdentifier,
    buffer, len, err);
  
  len = uloc_getKeywordValue (locale, ICU_COLLATION_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleCollationIdentifier,
    buffer, len, err);
  
  len = uloc_getKeywordValue (locale, ICU_CURRENCY_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  CFLocaleAddKeyValuePairToDictionary (dict, kCFLocaleCurrencyCode, buffer,
    len, err);
  
  result = CFDictionaryCreateCopy (allocator, (CFDictionaryRef)dict);
  CFRelease(dict);
  return result;
}

CFStringRef
CFLocaleCreateLocaleIdentifierFromComponents (CFAllocatorRef allocator,
                                              CFDictionaryRef dictionary)
{
  /* A locale identifier only includes a few of the keys provided
     by CFLocale.  According to UTS #35 (http://unicode.org/reports/tr35/tr35-6.html)
     locales have the following format:
        locale_id      := base_locale_id options?
        base_locale_id := extended_RFC3066bis_identifiers
        options        := "@" key "=" type ("," key "=" type )*
    Keep in mind, however, that ICU's key separator is ";".  This shouldn't
    be a problem as long as we use ULOC_KEYWORD_ITEM_SEPARATOR (and friends).
  */
  CFMutableStringRef locale;
  CFStringRef ret;
  CFStringRef lang;
  CFStringRef country;
  CFStringRef script;
  CFStringRef variant;
  CFStringRef keyword;
  Boolean hasCountry;
  Boolean hasScript;
  Boolean hasVariant;
  Boolean separated = false;
  
  if (dictionary == NULL)
    return NULL;
  
  /* We'll return NULL if kCFLocaleLanguageCode doesn't exist.  This is the
     only key that is absolutely necessary.
  */
  if (!CFDictionaryGetValueIfPresent(dictionary, kCFLocaleLanguageCode,
      (const void **)&lang))
    return NULL;
  
  hasCountry = CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCountryCode,
    (const void **)&country);
  hasScript = CFDictionaryGetValueIfPresent(dictionary, kCFLocaleScriptCode,
    (const void **)&script);
  hasVariant = CFDictionaryGetValueIfPresent(dictionary, kCFLocaleVariantCode,
    (const void **)&variant);
  
#define TEST_CODE(x, y) (x ? "_" : ""), (x ? y : CFSTR(""))
  locale = CFStringCreateMutable (NULL, ULOC_FULLNAME_CAPACITY);
  CFStringAppendFormat (locale, NULL, CFSTR("%@%s%@%s%@%s%@"), lang,
    TEST_CODE(hasScript, script), TEST_CODE(hasCountry, country),
    TEST_CODE(hasVariant, variant));
#undef TEST_CODE
  // Use uloc_setKeywordValue() here since libicu is a required library.
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCalendarIdentifier,
      (const void **)&keyword))
    {
      CFStringAppend (locale, CFSTR("@"));
      CFStringAppendFormat (locale, NULL, CFSTR("calendar=%@"), keyword);
      separated = true;
    }
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCollationIdentifier,
      (const void **)&keyword))
    {
      if (separated == false)
        CFStringAppend (locale, CFSTR("@"));
      else
        CFStringAppend (locale, CFSTR(";"));
      CFStringAppendFormat (locale, NULL, CFSTR("collation=%@"), keyword);
      separated = true;
    }
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCurrencyCode,
      (const void **)&keyword))
    {
      if (separated == false)
        CFStringAppend (locale, CFSTR("@"));
      else
        CFStringAppend (locale, CFSTR(";"));
      CFStringAppendFormat (locale, NULL, CFSTR("currency=%@"), keyword);
      separated = true;
    }
  
  ret = CFStringCreateCopy (allocator, locale);
  CFRelease (locale);
  return ret;
}

CFTypeID
CFLocaleGetTypeID (void)
{
  return _kCFLocaleTypeID;
}

CFStringRef
CFLocaleCreateLocaleIdentifierFromWindowsLocaleCode (CFAllocatorRef allocator,
                                                     uint32_t lcid)
{
  CFStringRef result = NULL;
  char buffer[BUFFER_SIZE];
  UErrorCode err = U_ZERO_ERROR;
  
  uloc_getLocaleForLCID (lcid, buffer, BUFFER_SIZE-1, &err);
  if (U_FAILURE(err) && err != U_BUFFER_OVERFLOW_ERROR)
    return NULL;
  
  result =
    CFStringCreateWithCString (allocator, buffer, kCFStringEncodingUTF8);
  
  return result;
}

CFLocaleLanguageDirection
CFLocaleGetLanguageCharacterDirection (CFStringRef isoLangCode)
{
  char buffer[BUFFER_SIZE];
  ULayoutType result;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (isoLangCode, buffer, BUFFER_SIZE - 1,
      kCFStringEncodingUTF8))
    return kCFLocaleLanguageDirectionUnknown;
  
  result = uloc_getCharacterOrientation (buffer, &err);
  if (U_FAILURE(err))
    return kCFLocaleLanguageDirectionUnknown;
  
  return ICUToCFLocaleOrientation (result);
}

CFLocaleLanguageDirection
CFLocaleGetLanguageLineDirection (CFStringRef isoLangCode)
{
  char buffer[BUFFER_SIZE];
  ULayoutType result;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (isoLangCode, buffer, BUFFER_SIZE - 1,
      kCFStringEncodingUTF8))
    return kCFLocaleLanguageDirectionUnknown;
  
  result = uloc_getLineOrientation (buffer, &err);
  if (U_FAILURE(err))
    return kCFLocaleLanguageDirectionUnknown;
  
  return ICUToCFLocaleOrientation (result);
}

uint32_t
CFLocaleGetWindowsLocaleCodeFromLocaleIdentifier (CFStringRef localeIdent)
{
  char buffer[BUFFER_SIZE];
  if (CFStringGetCString (localeIdent, buffer, BUFFER_SIZE - 1,
      kCFStringEncodingUTF8))
    return uloc_getLCID (buffer);
  return 0;
}
