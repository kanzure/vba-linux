noinst_LIBRARIES = libgbcom.a

libgbcom_a_SOURCES = \
	CheatSearch.h		\
	inputGlobal.h		\
	memgzio.h		\
	movie.h			\
	nesvideos-piece.h	\
	System.h		\
	Text.h			\
	unzip.h			\
	Util.h			\
	vbalua.h		\
				\
	CheatSearch.cpp		\
	lua-engine.cpp		\
	memgzio.c		\
	movie.cpp		\
	nesvideos-piece.cpp	\
	Text.cpp		\
	unzip.cpp		\
	Util.cpp		



AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
