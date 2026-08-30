/* func_80062020 (text1b.c) — CANDIDATE, honest pure-C floor 4 (NOT a match).
 *
 * RE-MEASURED THIS SESSION (grind s8, forensics modality, 2026-08-30) on the live
 * chassis, with this body pasted over the INCLUDE_ASM line at src/text1b.c:3853:
 *   sandbox func_80062020 --disable all -> score 4, build_insns 35,
 *     target_insns 38, rules_dropped 0
 *
 * WHY THIS BODY AND NOT THE s7 ONE: the s7 candidate spelled the last store as
 * `*(s32 *)((u8 *)&D_800F1198 + ofs) = 0;` while cols b/c went through `p`. That
 * dual-spelling epilogue closed at distance 0 but was FAILed by the Judge
 * (docs/grind/decisions.md 2026-08-25 21:17) and again by the layer-1
 * cheat-reviewer (2026-08-30 18:43); it is now a BANNED construct for this
 * function in state.json and is banked at rejected/layer1-fail-0830-1843.c.
 * This file therefore holds the best UNCONTESTED form again: one uniform tree
 * shape for all three terminator stores.
 *
 * THE RESIDUAL (4 insns): target writes the terminator row as
 *   la $v0,D_800F1198 ; addu $v0,$v1,$v0 ; sw $0,8($v0) ; sw $0,4($v0)   <- shared
 *   lui $at,%hi(D_800F1198) ; addu $at,$at,$v1 ; sw $0,%lo(D_800F1198)($at) <- LO_SUM
 * i.e. TWO address forms on one row. This body emits `sw $0,0($v0)` for column a.
 *
 * KEY LEVERS retained: (1) s1 — the source row is read through a FIXED-base
 * indexed form so GCC strength-reduces it to one walking giv; (2) s2 — `ofs`,
 * the loop byte-offset biv allocated to $v1, is reused to carry the terminator
 * index, which seats the index in the target's register.
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
