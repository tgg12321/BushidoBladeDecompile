/* REJECTED (s1): score 31 (vs floor 13). Two independent kills in one form:
 * 1. Compound `v = (v & ~K) | b;` creates an intermediate pseudo per group ->
 *    and-result lands in a scratch reg and v is pushed to $a2 (target keeps v
 *    in $v0 with in-place `and $v0,$v0,$v1`). The in-place spelling
 *    `v &= ~K; v |= b;` is required.
 * 2. Mask-then-shift `(arg0 & 0x10) >> 1` is NOT canonicalized by GCC 2.7.2
 *    combine -> emits `andi;sra` (signed) / `andi;srl` (unsigned), never the
 *    target's `srl;andi`. Shift-first spelling is required.
 * Also killed separately: signed shift-first `(arg0 >> 1) & 0x8` emits `sra`
 * (combine does not convert ashiftrt->lshiftrt under the mask) -> score 1,
 * one insn off. Unsigned-flavored spelling required for all three shifts.
 */
void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04;
    s32 v = *p;
    v = (v & ~0x1) | (arg0 & 0x1);
    v = (v & ~0x2) | (arg0 & 0x2);
    v = (v & ~0x4) | (arg0 & 0x4);
    v = (v & ~0x8) | ((arg0 & 0x10) >> 1);
    v = (v & ~0x10) | ((arg0 & 0x20) >> 1);
    v = (v & ~0x20) | ((arg0 & 0x40) >> 1);
    v = (v & ~0x40) | ((arg0 & 0x8) << 3);
    v = (v & ~0x80) | (arg0 & 0x80);
    *p = v;
}
