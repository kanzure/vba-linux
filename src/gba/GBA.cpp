#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include "../Port.h"
#include "../NLS.h"
#include "GBA.h"
//#include "GBAGlobals.h"
#include "GBACheats.h" // FIXME: SDL requires this included before "GBAinline.h"
#include "GBAinline.h"
#include "GBAGfx.h"
#include "GBASound.h"
#include "EEprom.h"
#include "Flash.h"
#include "Sram.h"
#include "bios.h"
#include "elf.h"
#include "agbprint.h"
#include "../common/unzip.h"
#include "../common/Util.h"
#include "../common/movie.h"
#include "../common/vbalua.h"

#ifdef PROFILING
#include "../prof/prof.h"
#endif

#define UPDATE_REG(address, value) WRITE16LE(((u16 *)&ioMem[address]), value)

#ifdef __GNUC__
#define _stricmp strcasecmp
#endif

#define CPU_BREAK_LOOP \
    cpuSavedTicks	 = cpuSavedTicks - *extCpuLoopTicks; \
    *extCpuLoopTicks = *extClockTicks;

#define CPU_BREAK_LOOP_2 \
    cpuSavedTicks	 = cpuSavedTicks - *extCpuLoopTicks; \
    *extCpuLoopTicks = *extClockTicks; \
    *extTicks		 = *extClockTicks;

int32 cpuDmaTicksToUpdate = 0;
int32 cpuDmaCount		  = 0;
bool8 cpuDmaHack		  = 0;
u32	  cpuDmaLast		  = 0;
int32 dummyAddress		  = 0;

int32 *extCpuLoopTicks = NULL;
int32 *extClockTicks   = NULL;
int32 *extTicks		   = NULL;

#if (defined(WIN32) && !defined(SDL))
HANDLE mapROM;        // shared memory handles
HANDLE mapWORKRAM;
HANDLE mapBIOS;
HANDLE mapIRAM;
HANDLE mapPALETTERAM;
HANDLE mapVRAM;
HANDLE mapOAM;
HANDLE mapPIX;
HANDLE mapIOMEM;
#endif

int32 gbaSaveType = 0;      // used to remember the save type on reset
bool8 intState	  = false;
bool8 stopState	  = false;
bool8 holdState	  = false;
int32 holdType	  = 0;
bool8 cpuSramEnabled		 = true;
bool8 cpuFlashEnabled		 = true;
bool8 cpuEEPROMEnabled		 = true;
bool8 cpuEEPROMSensorEnabled = false;

#ifdef PROFILING
int profilingTicks		  = 0;
int profilingTicksReload  = 0;
static char *profilBuffer = NULL;
static int	 profilSize	  = 0;
static u32	 profilLowPC  = 0;
static int	 profilScale  = 0;
#endif
bool8 freezeWorkRAM[0x40000];
bool8 freezeInternalRAM[0x8000];
int32 lcdTicks = 960;
bool8 timer0On = false;
int32 timer0Ticks		= 0;
int32 timer0Reload		= 0;
int32 timer0ClockReload = 0;
bool8 timer1On			= false;
int32 timer1Ticks		= 0;
int32 timer1Reload		= 0;
int32 timer1ClockReload = 0;
bool8 timer2On			= false;
int32 timer2Ticks		= 0;
int32 timer2Reload		= 0;
int32 timer2ClockReload = 0;
bool8 timer3On			= false;
int32 timer3Ticks		= 0;
int32 timer3Reload		= 0;
int32 timer3ClockReload = 0;
u32	  dma0Source		= 0;
u32	  dma0Dest			= 0;
u32	  dma1Source		= 0;
u32	  dma1Dest			= 0;
u32	  dma2Source		= 0;
u32	  dma2Dest			= 0;
u32	  dma3Source		= 0;
u32	  dma3Dest			= 0;
void  (*cpuSaveGameFunc)(u32, u8) = flashSaveDecide;
void  (*renderLine)() = mode0RenderLine;
bool8 fxOn = false;
bool8 windowOn		 = false;
int32 frameSkipCount = 0;
u32	  gbaLastTime	 = 0;
int32 gbaFrameCount	 = 0;
bool8 prefetchActive = false, prefetchPrevActive = false, prefetchApplies = false;
char  buffer[1024];
FILE *out = NULL;

static bool newFrame = true;
static bool pauseAfterFrameAdvance = false;

const int32 TIMER_TICKS[4] = {
	1,
	64,
	256,
	1024
};

const int32 thumbCycles[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 3
	1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 4
	2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 5
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 6
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 7
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 8
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 9
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // a
	1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 4, 1, 1,  // b
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // c
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3,  // d
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // e
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   // f
};

const int32 gamepakRamWaitState[4] = { 4, 3, 2, 8 };
const int32 gamepakWaitState[8] =  { 4, 3, 2, 8, 4, 3, 2, 8 };
const int32 gamepakWaitState0[8] = { 2, 2, 2, 2, 1, 1, 1, 1 };
const int32 gamepakWaitState1[8] = { 4, 4, 4, 4, 1, 1, 1, 1 };
const int32 gamepakWaitState2[8] = { 8, 8, 8, 8, 1, 1, 1, 1 };

