/*
 * func_80034F88 — best NON-BANNED form, grind session s14 (forensics modality).
 * Honest sandbox (`sandbox func_80034F88 --disable all`): **score 10**,
 *   49 build insns vs 49 target insns.  Previous best non-banned form was
 *   s12b/s13's `static inline` helper at 13, so this is a floor drop 13 -> 10.
 * NOT installed in src/ (src/code6cac_b.c is left at its committed state, as
 *   s12b and s13 also left it).
 *
 * =====================================================================
 * WHAT THIS FORM IS, AND THE CLASSIFICATION CAVEAT UP FRONT
 * =====================================================================
 * ONE pointer local (`u8 *q`) declared once, ASSIGNED `&D_80106A73` three
 * times: before the mask, and again at the top of the bit-2 and bit-4 blocks.
 * The two later assignments are value-redundant — `q` already holds that
 * address when they execute.  That is a dead self-assign to a LOCAL, which is
 * on the frozen SOTN-sanctioned list, but it is also unmistakably in the same
 * INTENT family as the construct the driver banned for this function (four
 * repeated `u8 *q = &D_80106A73;` block-scoped declarations).  This file is
 * banked as the best MEASURED non-banned form and as the carrier of the s14
 * mechanism; it is NOT a submittable candidate and must NOT be sent to a Judge
 * without a ruling.  See the s14 section of evidence.md for the full argument
 * and the ruling question.
 *
 * =====================================================================
 * WHY IT WORKS — the s14 mechanism, read out of cc1 -da dumps
 * =====================================================================
 * 1. Each flag block is a TWO-ARMED `if/else`, so it emits a conditional
 *    branch, an unconditional jump, a barrier and TWO code labels.  On that
 *    chassis `cse_end_of_basic_block` ENDS the cse basic block at the join
 *    label and the value table is flushed.  (s2 concluded the join labels are
 *    NOT cse boundaries, citing the LABEL_NUSES == 1 "branch skips a block"
 *    extension at cse.c:8102-8184.  That was correct for s2's ONE-ARMED
 *    `val2 = val|K; if (!c) val2 = val;` chassis, where the branch really does
 *    skip a block and fall through.  It is FALSE on the two-armed chassis every
 *    form since s10 has used.  The dumps show the flush directly.)
 * 2. A pointer pseudo RE-USED across the flush loses nothing that matters: the
 *    previous block's store and the next block's load are the identical address
 *    rtx `(mem:QI (reg 74))` inside the same post-flush block, so cse records
 *    the store and forwards it — the reload dies — and one pseudo means one
 *    lui/addiu for the whole function.  That is the 29-point one-handle form.
 * 3. A FRESH SET of the address AFTER the flush changes both halves at once:
 *    the new `(set (reg N) (symbol_ref "D_80106A73"))` finds nothing in the
 *    flushed table so it SURVIVES as a real lui/addiu materialisation, and the
 *    previous block's store — addressed through a pseudo whose value the table
 *    no longer knows — hashes differently from the new block's load, so the
 *    load SURVIVES as a real lbu.  Reloads AND an unfolded shared base at the
 *    same time, with zero volatile.  This is the answer to the tension s1-s3
 *    called "mutually exclusive".
 * 4. THE NEW PART: the fresh set does not need a fresh DECLARATION.  A plain
 *    re-ASSIGNMENT of the same local has the identical effect, because what cse
 *    keys on is whether the symbol_ref is in the (flushed) table, not on how
 *    many C objects exist.  RTL proof: s14/rtl/r1/code6cac_b.i.cse retains
 *    THREE `(set (reg/v:SI 74) (symbol_ref "D_80106A73"))` insns (14, 59, 93)
 *    and all eight QI mems, while s14/rtl/w1/code6cac_b.i.cse has one such set
 *    and only five QI mems (three loads forwarded away).
 * 5. Corollary that explains a measurement: an assignment placed BEFORE a flush
 *    is deleted by cse as redundant.  The bit-1 block's assignment in the
 *    "assign in every block" variant is gone from the .cse dump, which is why
 *    that variant and this one compile to byte-identical code and both score 10.
 *
 * =====================================================================
 * THE RESIDUAL 10 POINTS — entirely the FIRST SEGMENT's registers
 * =====================================================================
 * Side-by-side (s14/sbs.py): the bit-2 and bit-4 blocks and the whole copy loop
 * are instruction- AND register-identical to target.  Everything that differs
 * is the mask + bit-1 segment: target puts the base in a0 and the loaded byte
 * in v1, this build swaps them (base v1, byte a0), and target's bit-1 block has
 * a load-delay `nop` where this build has a memory op.  With ONE pseudo re-set
 * three times, local-alloc gives the pointer a single hard register for the
 * whole function, so the first segment cannot be allocated differently from the
 * rest.  FOUR DISTINCT pseudos (the banned form) is what produces the target
 * allocation — and 2- and 3-pseudo splits are measured WORSE, not better
 * (21 and 23), so this is not a monotone "more handles is better" axis.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
