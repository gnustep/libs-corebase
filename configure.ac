#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libgnustep-corebase], [0.2], [bug-gnustep@gnu.org])
AC_CONFIG_SRCDIR([Source/CFBase.c])
AC_CONFIG_HEADERS([Source/config.h])

#--------------------------------------------------------------------
# Find the compiler
#--------------------------------------------------------------------
AC_PROG_CC
AC_PROG_CPP

# Check for header files.
AC_HEADER_STDC

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
                       [122484], [gs_cv_c_data_model="LP32"],
                       [124484], [gs_cv_c_data_model="ILP32"],
                       [124488], [gs_cv_c_data_model="LLP64"],
                       [124888], [gs_cv_c_data_model="LP64"],
                       [gs_cv_c_data_model="no"])])
AC_DEFINE_UNQUOTED([DATA_MODEL], [DATA_MODEL_$gs_cv_c_data_model], [Define to the architecture's data model.])
]) # GS_CHECK_DATA_MODEL
GS_CHECK_DATA_MODEL
AC_SUBST([GS_SINT8_T], ["signed char"])
AC_SUBST([GS_UINT8_T], ["unsigned char"])
AC_SUBST([GS_SINT16_T], ["signed short"])
AC_SUBST([GS_UINT16_T], ["unsigned short"])
AC_SUBST([GS_SINT64_T], ["signed long long"])
AC_SUBST([GS_UINT64_T], ["unsigned long long"])
AS_CASE([$gs_cv_c_data_model],
        [LP32], [AC_SUBST([GS_SINT32_T], ["signed long"])
                AC_SUBST([GS_UINT32_T], ["unsigned long"])
                AC_SUBST([GS_SINTPTR_T], ["signed long"])
                AC_SUBST([GS_UINTPTR_T], ["unsigned long"])],
        [ILP32], [AC_SUBST([GS_SINT32_T], ["signed int"])
                 AC_SUBST([GS_UINT32_T], ["unsigned int"])
                 AC_SUBST([GS_SINTPTR_T], ["signed int"])
                 AC_SUBST([GS_UINTPTR_T], ["unsigned int"])],
        [LLP64], [AC_SUBST([GS_SINT32_T], ["signed int"])
                 AC_SUBST([GS_UINT32_T], ["unsigned int"])
                 AC_SUBST([GS_SINTPTR_T], ["signed long long"])
                 AC_SUBST([GS_UINTPTR_T], ["unsigned long long"])],
        [LP64], [AC_SUBST([GS_SINT32_T], ["signed int"])
                AC_SUBST([GS_UINT32_T], ["unsigned int"])
                AC_SUBST([GS_SINTPTR_T], ["signed long"])
                AC_SUBST([GS_UINTPTR_T], ["unsigned long"])],
        [AC_MSG_ERROR([Could not identify the architecture's data model.])])

AC_CHECK_SIZEOF([double])
AC_CHECK_SIZEOF([long double])

#---
# Support disabling conversion to/from the encodings in CFStringEncodingExt.h
#---
#AC_ARG_ENABLE([external-encodings],
#  [AS_HELP_STRING([--without-external-encodings],
#    [Disable the ability to convert to and from the external encodings defined in CFStringEncodingExt.h.])],
#  [enable_external_encoding=$enableval], [enable_external_encodings=yes])
#AS_IF([test "x$enable_external_encodings" = xyes],
#  [AC_DEFINE([ENABLE_EXTERNAL_ENCODINGS], [1],
#    [Define to 1 if external encodings are enabled.])])

#---
# Check for ICU
#---
m4_define([GS_CHECK_ICU],
[# Adapted from icu.m4 in GNUstep-base
AC_PATH_PROG(ICU_CONFIG, icu-config, [no])
AC_MSG_CHECKING(for ICU $1 or later)
AS_IF([test x"$ICU_CONFIG" = xno],
      [AC_MSG_ERROR([Could not find 'icu-config'.  Please install ICU or specify --without-icu.])])
ICU_VERSION=`$ICU_CONFIG --version`
found=`expr $ICU_VERSION \>= $1`
AS_IF([test x"$found" = x1],
      [AC_MSG_RESULT(yes)
      ICU_LIBS=`$ICU_CONFIG --ldflags-libsonly`
      LIBS="$LIBS $ICU_LIBS"],
      [AC_MSG_RESULT(no)
      AC_MSG_ERROR([Could not find ICU $1 or later])])

AC_CHECK_HEADERS([unicode/ucal.h unicode/uchar.h unicode/ucnv.h unicode/ucol.h unicode/ucurr.h unicode/udat.h unicode/udatpg.h unicode/uloc.h unicode/ulocdata.h unicode/unorm.h unicode/unum.h unicode/usearch.h unicode/ustring.h unicode/utrans.h],
                  [], AC_MSG_ERROR([Could not find required ICU headers.]))

AC_DEFINE([HAVE_ICU], [1],
          [Define to 1 if you have International Components for Unicode.])
]) # GS_CHECK_ICU
GS_CHECK_ICU([4.0])

#---
# Check for Grand Central Dispatch (libdispatch)
#---
AC_ARG_WITH([gcd],
  [AS_HELP_STRING([--without-gcd],
    [Explicitly compiles without Grand Central Dispatch support. Configure will attempt to find a compatible library, use this option if you do not wish to have the support.  The opposite option, --with-gcd, is also supported and should be used if Grand Central Dispatch support is absolutely required.  A compatible library will required (http://www.nickhutchinson.me/libdispatch).])],
  [with_gcd=$withval], [with_gcd=maybe])
AS_IF([test "$with_gcd" != "no"],
      [AC_CHECK_HEADER(dispatch/dispatch.h)
      AS_IF([test x"$ac_cv_header_dispatch_dispatch_h" = "xno"],
            AS_IF([test x"$with_gcd" = "xyes"],
                  AC_MSG_ERROR([Could not find the Grand Central Dispatch headers.])),
      	    AC_CHECK_LIB(dispatch, dispatch_get_main_queue_eventfd_np)
            AS_IF([test x"$ac_cv_lib_dispatch_dispatch_get_main_queue_eventfd_np" = "xno" -a x"$with_gcd" = "xyes"],
                  AC_MSG_ERROR([Compatible libdispatch not found for GCD support!])))])

#---
# Check for the Objective-C runtime
#---
AC_ARG_WITH([objc],
  [AS_HELP_STRING([--without-objc],
    [Compiles without support for Objective-C.])],
  [with_objc=$withval], [with_objc=yes])
AS_IF([test "x$with_objc" != xno],
  [AC_PATH_PROG(GNUSTEP_CONFIG, [gnustep-config], [no])
  AS_IF([test x"$GNUSTEP_CONFIG" != "xno"],
    [MAKECC=`gnustep-config --variable=CC`
    if test "$CC" != "$MAKECC"; then
      AC_MSG_ERROR([You are running configure with the compiler ($CC) set to a different value from that used by gnustep-make ($MAKECC).  Please run configure again with your environment set to match your gnustep-make])
      exit 1
    fi
    OBJC_LIBS=`$GNUSTEP_CONFIG --base-libs`
    LIBS="$LIBS $OBJC_LIBS"
    AC_DEFINE(HAVE_LIBOBJC, 1)],
    [AC_CHECK_HEADER(objc/runtime.h)
    AS_IF([test "x$ac_cv_header_objc_runtime_h" = xno],
      [AC_MSG_ERROR([Could not find the Objective-C runtime header.])])
    AC_CHECK_LIB(objc, objc_getClass)
    AS_IF([test "x$ac_cv_lib_objc_objc_getClass" = xno],
      [AC_CHECK_LIB(objc2, objc_getClass)
      AS_IF([test "x$ac_cv_lib_objc2_objc_getClass" = xno],
        [AC_MSG_ERROR([Objective-C library not found!  Use --without-objc-bridge to explicitly disable the toll-free bridge or install the Objective-C library.])])])])])

#---
# Check for zoneinfo directory
#---
AC_ARG_WITH([zoneinfo],
  [AS_HELP_STRING([--with-zoneinfo=DIR],
    [Directory of the Time zone object files.])],
  [], [with_zoneinfo=no])
AC_MSG_CHECKING([zoneinfo directory])
AS_IF([test "x$with_zoneinfo" = xno],
  [AS_IF([test -d "/usr/share/zoneinfo"],
    [with_zoneinfo='/usr/share/zoneinfo'],
  [test -d "/usr/lib/zoneinfo"],
    [with_zoneinfo='/usr/lib/zoneinfo'],
  [test -d "/usr/local/share/zoneinfo"],
    [with_zoneinfo='/usr/local/share/zoneinfo'],
  [test -d "/usr/local/lib/zoneinfo"],
    [with_zoneinfo='/usr/local/lib/zoneinfo'],
  [test -d "/etc/zoneinfo"],
    [with_zoneinfo='/etc/zoneinfo'],
  [test -d "/usr/local/etc/zoneinfo"],
    [with_zoneinfo='/usr/local/etc/zoneinfo'])])
AS_IF([test "$with_zoneinfo" = "no" || test ! -d "$with_zoneinfo"],
  [AC_MSG_ERROR([Please specify a valid zoneinfo directory.])])
AC_MSG_RESULT($with_zoneinfo)
AC_DEFINE_UNQUOTED([TZDIR], ["$with_zoneinfo"],
  [Define to the directory contain time zone object files.])

AC_CONFIG_FILES([Source/GNUmakefile Headers/CoreFoundation/CFBase.h])

AC_OUTPUT