int32 memoryWait[16] =
{ 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
int32 memoryWait32[16] =
{ 0, 0, 9, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 0 };
int32 memoryWaitSeq[16] =
{ 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 };
int32 memoryWaitSeq32[16] =
{ 2, 0, 3, 0, 0, 2, 2, 0, 4, 4, 8, 8, 16, 16, 8, 0 };
int32 memoryWaitFetch[16] =
{ 3, 0, 3, 0, 0, 1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
int32 memoryWaitFetch32[16] =
{ 6, 0, 6, 0, 0, 2, 2, 0, 8, 8, 8, 8, 8, 8, 8, 0 };

const int32 cpuMemoryWait[16] = {
	0, 0, 2, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 0, 0
};
const int32 cpuMemoryWait32[16] = {
	0, 0, 3, 0, 0, 0, 0, 0,
	3, 3, 3, 3, 3, 3, 0, 0
};

const bool8 memory32[16] = {
	true, false, false, true, true, false, false, true, false, false, false, false, false, false, true, false
};

u8 biosProtected[4];

u8 cpuBitsSet[256];
u8 cpuLowestBitSet[256];

#ifdef WORDS_BIGENDIAN
bool8 cpuBiosSwapped = false;
#endif

u32 myROM[] = {
	0xEA000006,
	0xEA000093,
	0xEA000006,
	0x00000000,
	0x00000000,
	0x00000000,
	0xEA000088,
	0x00000000,
	0xE3A00302,
	0xE1A0F000,
	0xE92D5800,
	0xE55EC002,
	0xE28FB03C,
	0xE79BC10C,
	0xE14FB000,
	0xE92D0800,
	0xE20BB080,
	0xE38BB01F,
	0xE129F00B,
	0xE92D4004,
	0xE1A0E00F,
	0xE12FFF1C,
	0xE8BD4004,
	0xE3A0C0D3,
	0xE129F00C,
	0xE8BD0800,
	0xE169F00B,
	0xE8BD5800,
	0xE1B0F00E,
	0x0000009C,
	0x0000009C,
	0x0000009C,
	0x0000009C,
	0x000001F8,
	0x000001F0,
	0x000000AC,
	0x000000A0,
	0x000000FC,
	0x00000168,
	0xE12FFF1E,
	0xE1A03000,
	0xE1A00001,
	0xE1A01003,
	0xE2113102,
	0x42611000,
	0xE033C040,
	0x22600000,
	0xE1B02001,
	0xE15200A0,
	0x91A02082,
	0x3AFFFFFC,
	0xE1500002,
	0xE0A33003,
	0x20400002,
	0xE1320001,
	0x11A020A2,
	0x1AFFFFF9,
	0xE1A01000,
	0xE1A00003,
	0xE1B0C08C,
	0x22600000,
	0x42611000,
	0xE12FFF1E,
	0xE92D0010,
	0xE1A0C000,
	0xE3A01001,
	0xE1500001,
	0x81A000A0,
	0x81A01081,
	0x8AFFFFFB,
	0xE1A0000C,
	0xE1A04001,
	0xE3A03000,
	0xE1A02001,
	0xE15200A0,
	0x91A02082,
	0x3AFFFFFC,
	0xE1500002,
	0xE0A33003,
	0x20400002,
	0xE1320001,
	0x11A020A2,
	0x1AFFFFF9,
	0xE0811003,
	0xE1B010A1,
	0xE1510004,
	0x3AFFFFEE,
	0xE1A00004,
	0xE8BD0010,
	0xE12FFF1E,
	0xE0010090,
	0xE1A01741,
	0xE2611000,
	0xE3A030A9,
	0xE0030391,
	0xE1A03743,
	0xE2833E39,
	0xE0030391,
	0xE1A03743,
	0xE2833C09,
	0xE283301C,
	0xE0030391,
	0xE1A03743,
	0xE2833C0F,
	0xE28330B6,
	0xE0030391,
	0xE1A03743,
	0xE2833C16,
	0xE28330AA,
	0xE0030391,
	0xE1A03743,
	0xE2833A02,
	0xE2833081,
	0xE0030391,
	0xE1A03743,
	0xE2833C36,
	0xE2833051,
	0xE0030391,
	0xE1A03743,
	0xE2833CA2,
	0xE28330F9,
	0xE0000093,
	0xE1A00840,
	0xE12FFF1E,
	0xE3A00001,
	0xE3A01001,
	0xE92D4010,
	0xE3A0C301,
	0xE3A03000,
	0xE3A04001,
	0xE3500000,
	0x1B000004,
	0xE5CC3301,
	0xEB000002,
	0x0AFFFFFC,
	0xE8BD4010,
	0xE12FFF1E,
	0xE5CC3208,
	0xE15C20B8,
	0xE0110002,
	0x10200002,
	0x114C00B8,
	0xE5CC4208,
	0xE12FFF1E,
	0xE92D500F,
	0xE3A00301,
	0xE1A0E00F,
	0xE510F004,
	0xE8BD500F,
	0xE25EF004,
	0xE59FD044,
	0xE92D5000,
	0xE14FC000,
	0xE10FE000,
	0xE92D5000,
	0xE3A0C302,
	0xE5DCE09C,
	0xE35E00A5,
	0x1A000004,
	0x05DCE0B4,
	0x021EE080,
	0xE28FE004,
	0x159FF018,
	0x059FF018,
	0xE59FD018,
	0xE8BD5000,
	0xE169F00C,
	0xE8BD5000,
	0xE25EF004,
	0x03007FF0,
	0x09FE2000,
	0x09FFC000,
	0x03007FE0
};

variable_desc saveGameStruct[] = {
	{ &DISPCNT,			  sizeof(u16)				 },
	{ &DISPSTAT,		  sizeof(u16)				 },
	{ &VCOUNT,			  sizeof(u16)				 },
	{ &BG0CNT,			  sizeof(u16)				 },
	{ &BG1CNT,			  sizeof(u16)				 },
	{ &BG2CNT,			  sizeof(u16)				 },
	{ &BG3CNT,			  sizeof(u16)				 },
	{ &BG0HOFS,			  sizeof(u16)				 },
	{ &BG0VOFS,			  sizeof(u16)				 },
	{ &BG1HOFS,			  sizeof(u16)				 },
	{ &BG1VOFS,			  sizeof(u16)				 },
	{ &BG2HOFS,			  sizeof(u16)				 },
	{ &BG2VOFS,			  sizeof(u16)				 },
	{ &BG3HOFS,			  sizeof(u16)				 },
	{ &BG3VOFS,			  sizeof(u16)				 },
	{ &BG2PA,			  sizeof(u16)				 },
	{ &BG2PB,			  sizeof(u16)				 },
	{ &BG2PC,			  sizeof(u16)				 },
	{ &BG2PD,			  sizeof(u16)				 },
	{ &BG2X_L,			  sizeof(u16)				 },
	{ &BG2X_H,			  sizeof(u16)				 },
	{ &BG2Y_L,			  sizeof(u16)				 },
	{ &BG2Y_H,			  sizeof(u16)				 },
	{ &BG3PA,			  sizeof(u16)				 },
	{ &BG3PB,			  sizeof(u16)				 },
	{ &BG3PC,			  sizeof(u16)				 },
	{ &BG3PD,			  sizeof(u16)				 },
	{ &BG3X_L,			  sizeof(u16)				 },
	{ &BG3X_H,			  sizeof(u16)				 },
	{ &BG3Y_L,			  sizeof(u16)				 },
	{ &BG3Y_H,			  sizeof(u16)				 },
	{ &WIN0H,			  sizeof(u16)				 },
	{ &WIN1H,			  sizeof(u16)				 },
	{ &WIN0V,			  sizeof(u16)				 },
	{ &WIN1V,			  sizeof(u16)				 },
	{ &WININ,			  sizeof(u16)				 },
	{ &WINOUT,			  sizeof(u16)				 },
	{ &MOSAIC,			  sizeof(u16)				 },
	{ &BLDMOD,			  sizeof(u16)				 },
	{ &COLEV,			  sizeof(u16)				 },
	{ &COLY,			  sizeof(u16)				 },
	{ &DM0SAD_L,		  sizeof(u16)				 },
	{ &DM0SAD_H,		  sizeof(u16)				 },
	{ &DM0DAD_L,		  sizeof(u16)				 },
	{ &DM0DAD_H,		  sizeof(u16)				 },
	{ &DM0CNT_L,		  sizeof(u16)				 },
	{ &DM0CNT_H,		  sizeof(u16)				 },
	{ &DM1SAD_L,		  sizeof(u16)				 },
	{ &DM1SAD_H,		  sizeof(u16)				 },
	{ &DM1DAD_L,		  sizeof(u16)				 },
	{ &DM1DAD_H,		  sizeof(u16)				 },
	{ &DM1CNT_L,		  sizeof(u16)				 },
	{ &DM1CNT_H,		  sizeof(u16)				 },
	{ &DM2SAD_L,		  sizeof(u16)				 },
	{ &DM2SAD_H,		  sizeof(u16)				 },
	{ &DM2DAD_L,		  sizeof(u16)				 },
	{ &DM2DAD_H,		  sizeof(u16)				 },
	{ &DM2CNT_L,		  sizeof(u16)				 },
	{ &DM2CNT_H,		  sizeof(u16)				 },
	{ &DM3SAD_L,		  sizeof(u16)				 },
	{ &DM3SAD_H,		  sizeof(u16)				 },
	{ &DM3DAD_L,		  sizeof(u16)				 },
	{ &DM3DAD_H,		  sizeof(u16)				 },
	{ &DM3CNT_L,		  sizeof(u16)				 },
	{ &DM3CNT_H,		  sizeof(u16)				 },
	{ &TM0D,			  sizeof(u16)				 },
	{ &TM0CNT,			  sizeof(u16)				 },
	{ &TM1D,			  sizeof(u16)				 },
	{ &TM1CNT,			  sizeof(u16)				 },
	{ &TM2D,			  sizeof(u16)				 },
	{ &TM2CNT,			  sizeof(u16)				 },
	{ &TM3D,			  sizeof(u16)				 },
	{ &TM3CNT,			  sizeof(u16)				 },
	{ &P1,				  sizeof(u16)				 },
	{ &IE,				  sizeof(u16)				 },
	{ &IF,				  sizeof(u16)				 },
	{ &IME,				  sizeof(u16)				 },
	{ &holdState,		  sizeof(bool8)				 },
	{ &holdType,		  sizeof(int32)				 },
	{ &lcdTicks,		  sizeof(int32)				 },
	{ &timer0On,		  sizeof(bool8)				 },
	{ &timer0Ticks,		  sizeof(int32)				 },
	{ &timer0Reload,	  sizeof(int32)				 },
	{ &timer0ClockReload, sizeof(int32)				 },
	{ &timer1On,		  sizeof(bool8)				 },
	{ &timer1Ticks,		  sizeof(int32)				 },
	{ &timer1Reload,	  sizeof(int32)				 },
	{ &timer1ClockReload, sizeof(int32)				 },
	{ &timer2On,		  sizeof(bool8)				 },
	{ &timer2Ticks,		  sizeof(int32)				 },
	{ &timer2Reload,	  sizeof(int32)				 },
	{ &timer2ClockReload, sizeof(int32)				 },
	{ &timer3On,		  sizeof(bool8)				 },
	{ &timer3Ticks,		  sizeof(int32)				 },
	{ &timer3Reload,	  sizeof(int32)				 },
	{ &timer3ClockReload, sizeof(int32)				 },
	{ &dma0Source,		  sizeof(u32)				 },
	{ &dma0Dest,		  sizeof(u32)				 },
	{ &dma1Source,		  sizeof(u32)				 },
	{ &dma1Dest,		  sizeof(u32)				 },
	{ &dma2Source,		  sizeof(u32)				 },
	{ &dma2Dest,		  sizeof(u32)				 },
	{ &dma3Source,		  sizeof(u32)				 },
	{ &dma3Dest,		  sizeof(u32)				 },
	{ &fxOn,			  sizeof(bool8)				 },
	{ &windowOn,		  sizeof(bool8)				 },
	{ &N_FLAG,			  sizeof(bool8)				 },
	{ &C_FLAG,			  sizeof(bool8)				 },
	{ &Z_FLAG,			  sizeof(bool8)				 },
	{ &V_FLAG,			  sizeof(bool8)				 },
	{ &armState,		  sizeof(bool8)				 },
	{ &armIrqEnable,	  sizeof(bool8)				 },
	{ &armNextPC,		  sizeof(u32)				 },
	{ &armMode,			  sizeof(int32)				 },
	{ &saveType,		  sizeof(int32)				 },
	{ NULL,				  0							 }
};

//int cpuLoopTicks = 0;
int cpuSavedTicks = 0;

#ifdef PROFILING
void cpuProfil(char *buf, int size, u32 lowPC, int scale)
{
	profilBuffer = buf;
	profilSize	 = size;
	profilLowPC	 = lowPC;
	profilScale	 = scale;
}

void cpuEnableProfiling(int hz)
{
	if (hz == 0)
		hz = 100;
	profilingTicks = profilingTicksReload = 16777216 / hz;
	profSetHertz(hz);
}

#endif

inline int CPUUpdateTicksAccess32(u32 address)
{
	return memoryWait32[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccess16(u32 address)
{
	return memoryWait[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccessSeq32(u32 address)
{
	return memoryWaitSeq32[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccessSeq16(u32 address)
{
	return memoryWaitSeq[(address >> 24) & 15];
}

inline int CPUUpdateTicks()
{
	int cpuLoopTicks = lcdTicks;

	if (soundTicks < cpuLoopTicks)
		cpuLoopTicks = soundTicks;

	if (timer0On && !(TM0CNT & 4) && (timer0Ticks < cpuLoopTicks))
	{
		cpuLoopTicks = timer0Ticks;
	}
	if (timer1On && !(TM1CNT & 4) && (timer1Ticks < cpuLoopTicks))
	{
		cpuLoopTicks = timer1Ticks;
	}
	if (timer2On && !(TM2CNT & 4) && (timer2Ticks < cpuLoopTicks))
	{
		cpuLoopTicks = timer2Ticks;
	}
	if (timer3On && !(TM3CNT & 4) && (timer3Ticks < cpuLoopTicks))
	{
		cpuLoopTicks = timer3Ticks;
	}
#ifdef PROFILING
	if (profilingTicksReload != 0)
	{
		if (profilingTicks < cpuLoopTicks)
		{
			cpuLoopTicks = profilingTicks;
		}
	}
#endif
	cpuSavedTicks = cpuLoopTicks;
	return cpuLoopTicks;
}

void CPUUpdateWindow0()
{
	int x00 = WIN0H >> 8;
	int x01 = WIN0H & 255;

	if (x00 <= x01)
	{
		for (int i = 0; i < 240; i++)
		{
			gfxInWin0[i] = (i >= x00 && i < x01);
		}
	}
	else
	{
		for (int i = 0; i < 240; i++)
		{
			gfxInWin0[i] = (i >= x00 || i < x01);
		}
	}
}

void CPUUpdateWindow1()
{
	int x00 = WIN1H >> 8;
	int x01 = WIN1H & 255;

	if (x00 <= x01)
	{
		for (int i = 0; i < 240; i++)
		{
			gfxInWin1[i] = (i >= x00 && i < x01);
		}
	}
	else
	{
		for (int i = 0; i < 240; i++)
		{
			gfxInWin1[i] = (i >= x00 || i < x01);
		}
	}
}

#define CLEAR_ARRAY(a) \
	{ \
		u32 *array = (a); \
		for (int i = 0; i < 240; i++) { \
			*array++ = 0x80000000; \
		} \
	} \

void CPUUpdateRenderBuffers(bool force)
{
	if (!(layerEnable & 0x0100) || force)
	{
		CLEAR_ARRAY(line0);
	}
	if (!(layerEnable & 0x0200) || force)
	{
		CLEAR_ARRAY(line1);
	}
	if (!(layerEnable & 0x0400) || force)
	{
		CLEAR_ARRAY(line2);
	}
	if (!(layerEnable & 0x0800) || force)
	{
		CLEAR_ARRAY(line3);
	}
}

bool CPUWriteStateToStream(gzFile gzFile)
{
	utilWriteInt(gzFile, SAVE_GAME_VERSION);

	utilGzWrite(gzFile, &rom[0xa0], 16);

	utilWriteInt(gzFile, useBios);

	utilGzWrite(gzFile, &reg[0], sizeof(reg));

	utilWriteData(gzFile, saveGameStruct);

	// new to version 0.7.1
	utilWriteInt(gzFile, stopState);
	// new to version 0.8
	utilWriteInt(gzFile, intState);

	utilGzWrite(gzFile, internalRAM, 0x8000);
	utilGzWrite(gzFile, paletteRAM, 0x400);
	utilGzWrite(gzFile, workRAM, 0x40000);
	utilGzWrite(gzFile, vram, 0x20000);
	utilGzWrite(gzFile, oam, 0x400);
	utilGzWrite(gzFile, pix, 4 * 241 * 162);
	utilGzWrite(gzFile, ioMem, 0x400);

	eepromSaveGame(gzFile);
	flashSaveGame(gzFile);
	soundSaveGame(gzFile);

	cheatsSaveGame(gzFile);

	// version 1.5
	rtcSaveGame(gzFile);

	// SAVE_GAME_VERSION_9 (new to re-recording version which is based on 1.72)
	{
		extern int32 sensorX, sensorY; // from SDL.cpp
		utilGzWrite(gzFile, &sensorX, sizeof(sensorX));
		utilGzWrite(gzFile, &sensorY, sizeof(sensorY));
		bool8 movieActive = VBAMovieActive();
		utilGzWrite(gzFile, &movieActive, sizeof(movieActive));
		if (movieActive)
		{
			uint8 *movie_freeze_buf	 = NULL;
			uint32 movie_freeze_size = 0;

			VBAMovieFreeze(&movie_freeze_buf, &movie_freeze_size);
			if (movie_freeze_buf)
			{
				utilGzWrite(gzFile, &movie_freeze_size, sizeof(movie_freeze_size));
				utilGzWrite(gzFile, movie_freeze_buf, movie_freeze_size);
				delete [] movie_freeze_buf;
			}
			else
			{
				systemMessage(0, N_("Failed to save movie snapshot."));
				return false;
			}
		}
		utilGzWrite(gzFile, &GBASystemCounters.frameCount, sizeof(GBASystemCounters.frameCount));
	}

	// SAVE_GAME_VERSION_10
	{
		utilGzWrite(gzFile, memoryWait, 16 * sizeof(int32));
		utilGzWrite(gzFile, memoryWait32, 16 * sizeof(int32));
		utilGzWrite(gzFile, memoryWaitSeq, 16 * sizeof(int32));
		utilGzWrite(gzFile, memoryWaitSeq32, 16 * sizeof(int32));
		utilGzWrite(gzFile, memoryWaitFetch, 16 * sizeof(int32));
		utilGzWrite(gzFile, memoryWaitFetch32, 16 * sizeof(int32));
	}

	// SAVE_GAME_VERSION_11
	{
		utilGzWrite(gzFile, &prefetchActive, sizeof(bool8));
		utilGzWrite(gzFile, &prefetchPrevActive, sizeof(bool8));
		utilGzWrite(gzFile, &prefetchApplies, sizeof(bool8));
	}

	// SAVE_GAME_VERSION_12
	{
		utilGzWrite(gzFile, &memLagTempEnabled, sizeof(bool8)); // necessary
		utilGzWrite(gzFile, &speedHack, sizeof(bool8)); // just in case it's ever used...
	}

	// SAVE_GAME_VERSION_13
	{
		utilGzWrite(gzFile, &GBASystemCounters.lagCount, sizeof(GBASystemCounters.lagCount));
		utilGzWrite(gzFile, &GBASystemCounters.lagged, sizeof(GBASystemCounters.lagged));
		utilGzWrite(gzFile, &GBASystemCounters.laggedLast, sizeof(GBASystemCounters.laggedLast));
	}

	return true;
}

bool CPUWriteState(const char *file)
{
	gzFile gzFile = utilGzOpen(file, "wb");

	if (gzFile == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), file);
		return false;
	}

	bool res = CPUWriteStateToStream(gzFile);

	utilGzClose(gzFile);

	return res;
}

bool CPUWriteMemState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "w");

	if (gzFile == NULL)
	{
		return false;
	}

	bool res = CPUWriteStateToStream(gzFile);

	long pos = utilGzTell(gzFile) + 8;

	if (pos >= (available))
		res = false;

	utilGzClose(gzFile);

	return res;
}

static int	tempStateID	  = 0;
static int	tempFailCount = 0;
static bool backupSafe	  = true;

bool CPUReadStateFromStream(gzFile gzFile)
{
	bool8 ub;
	char  tempBackupName [128];
	if (backupSafe)
	{
		sprintf(tempBackupName, "gbatempsave%d.sav", tempStateID++);
		CPUWriteState(tempBackupName);
	}

	int version = utilReadInt(gzFile);

	if (version > SAVE_GAME_VERSION || version < SAVE_GAME_VERSION_1)
	{
		systemMessage(MSG_UNSUPPORTED_VBA_SGM,
		              N_("Unsupported VisualBoyAdvance save game version %d"),
		              version);
		goto failedLoad;
	}

	u8 romname[17];

	utilGzRead(gzFile, romname, 16);

	if (memcmp(&rom[0xa0], romname, 16) != 0)
	{
		romname[16] = 0;
		for (int i = 0; i < 16; i++)
			if (romname[i] < 32)
				romname[i] = 32;
		systemMessage(MSG_CANNOT_LOAD_SGM, N_("Cannot load save game for %s"), romname);
		goto failedLoad;
	}

	ub = utilReadInt(gzFile) ? true : false;

	if (ub != useBios)
	{
		if (useBios)
			systemMessage(MSG_SAVE_GAME_NOT_USING_BIOS,
			              N_("Save game is not using the BIOS files"));
		else
			systemMessage(MSG_SAVE_GAME_USING_BIOS,
			              N_("Save game is using the BIOS file"));
		goto failedLoad;
	}

	utilGzRead(gzFile, &reg[0], sizeof(reg));

	utilReadData(gzFile, saveGameStruct);

	if (version < SAVE_GAME_VERSION_3)
		stopState = false;
	else
		stopState = utilReadInt(gzFile) ? true : false;

	if (version < SAVE_GAME_VERSION_4)
		intState = false;
	else
		intState = utilReadInt(gzFile) ? true : false;

	utilGzRead(gzFile, internalRAM, 0x8000);
	utilGzRead(gzFile, paletteRAM, 0x400);
	utilGzRead(gzFile, workRAM, 0x40000);
	utilGzRead(gzFile, vram, 0x20000);
	utilGzRead(gzFile, oam, 0x400);
	if (version < SAVE_GAME_VERSION_6)
		utilGzRead(gzFile, pix, 4 * 240 * 160);
	else
		utilGzRead(gzFile, pix, 4 * 241 * 162);
	utilGzRead(gzFile, ioMem, 0x400);

	eepromReadGame(gzFile, version);
	flashReadGame(gzFile, version);
	soundReadGame(gzFile, version);

	if (version > SAVE_GAME_VERSION_1)
	{
		cheatsReadGame(gzFile);
	}
	if (version > SAVE_GAME_VERSION_6)
	{
		rtcReadGame(gzFile);
	}

	if (version <= SAVE_GAME_VERSION_7)
	{
		u32 temp;
#define SWAP(a, b, c) \
	temp = (a); \
	(a)	 = (b) << 16 | (c); \
	(b)	 = (temp) >> 16; \
	(c)	 = (temp) & 0xFFFF;

		SWAP(dma0Source, DM0SAD_H, DM0SAD_L);
		SWAP(dma0Dest,   DM0DAD_H, DM0DAD_L);
		SWAP(dma1Source, DM1SAD_H, DM1SAD_L);
		SWAP(dma1Dest,   DM1DAD_H, DM1DAD_L);
		SWAP(dma2Source, DM2SAD_H, DM2SAD_L);
		SWAP(dma2Dest,   DM2DAD_H, DM2DAD_L);
		SWAP(dma3Source, DM3SAD_H, DM3SAD_L);
		SWAP(dma3Dest,   DM3DAD_H, DM3DAD_L);
	}

	// set pointers!
	layerEnable = layerSettings & DISPCNT;

	CPUUpdateRender();
	CPUUpdateRenderBuffers(true);
	CPUUpdateWindow0();
	CPUUpdateWindow1();
	gbaSaveType = 0;
	switch (saveType)
	{
	case 0:
		cpuSaveGameFunc = flashSaveDecide;
		break;
	case 1:
		cpuSaveGameFunc = sramWrite;
		gbaSaveType		= 1;
		break;
	case 2:
		cpuSaveGameFunc = flashWrite;
		gbaSaveType		= 2;
		break;
	default:
		systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
		              N_("Unsupported save type %d"), saveType);
		break;
	}
	if (eepromInUse)
		gbaSaveType = 3;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	if (version >= SAVE_GAME_VERSION_9) // new to re-recording version:
	{
		extern int32 sensorX, sensorY; // from SDL.cpp
		utilGzRead(gzFile, &sensorX, sizeof(sensorX));
		utilGzRead(gzFile, &sensorY, sizeof(sensorY));

		bool8 movieSnapshot;
		utilGzRead(gzFile, &movieSnapshot, sizeof(movieSnapshot));
		if (VBAMovieActive() && !movieSnapshot)
		{
			systemMessage(0, N_("Can't load a non-movie snapshot while a movie is active."));
			goto failedLoad;
		}

		if (movieSnapshot) // even if a movie isn't active we still want to parse through this in case other stuff is added
		                   // later on in the save format
		{
			uint32 movieInputDataSize = 0;
			utilGzRead(gzFile, &movieInputDataSize, sizeof(movieInputDataSize));
			uint8 *local_movie_data = new uint8[movieInputDataSize];
			int	   readBytes		= utilGzRead(gzFile, local_movie_data, movieInputDataSize);
			if (readBytes != movieInputDataSize)
			{
				systemMessage(0, N_("Corrupt movie snapshot."));
				if (local_movie_data)
					delete [] local_movie_data;
				goto failedLoad;
			}
			int code = VBAMovieUnfreeze(local_movie_data, movieInputDataSize);
			if (local_movie_data)
				delete [] local_movie_data;
			if (code != MOVIE_SUCCESS && VBAMovieActive())
			{
				char errStr [1024];
				strcpy(errStr, "Failed to load movie snapshot");
				switch (code)
				{
				case MOVIE_NOT_FROM_THIS_MOVIE:
					strcat(errStr, ";\nSnapshot not from this movie"); break;
				case MOVIE_NOT_FROM_A_MOVIE:
					strcat(errStr, ";\nNot a movie snapshot"); break;                   // shouldn't get here...
				case MOVIE_SNAPSHOT_INCONSISTENT:
					strcat(errStr, ";\nSnapshot inconsistent with movie"); break;
				case MOVIE_WRONG_FORMAT:
					strcat(errStr, ";\nWrong format"); break;
				}
				strcat(errStr, ".");
				systemMessage(0, N_(errStr));
				goto failedLoad;
			}
		}
		utilGzRead(gzFile, &GBASystemCounters.frameCount, sizeof(GBASystemCounters.frameCount));
	}
	if (version >= SAVE_GAME_VERSION_10)
	{
		utilGzRead(gzFile, memoryWait, 16 * sizeof(int32));
		utilGzRead(gzFile, memoryWait32, 16 * sizeof(int32));
		utilGzRead(gzFile, memoryWaitSeq, 16 * sizeof(int32));
		utilGzRead(gzFile, memoryWaitSeq32, 16 * sizeof(int32));
		utilGzRead(gzFile, memoryWaitFetch, 16 * sizeof(int32));
		utilGzRead(gzFile, memoryWaitFetch32, 16 * sizeof(int32));
	}
	if (version >= SAVE_GAME_VERSION_11)
	{
		utilGzRead(gzFile, &prefetchActive, sizeof(bool8));
		//if(prefetchActive && !prefetchPrevActive) systemScreenMessage("pre-fetch enabled",3,600);
		//if(!prefetchActive && prefetchPrevActive) systemScreenMessage("pre-fetch disabled",3,600);
		utilGzRead(gzFile, &prefetchPrevActive, sizeof(bool8));
		utilGzRead(gzFile, &prefetchApplies, sizeof(bool8));
	}
	if (version >= SAVE_GAME_VERSION_12)
	{
		utilGzRead(gzFile, &memLagTempEnabled, sizeof(bool8)); // necessary
		utilGzRead(gzFile, &speedHack, sizeof(bool8)); // just in case it's ever used...
	}
	if (version >= SAVE_GAME_VERSION_13)
	{
		utilGzRead(gzFile, &GBASystemCounters.lagCount, sizeof(GBASystemCounters.lagCount));
		utilGzRead(gzFile, &GBASystemCounters.lagged, sizeof(GBASystemCounters.lagged));
		utilGzRead(gzFile, &GBASystemCounters.laggedLast, sizeof(GBASystemCounters.laggedLast));
	}

	if (backupSafe)
	{
		remove(tempBackupName);
		tempFailCount = 0;
	}
	systemSetJoypad(0, ~P1 & 0x3FF);
	VBAUpdateButtonPressDisplay();
	VBAUpdateFrameCountDisplay();
	systemRefreshScreen();
	return true;

failedLoad:
	if (backupSafe)
	{
		tempFailCount++;
		if (tempFailCount < 3) // fail no more than 2 times in a row
			CPUReadState(tempBackupName);
		remove(tempBackupName);
	}
	return false;
}

bool CPUReadMemState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "r");

	backupSafe = false;
	bool res = CPUReadStateFromStream(gzFile);
	backupSafe = true;

	utilGzClose(gzFile);

	return res;
}

bool CPUReadState(const char *file)
{
	gzFile gzFile = utilGzOpen(file, "rb");

	if (gzFile == NULL)
		return false;

	bool res = CPUReadStateFromStream(gzFile);

	utilGzClose(gzFile);

	return res;
}

bool CPUExportEepromFile(const char *fileName)
{
	if (eepromInUse)
	{
		FILE *file = fopen(fileName, "wb");

		if (!file)
		{
			systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
			              fileName);
			return false;
		}

		for (int i = 0; i < eepromSize; )
		{
			for (int j = 0; j < 8; j++)
			{
				if (fwrite(&eepromData[i + 7 - j], 1, 1, file) != 1)
				{
					fclose(file);
					return false;
				}
			}
			i += 8;
		}
		fclose(file);
	}
	return true;
}

