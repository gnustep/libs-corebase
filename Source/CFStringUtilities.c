/* CFStringUtilities.c
   
   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: May, 2011
   
   This file is part of GNUstep CoreBase Library.
   
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
#include "CoreFoundation/CFArray.h"
#include "CoreFoundation/CFCharacterSet.h"
#include "CoreFoundation/CFLocale.h"
#include "CoreFoundation/CFString.h"
#include "GSPrivate.h"

#if defined(HAVE_UNICODE_UCOL_H)
#include <unicode/ucol.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#endif
#if defined(HAVE_UNICODE_ULOC_H)
#include <unicode/uloc.h>
#endif
#if defined(HAVE_UNICODE_USEARCH_H)
#include <unicode/usearch.h>
#endif
#if defined(HAVE_ICU_H)
#include <icu.h>
#endif



CF_INLINE UCollator *
CFStringICUCollatorOpen (CFStringCompareFlags options, CFLocaleRef loc)
{
  const char *cLocale;
  char buffer[ULOC_FULLNAME_CAPACITY];
  UCollator *ret;
  UErrorCode err = U_ZERO_ERROR;
  
  if (loc != NULL && (options & kCFCompareLocalized))
    cLocale = CFLocaleGetCStringIdentifier (loc, buffer, ULOC_FULLNAME_CAPACITY);
  else
    cLocale = NULL;
  
  ret = ucol_open (cLocale, &err);
  if (options)
    {
      if (options & kCFCompareNonliteral)
        ucol_setAttribute (ret, UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, &err);
      if (options & kCFCompareNumerically)
        ucol_setAttribute (ret, UCOL_NUMERIC_COLLATION, UCOL_ON, &err);
      if (options & kCFCompareDiacriticInsensitive)
        ucol_setAttribute (ret, UCOL_NORMALIZATION_MODE, UCOL_ON, &err);
      /* FIXME if (compareOptions & kCFCompareWidthInsensitive)
        */
      if (options & kCFCompareForcedOrdering)
        ucol_setAttribute (ret, UCOL_STRENGTH, UCOL_IDENTICAL, &err);
    }
  
  return ret;
}

CF_INLINE void
CFStringICUCollatorClose (UCollator *collator)
{
  ucol_close (collator);
}

static UniChar *
CFStringUCharFoldCase (CFAllocatorRef alloc, const UniChar *chars,
  CFIndex length, CFIndex *foldedLength)
{
  UErrorCode err = U_ZERO_ERROR;
  int32_t needed;
  UniChar *folded;

  needed = u_strFoldCase (NULL, 0, chars, length, U_FOLD_CASE_DEFAULT, &err);
  folded = CFAllocatorAllocate (alloc,
    (needed > 0 ? needed : 1) * sizeof(UniChar), 0);
  err = U_ZERO_ERROR;
  u_strFoldCase (folded, needed, chars, length, U_FOLD_CASE_DEFAULT, &err);
  if (U_FAILURE (err))
    {
      CFAllocatorDeallocate (alloc, folded);
      return NULL;
    }
  *foldedLength = needed;
  return folded;
}



CFRange
CFStringFind (CFStringRef str, CFStringRef stringToFind,
              CFStringCompareFlags compareOptions)
{
  CFRange ret;
  CFIndex len = CFStringGetLength (str);
  if (!CFStringFindWithOptionsAndLocale (str, stringToFind,
      CFRangeMake(0, len), compareOptions, NULL, &ret))
    ret = CFRangeMake (kCFNotFound, 0);
  
  return ret;
}

Boolean
CFStringFindWithOptions (CFStringRef str, CFStringRef stringToFind,
                         CFRange rangeToSearch,
                         CFStringCompareFlags searchOptions, CFRange *result)
{
  return CFStringFindWithOptionsAndLocale (str, stringToFind,
    rangeToSearch, searchOptions, NULL, result);
}

