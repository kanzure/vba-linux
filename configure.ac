AC_PREREQ([2.68])

AC_INIT([vba-rlm], [1.0])

AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign dist-bzip2])

dnl TODO: change this to gba.cpp or something
AC_CONFIG_SRCDIR([src/lua/lopcodes.c])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB



# SDL stuff 
SDL_VERSION=1.2.2
AM_PATH_SDL($SDL_VERSION,[
],[
AC_MSG_ERROR([*** Couldn't find SDL library (version >= $SDL_VERSION).])
])



# Checks for header files.
AC_CHECK_HEADERS([limits.h locale.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MKTIME
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([floor localeconv memchr memset  dnl
                modf pow setlocale sqrt         dnl
                strchr strcspn strerror         dnl
                strpbrk strrchr strstr strtoul])

AC_CONFIG_FILES([Makefile 
	         src/Makefile
                 src/lua/Makefile
		 src/gb/Makefile
		 src/gba/Makefile
		 src/common/Makefile])



AC_OUTPUT

