include $(GNUSTEP_MAKEFILES)/common.make

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBase.m \
  CFArray.m \
  CFData.m \
  CFDate.m \
  CFDictionary.m \
  CFError.m \
  CFNumber.m \
  CFPropertyList.m \
  CFRuntime.m \
  CFStream.m \
  CFString.m \
  CFURL.m \
  NSCFType.m \
  Identifiers.m

libgnustep-corebase_C_FILES = \
  CFByteOrder.c \
  CFLocale.c \
  CFNumberFormatter.c

ADDITIONAL_INCLUDE_DIRS = -I../Headers
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_FILES_INSTALL_DIR = CoreFoundation

libgnustep-corebase_HEADER_FILES = \
  CoreFoundation.h \
  CFBase.h \
  CFArray.h \
  CFByteOrder.h \
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
  CFURL.h

libgnustep-corebase_NEEDS_GUI = NO

include $(GNUSTEP_MAKEFILES)/library.make
