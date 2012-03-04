#include "../common/System.h"
#include "Flash.h"
#include "Sram.h"

u8 sramRead(u32 address)
{
	return flashSaveMemory[address & 0xFFFF];
}

void sramWrite(u32 address, u8 byte)
{
	flashSaveMemory[address & 0xFFFF] = byte;
	systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
}

