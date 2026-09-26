/* The code6cac_b.c functions that follow func_80034708, moved unchanged so
 * that func_80034708 can sit in its own -G8 unit (code6cac_b3.c) between them. */
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"

extern s32 *func_80077D00(void);
extern u8 D_801027A0;
extern u8 D_801027D8;
extern void func_800344B4(void);

/* TABLED: -4 bytes, score 1980. Target alternates v1/a0 for D_80106A50.flags address — unreproducible register allocation pattern */
/* s66 (solver, 2026-09-05) -- FLOOR 2 -> 0, and the whole body is confined to
 * src/code6cac_b.c.  49/49 instructions, byte-identical to
 * asm/funcs/func_80034F88.s; full clean-driver build SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle with ONLY this file edited.
 *
 * WHAT CLOSED THE BYTES.  s65 fitted GCC 2.7.2's global.c allocation priority
 * exactly (pri = floor_log2(nrefs) * nrefs * 10000 / live_length) and reduced
 * the residual to two arithmetic branches.  Branch (A): block 0's address
 * object reaches the target seating iff it is allocated before block 0's value
 * (pri 17500), i.e. iff floor_log2(n)*n > 49, i.e. n >= 16 references; its own
 * five references at live length 28 price at 3571.  s65 measured every obvious
 * byte-neutral reference lift dead (duplicated store into arms, split reads,
 * merged mask).  The lift that is free is a variable reuse: block 0's address
 * object and the copy loop's counter are ONE variable, so the loop's eleven
 * counter references (flow.c weights by loop depth) land on the address
 * allocno, AFTER its last pointer use, so the live length rises only 14 -> 21.
 * Measured model (tmp/grind/func_80034F88/s66/z2.model.json):
 *
 *   ord0 p74 c (flag/result)      19 refs / len 21 / pri 36190 -> $v0  TARGET
 *   ord1 p76 q (address + index)  16 refs / len 21 / pri 30476 -> $v1  TARGET
 *   ord2 p75 u (block-0 value)     7 refs / len  8 / pri 17500 -> $a0  TARGET
 *   ord3 p73 v (blocks-1/2 value)  6 refs / len 10 / pri 12000 -> $v1  TARGET
 *   ord4 p80 r (blocks-1/2 addr)   6 refs / len 19 / pri  6315 -> $a0  TARGET
 *   ord5 p72 p                     6 refs / len 34 / pri  3529 -> $a1  TARGET
 *
 * Block 0's value is no longer blocked out of $v1 by a conflict (the s64 route,
 * capped at score 2 because global.c:1275 gives one allocno one hard register):
 * $v1 is simply already held by the higher-priority address/counter allocno, so
 * find_reg scans on to $a0 -- the target register -- and the loop's byte temp
 * stays a plain block-local that local-alloc seats at $v0, also as the target.
 *
 * INTEGRATION (s62-s66 history). The copy loop's indexed store
 * `lui $at,%hi(..); addu $at,$at,$v1; sb $v0,%lo(..)($at)` targets the three
 * colour bytes 0x80106A70..72; since 2026-09-26 they are D_80106A50.color[3]
 * and the flags byte is D_80106A50.flags, members of the 0x24-byte FileRecord
 * declared in include/system.h.
 * Measured s66: sandbox score 0 (49/49) AND full clean-driver build SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
 */
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
         * hard register per allocno and GCC 2.7.2 does no live-range splitting.
         * lever-exhaustion: memory/grind/func_80034F88/hypotheses.md s53-s65. */
        u8 *q = &D_80106A50.flags;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = 0; /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards
                * the sb into the following lbu only while the stored value's
                * pseudo still holds it. lever-exhaustion: hypotheses.md s57-s62. */
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
             * reached from the block-0 object. lever-exhaustion: as above. */
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
         * used; the loop adds no instruction anywhere in the function.
         * lever-exhaustion: hypotheses.md s53-s65 -- s65's branch (A), whose
         * other byte-neutral spellings (duplicated store into arms, split
         * reads, merged mask) are all banked dead. */
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
       behind the two p-copies); lever-exhaustion: sessions 1-9 of
       memory/grind/func_8003504C/hypotheses.md. */
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
     * at the use sites creates a second address movable and measures 39 diffs
     * (tmp/grind/func_80035280/s5/xB.c),
     * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s1-s5 */
    FileTimeRec *base;
    s32 i;
    s32 flags;
    /* FAKE: the flag merge is staged through one fresh named intermediate per
     * merged bit instead of re-using a single accumulator; mechanism:
     * local-alloc.c:472's `reg_n_deaths == 1` eligibility test -- one death per
     * pseudo makes each merge result eligible for the target's seat, where a
     * single re-used accumulator has three deaths and is refused (single
     * accumulator measures 44 diffs, tmp/grind/func_80035280/s5/m4.c),
     * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s3 + s5 */
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
         * count from 55 to 59 (measured in the -dL dump,
         * tmp/grind/func_80035280/s5/last.loop), which is the first count that
         * refuses the hoist and leaves the lui/ori inside the loop exactly as
         * the target carries it. Every one of them holds a real value that is
         * stored to the target's own bytes, and combine folds the copies away
         * (build_insns 108 == target_insns 108),
         * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s1-s5 */
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
