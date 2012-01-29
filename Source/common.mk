#
# Common macros
#

ADDITIONAL_INCLUDE_DIRS = -I../Headers
ADDITIONAL_CFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700
ADDITIONAL_CXXFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700
ADDITIONAL_OBJCFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700

COREBASE_C_FILES = \
  CFArray.c \
  CFBag.c \
  CFBase.c \
  CFBinaryHeap.c \
  CFBitVector.c \
  CFCalendar.c \
  CFCharacterSet.c \
  CFData.c \
  CFDate.c \
  CFDateFormatter.c \
  CFDictionary.c \
  CFError.c \
  CFLocale.c \
  CFNumber.c \
  CFNumberFormatter.c \
  CFRuntime.c \
  CFSet.c \
  CFString.c \
  CFStringEncoding.c \
  CFStringFormat.c \
  CFStringUtilities.c \
  CFTimeZone.c \
  CFTree.c \
  CFURL.c \
  CFUUID.c \
  CFXMLNode.c \
  CFXMLParser.c \
  GSHashTable.c

COREBASE_HEADER_FILES = \
  CoreFoundation.h \
  CFArray.h \
  CFAttributedString.h \
  CFBag.h \
  CFBase.h \
  CFBinaryHeap.h \
  CFBitVector.h \
  CFBundle.h \
  CFByteOrder.h \
  CFCalendar.h \
  CFCharacterSet.h \
  CFData.h \
  CFDate.h \
  CFDateFormatter.h \
  CFDictionary.h \
  CFError.h \
  CFLocale.h \
  CFNumber.h \
  CFNumberFormatter.h \
  CFRunLoop.h \
  CFRuntime.h \
  CFSet.h \
  CFString.h \
  CFStringEncodingExt.h \
  CFTimeZone.h \
  CFTree.h \
  CFURL.h \
  CFUUID.h \
  CFXMLNode.h \
  CFXMLParser.h