Boolean
CFStringFindWithOptionsAndLocale (CFStringRef str,
                                  CFStringRef stringToFind,
                                  CFRange rangeToSearch,
                                  CFStringCompareFlags searchOptions,
                                  CFLocaleRef locale, CFRange *result)
{
  UniChar *pattern;
  UniChar *text;
  CFIndex patternLength;
  CFIndex textLength;
  CFIndex start;
  CFIndex end;
  CFAllocatorRef alloc;
  UCollator *ucol;
  UStringSearch *usrch;
  UErrorCode err = U_ZERO_ERROR;
  
  if (rangeToSearch.length == 0)
    return false;
  
  alloc = CFAllocatorGetDefault ();
  textLength = CFStringGetLength (stringToFind);
  if (textLength == 0)
    return false;
  
  patternLength = rangeToSearch.length;
  pattern = CFAllocatorAllocate (alloc, patternLength * sizeof(UniChar), 0);
  CFStringGetCharacters (str, rangeToSearch, pattern);
  
  text = CFAllocatorAllocate (alloc, textLength * sizeof(UniChar), 0);
  CFStringGetCharacters (stringToFind, CFRangeMake(0, textLength), text);

  if (searchOptions & kCFCompareCaseInsensitive)
    {
      UniChar *foldedPattern;
      UniChar *foldedText;
      CFIndex foldedPatternLength;
      CFIndex foldedTextLength;

      foldedPattern = CFStringUCharFoldCase (alloc, pattern, patternLength,
        &foldedPatternLength);
      foldedText = CFStringUCharFoldCase (alloc, text, textLength,
        &foldedTextLength);
      if (foldedPattern != NULL && foldedText != NULL)
        {
          CFAllocatorDeallocate (alloc, pattern);
          CFAllocatorDeallocate (alloc, text);
          pattern = foldedPattern;
          patternLength = foldedPatternLength;
          text = foldedText;
          textLength = foldedTextLength;
        }
      else
        {
          if (foldedPattern != NULL)
            CFAllocatorDeallocate (alloc, foldedPattern);
          if (foldedText != NULL)
            CFAllocatorDeallocate (alloc, foldedText);
        }
    }

  ucol = CFStringICUCollatorOpen (searchOptions, locale);
  usrch = usearch_openFromCollator (text, textLength, pattern, patternLength,
                                    ucol, NULL, &err);
  if (U_FAILURE(err))
    return false;
  
  /* FIXME: need to handle kCFCompareAnchored */
  if (searchOptions & kCFCompareBackwards)
    {
      start = usearch_last (usrch, &err);
    }
  else
    {
      start = usearch_first (usrch, &err);
    }
  if (start == USEARCH_DONE)
    {
      CFAllocatorDeallocate (alloc, pattern);
      CFAllocatorDeallocate (alloc, text);
      return false;
    }
  end = usearch_getMatchedLength (usrch);
  usearch_close (usrch);
  CFStringICUCollatorClose (ucol);
  
  if (result)
    *result = CFRangeMake (start + rangeToSearch.location, end);
  
  CFAllocatorDeallocate (alloc, pattern);
  CFAllocatorDeallocate (alloc, text);
  return true;
}

Boolean
CFStringHasPrefix (CFStringRef str, CFStringRef prefix)
{
  CFIndex len = CFStringGetLength (str);
  return CFStringFindWithOptionsAndLocale (str, prefix, CFRangeMake(0, len),
                                           kCFCompareAnchored, NULL, NULL);
}

Boolean
CFStringHasSuffix (CFStringRef str, CFStringRef suffix)
{
  CFIndex len = CFStringGetLength (str);
  return CFStringFindWithOptionsAndLocale (str, suffix, CFRangeMake(0, len),
    kCFCompareBackwards | kCFCompareAnchored, NULL, NULL);
}

CFComparisonResult
CFStringCompare (CFStringRef str1, CFStringRef str2,
                 CFStringCompareFlags compareOptions)
{
  CFIndex len = CFStringGetLength (str1);
  return CFStringCompareWithOptionsAndLocale (str1, str2, CFRangeMake(0, len),
                                              compareOptions, NULL);
}

CFComparisonResult
CFStringCompareWithOptions (CFStringRef str1, CFStringRef str2,
  CFRange rangeToCompare, CFStringCompareFlags compareOptions)
{
  return CFStringCompareWithOptionsAndLocale (str1, str2, rangeToCompare,
                                              compareOptions, NULL);
}

