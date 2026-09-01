/* REJECTED s1 (2026-09-01): common `row` pointer for the p_6a/p_7e bases,
 * int-domain offset-first. Hypothesis: coalesce the addu dest into the
 * offset's register (target: addu $v1,$v1,$v0). MEASURED: 15 -> 38 isolated
 * (build_insns 176): introducing `row` changes the CSE of the shared
 * (t0*10) subexpression and restructures the whole region. Also measured:
 * plain operand swap `(t0*10) + D_800A36A0 + 0x6A` inside the existing
 * two-pointer spelling is BYTE-NEUTRAL (canonicalized; still 15) — the
 * rows-62-64 dest-coalesce residual is not reachable by association-order
 * spelling. Needs an RA-level lever (which pseudo dies; see .lreg/.greg).
 */
{
    u8 *row = (u8 *)((t0 * 10) + (s32)D_800A36A0);
    s16 *p_6a = (s16 *)(row + 0x6A);
    s16 *p_7e = (s16 *)(row + 0x7E);
    /* ... */
}
