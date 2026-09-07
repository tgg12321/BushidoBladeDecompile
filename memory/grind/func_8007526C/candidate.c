/* candidate for func_8007526C (src/text1b.c) -- s6 (2026-09-07), ORDINARY C.
 *
 * MEASURED THIS SESSION on HEAD 9066e9ad with the CURRENT, UNMODIFIED build
 * configuration (no -msoft-float, no compiler-flag change of any kind):
 *   `sandbox func_8007526C --disable all` -> score 1, build_insns 90, target_insns 91.
 * Every one of the 90 emitted words is word-identical to asm/funcs/func_8007526C.s.
 * The single missing word is the load-delay `nop` at asm/funcs/func_8007526C.s:6, which
 * sits between `lw $a0, %gp_rel(D_800A36A0)($gp)` and the loop-top label .L80075278 whose
 * first insn is `lbu $v1, 0x10($a0)`.  That is the documented maspsx `.L`-vs-`$L`
 * is_label() blind spot ([[maspsx-is-label-dot-prefix]], .claude/rules/maspsx-label-nop-gate.md),
 * i.e. an assembler-layer gate, not a C-spelling question.
 *
 * PROVEN with the gate on (tmp/grind/func_8007526C/s6/repro.sh + cmp.py): current flags
 * plus func_8007526C in a temporary copy of maspsx_label_nop_funcs.txt builds **91 insns,
 * 1 masked word diff vs the target**, and that one word is `lw $a0,0($gp)` = 8f840000
 * un-relocated vs 8f8405d4 in the target -- an R_MIPS_GPREL16 addend that `ld` fills.
 * No repo gate file was modified; the list was passed as --label-nop-funcs=<temp copy>.
 *
 * WHY THIS BODY (all ordinary C, no FAKE construct, no sanctioned-family claim needed):
 *  - The loop is spelled as a label + backward `goto`.  GCC 2.7.2 emits
 *    NOTE_INSN_LOOP_BEG only for while/for/do statements, so loop.c never processes a
 *    goto-spelled loop: no move_movables hoist of the four switch-comparison constants
 *    (the entire 13-point residual every session s1-s5 fought) and no strength-reduction
 *    giv, which is what biased every field offset by +0x10 in the pointer-bump do/while
 *    spelling (rejected/pointer-increment-biases-iv-by-0x10.c).  Backward-goto loops are
 *    ordinary C and ship in SOTN master PSX sources: src/main/main.c:40,
 *    src/main/psxsdk/libc/sprintf.c:96, src/dra/5F60C.c:579
 *    (docs/reference/sotn-construct-index.md:2723, :2725, :2705).
 *  - `lim` is a real named constant local holding 200: it is written once and READ at the
 *    three `sh $a3, ...` store sites in cases 1 and 3, so its value materialises in the
 *    target's bytes.  It is not a dead constant-holder.  The `>= 0xC8` comparisons stay
 *    literal because they assemble to `slti $v0,$v0,0xC8` immediates, exactly as the
 *    target does at asm/funcs/func_8007526C.s:33/57.
 *  - `p += 2` at the loop bottom is the target's own `addiu $a0,$a0,0x2` in the closing
 *    branch delay slot (asm/funcs/func_8007526C.s:100).
 *
 * REMAINING BLOCKER (not a C axis): func_8007526C must be listed in
 * maspsx_label_nop_funcs.txt.  See docs/grind/decisions.md 2026-09-07 (s6) and
 * memory/grind/func_8007526C/evidence.md [s6].
 */
void func_8007526C(void) {
    u8 *p;
    s32 i;
    s32 lim;

    i = 0;
    lim = 0xC8;
    p = D_800A36A0;
loop:
    switch (*(u8 *)(p + 0x10)) {
    case 1:
        *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
        if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
            if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
            }
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = lim;
            *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
            *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
        }
        break;
    case 3:
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
        if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
            *(u16 *)(p + 8) = lim;
            *(u16 *)(p + 0xC) = lim;
            *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
            *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
            }
        }
        break;
    case 2:
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
        if ((s16)*(u16 *)(p + 0xC) <= 0) {
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = 0;
            *(u16 *)(p + 0x10) = 0;
        }
        break;
    case 4:
        *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
        *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
        if ((s16)*(u16 *)(p + 0xC) <= 0) {
            *(u16 *)(p + 8) = 0;
            *(u16 *)(p + 0xC) = 0;
            *(u16 *)(p + 0x10) = 0;
        }
        break;
    }
    i++;
    p += 2;
    if (i < 2) goto loop;
}
