#ifndef VBA_PORT_H
#define VBA_PORT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ctime>

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char bool8;

//#ifdef HAVE_STDINT_H
#include <stdint.h>

// RLM combatibility junk

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;
typedef intptr_t pint;

#ifndef WIN32

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define _MAX_DIR PATH_MAX
#define _MAX_DRIVE 1
#define _MAX_FNAME PATH_MAX
#define _MAX_EXT PATH_MAX
#define _MAX_PATH PATH_MAX

#define ZeroMemory(a, b) memset((a), 0, (b))

void _makepath(char *path, const char *drive, const char *dir,
               const char *fname, const char *ext);
void _splitpath(const char *path, char *drive, char *dir, char *fname,
                char *ext);
#else /* WIN32 */
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif


// for consistency
static inline u8 swap8(u8 v)
{
	return v;
}

// swaps a 16-bit value
static inline u16 swap16(u16 v)
{
	return (v<<8)|(v>>8);
}

// swaps a 32-bit value
static inline u32 swap32(u32 v)
{
	return (v<<24)|((v<<8)&0xff0000)|((v>>8)&0xff00)|(v>>24);
}

#define READ8LE(x) \
    *((u8 *)x)

#define WRITE8LE(x, v) \
    *((u8 *)x) = (v)

#ifdef WORDS_BIGENDIAN
#if defined(__GNUC__) && defined(__ppc__)

#define READ16LE(base) \
    ({ unsigned short lhbrxResult;       \
       __asm__("lhbrx %0, 0, %1" : "=r" (lhbrxResult) : "r" (base) : "memory"); \
       lhbrxResult; })

#define READ32LE(base) \
    ({ unsigned long lwbrxResult; \
       __asm__("lwbrx %0, 0, %1" : "=r" (lwbrxResult) : "r" (base) : "memory"); \
       lwbrxResult; })

#define WRITE16LE(base, value) \
    __asm__("sthbrx %0, 0, %1" : : "r" (value), "r" (base) : "memory")

#define WRITE32LE(base, value) \
    __asm__("stwbrx %0, 0, %1" : : "r" (value), "r" (base) : "memory")

#else
#define READ16LE(x) \
    swap16(*((u16 *)(x)))
#define READ32LE(x) \
    swap32(*((u32 *)(x)))
#define WRITE16LE(x, v) \
    *((u16 *)x) = swap16((v))
#define WRITE32LE(x, v) \
    *((u32 *)x) = swap32((v))
#endif
#else
#define READ16LE(x) \
    *((u16 *)x)
#define READ32LE(x) \
    *((u32 *)x)
#define WRITE16LE(x, v) \
    *((u16 *)x) = (v)
#define WRITE32LE(x, v) \
    *((u32 *)x) = (v)
#endif

#ifndef CTASSERT
#define CTASSERT(x)  typedef char __assert ## y[(x) ? 1 : -1];
#endif

#endif // VBA_PORT_H
