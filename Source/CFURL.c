/* CFURL.c
   
   Copyright (C) 2012 Free Software Foundation, Inc.
   
   Written by: Stefan Bidigaray
   Date: January, 2012
   
   This file is part of the GNUstep CoreBase Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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

/* READ FIRST
 * Using RFC 3986 instead of 2396 because it is the latest URL/URI
 * specification when this file was written in January 2012.  The main
 * difference is that RFC 3986 adds IPv6 address support, allowing
 * this implementation to be a little more future proof than if we were
 * using only RFC 2396.
 */

#include "CoreFoundation/CFRuntime.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFURL.h"

#include "GSPrivate.h"
#include "GSObjCRuntime.h"
#include "GSUnicode.h"

#include <string.h>

#if defined(_WIN32)
#define CFURL_DEFAULT_PATH_STYLE kCFURLWindowsPathStyle
#else
#define CFURL_DEFAULT_PATH_STYLE kCFURLPOSIXPathStyle
#endif

#define URL_IS_LEGAL(c) (c > 0x0020 && c < 0x007F)
#define URL_IS_SCHEME(c) (CHAR_IS_ALPHA(c) || CHAR_IS_DIGIT(c) \
  || c == '+' || c == '-' || c == '.')
#define URL_IS_PCHAR(c) (URL_IS_UNRESERVED(c) || c == '%' \
  || URL_IS_SUB_DELIMS(c) || c == ':' || c == '@')

#define URL_IS_RESERVED(c) (URL_IS_GEN_DELIMS(c) || URL_IS_SUB_DELIMS(c))
#define URL_IS_UNRESERVED(c) (CHAR_IS_ALPHA(c) || CHAR_IS_DIGIT(c) \
  || c == '-' || c == '.' || c == '_' || c == '~')

#define URL_IS_GEN_DELIMS(c) (c == ':' || c == '/' || c == '?' \
  || c == '#' || c == '[' || c == ']' || c == '@')
#define URL_IS_SUB_DELIMS(c) (c == '!' || c == '$' || c == '&' || c == '\'' \
  || c == '(' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=')

static CFTypeID _kCFURLTypeID = 0;

struct __CFURL
{
  CFRuntimeBase _parent;
  CFStringRef   _urlString;
  CFURLRef      _baseURL;
  CFStringEncoding _encoding; /* The encoding of the escape characters */
  CFRange       _ranges[12]; /* CFURLComponentType ranges */
};

enum
{
  _kCFURLIsDecomposable = (1<<0),
  _kCFURLIsFileSystemPath = (1<<1)
};

CF_INLINE Boolean
CFURLIsDecomposable (CFURLRef url)
{
  return ((CFRuntimeBase *)url)->_flags.info & _kCFURLIsDecomposable
    ? true : false;
}

CF_INLINE Boolean
CFURLIsFileSystemPath (CFURLRef url)
{
  return ((CFRuntimeBase *)url)->_flags.info & _kCFURLIsFileSystemPath
    ? true : false;
}

CF_INLINE void
CFURLSetIsDecomposable (CFURLRef url)
{
  ((CFRuntimeBase *)url)->_flags.info |= _kCFURLIsDecomposable;
}

CF_INLINE void
CFURLSetIsFileSystemPath (CFURLRef url)
{
  ((CFRuntimeBase *)url)->_flags.info |= _kCFURLIsFileSystemPath;
}



static void
CFURLFinalize (CFTypeRef cf)
{
  CFURLRef url = (CFURLRef)cf;
  
  CFRelease (url->_urlString);
  if (url->_baseURL)
    CFRelease (url->_baseURL);
}

static Boolean
CFURLEqual (CFTypeRef cf1, CFTypeRef cf2)
{
  Boolean equal;
  CFURLRef url1 = CFURLCopyAbsoluteURL ((CFURLRef)cf1);
  CFURLRef url2 = CFURLCopyAbsoluteURL ((CFURLRef)cf2);
  
  equal = CFEqual (CFURLGetString(url1), CFURLGetString(url2));
  
  CFRelease (url1);
  CFRelease (url2);
  return equal;
}

static CFHashCode
CFURLHash (CFTypeRef cf)
{
  CFHashCode hash;
  CFURLRef url = CFURLCopyAbsoluteURL ((CFURLRef)cf);
  
  hash = CFHash (CFURLGetString(url));
  
  CFRelease (url);
  return hash;
}

static CFStringRef
CFURLCopyFormattingDesc (CFTypeRef cf, CFDictionaryRef formatOptions)
{
  CFStringRef str;
  CFURLRef url = CFURLCopyAbsoluteURL ((CFURLRef)cf);
  
  str = CFRetain (CFURLGetString(url));
  
  CFRelease (url);
  return str;
}

static const CFRuntimeClass CFURLClass =
{
  0,
  "CFURL",
  NULL,
  NULL,
  CFURLFinalize,
  CFURLEqual,
  CFURLHash,
  CFURLCopyFormattingDesc,
  NULL
};

void CFURLInitialize (void)
{
  _kCFURLTypeID = _CFRuntimeRegisterClass (&CFURLClass);
}



CFTypeID
CFURLGetTypeID (void)
{
  return _kCFURLTypeID;
}

