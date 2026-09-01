/* REJECTED (session 15, 2026-09-01, forensics) — "P1", the owner-directed
 * Ruling-A named probe, forward direction: the banned multi-write carrier
 * `wid` split into TWO DISTINCT fresh locals, each written exactly once
 * (`wid = i + j; idx = wid;` at the inner-loop top, `wid2 = idx2 + idx;` for
 * the *3 sum, the three word stores reading `wid2`).  Every local in the body
 * is written once, so the form is textually and semantically distinct from
 * every entry on this function's banned list and is exactly the shape the
 * 2026-08-31 amended named-intermediate family (once-written) describes.
 *
 * MEASURED: `sandbox func_800645B0 --disable all` = 3 / 78, build_insns 78
 * (SB control re-measured the same session = 1 / 78).  The residual moves
 * WHOLESALE to the inner-loop head: objdump positions 11/12 (ours
 * `li v1,1` then `addu s0,s3,a0`; target `addu s0,s3,a0` then `li v1,1`) and
 * position 65 (the back-edge delay slot, target steals the addu, we steal the
 * li) — i.e. the WD fresh-destination residual, unchanged.
 *
 * PASS ATTRIBUTION (dumps, this session): the copy is DELETED BY cse.c before
 * loop.c or sched.c ever see it.  tmp/grind/func_800645B0/s15/p1.rtl.txt has
 * `(insn 41 38 44 (set (reg/v:SI 74) (reg/v:SI 79)))` — the `idx = wid` copy —
 * and p1.cse.txt contains ZERO occurrences of insn 41 and ZERO references to
 * pseudo 74: cse_insn propagated reg79 into the shift and the sum and deleted
 * the copy.  reg_n_sets for the surviving loop-top carrier is therefore still
 * 1, sched.c's birthing_insn_p keeps its max-priority lift (H61/H62), and the
 * loop-head placement is lost.  A once-written named intermediate that holds a
 * COPY can never raise a set count in this compiler.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    s32 wid2;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            wid = i + j;
            idx = wid;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid2 = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid2 << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid2 << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid2 << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
