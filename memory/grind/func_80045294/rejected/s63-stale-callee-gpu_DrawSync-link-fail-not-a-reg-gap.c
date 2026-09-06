/* s63 NOT A C-FORM REJECTION - STALE CALLEE NAME. This is the s62 F6 body exactly as the driver
   verified it on 2026-09-06 09:27: sandbox 0/83, layer-1 PASS, then `verify-oracle --rebuild --allow-dirty`
   raised RuntimeError 'link failed: undefined reference to gpu_DrawSync' (engine/pipeline.py:130) 9 s after
   retire. `gpu_DrawSync` was retired by the naming wave; the symbol is `DrawSync` (include/code6cac.h:511,
   target bytes `jal DrawSync` at asm/funcs/func_80045294.s 0x80045310). The sandbox masks jal targets, so the
   score was 0 while the link was impossible. The driver's constraint 'masked-0 register diff class - reg-alloc
   gap is real' is a misdiagnosis: with `DrawSync(0)` the same body full-builds to SHA1
   62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_80045294/s63/verify_f6_drawsync.txt).
   Do not resubmit a body naming gpu_DrawSync. */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1;
    s32 s4;
    s32 count;
    s32 s5;

    /* !FAKE: `i++; i--;` cancellation pair (F6 family, semantically-null
       statement pair, .claude/rules/no-new-park-categories.md:402).
       what: keeps the loop counter `i` out of the parameter's cse quantity
       while the `a0 << 4` offset below is processed, so the shift reads the
       parameter register ($s2) as the target does; without the pair cse
       canonicalises the shift to the copy `i` ($s0) (make_regs_eqv,
       tools/gcc-2.7.2/cse.c:842-857), the single residual of 61 sessions.
       mechanism: cse_insn invalidates the destination of the self-referencing
       `i = i + 1` and remove_invalid_refs drops its table entry, so cse1 and
       cse2 both keep the pair and the copy; combine.c cancels the pair back
       into the plain copy in the same insn slot, keeping copy-before-shift
       order for the sched.c:2464 LUID tiebreak.
       lever-exhaustion: memory/grind/func_80045294/hypotheses.md (s1-s62,
       80 banked rejected forms; s62 D1-D10 show every constant or
       negation chain is re-folded by cse.c:5020/5497 and scores 1). */
    i++;
    i--;
    v1 = a0 << 4;
    s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    count = D_800A33AC;
    s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        i = a0;
        if (i < D_800A33AC) {
            v1 = i << 4;
            a0 = (s32)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *(s32 *)a0 += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                a0 += 0x10;
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
