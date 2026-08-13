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
 *
 * s18 (synthesis) re-measured this body as variant a0_base — still 10 at 49
 * insns — and added nothing to it either, but closed the last hole in the
 * ceiling proof and produced the strongest statement the function has yielded
 * (full write-up under "==== s18 (synthesis) ====" in evidence.md).
 *
 * (1) The ceiling proof's load-bearing step, "one C object is one DECL_RTL is
 * one pseudo", had only ever been checked for SCALAR pointer locals — and it
 * is not a theorem for AGGREGATES, since a frame-resident array/struct loads
 * each member read into a fresh pseudo. Measured both ways: a single-slot
 * aggregate (`u8 *qa[1];`, `struct { u8 *b; } s;`) is scalarised to ONE pseudo
 * and emits a stream BYTE-FOR-BYTE identical to this file's (10 / 49 insns,
 * same census); a two-slot aggregate does get two address pseudos but becomes
 * frame-resident — prologue -24 → -32, `sw v1,16(sp)` plus per-use reloads,
 * 58 insns, score 35. So the aggregate family neither rescues the single-object
 * floor nor hides a route to the target, and step (d) now rests on a
 * measurement. Banked at rejected/aggregate-address-holder-{single-slot-score10,
 * two-slot-score35}.c.
 *
 * (2) The target's own instruction ORDER proves the ORIGINAL source held two
 * simultaneously-live address objects: `lui $a0,%hi` (80034FC8) and
 * `addiu $a0` (80034FCC) are emitted BEFORE `sb $v0,0x0($v1)` (80034FD0), i.e.
 * the second base is materialised while the first is still the live base of a
 * pending store. With one C object those are the same pseudo, so that set is a
 * def of the register the store reads — no GCC pass may hoist it above the
 * use, at any optimisation level. This build emits the forced opposite order
 * (`sb v0,0(a0)` then `lui a0` / `addiu a0`). The remaining gap is therefore a
 * SOURCE-MODEL gap, not a search gap: the claim is no longer "our reproduction
 * cannot get two base registers from one C object" but "the original source
 * necessarily contained at least two". That is the escalation packet's lead
 * evidence; it is NOT a licence for a session to self-approve a multi-object
 * body (checklist T5), and this file stays the best ADMISSIBLE form.
 *
 * s19 (structural) re-measured this body as variant a0_base � still 10 at 49
 * insns, lbu 175 / sb 164 / lui 456 � and added nothing to it, but closed the
 * structural axis by saturation as well as by mechanism (full write-up under
 * "==== s19 (structural) ====" in evidence.md). Eighteen further forms varied
 * the dimensions never varied ON THIS CHASSIS; TEN of them are exactly
 * codegen-neutral (same insn count, same census, same score): the type of the
 * other live pointer `p`, five spellings of the 0xF8 mask store, block-1
 * statement order, scope flattening, and declaration order both between the two
 * pointer objects and inside block 1. The eight that move the numbers move them
 * away from the target � hoisting a condition lets cse forward a flag store and
 * DELETES a reload (lbu 175 -> 174 -> 173 against the target's 176), and
 * lengthening any live range across the `func_80077D00()` call buys a
 * callee-saved register plus prologue growth. Nothing at source level moves
 * block 1 without changing how many address objects exist.
 *
 * s20 (structural) re-measured this body as variant base.c — still 10 at 49
 * insns, lbu 175 / sb 164 / lui 456 — and added nothing to it, but it
 * RE-DECOMPOSED the residual and closed one half of it (35 further forms; full
 * write-up under "==== s20 (structural) ====" in evidence.md).
 *
 * (1) The "missing memory access" half of the residual is GONE from the search
 * space. The target's second `lbu a0,0(v1)` and our maspsx nop occupy the same
 * position — the load-delay slot of `lw p[8]` — and the load is recoverable at
 * ZERO instruction cost from ONE C pointer object: eight forms that create a
 * fresh address pseudo between the mask store and block 1's read reach 49 insns
 * with the target's EXACT lbu 176 / sb 164 / lui 456 census. They all still score
 * exactly 10, because the recovered load merely trades a "nop vs lbu" difference
 * for an "lbu $v1 vs lbu $a0" one. This kills s17's claim that the reload costs
 * one extra lui and that 10 is an exactly balanced ceiling: the reload half is
 * free, and the whole ceiling is the register half. The construct that does it
 * (`q = q + 3; q = q - 3;`) is dead pointer arithmetic, fails checklist T1/T2/T3,
 * was never installed, and is banked as a DIAGNOSTIC chassis at
 * rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c.
 *
 * (2) The residual is therefore exactly two things, both requiring two
 * SIMULTANEOUSLY LIVE address values: block 1's $a0/$v1 naming (dead a fourth
 * independent way this session — all eight fresh-pseudo forms score 10), and the
 * second base's lui/addiu sitting after block 1's store instead of before it.
 * Hoisting the chassis's existing `q = &D_80106A73;` re-materialisation to just
 * before that store — pure statement order, no new construct — is measured dead
 * at 21 / 51 insns (25 / 48 if done in every block): the store then goes through
 * the NEW pseudo, the old one dies early so nothing is gained, and cse forwards
 * that store into the next block's read and deletes two further reloads. That is
 * s18's instruction-ordering proof reproduced from the source side.
 *
 * s21 (permuter) re-measured this body — still 10 at 49 insns, lbu 175 / sb 164
 * / lui 456 — and added nothing to it, but it ran the FIRST campaigns ever
 * seeded at the honest floor and produced the function's first semantically
 * correct sub-floor find (full write-up under "==== s21 (permuter) ====" in
 * evidence.md). Two campaigns, 81,779 iterations total, both harvested and
 * stopped in session.
 *
 * Seed 1 = THIS body (score 10, the chassis no prior campaign had ever seeded;
 * s4 seeded floor-18 chassis, s5 was directed on 18, s13 seeded the
 * inline-helper 13). 44,626 iterations, permuter metric 455 -> 340, six finds,
 * every one re-scored with the honest sandbox at 10 or WORSE (10 / 12 / 13 /
 * 13 / 14 / 21). No form the permuter can reach from the floor beats the floor;
 * this body is a local minimum of the random-structural neighbourhood as well
 * as of hand enumeration. s4-H1 reconfirmed a second time: the permuter's BEST
 * metric find (340) is sandbox 14, its WORST metric find (455) is sandbox 10.
 *
 * Seed 2 = e1, the direct-symbol block-1 read (score 11, 50 insns, target's lbu
 * 176 census). 37,153 iterations. Its two sub-floor finds are the SAME lever
 * and it is BANNED: a pointer COPY `new_var = q;`, a second C pointer object
 * aliasing D_80106A73, which the Judge's constraint names by that exact
 * spelling. Worth exactly 3 points at +1 instruction (e1's 11/50 -> 8/51).
 * The one that keeps the program's meaning is banked at
 * rejected/permuter-s21-pointer-copy-second-object-score8-BANNED.c; the other
 * deleted block 1's `& 1` and is banked as a MISCOMPILE. Every single-object
 * find on seed 2 scored 13 or 14 — worse than this body's 10.
 *
 * The load-bearing new fact for the escalation packet: the banned two-object
 * form does NOT reach 0. At 8/51 insns its whole residual is still block 1's
 * $a0/$v1 naming plus the `move` and `lui` it spends buying the reload — a copy
 * gives a second pseudo whose value comes FROM the first, which is not the
 * target's second base materialised from the symbol before block 1's store.
 * Zero previously needed THREE objects. So the ceiling statement is now
 * confirmed from a third independent direction (random search from both sides
 * of it), and the sub-floor scores the banned family unlocks are 8, not 0.
 *
 * s22 (escalation) re-measured this body once more — still 10 at 49/49 insns —
 * and did not change a line of it. s22 was the DISPOSITION session: it evaluated
 * the two endgame-lock AND-gates and both FAILED, so the owner's standing ruling
 * of 2026-07-27 was applied and filed in docs/grind/decisions.md as
 * "2026-08-13 — func_80034F88 — OWNER-ESCALATION — RESOLVED BY STANDING RULING
 * (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE".
 *
 *   Gate 1 (hand-written-asm evidence), run for the first time in 22 sessions:
 *   `tools/scan_hand_coded.py --single func_80034F88` = tier LOW, score 0/8, no
 *   STRONG (S1/S2/S6) signal → canonical asm refused.
 *
 *   Gate 2 (SOTN-master precedent for multiple live C handles on one global),
 *   censused first-hand against the local sotn-decomp master checkout (commit
 *   db41b28eee52969244a52cc269c8163d1ed8826a, 1,675 src .c files): NEGATIVE.
 *   Only 21 functions in all of SOTN give two distinct handles the identical
 *   address expression, and every one is a non-matching port, a global-to-global
 *   store, or two genuinely different walkers that merely start at the same node
 *   (ric/maria pl_blueprints `prim2 = prim1 = &g_PrimBuf[self->primIndex];`, with
 *   prim1 then advanced 16 nodes). The `fake`-annotated address-taking lines in
 *   the matched PSX trees are the FakePrim/VertexFake type-punning family, not
 *   second handles.
 *
 * The function is therefore parked as INCOMPLETE — OWNER-ACCEPTED: main keeps
 * the 31 regfix rules + three scheduling barriers that hold the byte-match (the
 * oracle stays green), the retained cheat is NOT sanctioned as a technique, and
 * THIS file remains the best admissible pure-C form at floor 10. It is eligible
 * for re-attempt if a genuinely new pure-C lever or new tooling appears.
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
