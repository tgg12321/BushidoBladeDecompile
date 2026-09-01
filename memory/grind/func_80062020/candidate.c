/* func_80062020 (src/text1b.c) - CANDIDATE (best UNCONTESTED form), honest floor 4.
 *
 * MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): func_80062020 is
 * represented on main as `INCLUDE_ASM("asm/funcs", func_80062020);` at src/text1b.c:3932.
 * This body was pasted over that line during grind s11 ONLY to re-measure the floor, and
 * src/text1b.c was restored to HEAD immediately afterwards. It is NOT applied to main.
 *
 * MEASURED (grind s11, 2026-08-31, live chassis, this body in place):
 *   sandbox func_80062020 --disable all -> score 4, build_insns 35, target_insns 38,
 *     rules_dropped 0, cheat_asm_stripped 166 (all from OTHER functions in text1b.c)
 *   Identical to the digit to the s8/s9/s10 measurements - the chassis is stable across
 *   four sessions and every banked chassis-relative conclusion remains spendable.
 *
 * WHY THIS BODY AND NOT THE DISTANCE-0 BODY. The distance-0 body (mixed epilogue: row[1]
 * and row[2] through the pointer, column 0 through the full `*(s32 *)((u8 *)&D_800F1198 +
 * ofs)` expression) reached distance 0 and a full-build oracle SHA1 match, and the ordered
 * adjudication mandated by owner ruling 6a (docs/grind/decisions.md:14836) and unblocked by
 * the owner ruling of 2026-08-31 was then RUN TO COMPLETION on it: Judge 2026-08-31 22:25
 * FAIL (comment-only defect), the comments were fixed, and layer-1 2026-08-31 22:31 FAILed
 * the fixed submission ON THE MERITS ("the exact construct already FAILed by two prior
 * Judges; deleting the incriminating comments does not change what the code does"). The
 * construct is now a mechanically-enforced entry in state.json banned_constructs, and
 * respelling a banned construct is the same construct. Ruling 6a's own "proof fails" branch
 * therefore governs: fidelity-limited INCLUDE_ASM at floor 4. The distance-0 body is banked
 * at rejected/judge-fail-0831-2224.c and rejected/layer1-fail-0831-2231.c; do not resubmit
 * it, in any spelling, absent a new owner grant.
 *
 * WHAT THIS BODY COSTS. It emits `sw $zero,0($v0)` for column a where the target emits the
 * LO_SUM lui/addu/sw triple - 35 build insns against a 38-insn target, residual 4. It is
 * clean pure C: 0 rules, 0 pins, 0 volatile, 0 dead vars, one uniform tree shape for all
 * three terminator stores, no construct that any reviewer has objected to across s1-s11.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    p[2] = 0;
    p[1] = 0;
    p[0] = 0;
}
