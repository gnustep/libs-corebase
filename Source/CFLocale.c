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

#include <pthread.h>
#include <unicode/uloc.h>
#include <unicode/ulocdata.h>
#include <unicode/ucurr.h>
#include <unicode/ucal.h>

#include "CoreFoundation/CFBase.h"
#include "CoreFoundation/CFCalendar.h"
#include "CoreFoundation/CFNumberFormatter.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFRuntime.h"

#include "CoreFoundation/CFLocale.h"

#define BUFFER_SIZE 1024

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

/* Use a spin lock because it's faster than a mutex.  Just make sure to not
   keep it locked for very long. */
static pthread_mutex_t _kCFLocaleLock = PTHREAD_MUTEX_INITIALIZER;

static CFTypeID _kCFLocaleTypeID;
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
  NULL,
  CFLocaleFinalize,
  CFLocaleEqual,
  CFLocaleHash,
  CFLocaleCopyFormattingDesc,
  NULL
};

void CFLocaleInitialize (void)
{
  _kCFLocaleTypeID = _CFRuntimeRegisterClass(&CFLocaleClass);
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

static CFTypeRef
CFLocaleCopyNumberFormatterDecimalProperty (CFLocaleRef loc, CFStringRef key)
{
  CFTypeRef result;
  CFNumberFormatterRef fmt;
  
  fmt = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterDecimalStyle); 
  result = CFNumberFormatterCopyProperty (fmt, key);
  
  CFRelease (fmt);
  return result;
}

static CFTypeRef
CFLocaleCopyNumberFormatterCurrencyProperty (CFLocaleRef loc, CFStringRef key)
{
  CFTypeRef result;
  CFNumberFormatterRef fmt;
  
  fmt = CFNumberFormatterCreate (NULL, loc, kCFNumberFormatterCurrencyStyle); 
  result = CFNumberFormatterCopyProperty (fmt, key);
  
  CFRelease (fmt);
  return result;
}

static inline CFArrayRef
_createArrayWithUEnumeration (CFAllocatorRef allocator, UEnumeration *en)
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
        CFStringCreateWithCharacters (allocator, current, (CFIndex)len);
      CFArrayAppendValue (mArray, string);
    }
  
  // Close it UEnumeration here so it doesn't get leaked.
  uenum_close (en);
  
  result = CFArrayCreateCopy (allocator, mArray);
  CFRelease (mArray);
  return result;
}

static inline CFCalendarRef
_createCalendar (CFAllocatorRef allocator, CFLocaleRef locale)
{
  CFCalendarRef result;
  CFStringRef calId;
  int len;
  char cLocale[ULOC_FULLNAME_CAPACITY];
  char buffer[ULOC_KEYWORDS_CAPACITY];
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (CFLocaleGetIdentifier(locale), cLocale,
      ULOC_FULLNAME_CAPACITY, CFStringGetSystemEncoding()))
    return NULL;
  
  len = uloc_getKeywordValue (cLocale, ICU_CALENDAR_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  if (U_SUCCESS(err) && len > 0)
    calId = CFStringCreateWithCString (allocator, buffer,
      CFStringGetSystemEncoding());
  else
    calId = kCFGregorianCalendar;
  
  result = CFCalendarCreateWithIdentifier (allocator, calId);
  CFRelease (calId);
  
  return result;
}



CFLocaleRef
CFLocaleCopyCurrent (void)
{
  CFLocaleRef result;
  
  pthread_mutex_lock (&_kCFLocaleLock);
  if (_kCFLocaleCurrent)
    {
      result = (CFLocaleRef)CFRetain (_kCFLocaleCurrent);
      pthread_mutex_unlock (&_kCFLocaleLock);
      return result;
    }
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, NULL);
  
  _kCFLocaleCurrent = (CFLocaleRef)CFRetain (result);
  pthread_mutex_unlock (&_kCFLocaleLock);
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
  pthread_mutex_lock (&_kCFLocaleLock);
  if (_kCFLocaleSystem)
    {
      result = (CFLocaleRef)CFRetain (_kCFLocaleSystem);
      pthread_mutex_unlock (&_kCFLocaleLock);
      return result;
    }
  
  result = CFLocaleCreate (kCFAllocatorSystemDefault, CFSTR(""));
  
  _kCFLocaleSystem = (CFLocaleRef)CFRetain (result);
  pthread_mutex_unlock (&_kCFLocaleLock);
  return result;
}

