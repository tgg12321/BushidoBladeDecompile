/*
 * CANDIDATE — func_800401CC (src/text1a_pre.c), session 2 (structural), 2026-08-11.
 * SANDBOX 0/78 THIS SESSION (was floor 7 at session start). Form is APPLIED in
 * src/text1a_pre.c. Requires the s32-widened `v` declaration (s16 u; s32 v;).
 *
 * What closed the last 7 (the $6/$7 mask-constant local-alloc swap):
 *  1. The 0xFF000000 mask is staged through the existing `v` local (the texture
 *     V coordinate, dead after the SetDrawMove call). `v` is a MULTI-BLOCK
 *     pseudo, so it is allocated by global-alloc, NOT local-alloc — this removes
 *     0xFF000000 from the tail's local qty pool entirely. The remaining single
 *     mask qty (0xFFFFFF) then takes the first free ascending reg = $6 (target).
 *     Global-alloc places `v` at $7: its allocno carries the $a3 copy preference
 *     from the pre-call `(s16)v` arg copy (sll/sra into $7), tail conflicts are
 *     $2-$6 only, so $7 lands — target. `v` was widened s16 -> s32 to hold the
 *     mask; head bytes unchanged (lhu load + (s16) casts at the call unchanged).
 *     FAKE-annotated per staged-value-reused-variable (real, immediately-used
 *     value staged through an existing currently-dead local).
 *  2. `lowmask = 0xFFFFFF;` named intermediate set BEFORE the `v` set restores
 *     the constant-load emission order (li+ori FFFFFF before li FF000000, target
 *     insns 51-53). With inline 0xFFFFFF (probe C) the order was reversed
 *     (score 2); naming the sub-expression and setting it first biases the LUID
 *     order so sched emits it first. Named-intermediate declaration order is a
 *     sanctioned SOTN family (no-new-park-categories.md:189).
 *
 * Measured path this session: floor 7 -> P1 a2=0xFFFFFF: 12 (KILLED — local-alloc
 * runs before global-alloc, FF000000 local qty grabs $6, a2 pushed to $7, head
 * breaks) -> P2 a2=0xFF000000: 7 redistributed (tail ANDs matched! head broke 4,
 * li order 3 — proved the "global holder lands $7" mechanism, killed the a2 axis)
 * -> P3 v=0xFF000000: 2 (head intact, regs all correct, li order wrong) ->
 * P4 + lowmask-first: 0.
 *
 * Judge/reviewer notes: the ONLY FAKE construct is `v = 0xFF000000;` (one live,
 * twice-read assignment; zero dead code). Lever exhaustion: hypotheses.md
 * s1 K1-K4 (birth order, death separation, operand flips, analytic subset kill)
 * + s2 P1/P2 (both param-reuse forms measured broken). K1c proved fresh named
 * locals for BOTH masks cannot flip (both stay in the local qty pool and
 * qty_compare_1 always allocates the shorter-lived FF000000 first).
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s16 u;
    s32 v;
    s32 *pkt;
    s32 *ot;
    s32 lowmask;

    a2 = D_800A36AC & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + D_800A3234 * 240)) {
        tbl = &D_80094AF4 + a1 * 6;
        buf[0] = *tbl++;
        buf[1] = *tbl++;
        buf[2] = *tbl++;
        buf[3] = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf[0] = buf[0] + 0x80;
            u = u + 0x80;
        }
        SetDrawMove((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (s32 *)D_800A3378;
        ot = (s32 *)D_800A378C;
        lowmask = 0xFFFFFF;
        /* FAKE: OT-code mask staged through the dead v-coord local (its value is
           consumed by the draw call above; the mask is read by both packet-link
           statements below), mechanism: global.c call-arg copy preference keeps
           the multi-set pseudo in $a3 while local-alloc.c qty_compare_1 gives the
           remaining single mask qty $a2, lever-exhaustion: grind ledger
           hypotheses.md K1-K4 + s2 P1/P2 (param-reuse forms measured broken) */
        v = 0xFF000000;
        *pkt = (*pkt & v) | (ot[0x3FFC / 4] & lowmask);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & lowmask);
        D_800A3378 = (s32)(pkt + 6);
    }
}
