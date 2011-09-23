include $(GNUSTEP_MAKEFILES)/common.make

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBundle.m \
  CFArray.m \
  CFDictionary.m \
  CFError.m \
  CFNumber.m \
  CFPropertyList.m \
  CFStream.m \
  CFURL.m \
  NSCFData.m \
  NSCFString.m \
  NSCFType.m \
  Identifiers.m

libgnustep-corebase_C_FILES = \
  CFBase.c \
  CFCalendar.c \
  CFData.c \
  CFDate.c \
  CFLocale.c \
  CFNumberFormatter.c \
  CFRuntime.c \
  CFString.c \
  CFStringEncoding.c \
  CFStringFormat.c \
  CFStringUtilities.c \
  CFTimeZone.c \
  CFUUID.c

ADDITIONAL_INCLUDE_DIRS = -I../Headers
libgnustep-corebase_LIBRARIES_DEPEND_UPON += `icu-config --ldflags-libsonly`
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_FILES_INSTALL_DIR = CoreFoundation

libgnustep-corebase_HEADER_FILES = \
  CoreFoundation.h \
  CFBase.h \
  CFBundle.h \
  CFArray.h \
  CFByteOrder.h \
  CFCalendar.h \
  CFCharacterSet.h \
  CFData.h \
  CFDate.h \
  CFDictionary.h \
  CFError.h \
  CFLocale.h \
  CFNumber.h \
  CFNumberFormatter.h \
  CFPropertyList.h \
  CFRuntime.h \
  CFStream.h \
  CFString.h \
  CFStringEncodingExt.h \
  CFTimeZone.h \
  CFURL.h \
  CFUUID.h

libgnustep-corebase_NEEDS_GUI = NO

include $(GNUSTEP_MAKEFILES)/library.make
