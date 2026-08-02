/* func_80052B00 — REJECTED form, grind session 8 (rederive, 2026-08-01).
 *
 * WHY IT IS DEAD: moving the eight `lw` INSIDE the __asm__ block makes the
 * block cheat-asm under this project's policy, and the honest sandbox strips
 * it. Measured, not argued:
 *
 *   engine.inlineasm.strip_cheat_asm_file() on this exact body returns
 *   stripped_count=1 and an EMPTY function body; `sandbox func_80052B00
 *   --disable all` then reports score 16 with build_insns 2 — which is NOT an
 *   improvement on the honest floor of 17 but the signature of a body that has
 *   been deleted wholesale. Two sibling spellings behave identically
 *   (asmloads8-ret-s32, asmloads8-mem-operand: build_insns 2), and the split
 *   spelling (loads in one asm producing eight outputs, the eight ctc2 in a
 *   second asm consuming them) reports build_insns 10 — the load-asm stripped,
 *   the ctc2-asm kept — which isolates the mechanism exactly.
 *
 * THE MECHANISM. `tools/classify_inline_asm.classify_template` puts `lw` in
 * CHEAT_ASM_OPS and `ctc2` in CANONICAL_ASM_OPS, and
 * `engine/inlineasm._block_category` classifies a block by whether ANY of its
 * split template instructions is canonical. A tooling detail decides the rest,
 * and it is worth knowing: `split_template` splits on the literal `\n` only, so
 * with a `"\n\t"`-joined template every instruction after the first keeps a
 * literal `\t` prefix and its first token reads `\tctc2`, which is in neither
 * table and therefore falls through to the default `cheat`. In practice a
 * multi-instruction block is classified by its FIRST instruction. Here that is
 * `lw` ⇒ cheat ⇒ stripped. (The s3 fused8 form survives because its first
 * template instruction is `ctc2`.)
 *
 * THE POLICY POINT IS SUBSTANTIVE, NOT AN ARTIFACT. An asm block that performs
 * its own general-purpose memory loads is doing work GCC is supposed to do, and
 * emitting those bytes from template text rather than from compilation — that is
 * exactly the injection family described in .claude/rules/inline-asm-injection.md.
 * The classifier's verdict is correct; the shape is not a pure-C path.
 *
 * WHY IT WAS WORTH MEASURING. This is the shape of the REAL PsyQ libgte macros
 * (`gte_SetRotMatrix` and friends put their `lw`s inside the asm with the matrix
 * pointer as %0), so it is the natural "transplant the library source" answer for
 * the rederive modality, and no session 1-7 form had ever moved an instruction
 * across the asm boundary — all 32 hand spellings and ~164k permuter iterations
 * kept GCC on the load side and asm on the ctc2 side. It is now measured.
 *
 * IT ALSO WOULD NOT HAVE MATCHED. Compiled standalone (cc1 accepts it; RC=0, see
 * tmp/grind/func_80052B00/s8/asmloads8.s) the block emits its loads with reload's
 * own register choice and in a PERMUTED order —
 *     lw $9,0($4)  lw $8,4($4)  lw $7,8($4)  lw $6,12($4)
 *     lw $5,16($4) lw $10,20($4) lw $3,24($4) lw $2,28($4)
 * — i.e. not $t0..$t7 and not ascending, so even with the strip disabled this
 * form is further from the target than the s3 fused8 body, not closer.
 */
void func_80052B00(s32 *matrix) {
    s32 t0;
    s32 t1;
    s32 t2;
    s32 t3;
    s32 t4;
    s32 t5;
    s32 t6;
    s32 t7;
    __asm__ volatile (
        "lw %0, 0(%8)\n\t"
        "lw %1, 4(%8)\n\t"
        "lw %2, 8(%8)\n\t"
        "lw %3, 12(%8)\n\t"
        "lw %4, 16(%8)\n\t"
        "lw %5, 20(%8)\n\t"
        "lw %6, 24(%8)\n\t"
        "lw %7, 28(%8)\n\t"
        "ctc2 %0, $0\n\t"
        "ctc2 %1, $1\n\t"
        "ctc2 %2, $2\n\t"
        "ctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\t"
        "ctc2 %5, $5\n\t"
        "ctc2 %6, $6\n\t"
        "ctc2 %7, $7"
        : "=&r"(t0), "=&r"(t1), "=&r"(t2), "=&r"(t3),
          "=&r"(t4), "=&r"(t5), "=&r"(t6), "=&r"(t7)
        : "r"(matrix));
}
