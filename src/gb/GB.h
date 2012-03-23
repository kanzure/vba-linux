#ifndef VBA_GB_H
#define VBA_GB_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

typedef union
{
	struct
	{
#ifdef WORDS_BIGENDIAN
		u8 B1, B0;
#else
		u8 B0, B1;
#endif
	} B;
	u16 W;
} gbRegister;

extern bool gbLoadRom(const char *);
extern int gbEmulate(int);
extern bool gbIsGameboyRom(const char *);
extern void gbSoundReset();
extern void gbSoundSetQuality(int);
extern void gbReset(bool userReset = false);
extern void gbCleanUp();
extern bool gbWriteBatteryFile(const char *);
extern bool gbWriteBatteryFile(const char *, bool);
extern bool gbWriteBatteryToStream(gzFile);
extern bool gbReadBatteryFile(const char *);
extern bool gbReadBatteryFromStream(gzFile);
extern bool gbWriteSaveState(const char *);
extern bool gbWriteMemSaveState(char *, int);
extern bool gbReadSaveState(const char *);
extern bool gbReadMemSaveState(char *, int);
extern bool gbReadSaveStateFromStream(gzFile);
extern bool gbWriteSaveStateToStream(gzFile);
extern void gbSgbRenderBorder();
extern bool gbWritePNGFile(const char *);
extern bool gbWriteBMPFile(const char *);
extern bool gbReadGSASnapshot(const char *);

extern int getRamSize();
extern int getRomSize();

extern void storeMemory(int32 *);
extern void writeMemory(int32 *);

extern void storeRam(int32 *);
extern void storeRom(int32 *);
extern void writeRom(int32 *);

extern void storeWRam(int32 *);
extern void storeVRam(int32 *);
extern void storeRegisters(int32 *);
extern void setRegisters(int32 *);

extern long gbWriteMemSaveStatePos(char *, int);

extern struct EmulatedSystem GBSystem;
extern struct EmulatedSystemCounters &GBSystemCounters;

#endif // VBA_GB_H
