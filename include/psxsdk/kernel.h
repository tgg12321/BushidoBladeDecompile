#ifndef PSXSDK_KERNEL_H
#define PSXSDK_KERNEL_H

/* PsyQ kernel types (Sony's kernel.h, which holds struct EXEC; SOTN declares
 * EXEC in include/psxsdk/libapi.h). */

#include "common.h"

/* PsyQ libapi struct EXEC: the PS-EXE header body (0x3C bytes, 0x10 into the
 * image). */
typedef struct EXEC {
    u32 pc0, gp0;
    u32 t_addr, t_size;
    u32 d_addr, d_size;
    u32 b_addr, b_size;
    u32 s_addr, s_size;
    u32 sp, fp, gp, ret, base;
} EXEC;

#endif /* PSXSDK_KERNEL_H */
