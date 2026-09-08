/* REJECTED (s3 structural, 2026-08-20): cmd declared u8 instead of u32.
 * sandbox --disable all = 38, build_insns 69 (target 68) — one EXTRA
 * instruction and the shape perturbs. QImode cmd forces extra narrowing;
 * strictly worse than the 27-floor baseline. Type axis T1 of the s2
 * frontier sweep. T2 (val as u32) and T3 (c as u32 + explicit & 0xFF at
 * the cmd copy) both measured FLAT 27, 68/68 — the whole type axis is
 * measured dead as a closer.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u8 cmd;   /* <-- the change; everything else == candidate.c */
    u8 val;
    /* body identical to candidate.c */
}