bool CPUWriteBatteryToStream(gzFile gzFile)
{
	if (!gzFile)
		return false;

	utilWriteInt(gzFile, SAVE_GAME_VERSION);

	// for simplicity, we put both types of battery files should be in the stream, even if one's empty
	eepromSaveGame(gzFile);
	flashSaveGame(gzFile);

	return true;
}

bool CPUWriteBatteryFile(const char *fileName)
{
	if (gbaSaveType == 0)
	{
		if (eepromInUse)
			gbaSaveType = 3;
		else
			switch (saveType)
			{
			case 1:
				gbaSaveType = 1;
				break;
			case 2:
				gbaSaveType = 2;
				break;
			}
	}

	if (gbaSaveType)
	{
		FILE *file = fopen(fileName, "wb");

		if (!file)
		{
			systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
			              fileName);
			return false;
		}

		// only save if Flash/Sram in use or EEprom in use
		if (gbaSaveType != 3)
		{
			if (gbaSaveType == 2)
			{
				if (fwrite(flashSaveMemory, 1, flashSize, file) != (size_t)flashSize)
				{
					fclose(file);
					return false;
				}
			}
			else
			{
				if (fwrite(flashSaveMemory, 1, 0x10000, file) != 0x10000)
				{
					fclose(file);
					return false;
				}
			}
		}
		else
		{
			if (fwrite(eepromData, 1, eepromSize, file) != (size_t)eepromSize)
			{
				fclose(file);
				return false;
			}
		}
		fclose(file);
	}

	return true;
}

bool CPUReadGSASnapshot(const char *fileName)
{
	int	  i;
	FILE *file = fopen(fileName, "rb");

	if (!file)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
		return false;
	}

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	// long size = ftell(file);
	fseek(file, 0x0, SEEK_SET);
	fread(&i, 1, 4, file);
	fseek(file, i, SEEK_CUR); // Skip SharkPortSave
	fseek(file, 4, SEEK_CUR); // skip some sort of flag
	fread(&i, 1, 4, file); // name length
	fseek(file, i, SEEK_CUR); // skip name
	fread(&i, 1, 4, file); // desc length
	fseek(file, i, SEEK_CUR); // skip desc
	fread(&i, 1, 4, file); // notes length
	fseek(file, i, SEEK_CUR); // skip notes
	int saveSize;
	fread(&saveSize, 1, 4, file); // read length
	saveSize -= 0x1c; // remove header size
	char buffer[17];
	char buffer2[17];
	fread(buffer, 1, 16, file);
	buffer[16] = 0;
	for (i = 0; i < 16; i++)
		if (buffer[i] < 32)
			buffer[i] = 32;
	memcpy(buffer2, &rom[0xa0], 16);
	buffer2[16] = 0;
	for (i = 0; i < 16; i++)
		if (buffer2[i] < 32)
			buffer2[i] = 32;
	if (memcmp(buffer, buffer2, 16))
	{
		systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
		              N_("Cannot import snapshot for %s. Current game is %s"),
		              buffer,
		              buffer2);
		fclose(file);
		return false;
	}
	fseek(file, 12, SEEK_CUR); // skip some flags
	if (saveSize >= 65536)
	{
		if (fread(flashSaveMemory, 1, saveSize, file) != (size_t)saveSize)
		{
			fclose(file);
			return false;
		}
	}
	else
	{
		systemMessage(MSG_UNSUPPORTED_SNAPSHOT_FILE,
		              N_("Unsupported snapshot file %s"),
		              fileName);
		fclose(file);
		return false;
	}
	fclose(file);
	CPUReset();
	return true;
}

bool CPUWriteGSASnapshot(const char *fileName,
                         const char *title,
                         const char *desc,
                         const char *notes)
{
	FILE *file = fopen(fileName, "wb");

	if (!file)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
		return false;
	}

	u8 buffer[17];

	utilPutDword(buffer, 0x0d); // SharkPortSave length
	fwrite(buffer, 1, 4, file);
	fwrite("SharkPortSave", 1, 0x0d, file);
	utilPutDword(buffer, 0x000f0000);
	fwrite(buffer, 1, 4, file); // save type 0x000f0000 = GBA save
	utilPutDword(buffer, strlen(title));
	fwrite(buffer, 1, 4, file); // title length
	fwrite(title, 1, strlen(title), file);
	utilPutDword(buffer, strlen(desc));
	fwrite(buffer, 1, 4, file); // desc length
	fwrite(desc, 1, strlen(desc), file);
	utilPutDword(buffer, strlen(notes));
	fwrite(buffer, 1, 4, file); // notes length
	fwrite(notes, 1, strlen(notes), file);
	int saveSize = 0x10000;
	if (gbaSaveType == 2)
		saveSize = flashSize;
	int totalSize = saveSize + 0x1c;

	utilPutDword(buffer, totalSize); // length of remainder of save - CRC
	fwrite(buffer, 1, 4, file);

	char temp[0x2001c];
	memset(temp, 0, 28);
	memcpy(temp, &rom[0xa0], 16); // copy internal name
	temp[0x10] = rom[0xbe]; // reserved area (old checksum)
	temp[0x11] = rom[0xbf]; // reserved area (old checksum)
	temp[0x12] = rom[0xbd]; // complement check
	temp[0x13] = rom[0xb0]; // maker
	temp[0x14] = 1; // 1 save ?
	memcpy(&temp[0x1c], flashSaveMemory, saveSize); // copy save
	fwrite(temp, 1, totalSize, file); // write save + header
	u32 crc = 0;

	for (int i = 0; i < totalSize; i++)
	{
		crc += ((u32)temp[i] << (crc % 0x18));
	}

	utilPutDword(buffer, crc);
	fwrite(buffer, 1, 4, file); // CRC?

	fclose(file);
	return true;
}

