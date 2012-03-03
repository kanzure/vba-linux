#ifndef VBA_GB_SOUND_H
#define VBA_GB_SOUND_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NR10 0xff10
#define NR11 0xff11
#define NR12 0xff12
#define NR13 0xff13
#define NR14 0xff14
#define NR21 0xff16
#define NR22 0xff17
#define NR23 0xff18
#define NR24 0xff19
#define NR30 0xff1a
#define NR31 0xff1b
#define NR32 0xff1c
#define NR33 0xff1d
#define NR34 0xff1e
#define NR41 0xff20
#define NR42 0xff21
#define NR43 0xff22
#define NR44 0xff23
#define NR50 0xff24
#define NR51 0xff25
#define NR52 0xff26

#define SOUND_EVENT(address, value) \
    gbSoundEvent(address, value)

extern void gbSoundTick();
extern void gbSoundPause();
extern void gbSoundResume();
extern void gbSoundEnable(int);
extern void gbSoundDisable(int);
extern int gbSoundGetEnable();
extern void gbSoundReset();
extern void gbSoundSaveGame(gzFile);
extern void gbSoundReadGame(int, gzFile);
extern void gbSoundEvent(register u16, register int);
extern void gbSoundSetQuality(int);

typedef int32 soundtick_t;

extern soundtick_t soundTicks;
extern int32       soundQuality;
extern soundtick_t SOUND_CLOCK_TICKS;

#endif // VBA_GB_SOUND_H
