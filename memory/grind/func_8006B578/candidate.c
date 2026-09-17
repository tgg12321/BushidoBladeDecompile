/* func_8006B578 — candidate body. Chassis unchanged since session 3; re-measured
 * at sessions 4, 5, 6, 7 and 7b at sandbox --disable all score 2, 200/200 insns,
 * 0 source-level hunks, 0 operand-only hunks, 22 not-scored hunks.
 *
 * MIGRATION BANNER (do not misread this file as HEAD state): src/text1b.c on main
 * carries `INCLUDE_ASM("asm/funcs", func_8006B578);` per asm-until-matched. This file
 * is NOT on main. It is the body each session applies to src/text1b.c for the duration
 * of its measurements and then reverts. C lands in src/ exactly once, at COMPLETED-C.
 *
 * WHY IT IS NOT AT ZERO, AND WHY IT NEVER CAN BE VIA THE SANDBOX (session 7, H13/H14):
 *   - All 22 remaining diff hunks differ by ONE constant object offset (0x1A294) with no
 *     non-address hunk: there is no branch-sense, scheduling or allocation divergence left.
 *   - The jump table GCC synthesizes for the second switch is CONTENT-IDENTICAL to the
 *     extracted `jtbl_80015988` array (all six entries differ by the same section-base
 *     delta 0x80062164).
 *   - The whole score-2 residual is the dispatch pair `lui at,%hi(..)/lw v0,%lo(..)(at)`:
 *     the reference object (built from asm) names the EXTERNAL symbol jtbl_80015988, which
 *     the rodata-cleanup project parked in a different TU (src/text1a_b_pre_rodata.c:409),
 *     while ours carries a section-relative reloc against our own .rodata. engine/score.py
 *     masks those via two code paths that can never compare equal, so the sandbox score is
 *     pinned at 2 by construction. The FULL-BUILD ORACLE is the only instrument that can
 *     certify this function.
 *   - Closing it is a linker-script + second-source-file change (delete jtbl_80015988, split
 *     text1a_b_pre_rodata.c around it, move build/src/text1b.o(.rodata) from bb2.ld:66 to the
 *     new boundary) — the exact pattern bb2.ld:59-61 already implements for func_80077B30's
 *     switch table. Filed as an INTEGRATION HANDOFF in docs/grind/decisions.md (2026-09-16).
 *
 *   - SESSION 7b VERIFIED THE SPLIT POINT (H18): the rodata run 0x80015940-0x80015A3C is
 *     ENTIRELY src/text1b.c-owned (jtbl_80015940 -> func_80065800, jtbl_80015988 -> this
 *     function, jtbl_800159B0 -> func_8006E534, jtbl_800159D0 -> func_8006ECF4,
 *     jtbl_80015A0C -> func_800747D8 (src/text1b.c:8122 — the s7 sketch wrongly put it
 *     outside text1b.c), jtbl_80015A24 -> func_80077374). build/src/text1b.o has no
 *     .rodata section today (objdump -h), so the bb2.ld:66 slot must be inserted at
 *     exactly 0x80015988, between jtbl_80015940 and D_800159A0, with jtbl_80015988 deleted.
 *   - SESSION 7b ALSO KILLED the one attack that would have avoided the cross-TU change
 *     (H17): dispatching via `goto *(void *)jtbl_80015988[k]` through the extern extracted
 *     table drops to 95 insns because jump.c:185 deletes the five blocks reachable only
 *     through the computed jump, and it would hardcode original code addresses regardless.
 *   - SESSION 8 (solver) CLOSED THE RA AND SCHEDULER AXES with a typed verdict:
 *     tools/ra_solver/inverse_compose.py classify (object mode) returns FIRST DIVERGENCE
 *     PRE-RA, "no backend � the residual is upstream of every model", so inverse.py and
 *     inverse_sched.py/perturb.py are mechanically inapplicable here. The PRE-RA label is
 *     itself a RELOCATION-rendering artifact, not an RTL-shape difference: the classifier
 *     blanks registers but not reloc symbols, and objdump -r shows the target carrying
 *     R_MIPS_HI16/LO16 against the external symbol jtbl_80015988 exactly where ours carries
 *     R_MIPS_HI16/LO16 against our own .rodata � same reloc types, same two instruction
 *     slots, offsets differing by the same constant 0x1A294. Whole-stream check: 22 unmasked
 *     differences, ALL branch/jump targets, distinct delta set exactly {0x1A294}; masked
 *     residual exactly 2 insns (#74 lui at, #76 lw v0).
 *   - SESSION 8 RE-AUDITED the two s5 "score-neutral" kills with the BYTE instrument (the
 *     score is pinned at 2, so score-neutrality alone could hide a masked-operand change):
 *     `s16 hi` and the reversed local declaration order both measure score 2 / 200 insns /
 *     delta set {0x1A294} / the same 2 masked hunks. Both are BYTE-EQUIVALENT to this body,
 *     not merely score-equal. This body stays the form of record; those two respellings are
 *     interchangeable with it for the integration handoff.
 * No FAKE / cheat constructs anywhere in this body — ordinary C only (switches, u32 casts,
 * real named intermediates holding real values, and forward `goto`s into the switch's own
 * shared consequence block that mirror the target's own physical layout, see H5).
 * tools/fake_ablate.py reports nothing to ablate.
 */
s32 func_8006B578(s32 *arg0, s32 *arg1) {
    u32 v;
    s32 sp10;
    s32 ret;
    s32 hi;
    s32 var_s2 = 0;

    v = *(u32 *)arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0((u32 *)&sp10, 0, (s16 *)(D_800A34FC + 0xC), &D_800A350C);
    hi = ret >> 16;
    switch (hi) {
    case 1: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0x1400) {
            D_800A34F8 = a0 & ~0x1C00;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) + 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    case 2: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0) {
            D_800A34F8 = (a0 & ~0x1C00) | 0x1400;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) - 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    }

    if (((u32)D_800A34F8 >> 10 & 7) >= 6) {
        goto tail;
    }
    switch ((u32)D_800A34F8 >> 10 & 7) {
    case 0:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~1u;
            u32 bit = f & 1;
            bit ^= 1;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 1:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~2u;
            u32 bit = (f >> 1) & 1;
            bit ^= 1;
            bit <<= 1;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 2:
        if ((ret & 0xFF) != 0) {
            s32 *p = (s32 *)D_800A3524;
            u32 f = (u32)p[8];
            u32 a3 = f & ~4u;
            u32 bit = (f >> 2) & 1;
            bit ^= 1;
            bit <<= 2;
            a3 |= bit;
            p[8] = (s32)a3;
            func_8005C650(0, 0x7F, 0x7F);
        }
    shared_400040:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            {
                u32 f2 = D_800A34F8;
                u32 m2 = f2 & ~0x1C00u;
                s32 c2 = ((f2 >> 10) & 7) + 1;
                D_800A34F8 = m2 | ((c2 & 7) << 10);
            }
        }
        goto tail;
    case 3:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
            var_s2 = 2;
        }
        goto tail;
    case 4:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 3;
        }
        goto tail;
    case 5:
        if (*(u32 *)arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 1;
        }
        goto tail;
    }
tail:
    if (*(u32 *)arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s2 = 1;
    }
    return var_s2;
}
