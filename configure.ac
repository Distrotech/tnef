AC_PREREQ([2.53])

dnl initialization
AC_INIT([tnef], [2.x], [verdammelt@users.sourceforge.net])
AC_CONFIG_SRCDIR([ChangeLog])
AC_CONFIG_AUX_DIR([config])

AC_REVISION($Revision: 1.1.2.1 $)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

dnl check for programs
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

AC_DISABLE_SHARED
AM_PROG_LIBTOOL

dnl check for executable extentions
AC_EXEEXT

dnl check for libraries

dnl check for headers
AC_HEADER_STDC
AC_CHECK_HEADERS(memory.h)
AC_CHECK_HEADERS(string.h)

dnl check for typedefs
AC_TYPE_SIZE_T

dnl check for structures 

dnl check for compiler characteristics.
AC_C_CONST
AC_CHECK_SIZEOF(int)

dnl check for library functions
AC_REPLACE_FUNCS(strdup getopt_long basename ldiv)
AC_CHECK_HEADERS(libintl.h)
AC_CHECK_FUNCS(strrchr memmove memset)
AC_FUNC_VPRINTF
AC_FUNC_MALLOC
dnl removed because it automatically checked for lstat.
dnl AC_FUNC_STAT

dnl check for system services

dnl extra arguments:
AM_MAINTAINER_MODE
AC_ARG_ENABLE( warnings, 
               [ --enable-warnings	enable compile warnings],
               AC_COMPILE_WARNINGS)
AC_ARG_ENABLE( debug,
               [ --enable-debug  enable debug build],
               AC_DEBUG_COMPILE)

dnl help out LIBTOOLS until we get a fix 
LTLIBOBJS=`echo $LIB@&t@OBJS | sed -e 's/\.o/\.lo/g'`
AC_SUBST(LTLIBOBJS)

dnl output
AC_CONFIG_FILES([Makefile BUGS tnef.spec])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([doc/Makefile doc/tnef.1])
AC_CONFIG_FILES([c/Makefile c/app/Makefile c/lib/Makefile c/port/Makefile])
AC_CONFIG_FILES([lisp/Makefile])
AC_CONFIG_FILES([tests/Makefile])

AC_OUTPUT

