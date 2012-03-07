lib_LTLIBRARIES = libvba.la

libvba_la_SOURCES = \
	clojure.c

libvba_la_LIBADD =   	\
	../common/libgbcom.la		\
	../gb/libgb.la			\
	../gba/libgba.la		\
	../filters/lib386.la 		\
	../filters/libfilter.la		\
	../prof/libprof.la		\
	../sdl/libgui.la		\
	../lua/libgblua.la       

AM_CPPFLAGS = \
	-I$(top_srcdir)/java/headers -I$(JAVA_INCLUDE_PATH) \
	-I$(JAVA_INCLUDE_PATH2)
