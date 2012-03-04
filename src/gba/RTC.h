#ifndef VBA_RTC_H
#define VBA_RTC_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern u16 rtcRead(u32 address);
extern bool rtcWrite(u32 address, u16 value);
extern void rtcEnable(bool);
extern bool rtcIsEnabled();
extern void rtcReset();

extern void rtcReadGame(gzFile gzFile);
extern void rtcSaveGame(gzFile gzFile);

#endif // VBA_RTC_H
