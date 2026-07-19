/* s39 KILLED (NEUTRAL — indistinguishable from baseline).
 * Signature-level parameter type variation: change `s32 a0` -> `u32 a0`,
 * keeping the body unchanged. Also tested `u32 a0, u32 a1` in the same
 * session, byte-identical.
 * Result: sandbox --disable all -> score=2, target_insns=83, build_insns=83
 * (byte-identical to baseline candidate).
 * Mechanism: consistent with s5's evidence that GCC 2.7.2 expand_shift
 * normalizes constant power-of-2 shifts to (ashift SI) regardless of
 * operand or destination signedness. Extends the signedness-agnostic
 * finding from operand-side (s5) and destination-decl-side (s12) to the
 * PARAM-SIGNATURE surface. The initial pseudo 72 (a0)'s DECL_MODE stays
 * SI; sign vs zero extension of the arg reg is elided because a0 is only
 * consumed by a shift (which cares about the low 32 bits only) and a
 * subsequent (set 77 72) move (mode-preserving). Neutral, not a lever.
 */
void func_80045294(u32 a0, u32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... body identical to baseline ... */
}
