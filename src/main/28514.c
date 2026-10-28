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

extern s32 g_memcard_card_loaded;
extern s32 g_memcard_info_load_state;
extern s32 D_800A3890;
extern s32 g_memcard_info_load_pending_result;

s32 func_80037D14(s32 port, s32 card) {
    s32 channel = (port << 4) + card;
    s32 ev;

    switch (g_memcard_info_load_state) {
    case 0:
        _card_info(channel);
        g_memcard_info_load_state = 1;
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
            g_memcard_info_load_pending_result = ev;
            if (g_memcard_card_loaded != 0) {
                g_memcard_info_load_state = 4;
            } else {
                g_memcard_info_load_state = 2;
            }
            break;
        case 3:
            g_memcard_info_load_pending_result = -1;
            g_memcard_info_load_state = 4;
            g_memcard_card_loaded = 0;
            break;
        case 4:
            g_memcard_info_load_pending_result = 2;
            memcard_AckHwEvents();
            _card_clear(channel);
            memcard_WaitHwEvent();
            g_memcard_info_load_state = 2;
            g_memcard_card_loaded = 0;
            break;
        case 2:
        default:
            g_memcard_info_load_pending_result = -3;
            g_memcard_info_load_state = 4;
            g_memcard_card_loaded = 0;
            break;
        }
        break;
    case 2:
        memcard_AckSwEvents();
        _card_load(channel);
        g_memcard_info_load_state = 3;
        g_memcard_poll_count = 0;
        break;
    case 3:
        ev = memcard_PollSwEventsTimeout();
        if (ev == 0) {
            break;
        }
        g_memcard_info_load_state = 4;
        switch (ev) {
        case 1:
            g_memcard_card_loaded = ev;
            break;
        case 3:
            g_memcard_info_load_pending_result = -1;
            g_memcard_card_loaded = 0;
            break;
        case 4:
            g_memcard_info_load_pending_result = -2;
            g_memcard_card_loaded = 0;
            break;
        case 2:
        default:
            g_memcard_info_load_pending_result = -3;
            g_memcard_card_loaded = 0;
            break;
        }
        break;
    case 4:
        D_800A3890 = g_memcard_info_load_pending_result;
        g_memcard_info_load_state = 0;
        break;
    }
    return D_800A3890;
}

/* Q65: initialized small data (.sdata), in address order. */
s32 g_memcard_card_loaded = 0;
s32 g_memcard_info_load_state = 0;
/* Q65: tentative definitions (COMMON) of the small data reached gp-relative. */
s32 g_memcard_info_load_pending_result;
s32 D_800A3890;
s32 g_memcard_poll_count;
