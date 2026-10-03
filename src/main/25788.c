/* The 4 game functions after func_80034708. .text 0x80034F88 (ROM 0x25788). Start boundary: G8
 * (the end of 24F08.c's -G8 unit); the last file of the EXPAND_LB run. */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"
#include "bb2_const.h"

extern s32 *func_80077D00(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
extern void func_800344B4(void);

/* Copies the record at func_80077D00() into the FileRecord D_80106A50
 * (include/game.h): flags bits 0-2 are cleared and re-set from p[8] bits 0-2,
 * then the three colour bytes at p+0x17 are copied to D_80106A50.color.
 * The register allocation is a global.c priority fit
 * (floor_log2(nrefs) * nrefs * 10000 / live_length); see the FAKEs below. */
void func_80034F88(void) {
    s32 *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block 0's own address object (a second C handle on
         * D_80106A50.flags), mechanism: global.c allocation priority
         * floor_log2(nrefs)*nrefs*10000/live_length -- blocks 1 and 2 cannot be
         * reached from this handle because global.c:1275 assigns exactly one
         * hard register per allocno and GCC 2.7.2 does no live-range splitting. */
        u8 *q = &D_80106A50.flags;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = 0; /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards
                * the sb into the following lbu only while the stored value's
                * pseudo still holds it. */
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
        {
            /* FAKE: the address object for flag blocks 1 and 2, mechanism:
             * global.c:1275 assigns exactly one hard register per allocno and
             * GCC 2.7.2 does no live-range splitting, so blocks 1/2 cannot be
             * reached from the block-0 object. */
            u8 *r = &D_80106A50.flags;

            v = *r;
            c = p[8] & 2;
            if (c) {
                c = v | 2;
            } else {
                c = v;
            }
            *r = c;

            r = &D_80106A50.flags;
            v = *r;
            c = p[8] & 4;
            if (c) {
                c = v | 4;
            } else {
                c = v;
            }
            *r = c;
        }

        /* FAKE: the copy loop's counter is staged through q, whose pointer
         * value is dead from block 0's store above and is never read again,
         * mechanism: flow.c counts REG_N_REFS per RTL insn weighted by loop
         * depth, so the loop's eleven counter references lift this allocno from
         * 5 refs / pri 3571 to 16 refs / pri 30476 and global.c seats it in $v1
         * before block 0's value allocno (pri 17500) is considered, which sends
         * that value to $a0 as the target has it.  Both values are real and
         * used; the loop adds no instruction anywhere in the function. Other
         * byte-neutral reference lifts (duplicated store into arms, split
         * reads, merged mask) do not reach the target seating. */
        for (q = 0; (s32)q < 3; q++) {
            c = *((u8 *)p + (s32)q + 0x17);
            D_80106A50.color[(s32)q] = c;
        }
    }
}
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *s;
    /* FAKE: 5 and 20 held in locals so their `li`s are pre-loop SOURCE insns
       whose LUIDs are lower than the walker copy `s = p`; mechanism: sched.c
       rank_for_schedule's INSN_LUID tie-break inside sched1's backward list
       schedule (written as literals they are loop.c movables, and move_movables
       inserts every movable after ALL pre-loop statements, which emits them
       behind the two p-copies). */
    s32 new_var;
    s32 new_var2;
    s32 *q;
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
        if ((u32)(lv - 3) < 2 || (s8)lv == new_var || (u32)(lv - 18) < 2 || (s8)lv == new_var2) {
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

    D_80102778.unk_C = ((u32)p[5] >> 4) & 0x3F;
    q = &p[8];
    D_80102778.unk_E = ((u32)*q >> 3) & 1;
    D_800A36F6 = 0;
    val = D_80102778.unk_D;

    if (val == 2) {
        D_800A389A = ((u32)p[5] >> 17) & 1;
        D_800A3788 = ((u32)p[5] >> 18) & 7;
    } else if (val == 5) {
        u32 idx;
        s32 sel;

        D_800A389B = (((u32)p[5] >> 10) & 3) + 3;
        idx = ((u32)p[5] >> 12) & 3;
        D_800A36CC = (&D_8008EC30)[idx];
        sel = 1;
        if ((u32)p[5] & 0x4000) {
            sel = 2;
        }
        D_800A37F8 = sel;
        s = &D_801027D8;
        D_800A38E1 = ((u32)p[5] >> 15) & 3;
        {
            s32 j = 0;
            u8 *dst_d = s;
            u8 *dst_a = &D_801027A0;
            do {
                s32 k = 0;
                u8 *da = dst_d;
                u8 *db = dst_a;
                s32 off = j << 1;
            loop_inner:
                {
                    u8 *pp = (u8 *)p + off;
                    *db = (&D_8008D55C)[pp[0]];
                    off += 10;
                    k++;
                    *da = pp[1];
                    db++;
                    da++;
                }
                if (k < 2) goto loop_inner;
                dst_d += 2;
                j++;
                dst_a += 2;
            } while (j < 5);
        }
    }

    func_800344B4();
}