static Boolean
CFURLStringParse (CFStringRef urlString, CFRange ranges[12])
{
  /* URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
   * hier-part     = "//" authority path-abempty
   *               / path-absolute
   *               / path-rootless
   *               / path-empty
   * URI-reference = URI / relative-ref
   * relative-ref  = "//" authority path-abempty
   *               / path-absolute
   *               / path-noscheme
   *               / path-empty
   */
  
  CFStringInlineBuffer iBuffer;
  CFIndex idx;
  CFIndex start;
  CFIndex resourceSpecifierStart;
  CFIndex length;
  UniChar c;
  Boolean hasScheme;
  Boolean hasNetLoc;
  
  for (idx = 0 ; idx < 12 ; ++idx)
    ranges[idx] = CFRangeMake (kCFNotFound, 0);
  
  length = CFStringGetLength (urlString);
  CFStringInitInlineBuffer (urlString, &iBuffer, CFRangeMake(0, length));
  idx = start = 0;
  resourceSpecifierStart = kCFNotFound;
  
  /* Since ':' can appear anywhere in the string, we'll try to find
   * the scheme first.
   */
  /* Scheme component */
  c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
  hasScheme = false;
  if (CHAR_IS_ALPHA(c))
    {
      do
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
        } while (URL_IS_SCHEME(c));
      
      if (c == ':')
        {
          ranges[kCFURLComponentScheme - 1] =
            CFRangeMake (start, idx - start - 1);
          hasScheme = true;
        }
      else
        {
          idx = 0; /* Start over... */
        }
      
      start = idx;
      c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
    }
  
  /* Parse the relative-ref:
   * hier-part    = "//" authority path-abempty
   *              / path-absolute
   *              / path-rootless
   *              / path-empty
   * relative-ref = "//" authority path-abempty
   *              / path-absolute
   *              / path-noscheme
   *              / path-empty
   */
  /* Net Location component */
  hasNetLoc = false;
  if (c == '/' && CFStringGetCharacterFromInlineBuffer (&iBuffer, idx) == '/')
    {
      /* "//" authority path-abempty */
      CFIndex end;
      CFIndex i;
      
      hasNetLoc = true;
      
      ++idx;
      i = idx;
      start = i;
      
      /* Go to the end so we can parse backwards. */
      do
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, i++);
        } while (URL_IS_PCHAR(c));
      
      ranges[kCFURLComponentNetLocation - 1] = CFRangeMake (start, i-start-1);
      end = i;
      i = start;
      
      while (i < end)
        {
          /* FIXME: RFC 3986 is a little vague on what goes in the
           * UserInfo section, and Apple requires both User Name and
           * Password. We simply consider everything after the first ':'
           * to be part of the password. This behavior needs to be tested.
           */
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, i++);
          if (c == '@')
            {
              CFIndex userInfoEnd;
              
              ranges[kCFURLComponentUserInfo - 1] =
                CFRangeMake (start, i - start);
              userInfoEnd = i;
              
              i = start;
              while (i < userInfoEnd)
                {
                  c = CFStringGetCharacterFromInlineBuffer (&iBuffer, i++);
                  if (c == ':')
                    {
                      ranges[kCFURLComponentPassword - 1] =
                        CFRangeMake (i, userInfoEnd - i - 1);
                      break;
                    }
                }
              
              ranges[kCFURLComponentUser - 1] =
                CFRangeMake (start, i - start - 1);
              start = userInfoEnd;
              break;
            }
        }
      
      i = end;
      idx = end; /* Set idx before we move it to search for a port. */
      
      /* Try to find a port. */
      while (i > start)
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, --i);
          if (c == ':')
            {
              ranges[kCFURLComponentPort - 1] = CFRangeMake (i, end - i);
              end = i;
              break;
            }
        }
      /* Whatever's left is the host name. */
      ranges[kCFURLComponentHost - 1] = CFRangeMake (start, end - start - 1);
      
      start = idx - 1; /* Shift back to just before '/' */
      c = CFStringGetCharacterFromInlineBuffer (&iBuffer, start);
    }
  
  /* Path component */
  while ((URL_IS_PCHAR(c) || c == '/') && c != ';')
    c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
  
  ranges[kCFURLComponentPath - 1] = CFRangeMake (start, idx - start - 1);
  if (hasScheme == false && hasNetLoc == false)
    resourceSpecifierStart = start;
  
  /* Parameter String component */
  if (c == ';')
    {
      start = idx;
      do
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
        } while (URL_IS_PCHAR(c) || c == '/');
      
      ranges[kCFURLComponentParameterString - 1] =
        CFRangeMake (start, idx - start - 1);
      ranges[kCFURLComponentPath - 1].length += idx - start;
      if (resourceSpecifierStart == kCFNotFound)
        resourceSpecifierStart = start - 1;
    }
  
  /* Query component */
  if (c == '?')
    {
      start = idx;
      do
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
        } while (URL_IS_PCHAR(c) || c == '/' || c == '?');
      
      ranges[kCFURLComponentQuery - 1] = CFRangeMake (start, idx - start - 1);
      if (resourceSpecifierStart == kCFNotFound)
        resourceSpecifierStart = start - 1;
    }
  
  /* Fragment component */
  if (c == '#')
    {
      start = idx;
      do
        {
          c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
        } while (URL_IS_PCHAR(c) || c == '/' || c == '?');
      
      ranges[kCFURLComponentFragment - 1] = CFRangeMake (start, idx-start-1);
      if (resourceSpecifierStart == kCFNotFound)
        resourceSpecifierStart = start - 1;
    }
  
  if (c != 0x0000)
    return false;
  
  if (resourceSpecifierStart != kCFNotFound)
    ranges[kCFURLComponentResourceSpecifier - 1] =
      CFRangeMake (resourceSpecifierStart, idx - resourceSpecifierStart);
  
  return true;
}

#define CFURL_SIZE sizeof(struct __CFURL) - sizeof(CFRuntimeBase)

static CFURLRef
CFURLCreate_internal (CFAllocatorRef alloc, CFStringRef string,
  CFURLRef baseURL, CFStringEncoding encoding)
{
  struct __CFURL *new;
  CFRange ranges[12];
  
  if (!CFURLStringParse (string, ranges))
    return NULL;
  
  new = (struct __CFURL*)_CFRuntimeCreateInstance (alloc, _kCFURLTypeID,
    CFURL_SIZE, 0);
  if (new)
    {
      new->_urlString = CFStringCreateCopy (alloc, string);
      if (ranges[kCFURLComponentScheme - 1].location == kCFNotFound
          && baseURL)
        {
          new->_baseURL = CFURLCopyAbsoluteURL (baseURL);
        }
      else
        {
          CFURLSetIsDecomposable (new);
        }
      new->_encoding = encoding;
      memcpy (new->_ranges, ranges, sizeof(ranges));
    }
  
  return new;
}

CFURLRef
CFURLCreateWithString (CFAllocatorRef alloc, CFStringRef string,
  CFURLRef baseURL)
{
  return CFURLCreate_internal (alloc, string, baseURL, kCFStringEncodingUTF8);
}

