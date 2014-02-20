#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([libgnustep-corebase], [0.2], [bug-gnustep@gnu.org])
AC_CONFIG_SRCDIR([Source/CFBase.c])
AC_CONFIG_HEADERS([Source/config.h])

# Checks for programs.
AC_PROG_CC
#AC_CHECK_TARGET_TOOL([LD], [ld])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h limits.h math.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

AC_C_INLINE
AC_C_BIGENDIAN

# Gather information about the data model so we can define the data types
m4_define([GS_CHECK_DATA_MODEL],
[AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([void *])
AC_DEFINE([DATA_MODEL_IP16], [122482], [Defined to IP16 data model.])
AC_DEFINE([DATA_MODEL_LP32], [122484], [Defined to LP32 data model.])
AC_DEFINE([DATA_MODEL_ILP32], [124484], [Defined to ILP32 data model.])
AC_DEFINE([DATA_MODEL_LLP64], [124488], [Defined to LLP64 data model.])
AC_DEFINE([DATA_MODEL_LP64], [124888], [Defined to LP64 data model.])
AC_CACHE_CHECK([for data model],
                [gs_cv_c_data_model],
                [_gs_data_model="$ac_cv_sizeof_char$ac_cv_sizeof_short$ac_cv_sizeof_int$ac_cv_sizeof_long$ac_cv_sizeof_long_long$ac_cv_sizeof_void_p"
                AS_CASE([$_gs_data_model],
                        [122482], [gs_cv_c_data_model="IP16"],
                        [122484], [gs_cv_c_data_model="LP32"],
                        [124484], [gs_cv_c_data_model="ILP32"],
                        [124488], [gs_cv_c_data_model="LLP64"],
                        [124888], [gs_cv_c_data_model="LP64"],
                        [gs_cv_c_data_model="no"])])
AC_DEFINE_UNQUOTED([DATA_MODEL], [DATA_MODEL_$gs_cv_c_data_model], [Define to the architecture's data model.])
]) # GS_CHECK_DATA_MODEL
GS_CHECK_DATA_MODEL
AC_SUBST([GS_INT8_T], ["char"])
AC_SUBST([GS_UINT8_T], ["unsigned char"])
AC_SUBST([GS_INT16_T], ["short"])
AC_SUBST([GS_UINT16_T], ["unsigned short"])
AC_SUBST([GS_INT64_T], ["long long"])
AC_SUBST([GS_UINT64_T], ["unsigned long long"])
AS_CASE([$gs_cv_c_data_model],
        [IP16], [AC_SUBST([GS_INT32_T], ["long"])
                AC_SUBST([GS_UINT32_T], ["unsigned long"])
                AC_SUBST([GS_INTPTR_T], ["int"])
                AC_SUBST([GS_UINTPTR_T], ["unsigned int"])],
        [LP32], [AC_SUBST([GS_INT32_T], ["long"])
                AC_SUBST([GS_UINT32_T], ["unsigned long"])
                AC_SUBST([GS_INTPTR_T], ["long"])
                AC_SUBST([GS_UINTPTR_T], ["unsigned long"])],
        [ILP32], [AC_SUBST([GS_INT32_T], ["int"])
                 AC_SUBST([GS_UINT32_T], ["unsigned int"])
                 AC_SUBST([GS_INTPTR_T], ["int"])
                 AC_SUBST([GS_UINTPTR_T], ["unsigned int"])],
        [LLP64], [AC_SUBST([GS_INT32_T], ["int"])
                 AC_SUBST([GS_UINT32_T], ["unsigned int"])
                 AC_SUBST([GS_INTPTR_T], ["long long"])
                 AC_SUBST([GS_UINTPTR_T], ["unsigned long long"])],
        [LP64], [AC_SUBST([GS_INT32_T], ["int"])
                 AC_SUBST([GS_UINT32_T], ["unsigned int"])
                 AC_SUBST([GS_INTPTR_T], ["long"])
                 AC_SUBST([GS_UINTPTR_T], ["unsigned long"])],
        [AC_MSG_ERROR([Could not identify the architecture's data model.])])

#---
# Check for ICU
#---
m4_define([GS_CHECK_ICU],
[# Adapted from icu.m4 in GNUstep-base
AC_PATH_PROG(ICU_CONFIG, icu-config, [no])
AC_MSG_CHECKING(for ICU $1 or later)
AS_IF([test "$ICU_CONFIG" = "no"],
      [AC_MSG_ERROR([Could not find 'icu-config'.  Please install ICU or specify --without-icu.])])
ICU_VERSION=`$ICU_CONFIG --version`
found=`expr $ICU_VERSION \>= $1`
AS_IF([test "$found" = "1"],
      [AC_MSG_RESULT(yes)
      ICU_LIBS=`$ICU_CONFIG --ldflags-libsonly`
      LIBS="$LIBS $ICU_LIBS"],
      [AC_MSG_RESULT(no)
      AC_MSG_ERROR([Could not find ICU $1 or later])])

AC_CHECK_HEADERS_ONCE([unicode/ucal.h unicode/uchar.h unicode/ucnv.h unicode/ucol.h unicode/ucurr.h unicode/udat.h unicode/udatpg.h unicode/uloc.h unicode/ulocdata.h unicode/unorm.h unicode/unum.h unicode/usearch.h unicode/ustring.h unicode/utrans.h])

# Double check we can actually compile with ICU
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <unicode/uversion.h>],
                                [UVersionInfo version_info;
                                 u_getVersion(version_info);])])

AC_DEFINE([HAVE_ICU], [1],
          [Define to 1 if you have International Components for Unicode.])
]) # GS_CHECK_ICU
GS_CHECK_ICU([4.0])

#---
# Check for the Objective-C runtime
#---
AC_ARG_ENABLE([objc-bridge],
  [AS_HELP_STRING([--disable-objc-bridge],
    [Disables the Objective-C toll-free bridge mechanism.])],
  [enable_objc_bridge=$enableval], [enable_objc_bridge=yes])
AS_IF([test "$enable_objc_bridge" = "yes"],
      [AC_PATH_PROG(GNUSTEP_CONFIG, [gnustep-config], [no])
      AS_IF([test "$GNUSTEP_CONFIG" != "no"],
            [OBJC_LIBS=`$GNUSTEP_CONFIG --base-libs`
            LIBS="$LIBS $OBJC_LIBS"
            AC_DEFINE(HAVE_LIBOBJC, 1)],
            [AC_CHECK_HEADER(objc/runtime.h)
            AS_IF([test "$ac_cv_header_objc_runtime_h" = "no"],
                  [AC_MSG_ERROR([Could not find the Objective-C runtime header.])
            ])
            AC_CHECK_LIB(objc, objc_getClass)
            AS_IF([test "$ac_cv_lib_objc_objc_getClass" = "no"],
                  [AC_CHECK_LIB(objc2, objc_getClass)
                  AS_IF([test "$ac_cv_lib_objc2_objc_getClass" = "no"],
                        [AC_MSG_ERROR([Objective-C library not found!  Use --disable-objc-bridge to explicitly disable the toll-free bridge or install the Objective-C library.])])])])])

#---
# Check for zoneinfo directory
#---
AC_ARG_WITH([zoneinfo-dir],
  [AS_HELP_STRING([--with-zoneinfo-dir=DIR],
    [Directory of the Time zone object files.])],
  [], [with_zoneinfo_dir=no])
AC_MSG_CHECKING([zoneinfo directory])
AS_IF([test "$with_zoneinfo_dir" = "no"],
      [AS_IF([test -d "/usr/share/zoneinfo"],
               [with_zoneinfo_dir='/usr/share/zoneinfo'],
             [test -d "/usr/lib/zoneinfo"],
               [with_zoneinfo_dir='/usr/lib/zoneinfo'],
             [test -d "/usr/local/share/zoneinfo"],
               [with_zoneinfo_dir='/usr/local/share/zoneinfo'],
             [test -d "/usr/local/lib/zoneinfo"],
               [with_zoneinfo_dir='/usr/local/lib/zoneinfo'],
             [test -d "/etc/zoneinfo"],
               [with_zoneinfo_dir='/etc/zoneinfo'],
             [test -d "/usr/local/etc/zoneinfo"],
               [with_zoneinfo_dir='/usr/local/etc/zoneinfo'])])
AS_IF([test "$with_zoneinfo_dir" = "no" || test ! -d "$with_zoneinfo_dir"],
      [AC_MSG_ERROR([Please specify a valid zoneinfo directory.])])
AC_MSG_RESULT($with_zoneinfo_dir)
AC_DEFINE_UNQUOTED([TZDIR], ["$with_zoneinfo_dir"],
                   [Define to the directory contain time zone object files.])

AC_CONFIG_FILES([Source/GNUmakefile Headers/CoreFoundation/CFBase.h])

AC_OUTPUT
