/*
 * func_80034F88 — best ADMISSIBLE pure-C form. Honest floor 10.
 *
 * `sandbox func_80034F88 --disable all` = score 10, 49 build insns vs 49 target
 * insns, lbu 175 / sb 164 / lui 456 (target census: 49 / lbu 176 / sb 164 /
 * lui 456). Re-measured again in s16 (rederive) as variant base.c,
 * tmp/grind/func_80034F88/s16/results.json. It is NOT a byte match and must not
 * be submitted as candidate-ready.
 *
 * s16 (rederive) added nothing to the body — 13 further structurally different
 * single-object shapes all scored 10 or worse — but it localised the residual
 * exactly and killed the last open dial. The full instruction-aligned
 * side-by-side is in evidence.md under "==== s16 (rederive) ====". In one line:
 * blocks 2 and 4 and the trailing copy loop are instruction- AND
 * register-identical to target; ALL 10 points are in block 1, as (a) the base
 * register being $a0 where target uses $v1 with the byte swapped the other way,
 * and (b) one missing post-store reload (our maspsx nop where target has a
 * second `lbu`), which is why the lbu census is 175 against the target's 176.
 * The $v1-vs-$a0 dial is dead three ways (declaration order inert; a block-1-only
 * live range wins $a1 not $v1; an explicit `register asm("$3")` pin is ignored
 * outright) and would in any case be a net LOSS: $a0 is what makes blocks 2 and 4
 * exact, so switching costs 10 lines to save 5.
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
 *
 * s17 (rederive) re-measured this body as variant a0_base — still 10 at 49
 * insns, lbu 175 / sb 164 / lui 456 — and added nothing to it, but sharpened
 * the ceiling statement in two ways (full write-up under "==== s17 (rederive)
 * ====" in evidence.md).
 *
 * (1) The residual's reload half is now priced EXACTLY. Making block 1's flag
 * READ alone a direct symbol access (`v = D_80106A73;`, one pointer object
 * still) scores 11 at 50 insns with lbu 176 / sb 164 — the target's exact
 * access census, a first for this chassis — and its entire cost is one extra
 * `lui $at`. So the base spends 1 point on the missing reload and that form
 * spends 1 point on the lui that buys it: an exactly balanced trade, because
 * the compiler charges one address materialisation either way. Getting the
 * reload WITHOUT the lui needs the read to go through a register whose address
 * rtx differs from the mask store's — a second address pseudo — a second C
 * pointer object. That is the banned construct, and this is the cleanest
 * arithmetic statement yet of why 10 is a ceiling and not a plateau.
 * Banked at rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c.
 *
 * (2) Two long-standing side questions are closed. The "0x80106A70..73 was one
 * declared 4-byte object" model, which s2 killed by citing relocation records
 * that a PS-X EXE does not have, is codegen-NEUTRAL: spelling every address
 * `&D_80106A70 + 3` gives the identical instruction count and census, and its
 * apparent +2 is a false distance from the unmasked R_MIPS_LO16 addend (the
 * linked bytes are identical). And the frontier's "address from a genuinely
 * different program value" bullet is dead: func_80077D00 returns &D_8009BD24,
 * 0x6AD4F away across a segment boundary, so no non-constant expression in
 * this function yields the flag address.
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