CFArrayRef
CFLocaleCopyAvailableLocaleIdentifiers (void)
{
  int32_t count;
  int32_t idx;
  CFMutableArrayRef mArray;
  
  pthread_mutex_lock (&_kCFLocaleLock);
  if (_kCFLocaleAvailableLocaleIdentifiers)
    {
      pthread_mutex_unlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleAvailableLocaleIdentifiers);
    }
  
  count = uloc_countAvailable ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  idx = 0;
  while (count > idx)
    {
      const char *str = uloc_getAvailable (idx);
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        str, CFStringGetSystemEncoding());
      CFArrayAppendValue (mArray, cfStr);
      CFRelease (cfStr);
      ++idx;
    }
  
  _kCFLocaleAvailableLocaleIdentifiers =
    CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  pthread_mutex_unlock (&_kCFLocaleLock);
  CFRelease (mArray);
  return (CFArrayRef)CFRetain (_kCFLocaleAvailableLocaleIdentifiers);
}

CFArrayRef
CFLocaleCopyISOCountryCodes (void)
{
  const char *const *cCodes;
  CFMutableArrayRef mArray;
  int idx;
  
  pthread_mutex_lock (&_kCFLocaleLock);
  if (_kCFLocaleISOCountryCodes)
    {
      pthread_mutex_unlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleISOCountryCodes);
    }
  
  cCodes = uloc_getISOCountries ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  for (idx = 0 ; cCodes[idx] ; ++idx)
    {
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        cCodes[idx], CFStringGetSystemEncoding());
      CFArrayAppendValue (mArray, cfStr);
      CFRelease (cfStr);
    }
  
  _kCFLocaleISOCountryCodes =
    CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  pthread_mutex_unlock (&_kCFLocaleLock);
  
  CFRelease (mArray);
  
  return (CFArrayRef)CFRetain (_kCFLocaleISOCountryCodes);
}

CFArrayRef
CFLocaleCopyISOLanguageCodes (void)
{
  const char *const *cCodes;
  CFMutableArrayRef mArray;
  int idx;
  
  pthread_mutex_lock (&_kCFLocaleLock);
  if (_kCFLocaleISOLanguageCodes)
    {
      pthread_mutex_unlock (&_kCFLocaleLock);
      return (CFArrayRef)CFRetain (_kCFLocaleISOLanguageCodes);
    }
  
  cCodes = uloc_getISOLanguages ();
  mArray = CFArrayCreateMutable (kCFAllocatorSystemDefault,
    0,
    &kCFTypeArrayCallBacks);
  for (idx = 0 ; cCodes[idx] != NULL ; ++idx)
    {
      CFStringRef cfStr = CFStringCreateWithCString (kCFAllocatorSystemDefault,
        cCodes[idx], CFStringGetSystemEncoding());
      CFArrayAppendValue (mArray, cfStr);
      CFRelease (cfStr);
    }
  
  _kCFLocaleISOLanguageCodes =
    CFArrayCreateCopy (kCFAllocatorSystemDefault, mArray);
  pthread_mutex_unlock (&_kCFLocaleLock);
  
  CFRelease (mArray);
  
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
  return _createArrayWithUEnumeration (NULL, en);
}

