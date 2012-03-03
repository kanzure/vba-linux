AC_PREREQ([2.68])

AC_INIT([vba-rlm], [1.0])

dnl TODO: change this to gba.cpp or something
AC_CONFIG_SRCDIR([src/lua/lopcodes.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([limits.h locale.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([floor localeconv memchr modf pow setlocale sqrt strchr strcspn strerror strpbrk strrchr strstr strtoul])

AC_CONFIG_FILES([Makefile 
	         src/Makefile
                 src/lua/Makefile])

AM_INIT_AUTOMAKE([dist-bzip2])

AC_OUTPUT