bool CPUImportEepromFile(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");

	if (!file)
		return false;

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (size == 512 || size == 0x2000)
	{
		if (fread(eepromData, 1, size, file) != (size_t)size)
		{
			fclose(file);
			return false;
		}
		for (int i = 0; i < size; )
		{
			u8 tmp = eepromData[i];
			eepromData[i]	  = eepromData[7 - i];
			eepromData[7 - i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i]	  = eepromData[7 - i];
			eepromData[7 - i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i]	  = eepromData[7 - i];
			eepromData[7 - i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i]	  = eepromData[7 - i];
			eepromData[7 - i] = tmp;
			i++;
			i += 4;
		}
	}
	else
	{
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}

bool CPUReadBatteryFromStream(gzFile gzFile)
{
	if (!gzFile)
		return false;

	int version = utilReadInt(gzFile);

	// for simplicity, we put both types of battery files should be in the stream, even if one's empty
	eepromReadGame(gzFile, version);
	flashReadGame(gzFile, version);

	return true;
}

bool CPUReadBatteryFile(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");

	if (!file)
		return false;

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	if (size == 512 || size == 0x2000)
	{
		if (fread(eepromData, 1, size, file) != (size_t)size)
		{
			fclose(file);
			return false;
		}
	}
	else
	{
		if (size == 0x20000)
		{
			if (fread(flashSaveMemory, 1, 0x20000, file) != 0x20000)
			{
				fclose(file);
				return false;
			}
			flashSetSize(0x20000);
		}
		else
		{
			if (fread(flashSaveMemory, 1, 0x10000, file) != 0x10000)
			{
				fclose(file);
				return false;
			}
			flashSetSize(0x10000);
		}
	}
	fclose(file);
	return true;
}

bool CPUWritePNGFile(const char *fileName)
{
	return utilWritePNGFile(fileName, 240, 160, pix);
}

bool CPUWriteBMPFile(const char *fileName)
{
	return utilWriteBMPFile(fileName, 240, 160, pix);
}

void CPUCleanUp()
{
	newFrame	  = true;

	GBASystemCounters.frameCount = 0;
	GBASystemCounters.lagCount	 = 0;
	GBASystemCounters.extraCount = 0;
	GBASystemCounters.lagged	 = true;
	GBASystemCounters.laggedLast = true;

#ifdef PROFILING
	if (profilingTicksReload)
	{
		profCleanup();
	}
#endif

#if (defined(WIN32) && !defined(SDL))
	#define FreeMappedMem(name, mapName, offset) \
	if (name != NULL) { \
		UnmapViewOfFile((name) - (offset)); \
		name = NULL; \
		CloseHandle(mapName); \
	}
#else
	#define FreeMappedMem(name, mapName, offset) \
	if (name != NULL) { \
		free(name); \
		name = NULL; \
	}
#endif

	FreeMappedMem(rom, mapROM, 0);
	FreeMappedMem(vram, mapVRAM, 0);
	FreeMappedMem(paletteRAM, mapPALETTERAM, 0);
	FreeMappedMem(internalRAM, mapIRAM, 0);
	FreeMappedMem(workRAM, mapWORKRAM, 0);
	FreeMappedMem(bios, mapBIOS, 0);
	FreeMappedMem(pix, mapPIX, 4);
	FreeMappedMem(oam, mapOAM, 0);
	FreeMappedMem(ioMem, mapIOMEM, 0);

	eepromErase();
	flashErase();

	elfCleanUp();

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	systemClearJoypads();
	systemResetSensor();

//	gbaLastTime = gbaFrameCount = 0;
	systemRefreshScreen();
}

int CPULoadRom(const char *szFile)
{
	int size = 0x2000000;

	if (rom != NULL)
	{
		CPUCleanUp();
	}

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	// size+4 is so RAM search and watch are safe to read any byte in the allocated region as a 4-byte int
#if (defined(WIN32) && !defined(SDL))
	#define AllocMappedMem(name, mapName, nameStr, size, useCalloc, offset) \
	mapName = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (size) + (offset) + (4), nameStr); \
	if ((mapName) && GetLastError() == ERROR_ALREADY_EXISTS) { \
		CloseHandle(mapName); \
		mapName = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (size) + (offset) + (4), NULL); \
	} \
	name = (u8 *)MapViewOfFile(mapName, FILE_MAP_WRITE, 0, 0, 0) + (offset); \
	if ((name) == NULL) { \
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"), nameStr); \
		CPUCleanUp(); \
		return 0; \
	} \
	memset(name, 0, size + 4);
#else
	#define AllocMappedMem(name, mapName, nameStr, size, useCalloc, offset) \
	name = (u8 *)(useCalloc ? calloc(1, size + 4) : malloc(size + 4)); \
	if ((name) == NULL) { \
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"), nameStr); \
		CPUCleanUp(); \
		return 0; \
	} \
	memset(name, 0, size + 4);
#endif

	AllocMappedMem(rom, mapROM, "vbaROM", 0x2000000, false, 0);
	AllocMappedMem(workRAM, mapWORKRAM, "vbaWORKRAM", 0x40000, true, 0);

	u8 *whereToLoad = rom;
	if (cpuIsMultiBoot)
		whereToLoad = workRAM;

	if (utilIsELF(szFile))
	{
		FILE *f = fopen(szFile, "rb");
		if (!f)
		{
			systemMessage(MSG_ERROR_OPENING_IMAGE, N_("Error opening image %s"),
			              szFile);
			FreeMappedMem(rom, mapROM, 0);
			FreeMappedMem(workRAM, mapWORKRAM, 0);
			return 0;
		}
		bool res = elfRead(szFile, size, f);
		if (!res || size == 0)
		{
			FreeMappedMem(rom, mapROM, 0);
			FreeMappedMem(workRAM, mapWORKRAM, 0);
			elfCleanUp();
			return 0;
		}
	}
	else if (!utilLoad(szFile,
	                   utilIsGBAImage,
	                   whereToLoad,
	                   size))
	{
		FreeMappedMem(rom, mapROM, 0);
		FreeMappedMem(workRAM, mapWORKRAM, 0);
		return 0;
	}

	u16 *temp = (u16 *)(rom + ((size + 1) & ~1));
	int	 i;
	for (i = (size + 1) & ~1; i < 0x2000000; i += 2)
	{
		WRITE16LE(temp, (i >> 1) & 0xFFFF);
		temp++;
	}

	AllocMappedMem(bios, mapBIOS, "vbaBIOS", 0x4000, true, 0);
	AllocMappedMem(internalRAM, mapIRAM, "vbaIRAM", 0x8000, true, 0);
	AllocMappedMem(paletteRAM, mapPALETTERAM, "vbaPALETTERAM", 0x400, true, 0);
	AllocMappedMem(vram, mapVRAM, "vbaVRAM", 0x20000, true, 0);
	AllocMappedMem(oam, mapOAM, "vbaOAM", 0x400, true, 0);

	// HACK: +4 at start to accomodate the 2xSaI filter reading out of bounds of the leftmost pixel
	AllocMappedMem(pix, mapPIX, "vbaPIX", 4 * 241 * 162, true, 4);
	AllocMappedMem(ioMem, mapIOMEM, "vbaIOMEM", 0x400, true, 0);

	CPUUpdateRenderBuffers(true);

	return size;
}

void CPUUpdateRender()
{
	switch (DISPCNT & 7)
	{
	case 0:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode0RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode0RenderLineNoWindow;
		else
			renderLine = mode0RenderLineAll;
		break;
	case 1:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode1RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode1RenderLineNoWindow;
		else
			renderLine = mode1RenderLineAll;
		break;
	case 2:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode2RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode2RenderLineNoWindow;
		else
			renderLine = mode2RenderLineAll;
		break;
	case 3:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode3RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode3RenderLineNoWindow;
		else
			renderLine = mode3RenderLineAll;
		break;
	case 4:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode4RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode4RenderLineNoWindow;
		else
			renderLine = mode4RenderLineAll;
		break;
	case 5:
		if ((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
		    cpuDisableSfx)
			renderLine = mode5RenderLine;
		else if (fxOn && !windowOn && !(layerEnable & 0x8000))
			renderLine = mode5RenderLineNoWindow;
		else
			renderLine = mode5RenderLineAll;
	default:
		break;
	}
}

void CPUUpdateCPSR()
{
	u32 CPSR = reg[16].I & 0x40;
	if (N_FLAG)
		CPSR |= 0x80000000;
	if (Z_FLAG)
		CPSR |= 0x40000000;
	if (C_FLAG)
		CPSR |= 0x20000000;
	if (V_FLAG)
		CPSR |= 0x10000000;
	if (!armState)
		CPSR |= 0x00000020;
	if (!armIrqEnable)
		CPSR |= 0x80;
	CPSR	 |= (armMode & 0x1F);
	reg[16].I = CPSR;
}

void CPUUpdateFlags(bool breakLoop)
{
	u32 CPSR = reg[16].I;

	N_FLAG		 = (CPSR & 0x80000000) ? true : false;
	Z_FLAG		 = (CPSR & 0x40000000) ? true : false;
	C_FLAG		 = (CPSR & 0x20000000) ? true : false;
	V_FLAG		 = (CPSR & 0x10000000) ? true : false;
	armState	 = (CPSR & 0x20) ? false : true;
	armIrqEnable = (CPSR & 0x80) ? false : true;
	if (breakLoop)
	{
		if (armIrqEnable && (IF & IE) && (IME & 1))
		{
			CPU_BREAK_LOOP_2;
		}
	}
}

void CPUUpdateFlags()
{
	CPUUpdateFlags(true);
}

#ifdef WORDS_BIGENDIAN
static void CPUSwap(volatile u32 *a, volatile u32 *b)
{
	volatile u32 c = *b;
	*b = *a;
	*a = c;
}

#else
static void CPUSwap(u32 *a, u32 *b)
{
	u32 c = *b;
	*b = *a;
	*a = c;
}

#endif

void CPUSwitchMode(int mode, bool saveState, bool breakLoop)
{
	//  if(armMode == mode)
	//    return;

	CPUUpdateCPSR();

	switch (armMode)
	{
	case 0x10:
	case 0x1F:
		reg[R13_USR].I = reg[13].I;
		reg[R14_USR].I = reg[14].I;
		reg[17].I	   = reg[16].I;
		break;
	case 0x11:
		CPUSwap(&reg[R8_FIQ].I, &reg[8].I);
		CPUSwap(&reg[R9_FIQ].I, &reg[9].I);
		CPUSwap(&reg[R10_FIQ].I, &reg[10].I);
		CPUSwap(&reg[R11_FIQ].I, &reg[11].I);
		CPUSwap(&reg[R12_FIQ].I, &reg[12].I);
		reg[R13_FIQ].I	= reg[13].I;
		reg[R14_FIQ].I	= reg[14].I;
		reg[SPSR_FIQ].I = reg[17].I;
		break;
	case 0x12:
		reg[R13_IRQ].I	= reg[13].I;
		reg[R14_IRQ].I	= reg[14].I;
		reg[SPSR_IRQ].I =  reg[17].I;
		break;
	case 0x13:
		reg[R13_SVC].I	= reg[13].I;
		reg[R14_SVC].I	= reg[14].I;
		reg[SPSR_SVC].I =  reg[17].I;
		break;
	case 0x17:
		reg[R13_ABT].I	= reg[13].I;
		reg[R14_ABT].I	= reg[14].I;
		reg[SPSR_ABT].I =  reg[17].I;
		break;
	case 0x1b:
		reg[R13_UND].I	= reg[13].I;
		reg[R14_UND].I	= reg[14].I;
		reg[SPSR_UND].I =  reg[17].I;
		break;
	}

	u32 CPSR = reg[16].I;
	u32 SPSR = reg[17].I;

	switch (mode)
	{
	case 0x10:
	case 0x1F:
		reg[13].I = reg[R13_USR].I;
		reg[14].I = reg[R14_USR].I;
		reg[16].I = SPSR;
		break;
	case 0x11:
		CPUSwap(&reg[8].I, &reg[R8_FIQ].I);
		CPUSwap(&reg[9].I, &reg[R9_FIQ].I);
		CPUSwap(&reg[10].I, &reg[R10_FIQ].I);
		CPUSwap(&reg[11].I, &reg[R11_FIQ].I);
		CPUSwap(&reg[12].I, &reg[R12_FIQ].I);
		reg[13].I = reg[R13_FIQ].I;
		reg[14].I = reg[R14_FIQ].I;
		if (saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_FIQ].I;
		break;
	case 0x12:
		reg[13].I = reg[R13_IRQ].I;
		reg[14].I = reg[R14_IRQ].I;
		reg[16].I = SPSR;
		if (saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_IRQ].I;
		break;
	case 0x13:
		reg[13].I = reg[R13_SVC].I;
		reg[14].I = reg[R14_SVC].I;
		reg[16].I = SPSR;
		if (saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_SVC].I;
		break;
	case 0x17:
		reg[13].I = reg[R13_ABT].I;
		reg[14].I = reg[R14_ABT].I;
		reg[16].I = SPSR;
		if (saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_ABT].I;
		break;
	case 0x1b:
		reg[13].I = reg[R13_UND].I;
		reg[14].I = reg[R14_UND].I;
		reg[16].I = SPSR;
		if (saveState)
			reg[17].I = CPSR;
		else
			reg[17].I = reg[SPSR_UND].I;
		break;
	default:
		systemMessage(MSG_UNSUPPORTED_ARM_MODE, N_("Unsupported ARM mode %02x"), mode);
		break;
	}
	armMode = mode;
	CPUUpdateFlags(breakLoop);
	CPUUpdateCPSR();
}

void CPUSwitchMode(int mode, bool saveState)
{
	CPUSwitchMode(mode, saveState, true);
}

void CPUUndefinedException()
{
	u32	 PC = reg[15].I;
	bool savedArmState = armState;
	CPUSwitchMode(0x1b, true, false);
	reg[14].I	 = PC - (savedArmState ? 4 : 2);
	reg[15].I	 = 0x04;
	armState	 = true;
	armIrqEnable = false;
	armNextPC	 = 0x04;
	reg[15].I	+= 4;
}

void CPUSoftwareInterrupt()
{
	u32	 PC = reg[15].I;
	bool savedArmState = armState;
	CPUSwitchMode(0x13, true, false);
	reg[14].I	 = PC - (savedArmState ? 4 : 2);
	reg[15].I	 = 0x08;
	armState	 = true;
	armIrqEnable = false;
	armNextPC	 = 0x08;
	reg[15].I	+= 4;
}

void CPUSoftwareInterrupt(int comment)
{
	static bool disableMessage = false;
	if (armState)
		comment >>= 16;
#ifdef BKPT_SUPPORT
	if (comment == 0xff)
	{
		extern void (*dbgOutput)(char *, u32);
		dbgOutput(NULL, reg[0].I);
		return;
	}
#endif
#ifdef PROFILING
	if (comment == 0xfe)
	{
		profStartup(reg[0].I, reg[1].I);
		return;
	}
	if (comment == 0xfd)
	{
		profControl(reg[0].I);
		return;
	}
	if (comment == 0xfc)
	{
		profCleanup();
		return;
	}
	if (comment == 0xfb)
	{
		profCount();
		return;
	}
#endif
	if (comment == 0xfa)
	{
		agbPrintFlush();
		return;
	}
#ifdef SDL
	if (comment == 0xf9)
	{
		emulating = 0;
		CPU_BREAK_LOOP_2;
		return;
	}
#endif
	if (useBios)
	{
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("SWI: %08x at %08x (0x%08x,0x%08x,0x%08x,VCOUNT = %2d)\n", comment,
			    armState ? armNextPC - 4 : armNextPC - 2,
			    reg[0].I,
			    reg[1].I,
			    reg[2].I,
			    VCOUNT);
		}
#endif
		CPUSoftwareInterrupt();
		return;
	}
	// This would be correct, but it causes problems if uncommented
	//  else {
	//    biosProtected = 0xe3a02004;
	//  }

	switch (comment)
	{
	case 0x00:
		BIOS_SoftReset();
		break;
	case 0x01:
		BIOS_RegisterRamReset();
		break;
	case 0x02:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("Halt: (VCOUNT = %2d)\n",
			    VCOUNT);
		}
#endif
		holdState = true;
		holdType  = -1;
		break;
	case 0x03:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("Stop: (VCOUNT = %2d)\n",
			    VCOUNT);
		}
#endif
		holdState = true;
		holdType  = -1;
		stopState = true;
		break;
	case 0x04:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("IntrWait: 0x%08x,0x%08x (VCOUNT = %2d)\n",
			    reg[0].I,
			    reg[1].I,
			    VCOUNT);
		}
#endif
		CPUSoftwareInterrupt();
		break;
	case 0x05:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("VBlankIntrWait: (VCOUNT = %2d)\n",
			    VCOUNT);
		}
#endif
		CPUSoftwareInterrupt();
		break;
	case 0x06:
		CPUSoftwareInterrupt();
		break;
	case 0x07:
		CPUSoftwareInterrupt();
		break;
	case 0x08:
		BIOS_Sqrt();
		break;
	case 0x09:
		BIOS_ArcTan();
		break;
	case 0x0A:
		BIOS_ArcTan2();
		break;
	case 0x0B:
		BIOS_CpuSet();
		break;
	case 0x0C:
		BIOS_CpuFastSet();
		break;
	case 0x0E:
		BIOS_BgAffineSet();
		break;
	case 0x0F:
		BIOS_ObjAffineSet();
		break;
	case 0x10:
		BIOS_BitUnPack();
		break;
	case 0x11:
		BIOS_LZ77UnCompWram();
		break;
	case 0x12:
		BIOS_LZ77UnCompVram();
		break;
	case 0x13:
		BIOS_HuffUnComp();
		break;
	case 0x14:
		BIOS_RLUnCompWram();
		break;
	case 0x15:
		BIOS_RLUnCompVram();
		break;
	case 0x16:
		BIOS_Diff8bitUnFilterWram();
		break;
	case 0x17:
		BIOS_Diff8bitUnFilterVram();
		break;
	case 0x18:
		BIOS_Diff16bitUnFilter();
		break;
	case 0x19:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("SoundBiasSet: 0x%08x (VCOUNT = %2d)\n",
			    reg[0].I,
			    VCOUNT);
		}
#endif
		if (reg[0].I)
			soundPause();
		else
			soundResume();
		break;
	case 0x1F:
		BIOS_MidiKey2Freq();
		break;
	case 0x2A:
		BIOS_SndDriverJmpTableCopy();
	// let it go, because we don't really emulate this function // FIXME (?)
	default:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_SWI)
		{
			log("SWI: %08x at %08x (0x%08x,0x%08x,0x%08x,VCOUNT = %2d)\n", comment,
			    armState ? armNextPC - 4 : armNextPC - 2,
			    reg[0].I,
			    reg[1].I,
			    reg[2].I,
			    VCOUNT);
		}
#endif

		if (!disableMessage)
		{
			systemMessage(MSG_UNSUPPORTED_BIOS_FUNCTION,
			              N_(
			                  "Unsupported BIOS function %02x called from %08x. A BIOS file is needed in order to get correct behaviour."),
			              comment,
			              armMode ? armNextPC - 4 : armNextPC - 2);
			disableMessage = true;
		}
		break;
	}
}

void CPUCompareVCOUNT()
{
	if (VCOUNT == (DISPSTAT >> 8))
	{
		DISPSTAT |= 4;
		UPDATE_REG(0x04, DISPSTAT);

		if (DISPSTAT & 0x20)
		{
			IF |= 4;
			UPDATE_REG(0x202, IF);
		}
	}
	else
	{
		DISPSTAT &= 0xFFFB;
		UPDATE_REG(0x4, DISPSTAT);
	}
}

void doDMA(u32 &s, u32 &d, u32 si, u32 di, u32 c, int transfer32)
{
	int sm = s >> 24;
	int dm = d >> 24;

	int sc = c;

	cpuDmaCount = c;

	if (transfer32)
	{
		s &= 0xFFFFFFFC;
		if (s < 0x02000000 && (reg[15].I >> 24))
		{
			while (c != 0)
			{
				CPUWriteMemory(d, 0);
				d += di;
				c--;
			}
		}
		else
		{
			while (c != 0)
			{
				CPUWriteMemory(d, CPUReadMemory(s));
				d += di;
				s += si;
				c--;
			}
		}
	}
	else
	{
		s &= 0xFFFFFFFE;
		si = (int)si >> 1;
		di = (int)di >> 1;
		if (s < 0x02000000 && (reg[15].I >> 24))
		{
			while (c != 0)
			{
				CPUWriteHalfWord(d, 0);
				d += di;
				c--;
			}
		}
		else
		{
			while (c != 0)
			{
				cpuDmaLast = CPUReadHalfWord(s);
				CPUWriteHalfWord(d, cpuDmaLast);
				d += di;
				s += si;
				c--;
			}
		}
	}

	cpuDmaCount = 0;

	int sw = 1 + memoryWaitSeq[sm & 15];
	int dw = 1 + memoryWaitSeq[dm & 15];

	int totalTicks = 0;

	if (transfer32)
	{
		if (!memory32[sm & 15])
			sw <<= 1;
		if (!memory32[dm & 15])
			dw <<= 1;
	}

	totalTicks = (sw + dw) * sc;

	cpuDmaTicksToUpdate += totalTicks;

	if (*extCpuLoopTicks >= 0)
	{
		CPU_BREAK_LOOP;
	}
}