CFArrayRef
CFLocaleCopyCommonISOCurrencyCodes (void)
{
  UEnumeration *en;
  UErrorCode err = U_ZERO_ERROR;
  
  en = ucurr_openISOCurrencies (UCURR_COMMON, &err);
  if (U_FAILURE(err))
    return NULL;
  return _createArrayWithUEnumeration (NULL, en);
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
      ULOC_FULLNAME_CAPACITY, CFStringGetSystemEncoding()))
    return NULL;
  
  if (!CFStringGetCString (value, valueBuffer, BUFFER_SIZE,
      CFStringGetSystemEncoding()))
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
  char cLocale[ULOC_FULLNAME_CAPACITY];
  char buffer[BUFFER_SIZE];
  UniChar ubuffer[BUFFER_SIZE];
  int32_t length = 0;
  UErrorCode err = U_ZERO_ERROR;
  
  // Don't waste any time.
  if (locale == NULL || key == NULL)
    return NULL;
  if (key == kCFLocaleIdentifier)
    return locale->_identifier;
  
  // Make sure we haven't been through this already.
  if (CFDictionaryGetValueIfPresent(locale->_components, key,
      (const void **)&result))
    {
      return result;
    }
  
  if (!CFStringGetCString (locale->_identifier, cLocale,
      ULOC_FULLNAME_CAPACITY, CFStringGetSystemEncoding()))
    return NULL;
  
  if (key == kCFLocaleMeasurementSystem
      || key == kCFLocaleUsesMetricSystem)
    {
      UMeasurementSystem ums = ulocdata_getMeasurementSystem (cLocale, &err);
      if (key == kCFLocaleMeasurementSystem)
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
    }
  else if (key == kCFLocaleDecimalSeparator)
    result = CFLocaleCopyNumberFormatterDecimalProperty (locale,
      kCFNumberFormatterDecimalSeparator);
  else if (key == kCFLocaleGroupingSeparator)
    result = CFLocaleCopyNumberFormatterDecimalProperty (locale,
      kCFNumberFormatterGroupingSeparator);
  else if (key == kCFLocaleCurrencySymbol)
    result = CFLocaleCopyNumberFormatterCurrencyProperty (locale,
      kCFNumberFormatterCurrencySymbol);
  else if (key == kCFLocaleCurrencyCode)
    result = CFLocaleCopyNumberFormatterCurrencyProperty (locale,
      kCFNumberFormatterCurrencyCode);
#define GET_VALUE(func) do \
{ \
  length = func (cLocale, buffer, BUFFER_SIZE, &err); \
  if (U_FAILURE(err) || length <= 0) \
    return kCFNull; \
  else \
    result =  \
      CFStringCreateWithCString (NULL, buffer, CFStringGetSystemEncoding()); \
} while (0)
  else if (key == kCFLocaleLanguageCode)
    GET_VALUE(uloc_getLanguage);
  else if (key == kCFLocaleCountryCode)
    GET_VALUE(uloc_getCountry);
  else if (key == kCFLocaleScriptCode)
    GET_VALUE(uloc_getScript);
  else if (key == kCFLocaleVariantCode)
    GET_VALUE(uloc_getVariant);
  else if (key == kCFLocaleExemplarCharacterSet)
    {
      return kCFNull;
    }
  else if (key == kCFLocaleCalendarIdentifier)
    {
      if (uloc_getKeywordValue (cLocale, ICU_CALENDAR_KEY, buffer,
          BUFFER_SIZE, &err) > 0 && U_SUCCESS(err))
        {
          char *calIdent = buffer;
          if (strncmp(calIdent, "gregorian", sizeof("gregorian")-1) == 0)
            result = kCFGregorianCalendar;
        }
      else
        {
          result = kCFGregorianCalendar; // Default calendar
        }
    }
  else if (key == kCFLocaleCalendar)
    result = _createCalendar (CFGetAllocator(locale), locale);
  else if (key == kCFLocaleCollationIdentifier)
    {
      length = uloc_getKeywordValue (cLocale, ICU_COLLATION_KEY,
        buffer, BUFFER_SIZE, &err);
      if (U_FAILURE(err) || length <= 0)
        return kCFNull;
      result =  CFStringCreateWithCString (NULL, buffer,
        CFStringGetSystemEncoding());
    }
  else if (key == kCFLocaleCollatorIdentifier)
    {
      return kCFNull;
    }