static void
CFURLStringAppendByRemovingDotSegments (CFMutableStringRef string,
  UniChar *buffer, CFIndex length)
{
  UniChar *bufferStart;
  UniChar *bufferEnd;
  CFIndex pathStart;
  
  bufferStart = buffer;
  bufferEnd = bufferStart + length;
  pathStart = CFStringGetLength (string);
  
  /* Skip any '../' and './' */
  if (buffer < bufferEnd && *buffer == '.')
    {
      if ((buffer + 1) < bufferEnd && buffer[1] == '/')
        {
          buffer += 1;
        }
      else if ((buffer + 2) < bufferEnd && buffer[1] == '.'
          && buffer[2] == '/')
        {
          buffer += 2;
        }
    }
  bufferStart = buffer;
  
  /* Start checking for '/.' and '/..' */
  while (buffer < bufferEnd)
    {
      if ((buffer + 1) < bufferEnd && buffer[0] == '/'
          && buffer[1] == '.')
        {
          /* Skip '/./' or '/.'EOS */
          if (((buffer + 2) < bufferEnd && buffer[2] == '/')
              || (buffer + 2) == bufferEnd)
            {
              if ((buffer + 2) == bufferEnd)
                {
                  buffer[1] = '/';
                  buffer += 1;
                }
              else
                {
                  buffer += 2;
                }
            }
          /* Skip '/../' or '/..'EOS */
          else if (((buffer + 3) < bufferEnd && buffer[2] == '.'
                && buffer[3] == '/')
                || ((buffer + 3) == bufferEnd && buffer[2] == '.'))
            {
              CFStringInlineBuffer iBuffer;
              UniChar c;
              CFIndex i;
              CFIndex pathLength;
              
              pathLength = CFStringGetLength(string) - pathStart;
              CFStringInitInlineBuffer (string, &iBuffer,
                CFRangeMake(pathStart, pathLength));
              i = pathLength - 1;
              while (i >= 0)
                {
                  c = CFStringGetCharacterFromInlineBuffer (&iBuffer, i--);
                  if (c == '/')
                    break;
                }
              CFStringDelete (string,
                CFRangeMake(pathStart + i + 1, pathLength - i - 1));
              
              if ((buffer + 3) == bufferEnd)
                {
                  buffer[2] = '/';
                  buffer += 2;
                }
              else
                {
                  buffer += 3;
                }
            }
          /* Something like '/..*' */
          else
            {
              /* Skip onto the next '/' */
              do
                {
                  buffer++;
                } while (buffer < bufferEnd && *buffer != '/');
              CFStringAppendCharacters (string, bufferStart,
                buffer - bufferStart);
            }
        }
      else
        {
          /* Skip onto the next '/' */
          do
            {
              buffer++;
            } while (buffer < bufferEnd && *buffer != '/');
          CFStringAppendCharacters (string, bufferStart, buffer - bufferStart);
        }
      bufferStart = buffer;
    }
}

CFURLRef
CFURLCopyAbsoluteURL (CFURLRef relativeURL)
{
  CFAllocatorRef alloc;
  CFMutableStringRef targetString;
  CFStringRef relString;
  CFStringRef baseString;
  CFURLRef base;
  CFURLRef target;
  UniChar *buffer;
  CFIndex capacity;
  CFRange *relRanges;
  CFRange baseRanges[12];
  CFRange range;
  
  base = relativeURL->_baseURL;
  if (base == NULL)
    return CFRetain (relativeURL);
  
  /* This is a bit of a pain.  We can't assume _baseURL is a CFURL, so we
   * need to parse it before moving forward.  To avoid parsing the same
   * string twice (in the case that _baseURL is a CFURL) we check first.
   */
  baseString = CFURLGetString (base);
  if (CF_IS_OBJC(_kCFURLTypeID, base))
    CFURLStringParse (baseString, baseRanges);
  else
    memcpy (baseRanges, base->_ranges, sizeof(baseRanges));
  
  relString = relativeURL->_urlString;
  relRanges = (CFRange*)relativeURL->_ranges;
  
  alloc = CFGetAllocator(relativeURL);
  capacity = CFStringGetLength(relString) + CFStringGetLength(baseString);
  buffer = CFAllocatorAllocate (alloc, capacity * sizeof(UniChar), 0);
  targetString = CFStringCreateMutable (alloc, capacity);
  
  range = relRanges[kCFURLComponentScheme - 1];
  if (range.location != kCFNotFound)
    {
      /* Scheme */
      CFStringGetCharacters (relString, range, buffer);
      CFStringAppendCharacters (targetString, buffer, range.length);
      CFStringAppendCString (targetString, ":", kCFStringEncodingASCII);
      
      /* Authority */
      range = relRanges[kCFURLComponentNetLocation - 1];
      if (range.location != kCFNotFound)
        {
          CFStringAppendCString (targetString, "//", kCFStringEncodingASCII);
          CFStringGetCharacters (relString, range, buffer);
          CFStringAppendCharacters (targetString, buffer, range.length);
        }
      
      /* Path */
      range = relRanges[kCFURLComponentPath - 1];
      if (range.location != kCFNotFound)
        {
          CFStringGetCharacters (relString, range, buffer);
          CFURLStringAppendByRemovingDotSegments (targetString, buffer,
            range.length);
        }
      
      /* Query */
      range = relRanges[kCFURLComponentQuery - 1];
      if (range.location != kCFNotFound)
        {
          CFStringAppendCString (targetString, "?", kCFStringEncodingASCII);
          CFStringGetCharacters (relString, range, buffer);
          CFStringAppendCharacters (targetString, buffer, range.length);
        }
    }
  else
    {
      /* Scheme */
      range = baseRanges[kCFURLComponentScheme - 1];
      if (range.location != kCFNotFound)
        {
          CFStringGetCharacters (baseString, range, buffer);
          CFStringAppendCharacters (targetString, buffer, range.length);
          CFStringAppendCString (targetString, ":", kCFStringEncodingASCII);
        }
      
      range = relRanges[kCFURLComponentNetLocation - 1];
      if (range.location != kCFNotFound)
        {
          /* Authority */
          CFStringAppendCString (targetString, "//", kCFStringEncodingASCII);
          CFStringGetCharacters (relString, range, buffer);
          CFStringAppendCharacters (targetString, buffer, range.length);
          
          /* Path */
          range = relRanges[kCFURLComponentPath - 1];
          if (range.location != kCFNotFound)
            {
              CFStringGetCharacters (relString, range, buffer);
              CFURLStringAppendByRemovingDotSegments (targetString, buffer,
                range.length);
            }
          
          /* Query */
          range = relRanges[kCFURLComponentQuery - 1];
          if (range.location != kCFNotFound)
            {
              CFStringAppendCString (targetString, "?",
                kCFStringEncodingASCII);
              CFStringGetCharacters (relString, range, buffer);
              CFStringAppendCharacters (targetString, buffer, range.length);
            }
        }
      else
        {
          /* Authority */
          range = baseRanges[kCFURLComponentNetLocation - 1];
          if (range.location != kCFNotFound)
            {
              CFStringAppendCString (targetString, "//",
                kCFStringEncodingASCII);
              CFStringGetCharacters (baseString, range, buffer);
              CFStringAppendCharacters (targetString, buffer, range.length);
            }
          
          range = relRanges[kCFURLComponentPath - 1];
          if (range.location != kCFNotFound)
            {
              /* Path */
              if (range.length == 0)
                {
                  range = baseRanges[kCFURLComponentPath - 1];
                  if (range.location != kCFNotFound)
                    {
                      CFStringGetCharacters (baseString, range, buffer);
                      CFStringAppendCharacters (targetString, buffer,
                        range.length);
                    }
                              
                  /* Query */
                  range = relRanges[kCFURLComponentQuery - 1];
                  if (range.location != kCFNotFound)
                    {
                      CFStringAppendCString (targetString, "?",
                        kCFStringEncodingASCII);
                      CFStringGetCharacters (relString, range, buffer);
                      CFStringAppendCharacters (targetString, buffer,
                        range.length);
                    }
                  else
                    {
                      /* Query */
                      range = baseRanges[kCFURLComponentQuery - 1];
                      if (range.location != kCFNotFound)
                        {
                          CFStringAppendCString (targetString, "?",
                            kCFStringEncodingASCII);
                          CFStringGetCharacters (baseString, range, buffer);
                          CFStringAppendCharacters (targetString, buffer,
                            range.length);
                        }
                    }
                }
              else
                {
                  if (CFStringGetCharacterAtIndex(relString, range.location)
                      == '/')
                    {
                      CFStringGetCharacters (relString, range, buffer);
                      CFURLStringAppendByRemovingDotSegments (targetString,
                        buffer, range.length);
                    }
                  else
                    {
                      CFRange baseRange;
                      
                      baseRange = baseRanges[kCFURLComponentPath - 1];
                      
                      if (baseRange.location != kCFNotFound)
                        {
                          CFStringGetCharacters (baseString, baseRange, buffer);
                          
                          if (baseRange.length > 0
                              && buffer[baseRange.length - 1] != '/')
                            {
                              /* Remove last path component */
                              CFIndex count;
                              count = baseRange.length - 1;
                              while (buffer[--count] != '/');
                              baseRange.length = count + 1;
                            }
                        }
                      CFStringGetCharacters (relString, range,
                        &buffer[baseRange.length]);
                      range.length += baseRange.length;
                      
                      CFURLStringAppendByRemovingDotSegments (targetString,
                        buffer, range.length);
                    }
                  
                  range = relRanges[kCFURLComponentQuery - 1];
                  if (range.location != kCFNotFound)
                    {
                      CFStringAppendCString (targetString, "?",
                        kCFStringEncodingASCII);
                      CFStringGetCharacters (relString, range, buffer);
                      CFStringAppendCharacters (targetString, buffer,
                        range.length);
                    }
                }
            }
        }
    }
  
  /* Fragment */
  range = relRanges[kCFURLComponentFragment - 1];
  if (range.location != kCFNotFound)
    {
      CFStringAppendCString (targetString, "#", kCFStringEncodingASCII);
      CFStringGetCharacters (relString, range, buffer);
      CFStringAppendCharacters (targetString, buffer, range.length);
    }
  
  target = CFURLCreate_internal (alloc, targetString, NULL,
    kCFStringEncodingUTF8);
  CFRelease (targetString);
  CFAllocatorDeallocate (alloc, buffer);
  
  return target;
}

