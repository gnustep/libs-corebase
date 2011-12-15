include $(GNUSTEP_MAKEFILES)/common.make

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBundle.m \
  CFPropertyList.m \
  CFStream.m \
  CFURL.m \
  NSCFArray.m \
  NSCFData.m \
  NSCFError.m \
  NSCFString.m \
  NSCFType.m \
  Identifiers.m

libgnustep-corebase_C_FILES = \
  CFArray.c \
  CFBag.c \
  CFBase.c \
  CFCalendar.c \
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
  CFUUID.c \
  GSHashTable.c

libgnustep-corebase_LIBRARIES_DEPEND_UPON += `icu-config --ldflags-libsonly`
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_FILES_INSTALL_DIR = CoreFoundation

libgnustep-corebase_HEADER_FILES = \
  CoreFoundation.h \
  CFBag.h \
  CFBase.h \
  CFBundle.h \
  CFArray.h \
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
  CFPropertyList.h \
  CFRuntime.h \
  CFSet.h \
  CFStream.h \
  CFString.h \
  CFStringEncodingExt.h \
  CFTimeZone.h \
  CFURL.h \
  CFUUID.h

libgnustep-corebase_NEEDS_GUI = NO


ifeq ($(GNUSTEP_TARGET_OS), mingw32)
  ADDITIONAL_LDFLAGS = -L/mingw/bin
endif

ADDITIONAL_INCLUDE_DIRS = -I../Headers
ADDITIONAL_CFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700
ADDITIONAL_CXXFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700
ADDITIONAL_OBJCFLAGS = -DBUILDING_SELF -DMAC_OS_X_VERSION_MAX_ALLOWED=100700

include $(GNUSTEP_MAKEFILES)/library.make
