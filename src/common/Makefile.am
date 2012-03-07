noinst_LTLIBRARIES = libgbcom.la

libgbcom_la_SOURCES = \
	CheatSearch.h		\
	inputGlobal.h		\
	memgzio.h		\
	movie.h			\
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
	Text.cpp		\
	unzip.cpp		\
	Util.cpp		



AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions 