CFComparisonResult
CFStringCompareWithOptionsAndLocale (CFStringRef str1,
  CFStringRef str2, CFRange rangeToCompare,
  CFStringCompareFlags compareOptions, CFLocaleRef locale)
{
  CFComparisonResult ret;
  UniChar *string1;
  UniChar *string2;
  CFIndex length1;
  CFIndex length2;
  CFAllocatorRef alloc;
  UCollator *ucol;
  
  alloc = CFAllocatorGetDefault ();
  
  length1 = rangeToCompare.length;
  string1 = CFAllocatorAllocate (alloc, (length1) * sizeof(UniChar), 0);
  CFStringGetCharacters (str1, rangeToCompare, string1);
  
  length2 = CFStringGetLength (str2);
  string2 = CFAllocatorAllocate (alloc, (length2) * sizeof(UniChar), 0);
  CFStringGetCharacters (str2, CFRangeMake(0, length2), string2);
  
  ucol = CFStringICUCollatorOpen (compareOptions, locale);
  if (compareOptions & kCFCompareCaseInsensitive)
    {
      UniChar *folded1;
      UniChar *folded2;
      CFIndex foldedLength1;
      CFIndex foldedLength2;

      folded1 = CFStringUCharFoldCase (alloc, string1, length1, &foldedLength1);
      folded2 = CFStringUCharFoldCase (alloc, string2, length2, &foldedLength2);
      if (folded1 != NULL && folded2 != NULL)
        ret = (CFComparisonResult)ucol_strcoll (ucol, folded1, foldedLength1,
                                                folded2, foldedLength2);
      else
        ret = (CFComparisonResult)ucol_strcoll (ucol, string1, length1,
                                                string2, length2);
      if (folded1 != NULL)
        CFAllocatorDeallocate (alloc, folded1);
      if (folded2 != NULL)
        CFAllocatorDeallocate (alloc, folded2);
    }
  else
    {
      ret = (CFComparisonResult)ucol_strcoll (ucol, string2, length2, string1,
                                              length1);
    }
  CFStringICUCollatorClose (ucol);
  
  CFAllocatorDeallocate (alloc, string1);
  CFAllocatorDeallocate (alloc, string2);
  return ret;
}

Boolean
CFStringFindCharacterFromSet (CFStringRef str, CFCharacterSetRef theSet,
  CFRange rangeToSearch, CFStringCompareFlags searchOptions, CFRange *result)
{
  /* FIXME: Not really sure how to get this done. Input is welcome. */
  return false;
}

CFStringRef
CFStringCreateByCombiningStrings (CFAllocatorRef alloc, CFArrayRef theArray,
  CFStringRef separatorString)
{
  CFIndex idx;
  CFIndex count;
  CFMutableStringRef string;
  CFStringRef currentString;
  CFStringRef ret;
  
  count = CFArrayGetCount (theArray) - 1;
  if (count <= 0)
    return NULL;
  
  string = CFStringCreateMutable (NULL, 0);
  idx = 0;
  while (idx < count)
    {
      currentString = (CFStringRef)CFArrayGetValueAtIndex (theArray, idx++);
      CFStringAppend (string, currentString);
      CFStringAppend (string, separatorString);
    }
  currentString = CFArrayGetValueAtIndex (theArray, idx);
  CFStringAppend (string, currentString);
  
  ret = CFStringCreateCopy (alloc, string);
  CFRelease (string);
  return ret;
}

CFArrayRef
CFStringCreateArrayBySeparatingStrings (CFAllocatorRef alloc,
  CFStringRef str, CFStringRef separator)
{
  /* This is basically a port of -componentsSeparatedByString: */
  CFIndex end;
  CFRange search;
  CFRange found;
  CFStringRef tmp;
  CFArrayRef ret;
  CFMutableArrayRef array;
  
  array = CFArrayCreateMutable (alloc, 0, &kCFTypeArrayCallBacks);

  search = CFRangeMake (0, CFStringGetLength(str));
  end = search.length;
  while (CFStringFindWithOptions(str, separator, search, 0, &found))
  {
    CFRange current;
    current = CFRangeMake (search.location, found.location - search.location);
    
    tmp = CFStringCreateWithSubstring(alloc, str, current);
    CFArrayAppendValue (array, tmp);
    CFRelease (tmp);

    search = CFRangeMake (found.location + found.length,
                          end - found.location - found.length);
  }
  
  /* Add the last search string range */
  tmp = CFStringCreateWithSubstring(alloc, str, search);
  CFArrayAppendValue (array, tmp);
  CFRelease (tmp);
  
  ret = CFArrayCreateCopy (alloc, array);
  CFRelease(array);
  
  return ret;
}

CFArrayRef
CFStringCreateArrayWithFindResults (CFAllocatorRef alloc,
  CFStringRef str, CFStringRef stringToFind, CFRange rangeToSearch,
  CFStringCompareFlags compOpt)
{
  return NULL; /* FIXME */
}

/* These next two functions should be very similar.  According to Apple's
   documentation the only different between the two is that ...ParagraphBounds
   does not stop at Unicode NextLine or LineSeparactor characters.
   
   They can probably be implemented using ICU's break iterator.
*/
void
CFStringGetLineBounds (CFStringRef str, CFRange range,
  CFIndex *lineBeginIndex, CFIndex *lineEndIndex, CFIndex *contentsEndIndex)
{
  return;
}

void
CFStringGetParagraphBounds (CFStringRef string, CFRange range,
  CFIndex *parBeginIndex, CFIndex *parEndIndex, CFIndex *contentsEndIndex)
{
  return;
}

