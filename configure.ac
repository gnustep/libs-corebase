#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libgnustep-corebase], [0.2], [bug-gnustep@gnu.org])
AC_CONFIG_SRCDIR([Source/CFBase.c])
AC_CONFIG_HEADERS([Source/config.h])

if test -n "$GNUSTEP_MAKEFILES"; then
  #--------------------------------------------------------------------
  # Use config.guess, config.sub and install-sh provided by gnustep-make
  #--------------------------------------------------------------------
  AC_CONFIG_AUX_DIR($GNUSTEP_MAKEFILES)

  #--------------------------------------------------------------------
  # Determine the host, build, and target systems
  #--------------------------------------------------------------------
  AC_CANONICAL_TARGET([])
fi

#--------------------------------------------------------------------
# Find the compiler
#--------------------------------------------------------------------
AC_PROG_CPP
AC_PROG_CC
AC_PROG_OBJC

# Check for header files.
AC_HEADER_STDC

AC_C_INLINE
AC_C_BIGENDIAN(AC_SUBST([GS_WORDS_BIGENDIAN_DEFINE], ["#define __BIG_ENDIAN__ 1"]),
  AC_SUBST([GS_WORDS_BIGENDIAN_DEFINE], ["#define __LITTLE_ENDIAN__ 1"]),
  AC_MSG_ERROR([Could not determine CPU endianness.]))

# Gather information about the data model so we can define the data types
m4_define([GS_CHECK_DATA_MODEL],
[AC_CHECK_SIZEOF([char])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([void *])
AH_VERBATIM([DATA_MODEL_LP32], [/* Define LP32 data model. */
#define DATA_MODEL_LP32 122484])
AH_VERBATIM([DATA_MODEL_ILP32], [/* Define ILP32 data model. */
#define DATA_MODEL_ILP32 124484])
AH_VERBATIM([DATA_MODEL_LLP64], [/* Define LLP64 data model. */
#define DATA_MODEL_LLP64 124488])
AH_VERBATIM([DATA_MODEL_LP64], [/* Define LP64 data model. */
#define DATA_MODEL_LP64 124888])
AC_CACHE_CHECK([for data model],
               [gs_cv_c_data_model],
               [_gs_data_model="$ac_cv_sizeof_char$ac_cv_sizeof_short$ac_cv_sizeof_int$ac_cv_sizeof_long$ac_cv_sizeof_long_long$ac_cv_sizeof_void_p"
               AS_CASE([$_gs_data_model],
                       [122484], [gs_cv_c_data_model="LP32"],
                       [124484], [gs_cv_c_data_model="ILP32"],
                       [124488], [gs_cv_c_data_model="LLP64"],
                       [124888], [gs_cv_c_data_model="LP64"],
                       [gs_cv_c_data_model="unknown"])])
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


#--------------------------------------------------------------------
# Check for CFRunLoop support
# Android and QNX have pthread in libc instead of libpthread
#--------------------------------------------------------------------
AC_ARG_ENABLE(cfrunloop,
  [--disable-cfrunloop
    Disables CFRunLoop support and requiring pthread.
    Must be passed when building on Windows.],
  [],
  [AC_CHECK_HEADERS([unistd.h fcntl.h poll.h limits.h pthread.h],
    [AC_SEARCH_LIBS(pthread_join, [pthread pthreadG32 pthreadVC2],
      [AC_DEFINE(HAVE_CFRUNLOOP_SUPPORT, 1,
        [Define to 1 if the required libraries and headers for CFRunLoop are present.])],
      [AC_MSG_ERROR([Missing pthread library for CFRunLoop support.])])],
    [AC_MSG_ERROR([Missing headers for CFRunLoop support.])])])

#---
# Check for ICU
#---
AC_ARG_WITH([icu],
  AS_HELP_STRING([--without-icu],
    [Disable Internation Components for Unicode. WARNING: Many components will stop working without ICU support!]), [], [with_icu=yes])
AS_IF([test x"with_icu" != xno],
  [found_icu=no
  AS_CASE([$target_os],
    [mingw*|windows], [ # check for ICU bundled with Windows 10
      AC_CHECK_LIB(icu, udat_open,
        [AC_CHECK_HEADERS([icu.h], [found_icu=yes
          LIBS="$LIBS -licu"
          AC_MSG_NOTICE([Using ICU DLL from Windows 10 (requires version 1903)])])])])
  AS_IF([test $found_icu = no],
    [PKG_CHECK_MODULES([ICU], [icu-i18n > 49.0])
    LIBS="$LIBS $ICU_LIBS"; CFLAGS="$CFLAGS $ICU_CFLAGS"; INCLUDE_FLAGS="$INCLUDE_FLAGS $ICU_CFLAGS"
    AC_CHECK_HEADERS([unicode/ucal.h unicode/uchar.h unicode/ucnv.h unicode/ucol.h unicode/ucurr.h unicode/udat.h unicode/udatpg.h unicode/uloc.h unicode/ulocdata.h unicode/unorm2.h unicode/unum.h unicode/usearch.h unicode/uset.h unicode/ustring.h unicode/utrans.h], [],
      AC_MSG_ERROR([Could not find required ICU headers.]))])
  AC_DEFINE([HAVE_ICU], [1],
    [Define to 1 if you have International Components for Unicode.])])

#---
# Check for Grand Central Dispatch (libdispatch)
#---
AC_ARG_WITH([gcd],
  [AS_HELP_STRING([--without-gcd],
    [Explicitly compiles without Grand Central Dispatch support. Configure will attempt to find a compatible library, use this option if you do not wish to have the support.  The opposite option, --with-gcd, is also supported and should be used if Grand Central Dispatch support is absolutely required.  A compatible library will required (http://www.nickhutchinson.me/libdispatch).])],
  [], [with_gcd=maybe])
AS_IF([test "x$with_gcd" != xno],
  AC_CHECK_HEADERS([dispatch/dispatch.h],
    AC_CHECK_LIB([dispatch], [dispatch_get_main_queue_eventfd_np], ,
      AS_IF([test "x$with_gcd" = "xyes"],
        AC_MSG_ERROR([Compatible libdispatch not found for GCD support!]))
    ),
    AC_MSG_ERROR([Could not find the Grand Central Dispatch headers.])
  )
)

#---
# Check for the Objective-C runtime
#---
AC_ARG_WITH([objc],
  [AS_HELP_STRING([--without-objc],
    [Compiles without support for Objective-C. This option disables the Toll-Free Bridging mechanism.])],
  [], [with_objc=yes])
AS_IF([test "x$with_objc" != xno],
  AC_CHECK_HEADERS([objc/runtime.h],
    AC_SEARCH_LIBS(objc_getClass, [objc objc2], [],
      AC_MSG_ERROR([Objective-C library not found!  Use --without-objc-bridge to explicitly disable the toll-free bridge or install the Objective-C library.])),
    AC_MSG_ERROR([Could not find the Objective-C runtime header.])))

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

#---
# Check for libm
#---
AC_CHECK_LIB(m, modf)

#---
# Check for WinSock2
#---
AC_CHECK_HEADERS(ws2tcpip.h)

AC_CONFIG_FILES([Source/GNUmakefile Headers/CoreFoundation/CFBase.h])

AC_OUTPUT
