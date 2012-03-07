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
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_PATH_PROG(NASM, nasm)
AC_PROG_LIBTOOL


# Checks for libraries.
AC_CHECK_LIB(z, gzopen,
  , AC_MSG_ERROR([*** Cannot compile without zlib.]))
AC_CHECK_LIB(png, png_create_write_struct,
  , AC_MSG_ERROR([*** Cannot compile without libpng.]), [-lz])

SDL_VERSION=1.2.2
AM_PATH_SDL($SDL_VERSION, :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
AC_CHECK_LIB([SDL], [SDL_Init])

AC_CHECK_LIB([pthread], [pthread_create])



# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h unistd.h libintl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([floor ftruncate gethostbyname gethostname inet_ntoa localeconv memchr memset modf pow setlocale socket sqrt strcasecmp strchr strcspn strdup strerror strpbrk strrchr strstr strtoul])



# find JNI headers
AC_ARG_VAR([JAVA_INCLUDE_PATH], [The path to the jni.h])
AC_ARG_VAR([JAVA_INCLUDE_PATH2], [The path to jni_md.h])


if test "x$JAVA_INCLUDE_PATH" = "x"; then
   echo "----------------------------------"	
   echo "you must have JAVA_INCLUDE_PATH defined."
   echo "this is the path to jni.h"
   echo "----------------------------------"
fi

if test "x$JAVA_INCLUDE_PATH2" = "x"; then
   echo "----------------------------------"	
   echo "you must have JAVA_INCLUDE_PATH2 defined."
   echo "this is the path to jni_md.h"
   echo "----------------------------------"
fi

if test "x$JAVA_INCLUDE_PATH" = "x"; then
   AC_MSG_ERROR([JAVA_INCLUDE_PATH not defined])
fi

if test "x$JAVA_INCLUDE_PATH2" = "x"; then
   AC_MSG_ERROR([JAVA_INCLUDE_PATH2 not defined])	
fi

#AC_SUBST(JAVA_INCLUDE_PATH)
#AC_SUBST(JAVA_INCLUDE_PATH2)

AC_CONFIG_FILES([Makefile 
	         src/Makefile
                 src/lua/Makefile
		 src/clojure/Makefile
		 src/gb/Makefile
		 src/gba/Makefile
		 src/common/Makefile
		 src/SFMT/Makefile
		 src/filters/Makefile
		 src/prof/Makefile
		 src/sdl/Makefile])



### RLM: investigate this further
CXXFLAGS="$CXXFLAGS -DC_CORE"	


AC_OUTPUT

