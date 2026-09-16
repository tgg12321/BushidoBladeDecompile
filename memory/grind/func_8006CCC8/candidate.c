/* func_8006CCC8 — src/text1b.c — session s1 (recon)
 * Sandbox --disable all floor at end of s1: 94 (target_insns=189, build_insns=193).
 * Started at 189 (no_c_body / INCLUDE_ASM). This candidate IS applied to src/text1b.c
 * as of end of s1 — it is the resident body, not a draft.
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s (no
 * --context; m2c failed to parse src/text1b.c as context due to unsupported
 * preprocessor directives, so the reconstruction used field/offset inference only).
 * Field layout cross-checked against sibling func_8006CBD4 (same TU, same
 * D_800A34FC / D_800A3524 struct idiom: D_800A34FC is a pointer-typed s32 global
 * whose +0x28 region is an s16[2] per-index state array; D_800A3524 is a pointer
 * to a 3-element record array with u8 fields at +0x17/+0x1A/+0x1D).
 *
 * KNOWN REMAINING GAP (s1 evidence, not yet fixed): our build hoists the
 * sign-extension of `arg2` (the third param, raw a2 in target's $s7) out of the
 * `for (i...)` loop into its own callee-saved register (computed once before the
 * loop). Target does NOT hoist it — it keeps the RAW a2 value in $s7 across the
 * loop and recomputes the full sign-extension (sll 16; sra 16; srav i) EVERY
 * iteration inline. This is the classic loop.c invariant-hoist symptom documented
 * in .claude/rules/defeat-licm-hoist-var-reuse.md: `(s32)arg2` is a single-set,
 * non-trapping, loop-invariant pseudo, so scan_loop's move_movables always hoists
 * it for a loop this cheap/short. The sanctioned pure-C fix is to make that
 * pseudo's register MULTI-SET by routing an already-used loop-variant value
 * through the SAME C variable the target's register is reused for (read the target
 * asm to confirm which register — here it looks like $s7 itself never holds
 * anything else, so this specific case may need a different reused-variable
 * candidate; NOT yet confirmed which C variable to reuse — this is the top
 * frontier item for s2).
 *
 * First candidate (before removing the intermediate `slot`/field28 pointer local)
 * scored 142 and additionally spilled `ret` to the stack (frame growth 0x48->0x58)
 * because the extra `slot` pseudo register raised pressure past what LICM+RA could
 * fit — removing the cached pointer (writing the *(s16*)(...) field access inline
 * at each of the ~5 use sites, matching target's fresh-address-each-use pattern)
 * dropped the frame back to 0x48 (matches target) and the score to 94. That
 * "reject the cached pointer local, use fresh inline dereference at each syntactic
 * use site" lever is CONFIRMED and should NOT be re-tried in reverse.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s16 i;
    s16 lim;
    s32 shift;
    s32 mask;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    fade = 0;
    for (i = 0, shift = 0, mask = 0; i < 2; i++, shift += 0x10, mask += 2) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) <= 0) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = lim;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= lim) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = 0;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) + 1);
            }
        }

        if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 3) {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1A) & (0xF << fade)));
                    } else {
                        *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1A) & (0xF << fade)));
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 4) {
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 4) {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF0) + (*(rec + 0x1D) & (0xF << fade)));
                        } else {
                            *(rec + 0x17) = (u8)((*(rec + 0x17) & 0xF) + (*(rec + 0x1D) & (0xF << fade)));
                        }
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 0 && (*arg1 & (0xF0 << shift))) {
            func_8005C650(0, 0x7F, 0x7F);
            func_8006CBD4(i, *arg1);
        }
        fade += 4;
    }
    return ret;
}
