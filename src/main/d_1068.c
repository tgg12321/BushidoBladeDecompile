/* Data-only file: one zero word at .rodata 0x80010868 (ROM 0x1068), between
 * 24BF0.o's and menu_config.o's .rodata. Layout debt (Q101/Q102): one object
 * holding both files' rodata would emit it as the .align 3 pad of
 * menu_config.c's first jump table, but menu_config.c is cc1 -G8 by proof and
 * 24BF0.c is not, so no byte-neutral fold exists. */
#include "common.h"

static const u32 _bb2_101C_pre_lead = 0;
