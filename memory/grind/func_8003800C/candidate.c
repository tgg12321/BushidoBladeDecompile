/* MATCHED candidate body for damage_DebugDisp (func_8003800C) -- s16 (synthesis).
 * sandbox --disable all = 0 (79/79 insns, 0 rules dropped) AND full-build SHA1
 * == 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle) with this body in
 * src/code6cac_c_mid.c.  All prior FAKE constructs are GONE: the two
 * `do { x = 0; } while (0);` brackets (Region A RA-weighting, Region B
 * biv-init fold) and the `for(;;)`+continue fence on the CopyBlock loop are
 * all unnecessary on this chassis.  What replaced them is ONE structural
 * change plus one statement-order change:
 *
 *  1) ONE counter.  `j` is declared at function scope and serves BOTH the
 *     per-record checksum loop (0x24 bytes) AND the 0x16-entry fixup loop
 *     (which s1-s15 spelled as a second local `k`).  This is the C89 spelling
 *     a 1998 author would write, and it is the match lever:
 *     MECHANISM (global.c allocno_compare, pri = floor_log2(n_refs)*n_refs
 *     / live_length * 10000 * size) -- merging the two counters unions their
 *     live ranges, so reg_live_length(j) rises far enough that j's priority
 *     drops BELOW sum's; global_alloc then allocates sum first and sum takes
 *     $a0 (target's seat), j takes $a1, bp takes $v1.  Measured by
 *     tools/ra_solver (model exact 17/17 dispositions on this function):
 *     the inverse solver named `live_extend pseudo 79 (j)` as THE single-atom
 *     vector on the sum-first chassis (tmp/grind/func_8003800C/s16/
 *     inverse_sumfirst_goal.txt).
 *  2) sum=0 FIRST in the inner-loop preheader (`sum=0; bp=...; j=0;`).  The
 *     preheader's three inits are independent and all priority 1, so sched1
 *     emits them in source (LUID) order; target emits the $a0-init first,
 *     which is sum.  s1-s15 had `j=0` first, which matched the preheader BYTES
 *     only because j held $a0 -- the roles were swapped inside the loop.
 *
 * `j < 0x24U` (unsigned bound) is load-bearing and semantic: the byte index is
 * compared unsigned -> `sltiu $v0,$a1,0x24`, while the fixup loop's signed
 * `j < 0x16` -> `slti $v0,$a1,0x16`.  Both compares in the target use $a1 --
 * the same register -- which is what first suggested the two counters are one
 * variable in the original source.
 *
 * Region B keeps the index-based addressing (base + j*4 + 0x78 / base + j*2 +
 * 0xD0) ruled semantically faithful by the Judge on 2026-07-22 06:02.
 */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;
    /* FAKE: one counter 'j' serves both the per-record checksum loop and the
       0x16-entry fixup loop (C89 counter reuse), mechanism: global.c
       allocno_compare -- the merged live range lifts reg_live_length(j) so j's
       allocno priority falls below sum's and sum takes $a0 (target's seat),
       lever-exhaustion: memory/grind/func_8003800C/hypotheses.md s1-s15 */
    s32 j;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;

        sum = 0;
        bp = base + offset;
        j = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;

        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            do {
                *dst = *sp2;
                sp2++;
                dst++;
            } while (sp2 != end);
            *(s32 *)dst = *(s32 *)sp2;
        }

        j = 0;
        do {
            u16 *ptr = *(u16 **)(base + j * 4 + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(base + j * 2 + 0xD0);
            }
            j++;
        } while (j < 0x16);
    }

    return 1;
}