void CPUCheckDMA(int reason, int dmamask)
{
	cpuDmaHack = 0;
	// DMA 0
	if ((DM0CNT_H & 0x8000) && (dmamask & 1))
	{
		if (((DM0CNT_H >> 12) & 3) == reason)
		{
			u32 sourceIncrement = 4;
			u32 destIncrement	= 4;
			switch ((DM0CNT_H >> 7) & 3)
			{
			case 0:
				break;
			case 1:
				sourceIncrement = (u32) - 4;
				break;
			case 2:
				sourceIncrement = 0;
				break;
			}
			switch ((DM0CNT_H >> 5) & 3)
			{
			case 0:
				break;
			case 1:
				destIncrement = (u32) - 4;
				break;
			case 2:
				destIncrement = 0;
				break;
			}
#ifdef GBA_LOGGING
			if (systemVerbose & VERBOSE_DMA0)
			{
				int count = (DM0CNT_L ? DM0CNT_L : 0x4000) << 1;
				if (DM0CNT_H & 0x0400)
					count <<= 1;
				log("DMA0: s=%08x d=%08x c=%04x count=%08x\n", dma0Source, dma0Dest,
				    DM0CNT_H,
				    count);
			}
#endif
			doDMA(dma0Source, dma0Dest, sourceIncrement, destIncrement,
			      DM0CNT_L ? DM0CNT_L : 0x4000,
			      DM0CNT_H & 0x0400);
			cpuDmaHack = 1;
			if (DM0CNT_H & 0x4000)
			{
				IF |= 0x0100;
				UPDATE_REG(0x202, IF);
			}

			if (((DM0CNT_H >> 5) & 3) == 3)
			{
				dma0Dest = DM0DAD_L | (DM0DAD_H << 16);
			}

			if (!(DM0CNT_H & 0x0200) || (reason == 0))
			{
				DM0CNT_H &= 0x7FFF;
				UPDATE_REG(0xBA, DM0CNT_H);
			}
		}
	}

	// DMA 1
	if ((DM1CNT_H & 0x8000) && (dmamask & 2))
	{
		if (((DM1CNT_H >> 12) & 3) == reason)
		{
			u32 sourceIncrement = 4;
			u32 destIncrement	= 4;
			switch ((DM1CNT_H >> 7) & 3)
			{
			case 0:
				break;
			case 1:
				sourceIncrement = (u32) - 4;
				break;
			case 2:
				sourceIncrement = 0;
				break;
			}
			switch ((DM1CNT_H >> 5) & 3)
			{
			case 0:
				break;
			case 1:
				destIncrement = (u32) - 4;
				break;
			case 2:
				destIncrement = 0;
				break;
			}
			if (reason == 3)
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_DMA1)
				{
					log("DMA1: s=%08x d=%08x c=%04x count=%08x\n", dma1Source, dma1Dest,
					    DM1CNT_H,
					    16);
				}
#endif
				doDMA(dma1Source, dma1Dest, sourceIncrement, 0, 4,
				      0x0400);
			}
			else
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_DMA1)
				{
					int count = (DM1CNT_L ? DM1CNT_L : 0x4000) << 1;
					if (DM1CNT_H & 0x0400)
						count <<= 1;
					log("DMA1: s=%08x d=%08x c=%04x count=%08x\n", dma1Source, dma1Dest,
					    DM1CNT_H,
					    count);
				}
#endif
				doDMA(dma1Source, dma1Dest, sourceIncrement, destIncrement,
				      DM1CNT_L ? DM1CNT_L : 0x4000,
				      DM1CNT_H & 0x0400);
			}
			cpuDmaHack = 1;

			if (DM1CNT_H & 0x4000)
			{
				IF |= 0x0200;
				UPDATE_REG(0x202, IF);
			}

			if (((DM1CNT_H >> 5) & 3) == 3)
			{
				dma1Dest = DM1DAD_L | (DM1DAD_H << 16);
			}

			if (!(DM1CNT_H & 0x0200) || (reason == 0))
			{
				DM1CNT_H &= 0x7FFF;
				UPDATE_REG(0xC6, DM1CNT_H);
			}
		}
	}

	// DMA 2
	if ((DM2CNT_H & 0x8000) && (dmamask & 4))
	{
		if (((DM2CNT_H >> 12) & 3) == reason)
		{
			u32 sourceIncrement = 4;
			u32 destIncrement	= 4;
			switch ((DM2CNT_H >> 7) & 3)
			{
			case 0:
				break;
			case 1:
				sourceIncrement = (u32) - 4;
				break;
			case 2:
				sourceIncrement = 0;
				break;
			}
			switch ((DM2CNT_H >> 5) & 3)
			{
			case 0:
				break;
			case 1:
				destIncrement = (u32) - 4;
				break;
			case 2:
				destIncrement = 0;
				break;
			}
			if (reason == 3)
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_DMA2)
				{
					int count = (4) << 2;
					log("DMA2: s=%08x d=%08x c=%04x count=%08x\n", dma2Source, dma2Dest,
					    DM2CNT_H,
					    count);
				}
#endif
				doDMA(dma2Source, dma2Dest, sourceIncrement, 0, 4,
				      0x0400);
			}
			else
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_DMA2)
				{
					int count = (DM2CNT_L ? DM2CNT_L : 0x4000) << 1;
					if (DM2CNT_H & 0x0400)
						count <<= 1;
					log("DMA2: s=%08x d=%08x c=%04x count=%08x\n", dma2Source, dma2Dest,
					    DM2CNT_H,
					    count);
				}
#endif
				doDMA(dma2Source, dma2Dest, sourceIncrement, destIncrement,
				      DM2CNT_L ? DM2CNT_L : 0x4000,
				      DM2CNT_H & 0x0400);
			}
			cpuDmaHack = 1;
			if (DM2CNT_H & 0x4000)
			{
				IF |= 0x0400;
				UPDATE_REG(0x202, IF);
			}

			if (((DM2CNT_H >> 5) & 3) == 3)
			{
				dma2Dest = DM2DAD_L | (DM2DAD_H << 16);
			}

			if (!(DM2CNT_H & 0x0200) || (reason == 0))
			{
				DM2CNT_H &= 0x7FFF;
				UPDATE_REG(0xD2, DM2CNT_H);
			}
		}
	}

	// DMA 3
	if ((DM3CNT_H & 0x8000) && (dmamask & 8))
	{
		if (((DM3CNT_H >> 12) & 3) == reason)
		{
			u32 sourceIncrement = 4;
			u32 destIncrement	= 4;
			switch ((DM3CNT_H >> 7) & 3)
			{
			case 0:
				break;
			case 1:
				sourceIncrement = (u32) - 4;
				break;
			case 2:
				sourceIncrement = 0;
				break;
			}
			switch ((DM3CNT_H >> 5) & 3)
			{
			case 0:
				break;
			case 1:
				destIncrement = (u32) - 4;
				break;
			case 2:
				destIncrement = 0;
				break;
			}
#ifdef GBA_LOGGING
			if (systemVerbose & VERBOSE_DMA3)
			{
				int count = (DM3CNT_L ? DM3CNT_L : 0x10000) << 1;
				if (DM3CNT_H & 0x0400)
					count <<= 1;
				log("DMA3: s=%08x d=%08x c=%04x count=%08x\n", dma3Source, dma3Dest,
				    DM3CNT_H,
				    count);
			}
#endif
			doDMA(dma3Source, dma3Dest, sourceIncrement, destIncrement,
			      DM3CNT_L ? DM3CNT_L : 0x10000,
			      DM3CNT_H & 0x0400);
			if (DM3CNT_H & 0x4000)
			{
				IF |= 0x0800;
				UPDATE_REG(0x202, IF);
			}

			if (((DM3CNT_H >> 5) & 3) == 3)
			{
				dma3Dest = DM3DAD_L | (DM3DAD_H << 16);
			}

			if (!(DM3CNT_H & 0x0200) || (reason == 0))
			{
				DM3CNT_H &= 0x7FFF;
				UPDATE_REG(0xDE, DM3CNT_H);
			}
		}
	}
	cpuDmaHack = 0;
}

void CPUUpdateRegister(u32 address, u16 value)
{
	switch (address)
	{
	case 0x00:
	{
		bool change	  = ((DISPCNT ^ value) & 0x80) ? true : false;
		bool changeBG = ((DISPCNT ^ value) & 0x0F00) ? true : false;
		DISPCNT = (value & 0xFFF7);
		UPDATE_REG(0x00, DISPCNT);
		layerEnable = layerSettings & value;
		windowOn	= (layerEnable & 0x6000) ? true : false;
		if (change && !((value & 0x80)))
		{
			if (!(DISPSTAT & 1))
			{
				lcdTicks = 960;
				//      VCOUNT = 0;
				//      UPDATE_REG(0x06, VCOUNT);
				DISPSTAT &= 0xFFFC;
				UPDATE_REG(0x04, DISPSTAT);
				CPUCompareVCOUNT();
			}
			//        (*renderLine)();
		}
		CPUUpdateRender();
		// we only care about changes in BG0-BG3
		if (changeBG)
			CPUUpdateRenderBuffers(false);
		//      CPUUpdateTicks();
		break;
	}
	case 0x04:
		DISPSTAT = (value & 0xFF38) | (DISPSTAT & 7);
		UPDATE_REG(0x04, DISPSTAT);
		break;
	case 0x06:
		// not writable
		break;
	case 0x08:
		BG0CNT = (value & 0xDFCF);
		UPDATE_REG(0x08, BG0CNT);
		break;
	case 0x0A:
		BG1CNT = (value & 0xDFCF);
		UPDATE_REG(0x0A, BG1CNT);
		break;
	case 0x0C:
		BG2CNT = (value & 0xFFCF);
		UPDATE_REG(0x0C, BG2CNT);
		break;
	case 0x0E:
		BG3CNT = (value & 0xFFCF);
		UPDATE_REG(0x0E, BG3CNT);
		break;
	case 0x10:
		BG0HOFS = value & 511;
		UPDATE_REG(0x10, BG0HOFS);
		break;
	case 0x12:
		BG0VOFS = value & 511;
		UPDATE_REG(0x12, BG0VOFS);
		break;
	case 0x14:
		BG1HOFS = value & 511;
		UPDATE_REG(0x14, BG1HOFS);
		break;
	case 0x16:
		BG1VOFS = value & 511;
		UPDATE_REG(0x16, BG1VOFS);
		break;
	case 0x18:
		BG2HOFS = value & 511;
		UPDATE_REG(0x18, BG2HOFS);
		break;
	case 0x1A:
		BG2VOFS = value & 511;
		UPDATE_REG(0x1A, BG2VOFS);
		break;
	case 0x1C:
		BG3HOFS = value & 511;
		UPDATE_REG(0x1C, BG3HOFS);
		break;
	case 0x1E:
		BG3VOFS = value & 511;
		UPDATE_REG(0x1E, BG3VOFS);
		break;
	case 0x20:
		BG2PA = value;
		UPDATE_REG(0x20, BG2PA);
		break;
	case 0x22:
		BG2PB = value;
		UPDATE_REG(0x22, BG2PB);
		break;
	case 0x24:
		BG2PC = value;
		UPDATE_REG(0x24, BG2PC);
		break;
	case 0x26:
		BG2PD = value;
		UPDATE_REG(0x26, BG2PD);
		break;
	case 0x28:
		BG2X_L = value;
		UPDATE_REG(0x28, BG2X_L);
		gfxBG2Changed |= 1;
		break;
	case 0x2A:
		BG2X_H = (value & 0xFFF);
		UPDATE_REG(0x2A, BG2X_H);
		gfxBG2Changed |= 1;
		break;
	case 0x2C:
		BG2Y_L = value;
		UPDATE_REG(0x2C, BG2Y_L);
		gfxBG2Changed |= 2;
		break;
	case 0x2E:
		BG2Y_H = value & 0xFFF;
		UPDATE_REG(0x2E, BG2Y_H);
		gfxBG2Changed |= 2;
		break;
	case 0x30:
		BG3PA = value;
		UPDATE_REG(0x30, BG3PA);
		break;
	case 0x32:
		BG3PB = value;
		UPDATE_REG(0x32, BG3PB);
		break;
	case 0x34:
		BG3PC = value;
		UPDATE_REG(0x34, BG3PC);
		break;
	case 0x36:
		BG3PD = value;
		UPDATE_REG(0x36, BG3PD);
		break;
	case 0x38:
		BG3X_L = value;
		UPDATE_REG(0x38, BG3X_L);
		gfxBG3Changed |= 1;
		break;
	case 0x3A:
		BG3X_H = value & 0xFFF;
		UPDATE_REG(0x3A, BG3X_H);
		gfxBG3Changed |= 1;
		break;
	case 0x3C:
		BG3Y_L = value;
		UPDATE_REG(0x3C, BG3Y_L);
		gfxBG3Changed |= 2;
		break;
	case 0x3E:
		BG3Y_H = value & 0xFFF;
		UPDATE_REG(0x3E, BG3Y_H);
		gfxBG3Changed |= 2;
		break;
	case 0x40:
		WIN0H = value;
		UPDATE_REG(0x40, WIN0H);
		CPUUpdateWindow0();
		break;
	case 0x42:
		WIN1H = value;
		UPDATE_REG(0x42, WIN1H);
		CPUUpdateWindow1();
		break;
	case 0x44:
		WIN0V = value;
		UPDATE_REG(0x44, WIN0V);
		break;
	case 0x46:
		WIN1V = value;
		UPDATE_REG(0x46, WIN1V);
		break;
	case 0x48:
		WININ = value & 0x3F3F;
		UPDATE_REG(0x48, WININ);
		break;
	case 0x4A:
		WINOUT = value & 0x3F3F;
		UPDATE_REG(0x4A, WINOUT);
		break;
	case 0x4C:
		MOSAIC = value;
		UPDATE_REG(0x4C, MOSAIC);
		break;
	case 0x50:
		BLDMOD = value & 0x3FFF;
		UPDATE_REG(0x50, BLDMOD);
		fxOn = ((BLDMOD >> 6) & 3) != 0;
		CPUUpdateRender();
		break;
	case 0x52:
		COLEV = value & 0x1F1F;
		UPDATE_REG(0x52, COLEV);
		break;
	case 0x54:
		COLY = value & 0x1F;
		UPDATE_REG(0x54, COLY);
		break;
	case 0x60:
	case 0x62:
	case 0x64:
	case 0x68:
	case 0x6c:
	case 0x70:
	case 0x72:
	case 0x74:
	case 0x78:
	case 0x7c:
	case 0x80:
	case 0x84:
		soundEvent(address & 0xFF, (u8)(value & 0xFF));
		soundEvent((address & 0xFF) + 1, (u8)(value >> 8));
		break;
	case 0x82:
	case 0x88:
	case 0xa0:
	case 0xa2:
	case 0xa4:
	case 0xa6:
	case 0x90:
	case 0x92:
	case 0x94:
	case 0x96:
	case 0x98:
	case 0x9a:
	case 0x9c:
	case 0x9e:
		soundEvent(address & 0xFF, value);
		break;
	case 0xB0:
		DM0SAD_L = value;
		UPDATE_REG(0xB0, DM0SAD_L);
		break;
	case 0xB2:
		DM0SAD_H = value & 0x07FF;
		UPDATE_REG(0xB2, DM0SAD_H);
		break;
	case 0xB4:
		DM0DAD_L = value;
		UPDATE_REG(0xB4, DM0DAD_L);
		break;
	case 0xB6:
		DM0DAD_H = value & 0x07FF;
		UPDATE_REG(0xB6, DM0DAD_H);
		break;
	case 0xB8:
		DM0CNT_L = value & 0x3FFF;
		UPDATE_REG(0xB8, 0);
		break;
	case 0xBA:
	{
		bool start = ((DM0CNT_H ^ value) & 0x8000) ? true : false;
		value &= 0xF7E0;

		DM0CNT_H = value;
		UPDATE_REG(0xBA, DM0CNT_H);

		if (start && (value & 0x8000))
		{
			dma0Source = DM0SAD_L | (DM0SAD_H << 16);
			dma0Dest   = DM0DAD_L | (DM0DAD_H << 16);
			CPUCheckDMA(0, 1);
		}
		break;
	}
	case 0xBC:
		DM1SAD_L = value;
		UPDATE_REG(0xBC, DM1SAD_L);
		break;
	case 0xBE:
		DM1SAD_H = value & 0x0FFF;
		UPDATE_REG(0xBE, DM1SAD_H);
		break;
	case 0xC0:
		DM1DAD_L = value;
		UPDATE_REG(0xC0, DM1DAD_L);
		break;
	case 0xC2:
		DM1DAD_H = value & 0x07FF;
		UPDATE_REG(0xC2, DM1DAD_H);
		break;
	case 0xC4:
		DM1CNT_L = value & 0x3FFF;
		UPDATE_REG(0xC4, 0);
		break;
	case 0xC6:
	{
		bool start = ((DM1CNT_H ^ value) & 0x8000) ? true : false;
		value &= 0xF7E0;

		DM1CNT_H = value;
		UPDATE_REG(0xC6, DM1CNT_H);

		if (start && (value & 0x8000))
		{
			dma1Source = DM1SAD_L | (DM1SAD_H << 16);
			dma1Dest   = DM1DAD_L | (DM1DAD_H << 16);
			CPUCheckDMA(0, 2);
		}
		break;
	}
	case 0xC8:
		DM2SAD_L = value;
		UPDATE_REG(0xC8, DM2SAD_L);
		break;
	case 0xCA:
		DM2SAD_H = value & 0x0FFF;
		UPDATE_REG(0xCA, DM2SAD_H);
		break;
	case 0xCC:
		DM2DAD_L = value;
		UPDATE_REG(0xCC, DM2DAD_L);
		break;
	case 0xCE:
		DM2DAD_H = value & 0x07FF;
		UPDATE_REG(0xCE, DM2DAD_H);
		break;
	case 0xD0:
		DM2CNT_L = value & 0x3FFF;
		UPDATE_REG(0xD0, 0);
		break;
	case 0xD2:
	{
		bool start = ((DM2CNT_H ^ value) & 0x8000) ? true : false;

		value &= 0xF7E0;

		DM2CNT_H = value;
		UPDATE_REG(0xD2, DM2CNT_H);

		if (start && (value & 0x8000))
		{
			dma2Source = DM2SAD_L | (DM2SAD_H << 16);
			dma2Dest   = DM2DAD_L | (DM2DAD_H << 16);

			CPUCheckDMA(0, 4);
		}
		break;
	}
	case 0xD4:
		DM3SAD_L = value;
		UPDATE_REG(0xD4, DM3SAD_L);
		break;
	case 0xD6:
		DM3SAD_H = value & 0x0FFF;
		UPDATE_REG(0xD6, DM3SAD_H);
		break;
	case 0xD8:
		DM3DAD_L = value;
		UPDATE_REG(0xD8, DM3DAD_L);
		break;
	case 0xDA:
		DM3DAD_H = value & 0x0FFF;
		UPDATE_REG(0xDA, DM3DAD_H);
		break;
	case 0xDC:
		DM3CNT_L = value;
		UPDATE_REG(0xDC, 0);
		break;
	case 0xDE:
	{
		bool start = ((DM3CNT_H ^ value) & 0x8000) ? true : false;

		value &= 0xFFE0;

		DM3CNT_H = value;
		UPDATE_REG(0xDE, DM3CNT_H);

		if (start && (value & 0x8000))
		{
			dma3Source = DM3SAD_L | (DM3SAD_H << 16);
			dma3Dest   = DM3DAD_L | (DM3DAD_H << 16);
			CPUCheckDMA(0, 8);
		}
		break;
	}
	case 0x100:
		timer0Reload = value;
		break;
	case 0x102:
		timer0Ticks = timer0ClockReload = TIMER_TICKS[value & 3];
		if (!timer0On && (value & 0x80))
		{
			// reload the counter
			TM0D = timer0Reload;
			if (timer0ClockReload == 1)
				timer0Ticks = 0x10000 - TM0D;
			UPDATE_REG(0x100, TM0D);
		}
		timer0On = value & 0x80 ? true : false;
		TM0CNT	 = value & 0xC7;
		UPDATE_REG(0x102, TM0CNT);
		//    CPUUpdateTicks();
		break;
	case 0x104:
		timer1Reload = value;
		break;
	case 0x106:
		timer1Ticks = timer1ClockReload = TIMER_TICKS[value & 3];
		if (!timer1On && (value & 0x80))
		{
			// reload the counter
			TM1D = timer1Reload;
			if (timer1ClockReload == 1)
				timer1Ticks = 0x10000 - TM1D;
			UPDATE_REG(0x104, TM1D);
		}
		timer1On = value & 0x80 ? true : false;
		TM1CNT	 = value & 0xC7;
		UPDATE_REG(0x106, TM1CNT);
		break;
	case 0x108:
		timer2Reload = value;
		break;
	case 0x10A:
		timer2Ticks = timer2ClockReload = TIMER_TICKS[value & 3];
		if (!timer2On && (value & 0x80))
		{
			// reload the counter
			TM2D = timer2Reload;
			if (timer2ClockReload == 1)
				timer2Ticks = 0x10000 - TM2D;
			UPDATE_REG(0x108, TM2D);
		}
		timer2On = value & 0x80 ? true : false;
		TM2CNT	 = value & 0xC7;
		UPDATE_REG(0x10A, TM2CNT);
		break;
	case 0x10C:
		timer3Reload = value;
		break;
	case 0x10E:
		timer3Ticks = timer3ClockReload = TIMER_TICKS[value & 3];
		if (!timer3On && (value & 0x80))
		{
			// reload the counter
			TM3D = timer3Reload;
			if (timer3ClockReload == 1)
				timer3Ticks = 0x10000 - TM3D;
			UPDATE_REG(0x10C, TM3D);
		}
		timer3On = value & 0x80 ? true : false;
		TM3CNT	 = value & 0xC7;
		UPDATE_REG(0x10E, TM3CNT);
		break;
	case 0x128:
		if (value & 0x80)
		{
			value &= 0xff7f;
			if (value & 1 && (value & 0x4000))
			{
				UPDATE_REG(0x12a, 0xFF);
				IF |= 0x80;
				UPDATE_REG(0x202, IF);
				value &= 0x7f7f;
			}
		}
		UPDATE_REG(0x128, value);
		break;
	case 0x130:
		P1 |= (value & 0x3FF);
		UPDATE_REG(0x130, P1);
		break;
	case 0x132:
		UPDATE_REG(0x132, value & 0xC3FF);
		break;
	case 0x200:
		IE = value & 0x3FFF;
		UPDATE_REG(0x200, IE);
		if ((IME & 1) && (IF & IE) && armIrqEnable)
		{
			CPU_BREAK_LOOP_2;
		}
		break;
	case 0x202:
		IF ^= (value & IF);
		UPDATE_REG(0x202, IF);
		break;
	case 0x204:
	{
		int i;
		memoryWait[0x0e] = memoryWaitSeq[0x0e] = gamepakRamWaitState[value & 3];

		if (!speedHack)
		{
			memoryWait[0x08]	= memoryWait[0x09] = gamepakWaitState[(value >> 2) & 7];
			memoryWaitSeq[0x08] = memoryWaitSeq[0x09] =
			                          gamepakWaitState0[(value >> 2) & 7];

			memoryWait[0x0a]	= memoryWait[0x0b] = gamepakWaitState[(value >> 5) & 7];
			memoryWaitSeq[0x0a] = memoryWaitSeq[0x0b] =
			                          gamepakWaitState1[(value >> 5) & 7];

			memoryWait[0x0c]	= memoryWait[0x0d] = gamepakWaitState[(value >> 8) & 7];
			memoryWaitSeq[0x0c] = memoryWaitSeq[0x0d] =
			                          gamepakWaitState2[(value >> 8) & 7];
		}
		else
		{
			memoryWait[0x08]	= memoryWait[0x09] = 4;
			memoryWaitSeq[0x08] = memoryWaitSeq[0x09] = 2;

			memoryWait[0x0a]	= memoryWait[0x0b] = 4;
			memoryWaitSeq[0x0a] = memoryWaitSeq[0x0b] = 4;

			memoryWait[0x0c]	= memoryWait[0x0d] = 4;
			memoryWaitSeq[0x0c] = memoryWaitSeq[0x0d] = 8;
		}
		for (i = 0; i < 16; i++)
		{
			memoryWaitFetch32[i] = memoryWait32[i] = memoryWait[i] *
			                                         (memory32[i] ? 1 : 2);
			memoryWaitFetch[i] = memoryWait[i];
		}
		memoryWaitFetch32[3] += 1;
		memoryWaitFetch32[2] += 3;

		prefetchActive	= false;
		prefetchApplies = false;
		if (value & 0x4000)
		{
			for (i = 8; i < 16; i++)
			{
				memoryWaitFetch32[i] = 2 * cpuMemoryWait[i];
				memoryWaitFetch[i]	 = cpuMemoryWait[i];
			}
			if (((value & 3) == 3))
			{
				if (!memLagTempEnabled)
				{
					memoryWaitFetch[8]--; // hack to prevent inaccurately extreme lag at some points of many games (possibly
					                      // from no pre-fetch emulation)
					                      /// FIXME: how correct is this? Should it set the fetch to 0 or change fetch32 or
					                      // anything else?

					prefetchActive = true;
				}
				prefetchApplies = true;
			}
		}
		//if(prefetchActive && !prefetchPrevActive) systemScreenMessage("pre-fetch enabled",3,600);
		//if(!prefetchActive && prefetchPrevActive) systemScreenMessage("pre-fetch disabled",3,600);
		prefetchPrevActive = prefetchActive;

		UPDATE_REG(0x204, value);
		break;
	}
	case 0x208:
		IME = value & 1;
		UPDATE_REG(0x208, IME);
		if ((IME & 1) && (IF & IE) && armIrqEnable)
		{
			CPU_BREAK_LOOP_2;
		}
		break;
	case 0x300:
		if (value != 0)
			value &= 0xFFFE;
		UPDATE_REG(0x300, value);
		break;
	default:
		UPDATE_REG(address & 0x3FE, value);
		break;
	}
}

