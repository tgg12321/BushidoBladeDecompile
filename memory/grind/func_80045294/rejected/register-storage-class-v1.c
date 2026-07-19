/* s38 structural probe: plain `register` storage class on v1 (no asm binding).
 *
 * Hypothesis: `register s32 v1 = a0 << 4;` is a legitimate C storage-class
 * hint (NOT a `register T x asm("$N")` cheat pin). If GCC 2.7.2 honors it as
 * an RA priority boost or LUID-adjacent effect, it might shift the sll/move16
 * sched2 tie without introducing any coercion construct.
 *
 * Measurement: sandbox --disable all -> score=2, target_insns=83,
 * build_insns=83, rules_dropped=0. NEUTRAL. Verdict KILLED.
 *
 * Mechanism: GCC 2.7.2's `register` keyword without `asm()` binding is
 * advisory only; the register allocator makes its decisions from n_refs,
 * live-range, and conflict analysis, not from the storage-class hint.
 * The pseudo for v1 is created at the same tree LUID and gets the same
 * allocno priority as without the hint. sll's INSN_LUID and RA disposition
 * unchanged; sched2 tie preserved.
 *
 * Note: `register` alone (no asm) is NOT in the forbidden-cheat catalog --
 * it's ordinary C. This probe measures whether the vestigial hint has any
 * codegen effect on this function. It does not. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    register s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
