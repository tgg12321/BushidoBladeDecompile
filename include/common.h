#ifndef COMMON_H
#define COMMON_H

/* Basic types matching PsyQ/GCC 2.7.2 on MIPS */
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;

/* NULL */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* Boolean */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif /* COMMON_H */
