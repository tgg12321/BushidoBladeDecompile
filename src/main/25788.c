/* The 4 game functions after func_80034708. .text 0x80034F88 (ROM 0x25788).
 * Start boundary: G8 (the end of 24F08.c's -G8 unit). */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* Copies the record at func_80077D00() into the FileRecord D_80106A50:
 * flags bits 0-2 are cleared and re-set from p->unk20_0..2, then the three
 * bytes at p->unk17 are copied to D_80106A50.frec20. */
void func_80034F88(void) {
    Unk8009BD24Block *p;
    s32 i;

    p = func_80077D00();
    D_80106A50.flags &= ~7;
    D_80106A50.flags |= p->unk20_0 ? 1 : 0;
    D_80106A50.flags |= p->unk20_1 ? 2 : 0;
    D_80106A50.flags |= p->unk20_2 ? 4 : 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.frec20[i] = p->unk17[i];
    }
}

void func_8003504C(void) {
    Unk8009BD24Block *p;
    s32 i;
    u8 *s;
    /* FAKE: 5 and 20 held in locals so their `li`s are pre-loop insns
       scheduled ahead of the walker copy `s = p`; as literals they become loop
       movables emitted behind the two p-copies; literals: score 4. */
    s32 new_var;
    s32 new_var2;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    new_var = 5;
    new_var2 = 20;
    s = (u8 *)p;

    do {
        s32 var_7 = (&D_8008D55C)[s[0]];
        D_80102778.unk_4[i] = var_7;
        if ((u32)(var_7 - 3) < 2 || (s8)var_7 == new_var ||
            (u32)(var_7 - 18) < 2 || (s8)var_7 == new_var2) {
            if ((s8)D_80102778.unk_D == 0) {
                D_80102778.unk_4[i] = D_80102778.unk_4[i] - 3;
            }
        }
        tmp = s[1];
        s += 10;
        D_80102778.unk_4[4 + i] = 0;
        D_80102778.unk_4[2 + i] = tmp;
        i++;
    } while (i < 2);

    D_80102778.unk_C = p->unk14_4;
    D_80102778.unk_E = p->unk20_3;
    D_800A36F6 = 0;
    val = D_80102778.unk_D;

    if (val == 2) {
        D_800A389A = p->unk14_17;
        D_800A3788 = p->unk14_18;
    } else if (val == 5) {
        s32 var_8;

        g_fight_match_count = p->unk14_10 + 3;
        D_800A36CC = D_8008EC30[p->unk14_12];
        var_8 = 1;
        if (p->unk14_14) {
            var_8 = 2;
        }
        g_fight_win_target = var_8;
        s = D_801027D8[0];
        D_800A38E1 = p->unk14_15;
        {
            s32 j = 0;
            u8 *dst_d = s;
            u8 *dst_a = D_801027A0[0];
            do {
                s32 k = 0;
                u8 *var_13 = dst_d;
                u8 *var_14 = dst_a;
                s32 off = j << 1;
            loop_inner: {
                u8 *var_16 = (u8 *)p + off;
                *var_14 = (&D_8008D55C)[var_16[0]];
                off += 10;
                k++;
                *var_13 = var_16[1];
                var_14++;
                var_13++;
            }
                if (k < 2)
                    goto loop_inner;
                dst_d += 2;
                j++;
                dst_a += 2;
            } while (j < 5);
        }
    }

    func_800344B4();
}

void func_80035280(void) {
    Unk8009BD24Block *p;
    u8 *src;
    /* the three 8-byte clock records */
    TimeRec *times;
    s32 i;

    p = func_80077D00();
    i = 0;
    p->unk20_0 = D_80106A50.flags & 1;
    p->unk20_1 = (D_80106A50.flags >> 1) & 1;
    p->unk20_2 = (D_80106A50.flags >> 2) & 1;
    src = D_80106A50.frec20;
    for (; i < 3; i++) {
        p->unk17[i] = *src;
        p->unk1D[i] = *src;
        src++;
    }
    times = D_80106A50.times;
    for (i = 0; i < 3; i++) {
        /* FAKE: the clock fields and the raw record byte are each staged
         * through a named intermediate (var_4 / var_5 / var_6 / t); they raise
         * loop 2's insn count from 55 to 59, past loop.c's hoist threshold
         * (58), so the /1800 magic constant's lui/ori stays in the loop as in
         * the target. combine folds the copies away; direct stores: score 15
         * (Q129). */
        u8 var_4;
        u8 var_5;
        u8 var_6;
        s32 t;

        var_4 = times[i].unk_4 / 1800;
        p->unk21[i].unk0 = var_4;
        var_5 = (times[i].unk_4 / 30) % 60;
        p->unk21[i].unk1 = var_5;
        var_6 = (times[i].unk_4 % 30) * 100 / 30;
        p->unk21[i].unk2 = var_6;
        t = times[i].unk_0;
        p->unk21[i].unk3 = t;
    }
}

void func_80035430(void) {}
