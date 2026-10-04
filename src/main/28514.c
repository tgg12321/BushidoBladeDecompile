/* func_80037D14, a state machine over _card_info / _card_load / _card_clear. .text 0x80037D14 (ROM
 * 0x28514). Start boundary: LEGACY (a tooling split, no evidence either way). */
#include "common.h"
#include "bb2.h"
#include <psxsdk/libcard.h>

/* memcard_Format's (28708.c) path format. It sits in this file's .rodata, before
 * func_80037D14's jump table, whose .align 3 supplies the zero bytes after it. */
const char D_800109C8[] = "bu%1d%1d:";

extern s32 D_800A31E8;
extern s32 D_800A31EC;
extern s32 D_800A3890;
extern s32 D_800A3924;
extern s32 D_800A37F4;


s32 func_80037D14(s32 arg0, s32 arg1) {
    s32 p = (arg0 << 4) + arg1;
    s32 v1;
    s32 v0;

    switch (D_800A31EC) {
    case 0:
        _card_info(p);
        D_800A31EC = 1;
        D_800A3924 = 0;
        D_800A3890 = 0;
        break;
    case 1: {
        v1 = memcard_PollSwEventsTimeout();
        if (v1 == 0) break;
        if (v1 == 2) goto c1_e20;
        if (v1 < 3) {
            if (v1 == 1) goto c1_dc4;
            v0 = -3; goto c1_e24;
        }
        if (v1 == 3) goto c1_dec;
        if (v1 == 4) goto c1_df4;
        v0 = -3; goto c1_e24;
    c1_dc4:
        v0 = D_800A31E8;
        D_800A37F4 = v1;
        if (v0 == 0) goto c1_de0;
        D_800A31EC = 4;
        break;
    c1_de0:
        D_800A31EC = 2;
        break;
    c1_dec:
        v0 = -1;
        goto c1_e24;
    c1_df4:
        D_800A37F4 = 2;
        memcard_AckHwEvents();
        _card_clear(p);
        memcard_WaitHwEvent();
        D_800A31EC = 2;
        D_800A31E8 = 0;
        break;
    c1_e20:
        v0 = -3;
    c1_e24:
        D_800A37F4 = v0;
        D_800A31EC = 4;
        D_800A31E8 = 0;
        break;
    }
    case 2:
        memcard_AckSwEvents();
        _card_load(p);
        D_800A31EC = 3;
        D_800A3924 = 0;
        break;
    case 3: {
        v1 = memcard_PollSwEventsTimeout();
        if (v1 == 0) break;
        D_800A31EC = 4;
        if (v1 == 2) goto c3_ecc;
        if (v1 < 3) {
            if (v1 == 1) goto c3_eb8;
            v0 = -3; goto c3_ed0;
        }
        if (v1 == 3) goto c3_ec4;
        if (v1 == 4) goto c3_ea8;
        v0 = -3; goto c3_ed0;
    c3_eb8:
        D_800A31E8 = v1;
        break;
    c3_ec4:
        v0 = -1;
        goto c3_ed0;
    c3_ea8:
        v0 = -2;
        goto c3_ed0;
    c3_ecc:
        v0 = -3;
    c3_ed0:
        D_800A37F4 = v0;
        D_800A31E8 = 0;
        break;
    }
    case 4:
        D_800A3890 = D_800A37F4;
        D_800A31EC = 0;
        break;
    }
    return D_800A3890;
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A31E8 = 0;
s32 D_800A31EC = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 D_800A37F4;
s32 D_800A3890;
s32 D_800A3924;
