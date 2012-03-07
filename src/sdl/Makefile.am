noinst_LTLIBRARIES = libgui.la

#noinst_PROGRAMS = TestEmu

libgui_la_SOURCES = \
	Array.h		\
	debugger.h	\
	expr.cpp.h	\
	getopt.h	\
	RingBuffer.h	\
	SoundDriver.h	\
	SoundSDL.h	\
	Types.h		\
	exprNode.h	\
			\
	debugger.cpp	\
	expr.cpp	\
	expr-lex.cpp	\
	exprNode.cpp	\
	getopt1.c	\
	getopt.c	\
	SDL.cpp		\
	SoundSDL.cpp	


#TestEmu.cpp


# libvba_la_LIBADD =   	\
# 	../common/libgbcom.la		\
# 	../gb/libgb.la			\
# 	../gba/libgba.la		\
# 	../filters/lib386.la 		\
# 	../filters/libfilter.la		\
# 	../prof/libprof.la		\
# 	../clojure/libgbclojure.la	\
# 	../lua/libgblua.la       

# TestEmu_SOURCES = \
# 	TestEmu.cpp		\
# 	debugger.cpp		\
# 	debugger.h		\
# 	expr-lex.cpp		\
# 	expr.cpp		\
# 	expr.cpp.h		\
# 	exprNode.cpp		\
# 	exprNode.h		\
# 	../AutoBuild.h		\
# 	../NLS.h		\
# 	../Port.h

# TestEmu_LDADD = @VBA_LIBS@ @SDL_LIBS@

# TestEmu_DEPENDENCIES = @VBA_LIBS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
