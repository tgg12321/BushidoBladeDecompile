/* REJECTED (s3, structural) — frontier #1 "duplicated-statement-into-arms
 * byte-neutral bitpos ref-lift" is MECHANICALLY IMPOSSIBLE for this function.
 *
 * Goal was: lift bitpos nrefs 9->10 by duplicating a REAL bitpos-reading
 * statement into both sval arms so jump2 re-merges it byte-neutrally
 * (bitpos nrefs 10 @ ll57 -> pri ~5262 > sum 4838 -> flips RA to target,
 * return move $2,$10, at 67 insns).
 *
 * WHY DEAD (two independent grounds, both measured on standalone via
 * tmp/gccdbg/cc1 BB2_ALLOC_DEBUG, baseline standalone=60 insns):
 *  1. NO MERGE SLOT. Target's cross-jump-merged suffix (.L8006938C) is
 *     `addu $t2,$t2,$v0 ; sh $zero,($a3) ; sh $zero,($a2)` — it contains
 *     ZERO bitpos-reading instructions. The only bitpos reads in target are
 *     the two per-arm `sllv $v0,...,$t1` in the UN-merged blocks .L8006937C /
 *     .L80069388 (they differ: shift const 1 vs 2). So a duplicated
 *     bitpos-read has nowhere byte-neutral to merge.
 *  2. MEASURED: the form below (`probe = 4<<bitpos` in both arms, kept live
 *     via `return sum+probe`) compiles to 62 insns (+2 vs 60) — NOT
 *     byte-neutral (fails duplicated-statement prereq #2).
 *     The DCE variant (`return sum;`, probe dead) stays at 60 insns but
 *     bitpos nrefs stays 9 (flow deletes the dead store BEFORE ref-counting
 *     — dead stores are INERT for global RA in 2.7.2, exactly as
 *     duplicated-statement-into-arms.md states). No ref lift.
 *
 * => A real bitpos read costs an instruction; a dead one lifts no refs.
 * There is no real, byte-neutral, mergeable bitpos-reading statement in this
 * function's semantics. Frontier #1 requires no ruling-request: the
 * byte-neutrality precondition provably fails. */
s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 sum;
    s32 i;
    s32 a3_off;
    s32 bitpos;
    u32 *p;
    u32 maskA;
    u32 maskB;
    u32 v;
    s32 c;
    s16 sval;
    s32 probe;

    sum = 0; i = 0; arg1 <<= 4; a3_off = 0; bitpos = 0; p = &D_800A32D0;
    do {
        s32 idx4;
        arg3 = (s16 *)((s32)arg3 + a3_off);
        v = i * 4;
        maskB = *p << arg1;
        idx4 = v;
        maskA = *(u32 *)((s32)&D_800A32C8 + idx4) << arg1;
        if (*arg2 == 0) {
            v = *arg0;
            if (v & maskA) { *arg3 = 1; }
            else if (v & maskB) { c = -1; *arg3 = c; }
        } else {
            v = *arg0;
            if (v & maskA) { c = 6; *arg2 = c; }
            else if (v & maskB) { c = -6; *arg3 = c; }
            sval = *arg2;
            if (sval >= 6) {
                sum += 1 << bitpos; *arg3 = 0; *arg2 = 0;
                probe = 4 << bitpos;            /* duplicated bitpos read */
            } else if (sval < -5) {
                c = 2; sum += c << bitpos; *arg3 = 0; *arg2 = 0;
                probe = 4 << bitpos;            /* duplicated bitpos read */
            }
        }
        a3_off += 2; bitpos += 0x10; p++; i++;
        *arg2 = (u16)*arg2 + (u16)*arg3;
        arg2++;
    } while (i < 2);
    return sum + probe;   /* keeps probe live; +2 insns, not byte-neutral */
}
