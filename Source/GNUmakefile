include $(GNUSTEP_MAKEFILES)/common.make
include common.mk

PACKAGE_NAME = gnustep-corebase

LIBRARY_NAME = libgnustep-corebase

libgnustep-corebase_OBJC_FILES = \
  CFBundle.m \
  CFPropertyList.m \
  CFStream.m \
  NSCFArray.m \
  NSCFData.m \
  NSCFError.m \
  NSCFString.m \
  NSCFType.m

libgnustep-corebase_C_FILES = $(COREBASE_C_FILES)

libgnustep-corebase_LIBRARIES_DEPEND_UPON += `icu-config --ldflags-libsonly`
libgnustep-corebase_HEADER_FILES_DIR = ../Headers/CoreFoundation
libgnustep-corebase_HEADER_FILES_INSTALL_DIR = CoreFoundation

libgnustep-corebase_HEADER_FILES = $(COREBASE_HEADER_FILES)

libgnustep-corebase_NEEDS_GUI = NO


ifeq ($(GNUSTEP_TARGET_OS), mingw32)
  ADDITIONAL_LDFLAGS = -L/mingw/bin
endif

include $(GNUSTEP_MAKEFILES)/library.make