CFURLRef
CFURLCreateAbsoluteURLWithBytes (CFAllocatorRef alloc,
  const UInt8 *relativeURLBytes, CFIndex length, CFStringEncoding encoding,
  CFURLRef baseURL, Boolean useCompatibilityMode)
{
  /* FIXME: what to do with useCompatibilityMode? */
  CFURLRef url;
  CFStringRef str;
  
  str = CFStringCreateWithBytes (alloc, relativeURLBytes, length,
    encoding, false);
  if (str == NULL)
    return NULL;
  
  url = CFURLCreate_internal (alloc, str, baseURL, encoding);
  if (url)
    {
      CFURLRef tmp = CFURLCopyAbsoluteURL (url);
      CFRelease (url);
      url = tmp;
    }
  
  CFRelease (str);
  return url;
}

CFURLRef
CFURLCreateCopyAppendingPathComponent (CFAllocatorRef alloc, CFURLRef url,
  CFStringRef pathComponent, Boolean isDirectory)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateCopyAppendingPathExtension (CFAllocatorRef alloc, CFURLRef url,
  CFStringRef extension)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateCopyDeletingLastPathComponent (CFAllocatorRef alloc, CFURLRef url)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateCopyDeletingPathExtension (CFAllocatorRef alloc, CFURLRef url)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateFilePathURL (CFAllocatorRef alloc, CFURLRef url,
  CFErrorRef *error)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateFileReferenceURL (CFAllocatorRef alloc, CFURLRef url,
  CFErrorRef *error)
{
  return NULL; /* FIXME */
}

CFURLRef
CFURLCreateFromFileSystemRepresentation (CFAllocatorRef alloc,
  const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory)
{
  return CFURLCreateFromFileSystemRepresentationRelativeToBase (alloc,
    buffer, bufLen, isDirectory, NULL);
}

CFURLRef
CFURLCreateFromFileSystemRepresentationRelativeToBase (CFAllocatorRef alloc,
  const UInt8 *buffer, CFIndex bufLen, Boolean isDirectory, CFURLRef baseURL)
{
  CFURLRef ret;
  CFStringRef path;
  
  path = CFStringCreateWithBytesNoCopy (alloc, buffer, bufLen,
    GSStringGetFileSystemEncoding(), false, kCFAllocatorNull);
  ret = CFURLCreateWithFileSystemPathRelativeToBase (alloc, path,
    CFURL_DEFAULT_PATH_STYLE, isDirectory, baseURL);
  
  CFRelease (path);
  
  return ret;
}

CFURLRef
CFURLCreateWithFileSystemPath (CFAllocatorRef alloc,
 CFStringRef fileSystemPath, CFURLPathStyle style, Boolean isDirectory)
{
  CFURLRef ret;
  
  ret = CFURLCreateWithFileSystemPathRelativeToBase (alloc, fileSystemPath,
    style, isDirectory, NULL);
  
  return ret;
}

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#endif

