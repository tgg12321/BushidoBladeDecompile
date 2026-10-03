#ifndef PSXSDK_LIBC_H
#define PSXSDK_LIBC_H

/* PsyQ C library entry points (Sony's libc.h / ctype.h / setjmp.h; SOTN include/psxsdk/libc.h),
 * spelled as BB2's code already spells them: the LIBC2 C definitions in src/main/psxsdk/libc2/,
 * and for setjmp (an asm module) its caller's declaration. */

#include "common.h"

extern u8 toupper(u8);
extern u8 tolower(u8);
extern s32 strlen(u8 *);
extern u8 *strcpy(u8 *, u8 *);
extern u8 *memchr(u8 *, s32, s32);
extern void srand(s32);
extern s32 sprintf(char *, char *, ...);
extern s32 setjmp(s32 *);

#endif /* PSXSDK_LIBC_H */
