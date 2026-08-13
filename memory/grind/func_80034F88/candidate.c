/*
 * func_80034F88 — best ADMISSIBLE pure-C form. Honest floor 10.
 *
 * `sandbox func_80034F88 --disable all` = score 10, 49 build insns vs 49 target
 * insns, lbu 175 / sb 164 / lui 456 (target census: 49 / lbu 176 / sb 164 /
 * lui 456). Re-measured this session (s16 forensics, tmp/grind/func_80034F88/
 * s15c/results.json, variant r8.c). It is NOT a byte match and must not be
 * submitted as candidate-ready.
 *
 * WHY THIS FILE CHANGED (s16). candidate.c previously held the THREE-pointer-
 * object body that measures 0 but which the layer-1 cheat-reviewer FAILed on
 * 2026-08-13 15:24 and the Judge FAILed on 2026-08-13 16:36 as the banned
 * multi-handle construct respelled. Its own header said "DO NOT INSTALL AND
 * SUBMIT", which makes it a trap in the slot the pipeline reserves for the best
 * admissible form. That body is banked at
 *   memory/grind/func_80034F88/rejected/three-pointer-objects-judge-FAIL-score0.c
 * and the two-object variant at
 *   memory/grind/func_80034F88/rejected/two-pointer-objects-clean10-t1-banned.c
 * This file now holds the single-pointer-object form, which is inside the
 * Judge's binding constraint ("no form may declare or assign more than ONE C
 * pointer object aliasing D_80106A73").
 *
 * WHAT THE REMAINING 10 POINTS ARE, mechanically (s16 forensics; full write-up
 * in evidence.md under "==== s16 (forensics) ====").
 *
 * The target holds &D_80106A73 in TWO live ranges: $v1 across the 0xF8 mask and
 * the whole bit-1 block, and $a0 set twice — once before the bit-2 block, once
 * before the bit-4 block. Read literally off asm/funcs/func_80034F88.s.
 *
 * Hard registers are handed out per allocno. tools/gcc-2.7.2/global.c:426 is
 * `reg_allocno[i] = max_allocno++;` — one allocno per pseudo, the neighbouring
 * `reg_may_share` branch only MERGES allocnos — and find_reg gives each allocno
 * ONE hard register for its whole live range. GCC 2.7.2 has no live-range
 * splitting. So two base registers need two pseudos.
 *
 * Both routes to a second pseudo that is not a second named C object are dead,
 * measured and read in compiler source this session:
 *   - anonymous references (direct symbol access, pointer-arithmetic temporary)
 *     never create a pseudo at all — GCC keeps the symbol_ref inline in the MEM
 *     address on MIPS. Four spellings measured at 28 / 14 / 16 / 28, all worse
 *     than this form's 10, all with a HIGHER lui census than the target's 456.
 *   - spilling the single pointer so reload rematerialises per reference is
 *     closed by tools/gcc-2.7.2/reload.c:4128-4137, which REPLACES the MEM
 *     address with the REG_EQUIV constant (legal on MIPS) instead of allocating
 *     a reload register; the access degenerates to the plain-symbol family
 *     measured at 28-29.
 *
 * Therefore the 10-point residual is a structural ceiling of the compiler for
 * any single-pointer-object body, not a plateau in the search. Reaching 0
 * requires a second C pointer object, which is the banned construct. The
 * disposition question — is that a legitimate program-logic shape or a
 * register-allocation lever — is not a session's call to make; it is an owner /
 * escalation-modality question, and the escalation packet's evidence base is
 * the s16 block in evidence.md.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