CF_INLINE CFURLRef
CFURLCreateWithCurrentDirectory (CFAllocatorRef alloc)
{
  CFURLRef ret;
  CFStringRef cwd;
  CFMutableStringRef str;
#if defined(_WIN32)
  wchar_t buffer[MAX_PATH];
  DWORD length;
  
  length = GetCurrentDirectoryW (MAX_PATH, buffer);
  if (length == 0)
    return NULL;
  
  cwd = CFStringCreateWithBytesNoCopy (alloc, (const UInt8 *)buffer, length,
    GSStringGetFileSystemEncoding(), false, kCFAllocatorNull);
#else
  char buffer[1024];
  
  if (getcwd(buffer, 1024) == NULL)
    return NULL;
  
  cwd = CFStringCreateWithBytesNoCopy (alloc, (const UInt8 *)buffer,
    strlen(buffer), GSStringGetFileSystemEncoding(), false, kCFAllocatorNull);
#endif
  
  str = CFStringCreateMutable (alloc, 0);
  CFStringAppend (str, CFSTR("file://localhost/"));
  CFStringAppend (str, cwd);
  
  ret = CFURLCreateWithString (alloc, str, NULL);
  CFRelease (cwd);
  CFRelease (str);
  
  return ret;
}

static CFStringRef
CFURLCreateStringFromHFSPathStyle (CFAllocatorRef alloc,
  CFStringRef filePath, Boolean isAbsolute, Boolean isDirectory)
{
  return NULL; /* FIXME */
}

static CFStringRef
CFURLCreateStringFromWindowsPathStyle (CFAllocatorRef alloc,
  CFStringRef filePath, Boolean isAbsolute, Boolean isDirectory)
{
  CFMutableArrayRef comps;
  CFArrayRef tmp;
  CFStringRef ret;
  CFIndex count;
  CFIndex idx;
  
  tmp = CFStringCreateArrayBySeparatingStrings (alloc, filePath, CFSTR("\\"));
  comps = CFArrayCreateMutableCopy (alloc, 0, tmp);
  CFRelease (tmp);
  
  idx = 0;
  if (isAbsolute) /* Instead empty string to get leading '/' */
    {
      CFArrayInsertValueAtIndex (comps, 0, CFSTR(""));
      idx = 2; /* Skip drive letter */
    }
  
  count = CFArrayGetCount (comps);
  while (idx < count)
    {
      CFStringRef unescaped;
      CFStringRef escaped;
      
      unescaped = CFArrayGetValueAtIndex(comps, idx);
      escaped = CFURLCreateStringByAddingPercentEscapes (alloc, unescaped,
        NULL, NULL, kCFStringEncodingUTF8);
      if (escaped != unescaped)
        CFArraySetValueAtIndex (comps, idx, escaped);
      
      CFRelease (escaped);
      ++idx;
    }
  
  /* Add '/' for directories */
  if (isDirectory
      && !CFEqual(CFArrayGetValueAtIndex(comps, idx - 1), CFSTR("")))
    CFArrayInsertValueAtIndex (comps, count, CFSTR(""));
  
  ret = CFStringCreateByCombiningStrings (alloc, comps, CFSTR("/"));
  CFRelease (comps);
  
  return ret;
}

CFURLRef
CFURLCreateWithFileSystemPathRelativeToBase (CFAllocatorRef alloc,
  CFStringRef filePath, CFURLPathStyle style, Boolean isDirectory,
  CFURLRef baseURL)
{
  CFURLRef ret;
  CFStringRef path;
  Boolean abs;
  CFIndex filePathLen;
  
  switch (style)
    {
      case kCFURLPOSIXPathStyle:
        abs = (CFStringGetCharacterAtIndex(filePath, 0) == '/');
        path = CFURLCreateStringByAddingPercentEscapes (alloc, filePath,
          NULL, NULL, kCFStringEncodingUTF8);
        if (path != filePath)
          CFRetain (path);
        filePathLen = CFStringGetLength(path);
        if (isDirectory
            && CFStringGetCharacterAtIndex(path, filePathLen) != '/')
          {
            CFStringRef tmp;
            tmp = CFStringCreateWithFormat (alloc, NULL, CFSTR("%@/"), path);
            CFRelease (path);
            path = tmp;
          }
        break;
      case kCFURLHFSPathStyle:
        /* FIXME: I believe HFS path style is like POSIX with ':' instead
         * of '/' as the separator.
         */
        abs = (CFStringGetCharacterAtIndex(filePath, 0) == ':');
        path = CFURLCreateStringFromHFSPathStyle (alloc, filePath, abs,
          isDirectory);
        break;
      case kCFURLWindowsPathStyle:
        abs = (CFStringGetCharacterAtIndex(filePath, 1) == ':'
          && CFStringGetCharacterAtIndex(filePath, 2) == '\\');
        path = CFURLCreateStringFromWindowsPathStyle (alloc, filePath, abs,
          isDirectory);
        break;
      default:
        return NULL;
    }
  
  if (abs)
    {
      CFMutableStringRef tmp;
      tmp = CFStringCreateMutableCopy (alloc, 0, CFSTR("file://localhost"));
      CFStringAppend (tmp, path);
      
      CFRelease (path);
      path = (CFStringRef)tmp;
      baseURL = NULL;
    }
  else if (baseURL == NULL)
    {
      baseURL = CFURLCreateWithCurrentDirectory (alloc);
    }
  else
    {
      CFRetain (baseURL);
    }
  
  ret = CFURLCreate_internal (alloc, path, baseURL, kCFStringEncodingUTF8);
  if (ret)
    CFURLSetIsFileSystemPath (ret);
  
  CFRelease (path);
  if (baseURL)
    CFRelease (baseURL);
  
  return ret;
}

CFURLRef
CFURLCreateWithBytes (CFAllocatorRef alloc, const UInt8 *bytes, CFIndex length,
  CFStringEncoding encoding, CFURLRef baseURL)
{
  CFStringRef str;
  CFURLRef ret;
  
  str = CFStringCreateWithBytesNoCopy (NULL, bytes, length, encoding, false,
    kCFAllocatorNull);
  ret = CFURLCreate_internal (alloc, str, baseURL, encoding);
  
  CFRelease (str);
  
  return ret;
}

