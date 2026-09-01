/* func_80045878 (src/text1a_c.c) -- BYTE-EXACT PURE C, s14 (2026-08-31, solver).
 *
 * MEASURED THIS SESSION, with this body in place in src/text1a_c.c:
 *   sandbox func_80045878 --disable all -> score 0, build_insns 108 ==
 *     target_insns 108, scorable true, rules_dropped 0  (measured twice,
 *     the second time after a full oracle rebuild).
 *   verify-oracle -> "ok": true, "build_matches": true -- the FULL
 *     clean-driver build+link SHA1 still equals the oracle
 *     62efab4f73f992798c43e8c730aa43baa10bb4fa with this C replacing the
 *     INCLUDE_ASM line.
 *
 * ==================================================================
 * CONSTRUCT INVENTORY: **NONE**
 * ==================================================================
 * No carrier local, no fresh local of any kind, no dead store, no dead
 * mention, no alias, no FAKE annotation, no sanctioned-family claim.
 * The body is ordinary C.  Everything that thirteen previous sessions built
 * on top of the s12 "compliant plain tail" (the `p` carrier, the `s0 = (s32)p`
 * dead store, the `t` named intermediate, the static-inline tail helper) is
 * GONE.
 *
 * THE TWO CHANGES vs the long-standing shape, and why they are ordinary C:
 *
 *  (1) THE FUNCTION RETURNS THE OBJECT POINTER (`s16 *`, `return s1;`)
 *      instead of being declared `void`.  This is a correction of a wrong
 *      declaration, not a codegen device:
 *        - target's own bytes leave the object pointer in $v0 at `jr $ra`
 *          (`addu $v0, $s1, $zero` at .L800459DC, never clobbered after);
 *        - an ALREADY-MATCHED caller in this project, `func_80040510`
 *          (src/text1a_pre.c:173,179), declares `extern s32 *func_80045878(s32);`
 *          and consumes the result as a player object pointer
 *          (`ptr = func_80045878(a0); g_player_ptrs[a0] = (s32)ptr;
 *            func_80040594(ptr); ...`).
 *      So the function is "get-or-create the object for slot a0, initialise
 *      it, return it" -- and the target's tail base copy IS the return-value
 *      copy.  (SOTN prototype-contradiction norm: the bytes decide the
 *      declaration.)
 *
 *  (2) THE SIX TAIL STORES ARE WRITTEN IN PLAIN SOURCE ORDER with the
 *      `a0 + 3` store LAST of the half-word group.  Ordinary statement order
 *      in the source; no dead code, no duplication, nothing a reader would
 *      ask "why is this here?" about.  (Written with `s1[11] = a0 + 3;` FIRST
 *      instead, the same body is 109 insns / score 6: cse builds a HImode
 *      temp for the three `a0` half-word stores.  That is why the order is
 *      what it is, but the order itself is not a construct.)
 *
 * ==================================================================
 * WHY THIRTEEN SESSIONS MISSED IT (the mechanism, for the ledger)
 * ==================================================================
 * Every session from s2 on treated target's `addu $v0,$s1,$zero` as a BASE
 * COPY that had to be manufactured, and asked "which pseudo can be both
 * cse-canonical and block-local in the tail?".  Under a `void` signature the
 * answer is provably "only a fresh local with a deleted pre-tail mention"
 * (s12/s13b; re-derived closed-form in s14's H14.2), which is the construct
 * the Judge banned.  The insn is not a base copy: it is the RETURN VALUE
 * move.  Returning `s1` gives the tail base pseudo a copy-suggestion to hard
 * $v0 (local-alloc.c qty_phys_copy_sugg, the suggested-register pass that
 * runs BEFORE the ascending-order main pass), so the base takes $v0 and the
 * two 2-ref tail scratches fall to $v1 -- exactly target's seating -- with no
 * extra insn anywhere.  This is precisely the single minimal vector the
 * inverse solver reported for the block-13 seat: "hard $v0 live across
 * [6,20) -- a real argument/return value genuinely live across the block".
 *
 * Apply verbatim over the INCLUDE_ASM line in src/text1a_c.c.
 */
s16 *func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    s1[2] = a0;
    s1[4] = a1;
    s1[10] = a0;
    s1[8] = a0;
    s1[11] = a0 + 3;
    *((s32 *) (((s32) s1) + 0x18)) = 0x8000;
    return s1;
}
