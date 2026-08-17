/* REJECTED (session 3) — the same-variable split-init staging of `offset`.
 *
 * Shape:  s32 offset = (s16)a1;  ...  offset = offset * 0xB0;
 *         u8 *base;  base = (u8 *)(*base_ptr + offset);
 *
 * Measured (cc1-only gold diff, tmp/grind/func_80084A7C/s3): golddiff 19 vs 45
 * for the plain form — it DOES fix the whole $a2/$a3 swap (offset lands in $a2,
 * base_ptr in $a3, greg dispositions "82 in 6 / 80 in 7") because staging
 * shortens `offset`s live range and lifts its allocno_compare priority above
 * base_ptr's.  But the two-def variable leaves ONE extra instruction: the
 * chain's final `sll` writes a temp ($t0) which is copied into the variable
 * (`move $6,$8`), 124 insns vs target's 123 (pre-maspsx).  Copy-propagation
 * rewrote only the two early-branch uses to the temp, so both pseudos stay live
 * and the copy cannot be deleted.
 *
 * Every spelling of the staging measured the same 1-insn residual:
 *   offset = offset * 0xB0;  |  offset *= 0xB0;  |  offset = 0xB0 * offset;
 *   s32 offset = a1; (no cast)  |  u32 holder  |  a1 << 16 then (>>16)*0xB0
 *   three-stage (sign-extend, *0xB, *0x10)
 * Splitting with a SEPARATE stage variable (`s32 idx = (s16)a1;` then
 * `offset = idx * 0xB0;`) is completely INERT (golddiff 45) — the lever is the
 * same-variable two-def structure, not the extra name.
 *
 * DEAD because the session-3 winner (delete the `offset` local entirely) closes
 * the same allocation with no extra instruction and no added construct.
 */
void func_80084A7C(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1;
    u8 *base;
    s32 val;
    u32 threshold;

    offset = offset * 0xB0;
    base = (u8 *)(*base_ptr + offset);
    /* ... body identical to natural-offset-no-negation-floor24.c ... */
}