void CPUWriteHalfWordWrapped(u32 address, u16 value)
{
#ifdef GBA_LOGGING
	if (address & 1)
	{
		if (systemVerbose & VERBOSE_UNALIGNED_MEMORY)
		{
			log("Unaligned halfword write: %04x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
	}
#endif

	switch (address >> 24)
	{
	case 2:
#ifdef SDL
		if (*((u16 *)&freezeWorkRAM[address & 0x3FFFE]))
			cheatsWriteHalfWord((u16 *)&workRAM[address & 0x3FFFE],
			                    value,
			                    *((u16 *)&freezeWorkRAM[address & 0x3FFFE]));
		else
#endif
		WRITE16LE(((u16 *)&workRAM[address & 0x3FFFE]), value);
		break;
	case 3:
#ifdef SDL
		if (*((u16 *)&freezeInternalRAM[address & 0x7ffe]))
			cheatsWriteHalfWord((u16 *)&internalRAM[address & 0x7ffe],
			                    value,
			                    *((u16 *)&freezeInternalRAM[address & 0x7ffe]));
		else
#endif
		WRITE16LE(((u16 *)&internalRAM[address & 0x7ffe]), value);
		break;
	case 4:
		CPUUpdateRegister(address & 0x3fe, value);
		break;
	case 5:
		WRITE16LE(((u16 *)&paletteRAM[address & 0x3fe]), value);
		break;
	case 6:
		if (address & 0x10000)
			WRITE16LE(((u16 *)&vram[address & 0x17ffe]), value);
		else
			WRITE16LE(((u16 *)&vram[address & 0x1fffe]), value);
		break;
	case 7:
		WRITE16LE(((u16 *)&oam[address & 0x3fe]), value);
		break;
	case 8:
	case 9:
		if (address == 0x80000c4 || address == 0x80000c6 || address == 0x80000c8)
		{
			if (!rtcWrite(address, value))
				goto unwritable;
		}
		else if (!agbPrintWrite(address, value))
			goto unwritable;
		break;
	case 13:
		if (cpuEEPROMEnabled)
		{
			eepromWrite(address, (u8)(value & 0xFF));
			break;
		}
		goto unwritable;
	case 14:
		if (!eepromInUse | cpuSramEnabled | cpuFlashEnabled)
		{
			(*cpuSaveGameFunc)(address, (u8)(value & 0xFF));
			break;
		}
		goto unwritable;
	default:
unwritable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_WRITE)
		{
			log("Illegal halfword write: %04x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
#endif
		break;
	}
}

void CPUWriteHalfWord(u32 address, u16 value)
{
	CPUWriteHalfWordWrapped(address, value);
	CallRegisteredLuaMemHook(address, 2, value, LUAMEMHOOK_WRITE);
}

void CPUWriteByteWrapped(u32 address, u8 b)
{
	switch (address >> 24)
	{
	case 2:
#ifdef SDL
		if (freezeWorkRAM[address & 0x3FFFF])
			cheatsWriteByte(&workRAM[address & 0x3FFFF], b);
		else
#endif
		workRAM[address & 0x3FFFF] = b;
		break;
	case 3:
#ifdef SDL
		if (freezeInternalRAM[address & 0x7fff])
			cheatsWriteByte(&internalRAM[address & 0x7fff], b);
		else
#endif
		internalRAM[address & 0x7fff] = b;
		break;
	case 4:
		switch (address & 0x3FF)
		{
		case 0x301:
			if (b == 0x80)
				stopState = true;
			holdState = 1;
			holdType  = -1;
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x68:
		case 0x69:
		case 0x6c:
		case 0x6d:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x78:
		case 0x79:
		case 0x7c:
		case 0x7d:
		case 0x80:
		case 0x81:
		case 0x84:
		case 0x85:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
			soundEvent(address & 0xFF, b);
			break;
		default:
			//      if(address & 1) {
			//        CPUWriteHalfWord(address-1, (CPUReadHalfWord(address-1)&0x00FF)|((int)b<<8));
			//      } else
			if (address & 1)
				CPUUpdateRegister(address & 0x3fe,
				                  ((READ16LE(((u16 *)&ioMem[address & 0x3fe])))
				                   & 0x00FF) |
				                  b << 8);
			else
				CPUUpdateRegister(address & 0x3fe,
				                  ((READ16LE(((u16 *)&ioMem[address & 0x3fe])) & 0xFF00) | b));
		}
		break;
	case 5:
		// no need to switch
		*((u16 *)&paletteRAM[address & 0x3FE]) = (b << 8) | b;
		break;
	case 6:
		// no need to switch
		if (address & 0x10000)
			*((u16 *)&vram[address & 0x17FFE]) = (b << 8) | b;
		else
			*((u16 *)&vram[address & 0x1FFFE]) = (b << 8) | b;
		break;
	case 7:
		// no need to switch
		*((u16 *)&oam[address & 0x3FE]) = (b << 8) | b;
		break;
	case 13:
		if (cpuEEPROMEnabled)
		{
			eepromWrite(address, b);
			break;
		}
		goto unwritable;
	case 14:
		if (!eepromInUse | cpuSramEnabled | cpuFlashEnabled)
		{
			(*cpuSaveGameFunc)(address, b);
			break;
		}
	// default
	default:
unwritable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_WRITE)
		{
			log("Illegal byte write: %02x to %08x from %08x\n",
			    b,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
#endif
		break;
	}
}

void CPUWriteByte(u32 address, u8 b)
{
	CPUWriteByteWrapped(address, b);
	CallRegisteredLuaMemHook(address, 1, b, LUAMEMHOOK_WRITE);
}

bool CPULoadBios(const char *biosFileName, bool useBiosFile)
{
	useBios = false;
	if (useBiosFile)
	{
		useBios = utilLoadBIOS(bios, biosFileName, 4);
		if (!useBios)
		{
			systemMessage(MSG_INVALID_BIOS_FILE_SIZE, N_("Invalid GBA BIOS file"));
		}
	}

	if (!useBios)
	{
		// load internal BIOS
		memcpy(bios, myROM, sizeof(myROM));
	}

	return useBios;
}

void CPUInit()
{
#ifdef WORDS_BIGENDIAN
	if (!cpuBiosSwapped)
	{
		for (unsigned int i = 0; i < sizeof(myROM) / 4; i++)
		{
			WRITE32LE(&myROM[i], myROM[i]);
		}
		cpuBiosSwapped = true;
	}
#endif
	gbaSaveType = 0;
	eepromInUse = 0;
	saveType	= 0;

	if (!useBios)
	{
		// load internal BIOS
		memcpy(bios, myROM, sizeof(myROM));
	}

	biosProtected[0] = 0x00;
	biosProtected[1] = 0xf0;
	biosProtected[2] = 0x29;
	biosProtected[3] = 0xe1;

	int i = 0;
	for (i = 0; i < 256; i++)
	{
		int cpuBitSetCount = 0;
		int j;
		for (j = 0; j < 8; j++)
			if (i & (1 << j))
				cpuBitSetCount++;
		cpuBitsSet[i] = cpuBitSetCount;

		for (j = 0; j < 8; j++)
			if (i & (1 << j))
				break;
		cpuLowestBitSet[i] = j;
	}

	for (i = 0; i < 0x400; i++)
		ioReadable[i] = true;
	for (i = 0x10; i < 0x48; i++)
		ioReadable[i] = false;
	for (i = 0x4c; i < 0x50; i++)
		ioReadable[i] = false;
	for (i = 0x54; i < 0x60; i++)
		ioReadable[i] = false;
	for (i = 0x8c; i < 0x90; i++)
		ioReadable[i] = false;
	for (i = 0xa0; i < 0xb8; i++)
		ioReadable[i] = false;
	for (i = 0xbc; i < 0xc4; i++)
		ioReadable[i] = false;
	for (i = 0xc8; i < 0xd0; i++)
		ioReadable[i] = false;
	for (i = 0xd4; i < 0xdc; i++)
		ioReadable[i] = false;
	for (i = 0xe0; i < 0x100; i++)
		ioReadable[i] = false;
	for (i = 0x110; i < 0x120; i++)
		ioReadable[i] = false;
	for (i = 0x12c; i < 0x130; i++)
		ioReadable[i] = false;
	for (i = 0x138; i < 0x140; i++)
		ioReadable[i] = false;
	for (i = 0x144; i < 0x150; i++)
		ioReadable[i] = false;
	for (i = 0x15c; i < 0x200; i++)
		ioReadable[i] = false;
	for (i = 0x20c; i < 0x300; i++)
		ioReadable[i] = false;
	for (i = 0x304; i < 0x400; i++)
		ioReadable[i] = false;

	*((u16 *)&rom[0x1fe209c]) = 0xdffa;  // SWI 0xFA
	*((u16 *)&rom[0x1fe209e]) = 0x4770;  // BX LR

	{
		int32 origMemoryWaitFetch[16]	= { 3, 0, 3, 0, 0, 1, 1, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
		int32 origMemoryWaitFetch32[16] = { 6, 0, 6, 0, 0, 2, 2, 0, 8, 8, 8, 8, 8, 8, 8, 0 };
		memcpy(memoryWaitFetch, origMemoryWaitFetch, 16 * sizeof(int32));
		memcpy(memoryWaitFetch32, origMemoryWaitFetch32, 16 * sizeof(int32));
	}
}

void CPUReset(bool userReset)
{
	// movie must be closed while opening/creating a movie
	if (userReset && VBAMovieRecording())
	{
		VBAMovieSignalReset();
		return;
	}

	if (!VBAMovieActive())
	{
		GBASystemCounters.frameCount = 0;
		GBASystemCounters.lagCount	 = 0;
		GBASystemCounters.extraCount = 0;
		GBASystemCounters.lagged	 = true;
		GBASystemCounters.laggedLast = true;
	}

	if (gbaSaveType == 0)
	{
		if (eepromInUse)
			gbaSaveType = 3;
		else
			switch (saveType)
			{
			case 1:
				gbaSaveType = 1;
				break;
			case 2:
				gbaSaveType = 2;
				break;
			}
	}

	rtcReset();
	// clean registers
	memset(&reg[0], 0, sizeof(reg));
	// clean OAM
	memset(oam, 0, 0x400);
	// clean palette
	memset(paletteRAM, 0, 0x400);
	// clean picture
	memset(pix, 0, 4 * 241 * 162);
	// clean vram
	memset(vram, 0, 0x20000);
	// clean io memory
	memset(ioMem, 0, 0x400);
	// clean RAM
	memset(internalRAM, 0, 0x8000); /// FIXME: is it unsafe to erase ALL of this? Even the init code doesn't.
	memset(workRAM, 0, 0x40000); /// ditto

	DISPCNT	 = 0x0080;
	DISPSTAT = 0x0000;
	VCOUNT	 = 0x0000;
	BG0CNT	 = 0x0000;
	BG1CNT	 = 0x0000;
	BG2CNT	 = 0x0000;
	BG3CNT	 = 0x0000;
	BG0HOFS	 = 0x0000;
	BG0VOFS	 = 0x0000;
	BG1HOFS	 = 0x0000;
	BG1VOFS	 = 0x0000;
	BG2HOFS	 = 0x0000;
	BG2VOFS	 = 0x0000;
	BG3HOFS	 = 0x0000;
	BG3VOFS	 = 0x0000;
	BG2PA	 = 0x0100;
	BG2PB	 = 0x0000;
	BG2PC	 = 0x0000;
	BG2PD	 = 0x0100;
	BG2X_L	 = 0x0000;
	BG2X_H	 = 0x0000;
	BG2Y_L	 = 0x0000;
	BG2Y_H	 = 0x0000;
	BG3PA	 = 0x0100;
	BG3PB	 = 0x0000;
	BG3PC	 = 0x0000;
	BG3PD	 = 0x0100;
	BG3X_L	 = 0x0000;
	BG3X_H	 = 0x0000;
	BG3Y_L	 = 0x0000;
	BG3Y_H	 = 0x0000;
	WIN0H	 = 0x0000;
	WIN1H	 = 0x0000;
	WIN0V	 = 0x0000;
	WIN1V	 = 0x0000;
	WININ	 = 0x0000;
	WINOUT	 = 0x0000;
	MOSAIC	 = 0x0000;
	BLDMOD	 = 0x0000;
	COLEV	 = 0x0000;
	COLY	 = 0x0000;
	DM0SAD_L = 0x0000;
	DM0SAD_H = 0x0000;
	DM0DAD_L = 0x0000;
	DM0DAD_H = 0x0000;
	DM0CNT_L = 0x0000;
	DM0CNT_H = 0x0000;
	DM1SAD_L = 0x0000;
	DM1SAD_H = 0x0000;
	DM1DAD_L = 0x0000;
	DM1DAD_H = 0x0000;
	DM1CNT_L = 0x0000;
	DM1CNT_H = 0x0000;
	DM2SAD_L = 0x0000;
	DM2SAD_H = 0x0000;
	DM2DAD_L = 0x0000;
	DM2DAD_H = 0x0000;
	DM2CNT_L = 0x0000;
	DM2CNT_H = 0x0000;
	DM3SAD_L = 0x0000;
	DM3SAD_H = 0x0000;
	DM3DAD_L = 0x0000;
	DM3DAD_H = 0x0000;
	DM3CNT_L = 0x0000;
	DM3CNT_H = 0x0000;
	TM0D	 = 0x0000;
	TM0CNT	 = 0x0000;
	TM1D	 = 0x0000;
	TM1CNT	 = 0x0000;
	TM2D	 = 0x0000;
	TM2CNT	 = 0x0000;
	TM3D	 = 0x0000;
	TM3CNT	 = 0x0000;
	P1		 = 0x03FF;
	IE		 = 0x0000;
	IF		 = 0x0000;
	IME		 = 0x0000;

	armMode = 0x1F;

	if (cpuIsMultiBoot)
	{
		reg[13].I	   = 0x03007F00;
		reg[15].I	   = 0x02000000;
		reg[16].I	   = 0x00000000;
		reg[R13_IRQ].I = 0x03007FA0;
		reg[R13_SVC].I = 0x03007FE0;
		armIrqEnable   = true;
	}
	else
	{
		if (useBios && !skipBios)
		{
			reg[15].I	 = 0x00000000;
			armMode		 = 0x13;
			armIrqEnable = false;
		}
		else
		{
			reg[13].I	   = 0x03007F00;
			reg[15].I	   = 0x08000000;
			reg[16].I	   = 0x00000000;
			reg[R13_IRQ].I = 0x03007FA0;
			reg[R13_SVC].I = 0x03007FE0;
			armIrqEnable   = true;
		}
	}
	armState = true;
	C_FLAG	 = V_FLAG = N_FLAG = Z_FLAG = false;
	UPDATE_REG(0x00, DISPCNT);
	UPDATE_REG(0x20, BG2PA);
	UPDATE_REG(0x26, BG2PD);
	UPDATE_REG(0x30, BG3PA);
	UPDATE_REG(0x36, BG3PD);
	UPDATE_REG(0x130, P1);
	UPDATE_REG(0x88, 0x200);

	// disable FIQ
	reg[16].I |= 0x40;
	CPUUpdateCPSR();

	armNextPC  = reg[15].I;
	reg[15].I += 4;

	// reset internal state
	holdState = false;
	holdType  = 0;

	biosProtected[0] = 0x00;
	biosProtected[1] = 0xf0;
	biosProtected[2] = 0x29;
	biosProtected[3] = 0xe1;

	BIOS_RegisterRamReset();

	lcdTicks = 960;
	timer0On = false;
	timer0Ticks		  = 0;
	timer0Reload	  = 0;
	timer0ClockReload = 0;
	timer1On		  = false;
	timer1Ticks		  = 0;
	timer1Reload	  = 0;
	timer1ClockReload = 0;
	timer2On		  = false;
	timer2Ticks		  = 0;
	timer2Reload	  = 0;
	timer2ClockReload = 0;
	timer3On		  = false;
	timer3Ticks		  = 0;
	timer3Reload	  = 0;
	timer3ClockReload = 0;
	dma0Source		  = 0;
	dma0Dest		  = 0;
	dma1Source		  = 0;
	dma1Dest		  = 0;
	dma2Source		  = 0;
	dma2Dest		  = 0;
	dma3Source		  = 0;
	dma3Dest		  = 0;
	cpuSaveGameFunc	  = flashSaveDecide;
	renderLine		  = mode0RenderLine;
	fxOn			  = false;
	windowOn		  = false;
	frameSkipCount	  = 0;
	saveType		  = 0;
	layerEnable		  = DISPCNT & layerSettings;

	CPUUpdateRenderBuffers(true);

	for (int i = 0; i < 256; i++)
	{
		map[i].address = (u8 *)&dummyAddress;
		map[i].mask	   = 0;
	}

	map[0].address	= bios;
	map[0].mask		= 0x3FFF;
	map[2].address	= workRAM;
	map[2].mask		= 0x3FFFF;
	map[3].address	= internalRAM;
	map[3].mask		= 0x7FFF;
	map[4].address	= ioMem;
	map[4].mask		= 0x3FF;
	map[5].address	= paletteRAM;
	map[5].mask		= 0x3FF;
	map[6].address	= vram;
	map[6].mask		= 0x1FFFF;
	map[7].address	= oam;
	map[7].mask		= 0x3FF;
	map[8].address	= rom;
	map[8].mask		= 0x1FFFFFF;
	map[9].address	= rom;
	map[9].mask		= 0x1FFFFFF;
	map[10].address = rom;
	map[10].mask	= 0x1FFFFFF;
	map[12].address = rom;
	map[12].mask	= 0x1FFFFFF;
	map[14].address = flashSaveMemory;
	map[14].mask	= 0xFFFF;

	eepromReset();
	flashReset();

	soundReset();

	CPUUpdateWindow0();
	CPUUpdateWindow1();

	// make sure registers are correctly initialized if not using BIOS
	if (!useBios)
	{
		if (cpuIsMultiBoot)
			BIOS_RegisterRamReset(0xfe);
		else
			BIOS_RegisterRamReset(0xff);
	}
	else
	{
		if (cpuIsMultiBoot)
			BIOS_RegisterRamReset(0xfe);
	}

	switch (cpuSaveType)
	{
	case 0: // automatic
		cpuSramEnabled		   = true;
		cpuFlashEnabled		   = true;
		cpuEEPROMEnabled	   = true;
		cpuEEPROMSensorEnabled = false;
		break;
	case 1: // EEPROM
		cpuSramEnabled		   = false;
		cpuFlashEnabled		   = false;
		cpuEEPROMEnabled	   = true;
		cpuEEPROMSensorEnabled = false;
		break;
	case 2: // SRAM
		cpuSramEnabled		   = true;
		cpuFlashEnabled		   = false;
		cpuEEPROMEnabled	   = false;
		cpuEEPROMSensorEnabled = false;
		cpuSaveGameFunc		   = sramWrite;
		break;
	case 3: // FLASH
		cpuSramEnabled		   = false;
		cpuFlashEnabled		   = true;
		cpuEEPROMEnabled	   = false;
		cpuEEPROMSensorEnabled = false;
		cpuSaveGameFunc		   = flashWrite;
		break;
	case 4: // EEPROM+Sensor
		cpuSramEnabled		   = false;
		cpuFlashEnabled		   = false;
		cpuEEPROMEnabled	   = true;
		cpuEEPROMSensorEnabled = true;
		break;
	case 5: // NONE
		cpuSramEnabled		   = false;
		cpuFlashEnabled		   = false;
		cpuEEPROMEnabled	   = false;
		cpuEEPROMSensorEnabled = false;
		break;
	}

	systemResetSensor();

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	gbaLastTime	  = systemGetClock();
	gbaFrameCount = 0;

	systemRefreshScreen();
}

void CPUInterrupt()
{
	u32	 PC			= reg[15].I;
	bool savedState = armState;
	CPUSwitchMode(0x12, true, false);
	reg[14].I = PC;
	if (!savedState)
		reg[14].I += 2;
	reg[15].I	 = 0x18;
	armState	 = true;
	armIrqEnable = false;

	armNextPC  = reg[15].I;
	reg[15].I += 4;

	//  if(!holdState)
	biosProtected[0] = 0x02;
	biosProtected[1] = 0xc0;
	biosProtected[2] = 0x5e;
	biosProtected[3] = 0xe5;
}

void TogglePrefetchHack()
{
	memLagTempEnabled = !memLagTempEnabled;

	if (emulating)
	{
		extern bool8 prefetchActive, prefetchPrevActive, prefetchApplies;
		if (prefetchApplies && prefetchActive == memLagTempEnabled)
		{
			prefetchActive = !prefetchActive;
			//if(prefetchActive && !prefetchPrevActive) systemScreenMessage("pre-fetch enabled",3,600);
			//if(!prefetchActive && prefetchPrevActive) systemScreenMessage("pre-fetch disabled",3,600);
			extern int32 memoryWaitFetch [16];
			if (prefetchActive)
				memoryWaitFetch[8]--;
			else
				memoryWaitFetch[8]++;
			prefetchPrevActive = prefetchActive;
		}
	}
}

void SetPrefetchHack(bool set)
{
	if ((bool)memLagTempEnabled == set)
		TogglePrefetchHack();
}

#ifdef SDL
void log(const char *defaultMsg, ...)
{
	char	buffer[2048];
	va_list valist;

	va_start(valist, defaultMsg);
	vsprintf(buffer, defaultMsg, valist);

	if (out == NULL)
	{
		out = fopen("trace.log", "w");
	}

	fputs(buffer, out);

	va_end(valist);
}

#else
extern void winlog(const char *, ...);
#endif

void CPULoop(int _ticks)
{
	int32 ticks = _ticks;
	int32 clockTicks;
	int32 cpuLoopTicks	= 0;
	int32 timerOverflow = 0;
	// variables used by the CPU core

	extCpuLoopTicks = &cpuLoopTicks;
	extClockTicks	= &clockTicks;
	extTicks		= &ticks;

	cpuLoopTicks = CPUUpdateTicks();
	if (cpuLoopTicks > ticks)
	{
		cpuLoopTicks  = ticks;
		cpuSavedTicks = ticks;
	}

	if (intState)
	{
		cpuLoopTicks  = 5;
		cpuSavedTicks = 5;
	}

	if (newFrame)
	{
		extern void VBAOnExitingFrameBoundary();
		VBAOnExitingFrameBoundary();

		// update joystick information
		systemReadJoypads();

		u32 joy = systemGetJoypad(0, cpuEEPROMSensorEnabled);

//		if (cpuEEPROMSensorEnabled)
//			systemUpdateMotionSensor(0);

		P1 = 0x03FF ^ (joy & 0x3FF);
		UPDATE_REG(0x130, P1);
		u16 P1CNT = READ16LE(((u16 *)&ioMem[0x132]));
		// this seems wrong, but there are cases where the game
		// can enter the stop state without requesting an IRQ from
		// the joypad.
		if ((P1CNT & 0x4000) || stopState)
		{
			u16 p1 = (0x3FF ^ P1) & 0x3FF;
			if (P1CNT & 0x8000)
			{
				if (p1 == (P1CNT & 0x3FF))
				{
					IF |= 0x1000;
					UPDATE_REG(0x202, IF);
				}
			}
			else
			{
				if (p1 & P1CNT)
				{
					IF |= 0x1000;
					UPDATE_REG(0x202, IF);
				}
			}
		}

		// HACK: some special "buttons"
		extButtons = (joy >> 18);
		speedup	   = (extButtons & 1) != 0;

		VBAMovieResetIfRequested();

		CallRegisteredLuaFunctions(LUACALL_BEFOREEMULATION);

		newFrame = false;
	}

	for (;; )
	{
#ifndef FINAL_VERSION
		if (systemDebug)
		{
			if (systemDebug >= 10 && !holdState)
			{
				CPUUpdateCPSR();
				sprintf(
				    buffer,
				    "R00=%08x R01=%08x R02=%08x R03=%08x R04=%08x R05=%08x R06=%08x R07=%08x R08=%08x"
				    "R09=%08x R10=%08x R11=%08x R12=%08x R13=%08x R14=%08x R15=%08x R16=%08x R17=%08x\n",
				    reg[0].I,
				    reg[1].I,
				    reg[2].I,
				    reg[3].I,
				    reg[4].I,
				    reg[5].I,
				    reg[6].I,
				    reg[7].I,
				    reg[8].I,
				    reg[9].I,
				    reg[10].I,
				    reg[11].I,
				    reg[12].I,
				    reg[13].I,
				    reg[14].I,
				    reg[15].I,
				    reg[16].I,
				    reg[17].I);
#ifdef SDL
				log(buffer);
#else
				winlog(buffer);
#endif
			}
			else if (!holdState)
			{
				sprintf(buffer, "PC=%08x\n", armNextPC);
#ifdef SDL
				log(buffer);
#else
				winlog(buffer);
#endif
			}
		}
#endif

		if (!holdState)
		{
			if (armState)
			{
				CallRegisteredLuaMemHook(armNextPC, 4, CPUReadMemoryQuick(armNextPC), LUAMEMHOOK_EXEC);
#include "arm-new.h"
			}
			else
			{
				CallRegisteredLuaMemHook(armNextPC, 2, CPUReadHalfWordQuick(armNextPC), LUAMEMHOOK_EXEC);
#include "thumb.h"
			}
		}
		else
		{
			clockTicks = lcdTicks;

			if (soundTicks < clockTicks)
				clockTicks = soundTicks;

			if (timer0On && (timer0Ticks < clockTicks))
			{
				clockTicks = timer0Ticks;
			}
			if (timer1On && (timer1Ticks < clockTicks))
			{
				clockTicks = timer1Ticks;
			}
			if (timer2On && (timer2Ticks < clockTicks))
			{
				clockTicks = timer2Ticks;
			}
			if (timer3On && (timer3Ticks < clockTicks))
			{
				clockTicks = timer3Ticks;
			}
#ifdef PROFILING
			if (profilingTicksReload != 0)
			{
				if (profilingTicks < clockTicks)
				{
					clockTicks = profilingTicks;
				}
			}
#endif
		}

		cpuLoopTicks -= clockTicks;
		if ((cpuLoopTicks <= 0))
		{
			if (cpuSavedTicks)
			{
				clockTicks = cpuSavedTicks; // + cpuLoopTicks;
			}
			cpuDmaTicksToUpdate = -cpuLoopTicks;

updateLoop:
			lcdTicks -= clockTicks;

			if (lcdTicks <= 0)
			{
				if (DISPSTAT & 1) // V-BLANK
				{ // if in V-Blank mode, keep computing...
					if (DISPSTAT & 2)
					{
						lcdTicks += 960;
						VCOUNT++;
						UPDATE_REG(0x06, VCOUNT);
						DISPSTAT &= 0xFFFD;
						UPDATE_REG(0x04, DISPSTAT);
						CPUCompareVCOUNT();
					}
					else
					{
						lcdTicks += 272;
						DISPSTAT |= 2;
						UPDATE_REG(0x04, DISPSTAT);
						if (DISPSTAT & 16)
						{
							IF |= 2;
							UPDATE_REG(0x202, IF);
						}
					}

					if (VCOUNT >= 228)
					{
						DISPSTAT &= 0xFFFC;
						UPDATE_REG(0x04, DISPSTAT);
						VCOUNT = 0;
						UPDATE_REG(0x06, VCOUNT);
						CPUCompareVCOUNT();
					}
				}
				else
				{
					int framesToSkip = systemFramesToSkip();

					if (DISPSTAT & 2)
					{
						// if in H-Blank, leave it and move to drawing mode
						VCOUNT++;
						UPDATE_REG(0x06, VCOUNT);

						lcdTicks += 960;
						DISPSTAT &= 0xFFFD;
						if (VCOUNT == 160)
						{
							DISPSTAT |= 1;
							DISPSTAT &= 0xFFFD;
							UPDATE_REG(0x04, DISPSTAT);
							if (DISPSTAT & 0x0008)
							{
								IF |= 1;
								UPDATE_REG(0x202, IF);
							}
							CPUCheckDMA(1, 0x0f);

							systemFrame();

							++gbaFrameCount;
							u32 gbaCurrentTime = systemGetClock();
							if (gbaCurrentTime - gbaLastTime >= 1000)
							{
								systemShowSpeed(int(float(gbaFrameCount) * 100000 / (float(gbaCurrentTime - gbaLastTime) * 60) + .5f));
								gbaLastTime	  = gbaCurrentTime;
								gbaFrameCount = 0;
							}

							++GBASystemCounters.frameCount;
							if (GBASystemCounters.lagged)
							{
								++GBASystemCounters.lagCount;
							}
							GBASystemCounters.laggedLast = GBASystemCounters.lagged;
							GBASystemCounters.lagged	 = true;

							if (cheatsEnabled)
								cheatsCheckKeys(P1 ^ 0x3FF, extButtons);

							extern void VBAOnEnteringFrameBoundary();
							VBAOnEnteringFrameBoundary();

							newFrame = true;

							pauseAfterFrameAdvance = systemPauseOnFrame();

							if (frameSkipCount >= framesToSkip || pauseAfterFrameAdvance)
							{
								systemRenderFrame();
								frameSkipCount = 0;

								bool capturePressed = (extButtons & 2) != 0;
								if (capturePressed && !capturePrevious)
								{
									captureNumber = systemScreenCapture(captureNumber);
								}
								capturePrevious = capturePressed && !pauseAfterFrameAdvance;
							}
							else
							{
								++frameSkipCount;
							}

							if (pauseAfterFrameAdvance)
							{
								systemSetPause(true);
							}
						}

						UPDATE_REG(0x04, DISPSTAT);
						CPUCompareVCOUNT();
					}
					else
					{
						if (frameSkipCount >= framesToSkip || pauseAfterFrameAdvance)
						{
							(*renderLine)();

							switch (systemColorDepth)
							{
							case 16:
							{
								u16 *dest = (u16 *)pix + 242 * (VCOUNT + 1);
								for (int x = 0; x < 240; )
								{
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap16[lineMix[x++] & 0xFFFF];
								}
								// for filters that read past the screen
								*dest++ = 0;
								break;
							}
							case 24:
							{
								u8 *dest = (u8 *)pix + 240 * VCOUNT * 3;
								for (int x = 0; x < 240; )
								{
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;

									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;

									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;

									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
									*((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
									dest += 3;
								}
								break;
							}
							case 32:
							{
								u32 *dest = (u32 *)pix + 241 * (VCOUNT + 1);
								for (int x = 0; x < 240; )
								{
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];

									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
									*dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
								}
								break;
							}
							}
						}
						// entering H-Blank
						DISPSTAT |= 2;
						UPDATE_REG(0x04, DISPSTAT);
						lcdTicks += 272;
						CPUCheckDMA(2, 0x0f);
						if (DISPSTAT & 16)
						{
							IF |= 2;
							UPDATE_REG(0x202, IF);
						}
					}
				}
			}

			if (!stopState)
			{
				if (timer0On)
				{
					if (timer0ClockReload == 1)
					{
						u32 tm0d = TM0D + clockTicks;
						if (tm0d > 0xffff)
						{
							tm0d += timer0Reload;
							timerOverflow |= 1;
							soundTimerOverflow(0);
							if (TM0CNT & 0x40)
							{
								IF |= 0x08;
								UPDATE_REG(0x202, IF);
							}
						}
						TM0D		= tm0d & 0xFFFF;
						timer0Ticks = 0x10000 - TM0D;
						UPDATE_REG(0x100, TM0D);
					}
					else
					{
						timer0Ticks -= clockTicks;
						if (timer0Ticks <= 0)
						{
							timer0Ticks += timer0ClockReload;
							TM0D++;
							if (TM0D == 0)
							{
								TM0D = timer0Reload;
								timerOverflow |= 1;
								soundTimerOverflow(0);
								if (TM0CNT & 0x40)
								{
									IF |= 0x08;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x100, TM0D);
						}
					}
				}

				if (timer1On)
				{
					if (TM1CNT & 4)
					{
						if (timerOverflow & 1)
						{
							TM1D++;
							if (TM1D == 0)
							{
								TM1D += timer1Reload;
								timerOverflow |= 2;
								soundTimerOverflow(1);
								if (TM1CNT & 0x40)
								{
									IF |= 0x10;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x104, TM1D);
						}
					}
					else
					{
						if (timer1ClockReload == 1)
						{
							u32 tm1d = TM1D + clockTicks;
							if (tm1d > 0xffff)
							{
								tm1d += timer1Reload;
								timerOverflow |= 2;
								soundTimerOverflow(1);
								if (TM1CNT & 0x40)
								{
									IF |= 0x10;
									UPDATE_REG(0x202, IF);
								}
							}
							TM1D		= tm1d & 0xFFFF;
							timer1Ticks = 0x10000 - TM1D;
							UPDATE_REG(0x104, TM1D);
						}
						else
						{
							timer1Ticks -= clockTicks;
							if (timer1Ticks <= 0)
							{
								timer1Ticks += timer1ClockReload;
								TM1D++;

								if (TM1D == 0)
								{
									TM1D = timer1Reload;
									timerOverflow |= 2;
									soundTimerOverflow(1);
									if (TM1CNT & 0x40)
									{
										IF |= 0x10;
										UPDATE_REG(0x202, IF);
									}
								}
								UPDATE_REG(0x104, TM1D);
							}
						}
					}
				}

				if (timer2On)
				{
					if (TM2CNT & 4)
					{
						if (timerOverflow & 2)
						{
							TM2D++;
							if (TM2D == 0)
							{
								TM2D += timer2Reload;
								timerOverflow |= 4;
								if (TM2CNT & 0x40)
								{
									IF |= 0x20;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x108, TM2D);
						}
					}
					else
					{
						if (timer2ClockReload == 1)
						{
							u32 tm2d = TM2D + clockTicks;
							if (tm2d > 0xffff)
							{
								tm2d += timer2Reload;
								timerOverflow |= 4;
								if (TM2CNT & 0x40)
								{
									IF |= 0x20;
									UPDATE_REG(0x202, IF);
								}
							}
							TM2D		= tm2d & 0xFFFF;
							timer2Ticks = 0x10000 - TM2D;
							UPDATE_REG(0x108, TM2D);
						}
						else
						{
							timer2Ticks -= clockTicks;
							if (timer2Ticks <= 0)
							{
								timer2Ticks += timer2ClockReload;
								TM2D++;

								if (TM2D == 0)
								{
									TM2D = timer2Reload;
									timerOverflow |= 4;
									if (TM2CNT & 0x40)
									{
										IF |= 0x20;
										UPDATE_REG(0x202, IF);
									}
								}
								UPDATE_REG(0x108, TM2D);
							}
						}
					}
				}

				if (timer3On)
				{
					if (TM3CNT & 4)
					{
						if (timerOverflow & 4)
						{
							TM3D++;
							if (TM3D == 0)
							{
								TM3D += timer3Reload;
								if (TM3CNT & 0x40)
								{
									IF |= 0x40;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x10c, TM3D);
						}
					}
					else
					{
						if (timer3ClockReload == 1)
						{
							u32 tm3d = TM3D + clockTicks;
							if (tm3d > 0xffff)
							{
								tm3d += timer3Reload;
								if (TM3CNT & 0x40)
								{
									IF |= 0x40;
									UPDATE_REG(0x202, IF);
								}
							}
							TM3D		= tm3d & 0xFFFF;
							timer3Ticks = 0x10000 - TM3D;
							UPDATE_REG(0x10C, TM3D);
						}
						else
						{
							timer3Ticks -= clockTicks;
							if (timer3Ticks <= 0)
							{
								timer3Ticks += timer3ClockReload;
								TM3D++;

								if (TM3D == 0)
								{
									TM3D = timer3Reload;
									if (TM3CNT & 0x40)
									{
										IF |= 0x40;
										UPDATE_REG(0x202, IF);
									}
								}
								UPDATE_REG(0x10C, TM3D);
							}
						}
					}
				}
			}
			// we shouldn't be doing sound in stop state, but we lose synchronization
			// if sound is disabled, so in stop state, soundTick will just produce
			// mute sound
			soundTicks -= clockTicks;
			if (soundTicks < 1)
			{
				soundTick();
				soundTicks += SOUND_CLOCK_TICKS;
			}
			timerOverflow = 0;

#ifdef PROFILING
			profilingTicks -= clockTicks;
			if (profilingTicks <= 0)
			{
				profilingTicks += profilingTicksReload;
				if (profilBuffer && profilSize)
				{
					u16 *b	= (u16 *)profilBuffer;
					int	 pc = ((reg[15].I - profilLowPC) * profilScale) / 0x10000;
					if (pc >= 0 && pc < profilSize)
					{
						b[pc]++;
					}
				}
			}
#endif

			ticks		-= clockTicks;
			cpuLoopTicks = CPUUpdateTicks();

			// FIXME: it is too bad that it is still not determined whether the loop can be exited at this point
			if (cpuDmaTicksToUpdate > 0)
			{
				clockTicks = cpuSavedTicks;
				if (clockTicks > cpuDmaTicksToUpdate)
					clockTicks = cpuDmaTicksToUpdate;
				cpuDmaTicksToUpdate -= clockTicks;
				if (cpuDmaTicksToUpdate < 0)
					cpuDmaTicksToUpdate = 0;
				goto updateLoop;    // this is evil
			}

			if (IF && (IME & 1) && armIrqEnable)
			{
				int res = IF & IE;
				if (stopState)
					res &= 0x3080;
				if (res)
				{
					if (intState)
					{
						CPUInterrupt();
						intState = false;
						if (holdState)
						{
							holdState = false;
							stopState = false;
						}
					}
					else
					{
						if (!holdState)
						{
							intState	  = true;
							cpuLoopTicks  = 5;
							cpuSavedTicks = 5;
						}
						else
						{
							CPUInterrupt();
							if (holdState)
							{
								holdState = false;
								stopState = false;
							}
						}
					}
				}
			}

			if (useOldFrameTiming)
			{
				if (ticks <= 0)
				{
					newFrame = true;
					break;
				}
			}
			else if (newFrame)
			{
				// FIXME: it should be enough to use frameBoundary only if there were no need for supporting the old timing
				// but is there still any GBA .vbm that uses the old timing?
///				extern void VBAOnEnteringFrameBoundary();
///				VBAOnEnteringFrameBoundary();

				break;
			}
		}
	}
}

struct EmulatedSystem GBASystem =
{
	// emuMain
	CPULoop,
	// emuReset
	CPUReset,
	// emuCleanUp
	CPUCleanUp,
	// emuReadBattery
	CPUReadBatteryFile,
	// emuWriteBattery
	CPUWriteBatteryFile,
	// emuReadBatteryFromStream
	CPUReadBatteryFromStream,
	// emuWriteBatteryToStream
	CPUWriteBatteryToStream,
	// emuReadState
	CPUReadState,
	// emuWriteState
	CPUWriteState,
	// emuReadStateFromStream
	CPUReadStateFromStream,
	// emuWriteStateToStream
	CPUWriteStateToStream,
	// emuReadMemState
	CPUReadMemState,
	// emuWriteMemState
	CPUWriteMemState,
	// emuWritePNG
	CPUWritePNGFile,
	// emuWriteBMP
	CPUWriteBMPFile,
	// emuUpdateCPSR
	CPUUpdateCPSR,
	// emuHasDebugger
	true,
	// emuCount
#ifdef FINAL_VERSION
	250000,
#else
	5000,
#endif
};

// is there a reason to use more than one set of counters?
EmulatedSystemCounters &GBASystemCounters = systemCounters;

/*
   EmulatedSystemCounters GBASystemCounters =
   {
    // frameCount
    0,
    // lagCount
    0,
    // lagged
    true,
    // laggedLast
    true,
   };
 */


#undef CPU_BREAK_LOOP
#undef CPU_BREAK_LOOP2
