/* PsyQ 4.0 LIBCD EVENT: CdInit and its default callbacks def_cbsync, def_cbready, def_cbread. .text
 * 0x8007FF7C..0x8008008C, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "common.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libgpu/sys.c, ex display.c). */
extern void DeliverEvent(s32, s32);
extern s32 printf();

extern s32 CdReset(s32);
extern s32 CdSyncCallback(s32);
extern s32 CdReadCallback(s32);
extern s32 CdReadMode(s32);
void def_cbsync(void);
void def_cbready(void);
void def_cbread(void);



extern const char g_str_cdinit_fail[];

s32 CdInit(void) {
    s32 retries = 4;
loop:
    if (CdReset(1) != 1) {
        retries--;
        if (retries != -1) goto loop;
        printf(g_str_cdinit_fail);
        return 0;
    }
    CdSyncCallback((s32)&def_cbsync);
    CdReadyCallback((s32)&def_cbready);
    CdReadCallback((s32)&def_cbread);
    CdReadMode(0);
    return 1;
}

void def_cbsync(void) {
    DeliverEvent(0xF0000003, 0x20);
}

void def_cbready(void) {
    DeliverEvent(0xF0000003, 0x40);
}

void def_cbread(void) {
    DeliverEvent(0xF0000003, 0x40);
}