void func_80035280(void) {
    s32 *p;
    u8 *src;
    /* FAKE: one pointer to the three 8-byte clock records (D_80106A50.times),
     * hoisted above the loop rather than indexing D_80106A50.times[i] at each
     * use; mechanism: loop.c move_movables hoists the
     * address-materialisation movable into the loop-2 preheader exactly once,
     * giving the target's single $a2 record cursor -- writing the symbol inline
     * at the use sites creates a second address movable. */
    FileTimeRec *base;
    s32 i;
    s32 flags;
    /* FAKE: the flag merge is staged through one fresh named intermediate per
     * merged bit instead of re-using a single accumulator; mechanism:
     * local-alloc.c:472's `reg_n_deaths == 1` eligibility test -- one death per
     * pseudo makes each merge result eligible for the target's seat, where a
     * single re-used accumulator has three deaths and is refused. */
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    flags = p[8];
    flags0 = (flags & ~1) | (D_80106A50.flags & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (D_80106A50.flags & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (D_80106A50.flags & 4);
    p[8] = flags2;
    src = D_80106A50.color;
    for (; i < 3; i++) {
        ((u8 *)p + i)[0x17] = *src;
        ((u8 *)p + i)[0x1D] = *src;
        src++;
    }
    base = D_80106A50.times;
    for (i = 0; i < 3; i++) {
        /* FAKE: the three clock fields and the raw record byte are each staged
         * through a fresh named intermediate before their store; mechanism:
         * loop.c:1631's move_movables desirability test
         * `threshold * savings * m->lifetime >= insn_count`. The 0x91A2B3C5
         * (/1800) magic is a movable with savings 1 and lifetime 1, and
         * loop.c:532 fixes `threshold = (loop_has_call ? 1 : 2) *
         * (1 + n_non_fixed_regs)` = 58 on this -msoft-float configuration, so
         * the constant is hoisted into the loop-2 preheader for any
         * insn_count <= 58. These four intermediates raise loop 2's real-insn
         * count from 55 to 59 (-dL dump), which is the first count that
         * refuses the hoist and leaves the lui/ori inside the loop exactly as
         * the target carries it. Every one of them holds a real value that is
         * stored to the target's own bytes, and combine folds the copies away
         * (no instruction is added). */
        u8 mn;
        u8 sc;
        u8 hs;
        s32 t;

        mn = base[i].unk_4 / 1800;
        ((u8 *)p)[i * 4 + 0x21] = mn;
        sc = (base[i].unk_4 / 30) % 60;
        ((u8 *)p)[i * 4 + 0x22] = sc;
        hs = (base[i].unk_4 % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x23] = hs;
        t = base[i].unk_0;
        ((u8 *)p)[i * 4 + 0x24] = t;
    }
}
void func_80035430(void) {
}
