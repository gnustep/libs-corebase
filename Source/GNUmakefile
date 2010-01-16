include $(GNUSTEP_MAKEFILES)/common.make

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBase.m \
  CFArray.m \
  CFData.m \
  CFDictionary.m \
  CFString.m

ADDITIONAL_INCLUDE_DIRS = -I../Headers
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_INSTALL_DIR = /CoreFoundation

libgnustep-corebase_HEADER_FILES = \
  CFBase.h \
  CFArray.h \
  CFData.h \
  CFDictionary.h \
  CFCharacterSet.h \
  CFString.h

libgnustep-corebase_NEEDS_GUI = NO

include $(GNUSTEP_MAKEFILES)/library.make
