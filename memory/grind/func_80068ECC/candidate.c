/* func_80068ECC — sandbox distance 0 (s2, 2026-07-27). Applied in src/text1b.c.
 * Bit-extract respelling: replaces the Judge-FAILed distinct-width-cast form.
 * Each shift term is a structurally distinct single-bit extraction (shift
 * counts 4/5/6, masks 1<<3/1<<4/1<<5) — CSE has nothing to merge; combine's
 * simplify_shift_const folds each (ashift (and (lshiftrt x N) 1) M) triple
 * to the target's `srl 1; andi mask`. All casts uniform (u32), each required
 * for logical-shift semantics (sra otherwise). Layer-1 cheat-reviewer: PASS.
 */
void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04;
    s32 v = *p;
    v &= ~0x1; v |= arg0 & 0x1;
    v &= ~0x2; v |= arg0 & 0x2;
    v &= ~0x4; v |= arg0 & 0x4;
    v &= ~0x8; v |= (((u32)arg0 >> 4) & 1) << 3;
    v &= ~0x10; v |= (((u32)arg0 >> 5) & 1) << 4;
    v &= ~0x20; v |= (((u32)arg0 >> 6) & 1) << 5;
    v &= ~0x40; v |= (arg0 << 3) & 0x40;
    v &= ~0x80; v |= arg0 & 0x80;
    *p = v;
}