Boolean
CFURLCanBeDecomposed (CFURLRef url)
{
  if (CFURLIsDecomposable(url))
    return true;
  else
    return url->_baseURL ? CFURLCanBeDecomposed(url->_baseURL) : false;
}

static CFStringRef
CFURLCreateHFSStylePath (CFAllocatorRef alloc, CFStringRef path)
{
  return NULL;
}

static CFStringRef
CFURLCreateWindowsStylePath (CFAllocatorRef alloc, CFStringRef path)
{
  CFArrayRef comps;
  CFStringRef ret;
  
  comps = CFStringCreateArrayBySeparatingStrings (alloc, path, CFSTR("/"));
  if (CFEqual(CFArrayGetValueAtIndex(comps, 0), CFSTR("")))
    {
      CFMutableArrayRef tmp;
      
      tmp = CFArrayCreateMutableCopy (alloc, 0, comps);
      CFArrayRemoveValueAtIndex (tmp, 0);
      CFRelease (comps);
      comps = tmp;
    }
  
  ret = CFStringCreateByCombiningStrings (alloc, comps, CFSTR("\\"));
  CFRelease (comps);
  
  return ret;
}

CFStringRef
CFURLCopyFileSystemPath (CFURLRef url, CFURLPathStyle style)
{
  CFStringRef urlStr;
  CFStringRef path;
  CFAllocatorRef alloc;
  CFRange r;
  
  r = url->_ranges[kCFURLComponentPath - 1];
  if (r.location == kCFNotFound)
    return NULL;
  
  alloc = CFGetAllocator(url);
  urlStr = url->_urlString;
  
  if (r.length > 1)
    {
      CFStringRef tmp;
      
      if (CFStringGetCharacterAtIndex(urlStr, r.location + r.length - 1) == '/')
        r.length -= 1;
      tmp = CFStringCreateWithSubstring (alloc, urlStr, r);
      path = CFURLCreateStringByReplacingPercentEscapesUsingEncoding (alloc,
        tmp, CFSTR(""), url->_encoding);
      CFRelease (tmp);
    }
  else
    {
      path = CFSTR("/");
    }
  
  switch (style)
    {
      case kCFURLPOSIXPathStyle:
        /* Do nothing. */
        break;
      case kCFURLHFSPathStyle:
        {
          CFStringRef t;
          t = CFURLCreateHFSStylePath (CFGetAllocator(url), path);
          CFRelease (path);
          path = t;
        }
        break;
      case kCFURLWindowsPathStyle:
        {
          CFStringRef t;
          t = CFURLCreateWindowsStylePath (CFGetAllocator(url), path);
          CFRelease (path);
          path = t;
        }
        break;
      default:
        break;
    }
  
  return path;
}