#define GET_DELIMITER(type) do \
{ \
  ULocaleData *uld = ulocdata_open (cLocale, &err); \
  length = ulocdata_getDelimiter (uld, type, ubuffer, BUFFER_SIZE, &err); \
  if (U_FAILURE(err)) \
    return kCFNull; \
  result = CFStringCreateWithCharacters (NULL, ubuffer, length); \
} while (0)
  else if (key == kCFLocaleQuotationBeginDelimiterKey)
    GET_DELIMITER(ULOCDATA_QUOTATION_START);
  else if (key == kCFLocaleQuotationEndDelimiterKey)
    GET_DELIMITER(ULOCDATA_QUOTATION_END);
  else if (key == kCFLocaleAlternateQuotationBeginDelimiterKey)
    GET_DELIMITER(ULOCDATA_ALT_QUOTATION_START);
  else if (key == kCFLocaleAlternateQuotationEndDelimiterKey)
    GET_DELIMITER(ULOCDATA_ALT_QUOTATION_END);
  else
    return kCFNull;
  
  if (result == NULL)
    result = kCFNull;
  
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
             CFStringGetSystemEncoding()))
        return NULL;
      cLocale = buffer;
    }
  
  uloc_canonicalize (cLocale, canonical, ULOC_FULLNAME_CAPACITY, &err);
  if (U_FAILURE(err))
    return NULL;
  
  return CFStringCreateWithCString (allocator, canonical,
    CFStringGetSystemEncoding());
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
         CFStringGetSystemEncoding()))
    return NULL;
  
  uloc_canonicalize (cLocale, canonical, BUFFER_SIZE, &err);
  uloc_getLanguage (canonical, lang, ULOC_LANG_CAPACITY, &err);
  if (U_FAILURE(err))
    return NULL;
  
  result =
    CFStringCreateWithCString (allocator, lang, CFStringGetSystemEncoding());
  
  return result;
}

#define ADD_KEY_VALUE_TO_DICTIONARY(dictionary, key, value, length, error) do \
{ \
  if (U_SUCCESS(error) && length > 0) \
    { \
      CFStringRef value__ = CFStringCreateWithCString (NULL, value, \
        CFStringGetSystemEncoding()); \
      CFDictionaryAddValue (dictionary, key, value__); \
      CFRelease (value__); \
    } \
  error = U_ZERO_ERROR; \
} while (0)

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
      CFStringGetSystemEncoding()))
    return NULL;
  
  /* Using capacity = 7 because that's the most amount of keys we'll have:
      language_script_country_variant@calendar;collation;currency.
  */
  dict = CFDictionaryCreateMutable (NULL, 7,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  len = uloc_getLanguage (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleLanguageCode, buffer, len, err);

  len = uloc_getCountry (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleCountryCode, buffer, len, err);

  len = uloc_getScript (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleScriptCode, buffer, len, err);

  len = uloc_getVariant (locale, buffer, ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleVariantCode, buffer, len, err);
  
  len = uloc_getKeywordValue (locale, ICU_CALENDAR_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleCalendarIdentifier, buffer, len,
    err);
  
  len = uloc_getKeywordValue (locale, ICU_COLLATION_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleCollationIdentifier, buffer, len,
    err);
  
  len = uloc_getKeywordValue (locale, ICU_CURRENCY_KEY, buffer,
    ULOC_KEYWORDS_CAPACITY, &err);
  ADD_KEY_VALUE_TO_DICTIONARY(dict, kCFLocaleCurrencyCode, buffer, len, err);
  
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
  char locale[ULOC_FULLNAME_CAPACITY];
  char lang[ULOC_LANG_CAPACITY];
  char country[ULOC_COUNTRY_CAPACITY];
  char script[ULOC_SCRIPT_CAPACITY];
  char variant[ULOC_KEYWORDS_CAPACITY]; // use this for lack of a better one
  char keyword[ULOC_KEYWORD_AND_VALUES_CAPACITY];
  Boolean hasCountry;
  Boolean hasScript;
  Boolean hasVariant;
  UErrorCode err = U_ZERO_ERROR;
  CFStringRef value;
  
  if (dictionary == NULL)
    return NULL;
  
  /* We'll return NULL if kCFLocaleLanguageCode doesn't exist.  This is the
     only key that is absolutely necessary.
  */
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleLanguageCode,
      (const void **)&value))
    CFStringGetCString (value, lang, ULOC_LANG_CAPACITY, CFStringGetSystemEncoding());
  else
    return NULL;
  
  hasCountry =
    CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCountryCode,
    (const void **)&value);
  if (hasCountry)
    {
      CFStringGetCString (value, country, ULOC_COUNTRY_CAPACITY,
        CFStringGetSystemEncoding());
    }
  hasScript =
    CFDictionaryGetValueIfPresent(dictionary, kCFLocaleScriptCode,
    (const void **)&value);
  if (hasScript)
    {
      CFStringGetCString (value, script, ULOC_SCRIPT_CAPACITY,
        CFStringGetSystemEncoding());
    }
  hasVariant =
    CFDictionaryGetValueIfPresent(dictionary, kCFLocaleVariantCode,
    (const void **)&value);
  if (hasVariant)
    {
      CFStringGetCString (value, variant, ULOC_KEYWORDS_CAPACITY,
        CFStringGetSystemEncoding());
    }
  
