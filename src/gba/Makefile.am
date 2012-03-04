noinst_LIBRARIES = libgba.a

libgba_a_SOURCES = \
	agbprint.h	\
	armdis.h	\
	arm-new.h	\
	bios.h		\
	EEprom.h	\
	elf.h		\
	Flash.h		\
	GBACheats.h	\
	GBAGfx.h	\
	GBAGlobals.h	\
	GBA.h		\
	GBAinline.h	\
	GBASound.h	\
	RTC.h		\
	Sram.h		\
	thumb.h		\
			\
	agbprint.cpp	\
	armdis.cpp	\
	bios.cpp	\
	EEprom.cpp	\
	elf.cpp		\
	Flash.cpp	\
	GBACheats.cpp	\
	GBA.cpp		\
	GBAGfx.cpp	\
	GBAGlobals.cpp	\
	GBASound.cpp	\
	Mode0.cpp	\
	Mode1.cpp	\
	Mode2.cpp	\
	Mode3.cpp	\
	Mode4.cpp	\
	Mode5.cpp	\
	remote.cpp	\
	RTC.cpp		\
	Sram.cpp	


AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@
