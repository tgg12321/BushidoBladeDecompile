/* PsyQ 4.0 LIBCD EVENT: CdInit and its default callbacks def_cbsync,
 * def_cbready, def_cbread. .text 0x8007FF7C..0x8008008C, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libcd_internal.h"
#include <psxsdk/libapi.h>

/* .rodata 0x8001605C..0x80016074: this module's strings (Q106 D4: every reader
 * is in this file). */

const char g_str_cdinit_fail[24] = "CdInit: Init failed\n\0\0\0\0";

extern s32 printf();

void def_cbsync(u8 intr, u8 *result);
void def_cbready(u8 intr, u8 *result);
void def_cbread(u8 intr, u8 *result);

s32 CdInit(void) {
    s32 retries = 4;
    while (CdReset(1) != 1) {
        if (--retries == -1) {
            printf(g_str_cdinit_fail);
            return 0;
        }
    }
    CdSyncCallback(def_cbsync);
    CdReadyCallback(def_cbready);
    CdReadCallback(def_cbread);
    CdReadMode(0);
    return 1;
}

void def_cbsync(u8 intr, u8 *result) { DeliverEvent(0xF0000003, 0x20); }

void def_cbready(u8 intr, u8 *result) { DeliverEvent(0xF0000003, 0x40); }

void def_cbread(u8 intr, u8 *result) { DeliverEvent(0xF0000003, 0x40); }