CFStringRef
CFURLCopyFragment (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
  CFRange range = url->_ranges[kCFURLComponentFragment - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyFragment (url->_baseURL, charactersToLeaveEscaped);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyHostName (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentHost - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyHostName (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyLastPathComponent (CFURLRef url)
{
  return NULL; /* FIXME */
}

CFStringRef
CFURLCopyNetLocation (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentNetLocation - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyNetLocation (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyParameterString (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
  CFRange range = url->_ranges[kCFURLComponentParameterString - 1];
  if (range.location == kCFNotFound)
    return NULL;
  
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyPassword (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentPassword - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyPassword (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyPath (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentPath - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyPath (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyPathExtension (CFURLRef url)
{
  return NULL; /* FIXME */
}

CFStringRef
CFURLCopyQueryString (CFURLRef url, CFStringRef charactersToLeaveEscaped)
{
    CFRange range = url->_ranges[kCFURLComponentQuery - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyQueryString (url->_baseURL, charactersToLeaveEscaped);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyResourceSpecifier (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentResourceSpecifier - 1];
  if (range.location == kCFNotFound)
    return NULL;
  
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyScheme (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentScheme - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyScheme (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

CFStringRef
CFURLCopyStrictPath (CFURLRef url, Boolean *isAbsolute)
{
  CFStringRef path;
  Boolean abs = false;
  
  path = CFURLCopyPath(url);
  if (path)
    {
      if (CFStringGetCharacterAtIndex(path, 0) == '/')
        {
          CFStringRef tmp;
          
          abs = true;
          tmp = CFStringCreateWithSubstring (CFGetAllocator(url), path,
            CFRangeMake (1, CFStringGetLength(path) - 1));
          CFRelease (path);
          path = tmp;
        }
    }
  
  if (isAbsolute)
    *isAbsolute = abs;
  return path;
}

CFStringRef
CFURLCopyUserName (CFURLRef url)
{
  CFRange range = url->_ranges[kCFURLComponentUser - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLCopyUserName (url->_baseURL);
      return NULL;
    }
  return CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
}

SInt32
CFURLGetPortNumber (CFURLRef url)
{
  CFStringRef str;
  CFRange range;
  SInt32 intValue;
  
  range = url->_ranges[kCFURLComponentPort - 1];
  if (range.location == kCFNotFound)
    {
      if (url->_baseURL)
        return CFURLGetPortNumber (url->_baseURL);
      return -1;
    }
  
  str = CFStringCreateWithSubstring (CFGetAllocator(url), url->_urlString,
    range);
  intValue = CFStringGetIntValue (str);
  CFRelease (str);
  
  return intValue;
}

Boolean
CFURLHasDirectoryPath (CFURLRef url)
{
  CFStringRef str = CFURLGetString (url);
  return (CFStringGetCharacterAtIndex(str, CFStringGetLength(str) - 1) == '/');
}

CFDataRef
CFURLCreateData (CFAllocatorRef alloc, CFURLRef url, CFStringEncoding encoding,
  Boolean escapeWhiteSpace)
{
  CFDataRef ret;
  CFURLRef abs;
  CFStringRef absStr;
  
  abs = CFURLCopyAbsoluteURL (url);
  absStr = CFURLGetString (abs);
  if (escapeWhiteSpace)
    absStr = CFURLCreateStringByAddingPercentEscapes (alloc, absStr, NULL,
      CFSTR(" \r\n\t"), encoding);
  ret = CFStringCreateExternalRepresentation (alloc, absStr, encoding, 0);
  
  if (escapeWhiteSpace)
    CFRelease (absStr);
  CFRelease (abs);
  
  return ret;
}

/* This is the maximum number of UTF-8 bytes needed to represent everything
 * possible Unicode character
 */
#define MAX_BYTES 4

static Boolean
CFURLAppendPercentEscapedForCharacter (char **dst, UniChar c,
  CFStringEncoding enc)
{
  CFIndex len;
  UInt8 buffer[MAX_BYTES];
  const UniChar *source;
  
  source = &c;
  if ((len =
      GSFromUnicode(source, 1, enc, 0, false, buffer, MAX_BYTES, NULL)))
    {
      UInt8 hi;
      UInt8 lo;
      UInt8 *target;
      const UInt8 *end;
      
      target = buffer;
      end = target + len;
      do
        {
          (*(*dst)++) = '%';
          hi = ((*target >> 4) & 0x0F);
          lo = (*target & 0x0F);
          (*(*dst)++) = (hi > 9) ? hi + 'A' - 10 : hi + '0';
          (*(*dst)++) = (lo > 9) ? lo + 'A' - 10 : lo + '0';
          
          ++target;
        } while (target < end);
      
      return true;
    }
  
  return false;
}

static Boolean
CFURLStringContainsCharacter (CFStringRef toEscape, UniChar ch)
{
  CFStringInlineBuffer iBuffer;
  CFIndex sLength;
  CFIndex i;
  UniChar c;
  
  sLength = CFStringGetLength (toEscape);
  CFStringInitInlineBuffer (toEscape, &iBuffer, CFRangeMake (0, sLength));
  for (i = 0 ; i < sLength ; ++i)
    {
      c = CFStringGetCharacterFromInlineBuffer (&iBuffer, i);
      if (c == ch)
        return true;
    }
  
  return false;
}

CF_INLINE Boolean
CFURLShouldEscapeCharacter (UniChar c, CFStringRef leaveUnescaped,
  CFStringRef toEscape)
{
  if (URL_IS_UNRESERVED(c) || URL_IS_RESERVED(c))
    {
      if (toEscape && CFURLStringContainsCharacter(toEscape, c))
        return true;
      
      return false;
    }
  
  if (leaveUnescaped && CFURLStringContainsCharacter(leaveUnescaped, c))
    return false;
  
  return true;
}

CFStringRef
CFURLCreateStringByAddingPercentEscapes (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef leaveUnescaped,
  CFStringRef toEscape, CFStringEncoding encoding)
{
  CFStringInlineBuffer iBuffer;
  CFStringRef ret;
  CFIndex sLength;
  CFIndex idx;
  char *dst;
  char *dpos;
  UniChar c;
  
  sLength = CFStringGetLength (origString);
  CFStringInitInlineBuffer (origString, &iBuffer, CFRangeMake (0, sLength));
  
  dst = NULL;
  dpos = dst;
  for (idx = 0 ; idx < sLength ; ++idx)
    {
      c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx);
      if (CFURLShouldEscapeCharacter(c, leaveUnescaped, toEscape))
        {
          if (dst == NULL)
            {
              dst = CFAllocatorAllocate (alloc, sizeof(char) * sLength * 3, 0);
              CFStringGetBytes (origString, CFRangeMake(0, idx),
                kCFStringEncodingASCII, 0, false, (UInt8*)dst,
                sLength * 3, NULL);
              dpos = dst + idx;
            }
          if (!CFURLAppendPercentEscapedForCharacter (&dpos, c, encoding))
            {
              CFAllocatorDeallocate (alloc, dst);
              return NULL;
            }
        }
      else if (dst != NULL)
        {
            (*dpos++) = (char)c;
        }
    }
  
  if (dst)
    {
      ret = CFStringCreateWithBytes (alloc, (UInt8*)dst, (CFIndex)(dpos - dst),
        kCFStringEncodingASCII, false);
      CFAllocatorDeallocate (alloc, dst);
    }
  else
    {
      ret = CFRetain (origString);
    }
  
  return ret;
}

CFStringRef
CFURLCreateStringByReplacingPercentEscapes (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef leaveEscaped)
{
  return CFURLCreateStringByReplacingPercentEscapesUsingEncoding (alloc,
    origString, leaveEscaped, kCFStringEncodingUTF8);
}

CF_INLINE char
CFURLCharacterForPercentEscape (CFStringInlineBuffer *src, CFIndex *idx,
  CFStringEncoding enc)
{
  UInt8 bytes[MAX_BYTES];
  UInt8 tmp;
  UInt8 *str;
  UniChar c1;
  UniChar c2;
  UniChar c;
  CFIndex num;
  CFIndex i;
  CFIndex j;
  
  i = (*idx);
  j = 0;
  do
    {
      c1 = CFStringGetCharacterFromInlineBuffer (src, i++);
      c2 = CFStringGetCharacterFromInlineBuffer (src, i++);
      
      if (c1 <= '9')
        tmp = c1 - '0';
      else if (c1 <= 'F')
        tmp = c1 - 'A' + 10;
      else
        tmp = c1 - 'a' + 10;
      tmp = (tmp & 0x0F) << 4;
      if (c2 <= '9')
        tmp |= c2 - '0';
      else if (c2 <= 'F')
        tmp |= c2 - 'A' + 10;
      else
        tmp |= c2 - 'a' + 10;
      
      bytes[j++] = tmp;
    } while (CFStringGetCharacterFromInlineBuffer(src, i++) == '%'
             && j < MAX_BYTES);
  
  c = 0;
  str = bytes;
  num = GSToUnicode ((const UInt8*)str, j, enc, 0, false, &c, 1, NULL);
  if (num)
    {
      // For the first percent 2 characters get used, for all later three
      (*idx) += 2 + 3 * (num - 1);
    }
  return c;
}

CFStringRef
CFURLCreateStringByReplacingPercentEscapesUsingEncoding (CFAllocatorRef alloc,
  CFStringRef origString, CFStringRef leaveEscaped, CFStringEncoding encoding)
{
  CFStringInlineBuffer iBuffer;
  CFStringRef ret;
  CFIndex sLength;
  CFIndex idx;
  UniChar *dst;
  UniChar *dpos;
  UniChar c;
  
  sLength = CFStringGetLength (origString);
  CFStringInitInlineBuffer (origString, &iBuffer, CFRangeMake (0, sLength));
  
  dst = NULL;
  dpos = dst;
  idx = 0;
  while (idx < sLength)
    {
      c = CFStringGetCharacterFromInlineBuffer (&iBuffer, idx++);
      if (c == '%' && leaveEscaped && (idx + 2) < sLength)
        {
          UniChar repChar;
          
          if (dst == NULL)
            {
              dst = CFAllocatorAllocate (alloc, sizeof(UniChar) * sLength, 0);
              CFStringGetCharacters (origString, CFRangeMake(0, idx - 1),
                dst);
              dpos = dst + idx - 1;
            }
          repChar = CFURLCharacterForPercentEscape (&iBuffer, &idx, encoding);
          if (CFURLStringContainsCharacter(leaveEscaped, repChar))
            /* Skip the '%' */
            (*dpos++) = c;
          else
            (*dpos++) = repChar;
        }
      else if (dst != NULL)
        {
          (*dpos++) = c;
        }
    }
  
  if (dst)
    {
      ret = CFStringCreateWithCharacters (alloc, dst, (CFIndex)(dpos - dst));
      CFAllocatorDeallocate (alloc, dst);
    }
  else
    {
      ret = CFRetain (origString);
    }
  
  return ret;
}

CFStringRef
CFURLGetString (CFURLRef url)
{
  CF_OBJC_FUNCDISPATCHV(_kCFURLTypeID, CFStringRef, url, "relativeString");
  return url->_urlString;
}

CFURLRef
CFURLGetBaseURL (CFURLRef url)
{
  CF_OBJC_FUNCDISPATCHV(_kCFURLTypeID, CFURLRef, url, "baseURL");
  return url->_baseURL;
}

Boolean
CFURLGetFileSystemRepresentation (CFURLRef url, Boolean resolveAgainstBase,
  UInt8 *buffer, CFIndex bufLen)
{
  CFStringRef str;
  CFStringRef unescaped;
  CFStringEncoding enc;
  Boolean ret;
  
  if (CF_IS_OBJC(_kCFURLTypeID, url))
    enc = kCFStringEncodingUTF8;
  else
    enc = url->_encoding;
  
  if (resolveAgainstBase)
    url = CFURLCopyAbsoluteURL (url);
  str = CFURLCopyFileSystemPath (url, CFURL_DEFAULT_PATH_STYLE);
  if (resolveAgainstBase)
    CFRelease (url);
  unescaped = CFURLCreateStringByReplacingPercentEscapesUsingEncoding (NULL,
    str, CFSTR(""), enc);
  CFRelease (str);
  
  ret = CFStringGetFileSystemRepresentation (unescaped, (char*)buffer, bufLen);
  CFRelease (unescaped);
  
  return ret;
}

CFIndex
CFURLGetBytes (CFURLRef url, UInt8 *buffer, CFIndex bufLen)
{
  CFIndex used;
  CFIndex length;
  CFIndex converted;
  CFStringRef str;
  CFStringEncoding enc;
  
  if (CF_IS_OBJC(_kCFURLTypeID, url))
    enc = kCFStringEncodingUTF8;
  else
    enc = url->_encoding;
  
  str = CFURLGetString (url);
  length = CFStringGetLength (str);
  converted = CFStringGetBytes (str, CFRangeMake(0, length), enc, 0, false,
    buffer, bufLen, &used);
  if (converted != used)
    used = -1;
  
  return used;
}

CFRange
CFURLGetByteRangeForComponent (CFURLRef url, CFURLComponentType comp,
  CFRange *rangeIncludingSeparators)
{
  return CFRangeMake (kCFNotFound, 0); /* FIXME */
}

Boolean
CFURLResourceIsReachable (CFURLRef url, CFErrorRef *error)
{
  return false;
}

void
CFURLClearResourcePropertyCache (CFURLRef url)
{
  
}

void
CFURLClearResourcePropertyCacheForKey (CFURLRef url, CFStringRef key)
{
  
}

CFDictionaryRef
CFURLCopyResourcePropertiesForKeys (CFURLRef url, CFArrayRef keys,
  CFErrorRef *error)
{
  return NULL;
}

Boolean
CFURLCopyResourcePropertyForKey (CFURLRef url, CFStringRef key,
  void *propertyValueTypeRefPtr, CFErrorRef *error)
{
  return false;
}

CFDictionaryRef
CFURLCreateResourcePropertiesForKeysFromBookmarkData (CFAllocatorRef alloc,
  CFArrayRef resourcePropertiesToReturn, CFDataRef bookmark)
{
  return NULL; /* FIXME */
}

CFTypeRef
CFURLCreateResourcePropertyForKeyFromBookmarkData (CFAllocatorRef alloc,
  CFStringRef resourcePropertyKey, CFDataRef bookmark)
{
  return NULL; /* FIXME */
}

Boolean
CFURLSetResourcePropertiesForKeys (CFURLRef url,
  CFDictionaryRef keyedPropertyValues, CFErrorRef *error)
{
  return false;
}

Boolean
CFURLSetResourcePropertyForKey (CFURLRef url, CFStringRef key,
  CFTypeRef propertValue, CFErrorRef *error)
{
  return false;
}

void
CFURLSetTemporaryResourcePropertyForKey (CFURLRef url, CFStringRef key,
  CFTypeRef propertyValue)
{
  
}

CFURLRef
CFURLCreateByResolvingBookmarkData (CFAllocatorRef alloc, CFDataRef bookmark,
  CFURLBookmarkResolutionOptions options, CFURLRef relativeToURL,
  CFArrayRef resourcePropertiesToInclude, Boolean *isStale, CFErrorRef *error)
{
  return NULL; /* FIXME */
}

CFDataRef
CFURLCreateBookmarkData (CFAllocatorRef alloc, CFURLRef url,
  CFURLBookmarkCreationOptions options, CFArrayRef resourcePropertiesToInclude,
  CFURLRef relativeToURL, CFErrorRef *error)
{
  return NULL; /* FIXME */
}

CFDataRef
CFURLCreateBookmarkDataFromAliasRecord (CFAllocatorRef alloc,
  CFDataRef aliasRecordDataRef)
{
  return NULL; /* FIXME */
}

CFDataRef
CFURLCreateBookmarkDataFromFile (CFAllocatorRef alloc, CFURLRef fileURL,
  CFErrorRef *errorRef)
{
  return NULL; /* FIXME */
}

Boolean
CFURLWriteBookmarkDataToFile (CFDataRef bookmarkRef, CFURLRef fileURL,
  CFURLBookmarkFileCreationOptions options, CFErrorRef *errorRef)
{
  return false; /* FIXME */
}

