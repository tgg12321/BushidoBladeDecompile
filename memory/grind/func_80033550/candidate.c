/* func_80033550 — MATCHED (s16, 2026-09-03, synthesis modality).
 *
 * sandbox func_80033550 --disable all = score 0, target_insns 34,
 * build_insns 34, rules_dropped 0; full clean-driver `build` SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. Pure C, zero cheat-asm,
 * zero FAKE constructs, no header changes.
 *
 * THE FINDING (fifteen sessions of RA micro-levers were attacking the wrong
 * question). The residual was always the same 4 bytes: the arg0 pointer sat in
 * $a1 in every build and in $a3 in the target. The ledger's own instrumentation
 * (s14/s15) had reduced that to exact arithmetic: find_reg's pass-0 scan
 * (global.c:996-1001) needs hard_reg_conflicts[72] superset-of {2,3,4,5,6} to
 * skip past $a1/$a2, and the ledger proved no byte-free occupant could be put
 * in $a1 and $a2 while the body was written as three scalar loads followed by
 * three scalar stores. That premise was the error, not the arithmetic.
 *
 * Writing the tail as a single 12-byte STRUCT ASSIGNMENT instead of three
 * scalar load/store pairs changes the RTL that RA sees. GCC 2.7.2's MIPS block
 * move (expand_block_move / movstrsi, mips.c) emits all three loads before all
 * three stores AND keeps the source address register live across the whole
 * pattern, so the pointer pseudo no longer dies at the third load. Measured
 * with BB2_FINDREG_DEBUG=72 (tmp/grind/func_80033550/s16/findreg_stderr.txt):
 *
 *   scalar-triple body:  conflicts 2 3 4 29    -> pass0 first free 5  = $a1
 *   struct-copy body:    conflicts 2 3 4 5 6 29 -> pass0 first free 7 = $a3
 *
 * i.e. the conflict set the ledger had been trying to manufacture with dead
 * locals, live-range padding, duplicated arms and chain-extenders arrives for
 * free — and at exactly 34 instructions — the moment the copy is expressed as
 * the aggregate copy it actually is. No preference route, no someone_prefers,
 * no FAKE carrier was needed; the s14/s15 someone_prefers frontier is moot.
 *
 * The three stores compile to `sw $x, D_80107850+N($2)` (one symbol, offsets
 * 0/4/8) where the target asm names D_80107850 / D_80107854 / D_80107858 —
 * splat invented those per-word names from the %hi/%lo pairs; both spellings
 * assemble+link to identical bytes, confirmed by disassembling the linked ELF
 * (tmp/grind/func_80033550/s16/linked_disasm.txt: all 34 words identical to
 * asm/funcs/func_80033550.s).
 *
 * Header note: NO change to include/code6cac.h. D_80107850 stays
 * `extern s32` and the record type is applied at the use site by cast, so this
 * does NOT claim the aggregate-merge family (no-new-park-categories.md
 * 2026-08-17) and needs no base-register/stride evidence.
 *
 * Paste the typedef + body over `INCLUDE_ASM("asm/funcs", func_80033550);`
 * in src/code6cac_b.c. LF line endings required.
 */

/* 12-byte record copied into slot i of the table at D_80107850; D_800A3918
   is the parallel 6-entry in-use flag array. */
typedef struct {
    s32 word0;
    s32 word1;
    s32 word2;
} Word3;

void func_80033550(s32 *arg0)
{
    s32 i;

    for (i = 0; i < 6; i++) {
        if ((*(&D_800A3918 + i)) == 0) {
            break;
        }
    }
    if (i == 6) {
        return;
    }
    *(&D_800A3918 + i) = 1;
    *(Word3 *) (((u8 *) (&D_80107850)) + i * 12) = *(Word3 *) arg0;
}
