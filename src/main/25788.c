/* The 4 game functions after func_80034708. .text 0x80034F88 (ROM 0x25788).
 * Start boundary: G8 (the end of 24F08.c's -G8 unit). */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"
#include "bb2_const.h"

/* Copies the record at func_80077D00() into the FileRecord D_80106A50:
 * flags bits 0-2 are cleared and re-set from p->unk20_0..2, then the three
 * colour bytes at p->unk17 are copied to D_80106A50.color. */
void func_80034F88(void) {
    Unk8009BD24Block *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block 0's own address object (a second handle on
         * D_80106A50.flags) for register-allocation priority; GCC 2.7.2 gives
         * one hard register per allocno, so blocks 1/2 need their own. */
        u8 *q = &D_80106A50.flags;

        u = *q;
        u = u & 0xF8;
        *q = u;
        /* FAKE: cse2 value invalidator: stops cse2 forwarding the sb into the
         * following lbu; without it: score 27. */
        u = 0;
        u = *q;
        c = p->unk20_0;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
        {
            /* FAKE: the address object for flag blocks 1 and 2 (one hard
             * register per allocno, no live-range splitting); q reused or the
             * global direct: score 39. */
            u8 *r = &D_80106A50.flags;

            v = *r;
            /* FAKE: bits 1 and 2 read at their word position (<< 1 / << 2; c is
               only tested); read plain, each extract adds an srl: score 4;
               tested directly: 35. */
            c = p->unk20_1 << 1;
            if (c) {
                c = v | 2;
            } else {
                c = v;
            }
            *r = c;

            r = &D_80106A50.flags;
            v = *r;
            c = p->unk20_2 << 2;
            if (c) {
                c = v | 4;
            } else {
                c = v;
            }
            *r = c;
        }

        /* FAKE: the copy loop's counter is staged through the dead pointer q;
         * the loop-weighted references lift its allocation priority so it
         * takes $v1 and block 0's value lands in $a0, as in the target. No
         * instruction is added; a separate s32 counter: score 15. */
        for (q = 0; (s32)q < 3; q++) {
            c = p->unk17[(s32)q];
            D_80106A50.color[(s32)q] = c;
        }
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
        s32 lv = (&D_8008D55C)[s[0]];
        D_80102778.unk_4[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == new_var || (u32)(lv - 18) < 2 ||
            (s8)lv == new_var2) {
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
        s32 sel;

        D_800A389B = p->unk14_10 + 3;
        D_800A36CC = D_8008EC30[p->unk14_12];
        sel = 1;
        if (p->unk14_14) {
            sel = 2;
        }
        D_800A37F8 = sel;
        s = &D_801027D8;
        D_800A38E1 = p->unk14_15;
        {
            s32 j = 0;
            u8 *dst_d = s;
            u8 *dst_a = &D_801027A0;
            do {
                s32 k = 0;
                u8 *da = dst_d;
                u8 *db = dst_a;
                s32 off = j << 1;
            loop_inner: {
                u8 *pp = (u8 *)p + off;
                *db = (&D_8008D55C)[pp[0]];
                off += 10;
                k++;
                *da = pp[1];
                db++;
                da++;
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
    /* FAKE: one hoisted pointer to the three 8-byte clock records
     * (D_80106A50.times) gives the target's single $a2 record cursor; indexing
     * D_80106A50.times[i] at each use creates a second address movable
     * (score 24). */
    TimeRec *base;
    s32 i;

    p = func_80077D00();
    i = 0;
    p->unk20_0 = D_80106A50.flags & 1;
    p->unk20_1 = (D_80106A50.flags >> 1) & 1;
    p->unk20_2 = (D_80106A50.flags >> 2) & 1;
    src = D_80106A50.color;
    for (; i < 3; i++) {
        p->unk17[i] = *src;
        p->unk1D[i] = *src;
        src++;
    }
    base = D_80106A50.times;
    for (i = 0; i < 3; i++) {
        /* FAKE: the clock fields and the raw record byte are each staged
         * through a named intermediate; they raise loop 2's insn count from 55
         * to 59, past loop.c's hoist threshold (58), so the /1800 magic
         * constant's lui/ori stays in the loop as in the target. combine folds
         * the copies away; direct stores: score 15. */
        u8 mn;
        u8 sc;
        u8 hs;
        s32 t;

        mn = base[i].unk_4 / 1800;
        p->unk21[i].unk0 = mn;
        sc = (base[i].unk_4 / 30) % 60;
        p->unk21[i].unk1 = sc;
        hs = (base[i].unk_4 % 30) * 100 / 30;
        p->unk21[i].unk2 = hs;
        t = base[i].unk_0;
        p->unk21[i].unk3 = t;
    }
}

void func_80035430(void) {}