#define TEST_CODE(x, y) (x ? "_" : ""), (x ? y : "")
  snprintf (locale, ULOC_FULLNAME_CAPACITY, "%s%s%s%s%s%s%s",
    lang, TEST_CODE(hasScript, script), TEST_CODE(hasCountry, country),
    TEST_CODE(hasVariant, variant));
#undef TEST_CODE
  // Use uloc_setKeywordValue() here since libicu is a required library.
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCalendarIdentifier,
      (const void **)&value))
    {
      CFStringGetCString (value, keyword, ULOC_KEYWORDS_CAPACITY,
        CFStringGetSystemEncoding());
      uloc_setKeywordValue (ICU_CALENDAR_KEY, keyword, locale,
        ULOC_FULLNAME_CAPACITY, &err);
    }
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCollationIdentifier,
      (const void **)&value))
    {
      CFStringGetCString (value, keyword, ULOC_KEYWORDS_CAPACITY,
        CFStringGetSystemEncoding());
      uloc_setKeywordValue (ICU_COLLATION_KEY, keyword, locale,
        ULOC_FULLNAME_CAPACITY, &err);
    }
  if (CFDictionaryGetValueIfPresent(dictionary, kCFLocaleCurrencyCode,
      (const void **)&value))
    {
      CFStringGetCString (value, keyword, ULOC_KEYWORDS_CAPACITY,
        CFStringGetSystemEncoding());
      uloc_setKeywordValue (ICU_CURRENCY_KEY, keyword, locale,
        ULOC_FULLNAME_CAPACITY, &err);
    }
  
  return
    CFStringCreateWithCString (allocator, locale, CFStringGetSystemEncoding());
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
    CFStringCreateWithCString (allocator, buffer, CFStringGetSystemEncoding());
  
  return result;
}

CFLocaleLanguageDirection
CFLocaleGetLanguageCharacterDirection (CFStringRef isoLangCode)
{
  char buffer[BUFFER_SIZE];
  ULayoutType result;
  UErrorCode err = U_ZERO_ERROR;
  
  if (!CFStringGetCString (isoLangCode, buffer, BUFFER_SIZE - 1,
      CFStringGetSystemEncoding()))
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
      CFStringGetSystemEncoding()))
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
      CFStringGetSystemEncoding()))
    return uloc_getLCID (buffer);
  return 0;
}
