#ifndef SYSTEM_H
#define SYSTEM_H

/* System - IRQ, file I/O, memory card */

#include "common.h"

/* Named globals */
extern u8 g_file_disc_type;
extern u16 g_file_vram_timer;

/* Functions */
extern void irq_DisableInterrupts(void);

#endif /* SYSTEM_H */
