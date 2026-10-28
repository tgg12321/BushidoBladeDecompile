/* func_80037D14, a state machine over _card_info / _card_load / _card_clear.
 * .text 0x80037D14 (ROM 0x28514). Start boundary: LEGACY (a tooling split, no
 * evidence either way). */
#include "common.h"
#include "bb2.h"
#include <psxsdk/libcard.h>

/* memcard_Format's (28708.c) path format. It sits in this file's .rodata,
 * before func_80037D14's jump table, whose .align 3 supplies the zero bytes
 * after it. */
const char D_800109C8[] = "bu%1d%1d:";

extern s32 D_800A31E8;
extern s32 D_800A31EC;
extern s32 D_800A3890;
extern s32 D_800A37F4;

s32 func_80037D14(s32 port, s32 card) {
    s32 channel = (port << 4) + card;
    s32 ev;

    switch (D_800A31EC) {
    case 0:
        _card_info(channel);
        D_800A31EC = 1;
        g_memcard_poll_count = 0;
        D_800A3890 = 0;
        break;
    case 1:
        ev = memcard_PollSwEventsTimeout();
        if (ev == 0) {
            break;
        }
        switch (ev) {
        case 1:
            D_800A37F4 = ev;
            if (D_800A31E8 != 0) {
                D_800A31EC = 4;
            } else {
                D_800A31EC = 2;
            }
            break;
        case 3:
            D_800A37F4 = -1;
            D_800A31EC = 4;
            D_800A31E8 = 0;
            break;
        case 4:
            D_800A37F4 = 2;
            memcard_AckHwEvents();
            _card_clear(channel);
            memcard_WaitHwEvent();
            D_800A31EC = 2;
            D_800A31E8 = 0;
            break;
        case 2:
        default:
            D_800A37F4 = -3;
            D_800A31EC = 4;
            D_800A31E8 = 0;
            break;
        }
        break;
    case 2:
        memcard_AckSwEvents();
        _card_load(channel);
        D_800A31EC = 3;
        g_memcard_poll_count = 0;
        break;
    case 3:
        ev = memcard_PollSwEventsTimeout();
        if (ev == 0) {
            break;
        }
        D_800A31EC = 4;
        switch (ev) {
        case 1:
            D_800A31E8 = ev;
            break;
        case 3:
            D_800A37F4 = -1;
            D_800A31E8 = 0;
            break;
        case 4:
            D_800A37F4 = -2;
            D_800A31E8 = 0;
            break;
        case 2:
        default:
            D_800A37F4 = -3;
            D_800A31E8 = 0;
            break;
        }
        break;
    case 4:
        D_800A3890 = D_800A37F4;
        D_800A31EC = 0;
        break;
    }
    return D_800A3890;
}

/* Q65: initialized small data (.sdata), in address order. */
s32 D_800A31E8 = 0;
s32 D_800A31EC = 0;
/* Q65: tentative definitions (COMMON) of the small data reached gp-relative. */
s32 D_800A37F4;
s32 D_800A3890;
s32 g_memcard_poll_count;
