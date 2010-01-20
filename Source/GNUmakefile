include $(GNUSTEP_MAKEFILES)/common.make

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBase.m \
  CFArray.m \
  CFData.m \
  CFDictionary.m \
  CFError.m \
  CFString.m

ADDITIONAL_INCLUDE_DIRS = -I../Headers
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_FILES_INSTALL_DIR = CoreFoundation

libgnustep-corebase_HEADER_FILES = \
  CoreFoundation.h \
  CFBase.h \
  CFArray.h \
  CFCharacterSet.h \
  CFData.h \
  CFDictionary.h \
  CFError.h \
  CFString.h \
  CFStringEncodingExt.h

libgnustep-corebase_NEEDS_GUI = NO

include $(GNUSTEP_MAKEFILES)/library.make
