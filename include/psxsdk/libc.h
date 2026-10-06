#ifndef PSXSDK_LIBC_H
#define PSXSDK_LIBC_H

/* PsyQ C library entry points (Sony's stdio.h / stdlib.h / string.h / memory.h / ctype.h /
 * setjmp.h; SOTN include/psxsdk/libc.h). Each prototype agrees with its LIBC2 definition in
 * src/main/psxsdk/libc2/ (setjmp: an asm module); where that differs from PsyQ's header
 * spelling the entry carries a PsyQ: note. */

#include "common.h"

extern char toupper(char);
extern char tolower(char);
extern s32 strlen(char *);         /* PsyQ: int strlen() -- char * in its comment */
extern u8 *strcpy(u8 *, u8 *);     /* PsyQ: char *strcpy() -- char *, char * in its comment */
extern u8 *memchr(u8 *, s32, s32); /* PsyQ: void *memchr(unsigned char *, unsigned char, int) */
extern void *memmove(u8 *, u8 *, s32);
extern s32 rand(void);
extern void srand(u32);
extern s32 sprintf(char *, char *, ...);
extern void puts(char *);
extern s32 setjmp(s32 *);          /* PsyQ: int setjmp(jmp_buf) */

#endif /* PSXSDK_LIBC_H */
