/* SANDBOX 0 (29/29, --disable all, cheat_asm stripped, rules_dropped 3)
 * measured s1 2026-07-27; RE-MEASURED 0 with the form applied in
 * src/text1b.c s2 2026-07-27 (canonical verdict C, distance 0). ZERO
 * mechanical cheats (no pins, no volatile, no asm, no dead code).
 * RULING RESOLVED — docs/grind/decisions.md 2026-07-27 23:04 PASS:
 * the sibling-parallel form is ADMISSIBLE (SOTN variable-reuse sanction
 * + cross-sibling register-tracking byte evidence); must remain VERBATIM
 * (any drift voids the ruling). Historical context of the conflict:
 *
 * This is a VERBATIM symbol-transcription of committed COMPLETED-C sibling
 * func_8006517C (src/text1b.c:14554, commit 56800c35 2026-06-14, similarity
 * 1.000, 29/29 insns) — the sanctioned walking-pointer-serializes-parallel-
 * loads confirmed case. CONFLICT: this function has an on-file 2026-07-06
 * cheat-reviewer FAIL (rejected/p_reassign_pointer_reuse.c) against the
 * final `p = (s32 *)*bp; ... D_800F0D5C = (s32)p;` mechanism (value routed
 * through reassigned p so the store source lands in $v1), and the s1
 * layer-1 cheat-reviewer FAILed this form on the same family grounds while
 * flagging the sibling + rule for re-audit. Owner must resolve which
 * precedent governs. Measured alternatives (see rejected/): direct
 * `D_800F0D5C = *bp;` = 8; `t = *bp; ...; D_800F0D5C = t;` = 4.
 * Target fact: final insn pair is `lw $v1, 0x8($v1)` (loaded value
 * OVERWRITES the base register — the natural codegen of reassigning the
 * same C variable) then `sw $v1, D_800F0D5C` after the two flag stores.
 */
void func_800651F0(void) {
    s32 *p = (s32 *)D_800A347C;
    s32 *ap = p;
    s32 *bp = p;
    s32 t;
    D_800F0D3C = *ap++;
    D_800F0D40 = *ap++;
    t = *ap;
    D_800F0BC2 = 0;
    D_800F0D44 = t;
    D_800F0D54 = *bp++;
    D_800F0D58 = *bp++;
    p = (s32 *)*bp;
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = (s32)p;
}
