/* s33 (synthesis, 2026-09-05): re-measured on HEAD = score 10, 49/49 insns,
 * rules_dropped 0.  UNCHANGED as the best admissible form, but the residual is
 * now factored: s33 reached the target's blocks-0/1 $v1 address seat with a
 * SINGLE declared pointer object (rejected/s33-anon-symdiff-block0-blocks01-
 * EXACT-score13.c, blocks 0 and 1 byte-exact, score 13) by putting a
 * local-alloc-confined compiler temp in front of q; the seat swap then moved to
 * blocks 2/3, which this body gets right.  Both halves are individually
 * reachable from one object; jointly they need two pseudos surviving two cse
 * path boundaries, i.e. two named objects (global.c:426) -- the standing ban.
 */
/* MIGRATION NOTE (operator, 2026-09-01): main carries INCLUDE_ASM for this
 * function (asm-until-matched, owner ruling 2026-08-19). Any "HEAD"/"main"
 * state claims in the comments below predate that migration and describe the
 * chassis they were measured on, not the committed tree - verify against the
 * dated evidence.md facts before relying on them. */
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
 *
 * s23 (escalation / owner-directive SOLVER) re-measured this body ON THE
 * MIGRATED TREE and did not change a line of it. Two things it establishes.
 *
 * (1) CHASSIS. Since s22 the asm-until-matched sweep-3 migration (commit
 * 3a5882b2) made src/code6cac_b.c:2389 `INCLUDE_ASM("asm/funcs",
 * func_80034F88);`: the function now carries ZERO regfix rules, ZERO asmfix
 * rules and ZERO cheat-asm. s22's "main keeps the 31 regfix rules + three
 * scheduling barriers" is STALE — there is no retained cheat and no accepted
 * debt for this function. Installing this body (plus one `extern u8
 * D_80106A70;`) sandboxes at 10, 49/49 insns, rules_dropped 0, so the floor is
 * chassis-valid and every banked conclusion carries over.
 *
 * (2) THE SOLVER (the owner's 2026-08-24 queue directive, unexecuted through
 * s22, executed in full this session; report at
 * tmp/grind/func_80034F88/s23/ra_solver_report.txt). tools/ra_solver's forward
 * model is EXACT here — 9 allocnos, simulate.py reproduces sort order MATCH and
 * dispositions 9/9 vs the instrumented-cc1 ALLOCDBG ground truth, allocation
 * {72:$a1, 73:$v1, 74:$a0, 77:$v1, 78:$v0, 81:$v1, 82:$v0, 85:$v1, 86:$v0} with
 * pseudo 74 = this file's `q`. Both inverse queries for target's block-1 naming
 * (`--swap 74,73` and `--goal {"74": 3}`) return FORECLOSED over 218 single
 * perturbations across all eight modelled input classes. The tool names the
 * reason: $v1 and $a0 NEVER APPEAR AS HARD REGISTERS in this function's pre-RA
 * RTL, so global.c set_preference can never record a preference for either —
 * the preference lever does not exist here, which is why 22 sessions of
 * live-range / ref-count / declaration-order surgery were inert. The residual
 * is therefore not an "RA tie-break"; it is not a tie at all.
 *
 * And the ceiling is now measured from the ALLOCATOR'S INPUT SIDE: s20's
 * census-exact diagnostic chassis (rejected/roundtrip-fresh-pseudo-...-
 * DEAD-ARITH.c) does NOT create a second address pseudo — still 9 allocnos,
 * still the single pseudo 74, only nrefs 10->15 and livelen 29->30 — and is
 * FORECLOSED too. So on the one chassis whose instruction multiset matches the
 * target, the whole residual is RA and RA is closed. Target's two live base
 * registers need a second address ALLOCNO, i.e. a second pseudo, i.e. a second
 * C object aliasing D_80106A73: the banned construct.
 *
 * The only unmodelled mechanism the measurements do not exclude is local-alloc's
 * SUGGESTED-REGISTER pass (no DImode quantity here — all 9 allocnos are SI — and
 * no reload spill-retry, one spill and 9/9 pre-reload). Modelling it needs an
 * env-gated fprintf in tools/gcc-2.7.2/local-alloc.c block_alloc plus a parse in
 * tools/ra_solver/local_extract.py, both outside a grind session's surface. That
 * is the function's only live re-attempt route and the single decidable question
 * in the 2026-08-26 decisions.md entry.
 *
 * s24 (escalation — the owner's 2026-08-30 ruling 1 EXECUTED) re-measured this
 * body — still 10 at 49/49 insns, rules_dropped 0 — and did not change a line of
 * it. It closed the function's LAST open question. s23's frontier was one item:
 * local-alloc's SUGGESTED-REGISTER pass, the sole mechanism its FORECLOSED
 * verdicts were conditional on, needing instrumentation outside a grind
 * session's surface. The owner granted that work on 2026-08-30 and it now exists
 * on main (commit 70d6c905, BB2_SUGG_DEBUG + `local_extract.py --suggest`, model
 * validated 1578/1578 preference / 947/947 assignment over all 32 TUs). Run here
 * for the first time, it comes back INERT three independent ways:
 *   (a) all seven local-alloc quantities carry ncopysugg=0 / nsugg=0 / empty
 *       suggestion sets, and all seven find_free_reg calls have
 *       `used == first_used` — the identity restriction, so the pass cannot
 *       change a seat (local-alloc.c:2205-2213 is its only consumer);
 *   (b) the mechanism: combine_regs (local-alloc.c:1859-1899) records a
 *       suggestion ONLY when one side of a tieable copy is a HARD register, and
 *       $v1/$a0 never appear as hard regs in this function's pre-RA RTL — the
 *       same structural fact that kills global.c set_preference, so this was
 *       never an independent second chance;
 *   (c) decisively, the local-alloc pseudo set {76,79,83,87,89,91,93} is DISJOINT
 *       from s23's global allocno set {72,73,74,77,78,81,82,85,86}: pseudo 74,
 *       the `&D_80106A73` address object whose seat is the whole residual, is a
 *       GLOBAL allocno that local_alloc never touches.
 * qty_size measures 1 on every quantity, so the other named hook gap (DImode
 * mispricing) is absent too. `inverse.py local --sugg --goal {"74": 3}` returns
 * FORECLOSED on all four blocks. Report: tmp/grind/func_80034F88/s24/sugg_report.txt.
 *
 * The RA residual is therefore FORECLOSED UNCONDITIONALLY, not conditionally.
 * Both endgame-lock gates were re-run and both still FAIL (scan_hand_coded tier
 * LOW 0/8; the SOTN-master precedent census is NEGATIVE from a second independent
 * source this session — docs/reference/sotn-construct-index.md's 163 PSX
 * pointer_alias rows contain ZERO functions holding two live handles on one
 * address). This file remains the best ADMISSIBLE pure-C form at floor 10.
 *
 * s25 (synthesis) re-measured this body ON HEAD -- still score 10, 49/49 insns,
 * rules_dropped 0 -- and did not change a line of it. It re-derived the full
 * 49-vs-49 instruction alignment from scratch (evidence.md "==== s25
 * (synthesis) ===="), and it changes how the residual should be STATED:
 *
 * (1) The residual is blocks 0 and 1 wearing blocks 2/3's register convention.
 * Target: base=$v1 / value=$a0 in blocks 0-1, base=$a0 / value=$v1 in blocks
 * 2-3. This body: base=$a0 / value=$v1 EVERYWHERE, so blocks 2, 3, the copy
 * loop and the epilogue are exact and blocks 0-1 are mirrored. The $v1/$a0
 * "dial" 22 sessions chased is therefore a NET LOSS if it were ever granted:
 * swapping the one seat recovers ~8 points in blocks 0-1 and gives back ~12 in
 * blocks 2-3, which are currently 100% exact. This body is the ARGMAX of the
 * one-address-object seat space, not a point in it we failed to improve.
 *
 * (2) s20's fresh-address-pseudo escape is position-specific and does not reach
 * the residual. Moved to the block-1/block-2 boundary the round-trip is folded
 * away completely -- the emitted stream is BYTE-FOR-BYTE IDENTICAL to this
 * body's (score 10, 49 insns, empty diff). Used to REPLACE a block's
 * re-materialisation it deletes the lui/addiu pair (score 11 at 47 insns; 13 at
 * 45 for two blocks). The construct is a re-materialisation ENABLER (it makes a
 * cse-redundant `q = &D_80106A73;` set survive), not an allocno splitter: the
 * RTL temp is copy-propagated into q's pseudo and q still has exactly one
 * allocno (global.c:426). s20's "one C object = one pseudo is not a theorem"
 * reading is narrowed accordingly.
 *
 * (3) The DECLARATION was measured for the first time. Declaring
 * `extern u8 D_80106A70[4];` in include/code6cac.h and spelling the copy loop
 * `D_80106A70[i] = ...` scores 10 at 49 insns -- EXACTLY this body. So the
 * declaration pun on the loop line below (`*(&D_80106A70 + i)` against a scalar
 * extern, which the dispatch auto-scan flags and layer-1 would FAIL) is
 * removable at ZERO codegen cost. It is not a single-file edit: the header
 * change drags src/code6cac.c:340-342/:345 and src/code6cac_c_mid.c:205 with
 * it, so it is an integration handoff to be executed whenever this function
 * next has a submittable body. Reaching the flag byte through the same array
 * (`q = &D_80106A70[3];`) is byte-identical after link too (score 12 is the
 * known false R_MIPS_LO16-addend distance; objdump -r confirms
 * %lo(D_80106A70)+3 == the target's 0x6A73 immediate) -- but it creates no
 * second live base, so the aggregate model is hygiene here, not a lever.
 *
 * (4) Sibling func_80034708 (same TU, same byte) has no candidate.c and sits at
 * floor 542, so there was nothing to transplant -- but its target holds
 * D_80106A73 through a SINGLE long-lived pointer local (`s5`, cases 8/9
 * `^= 1` / `^= 2`), independent corroboration that this body's object model is
 * the codebase's idiom for this byte and that the plain-symbol family (28/29,
 * s16) is not.
 *
 * (5) s26 (solver) RE-FRAMES the residual and the ledger's F1/F2 ceiling
 * argument above. `inverse_compose.py classify` -- never run on this function
 * before -- reports FIRST DIVERGENCE: PRE-RA, not RA: `ours only: nop` /
 * `target only: lbu #,0(#)`. The cc1 dumps name the pass (the `(mem:QI` count
 * in this function's region is 8 in .rtl and .jump, 7 in .cse and thereafter),
 * so cse.c's first pass deletes block 1's byte reload (.rtl insn 29) by
 * forwarding block 0's stored value (insn 20), whose high bits are provably
 * zero because insn 18 is `and(...,248)`. So the residual is TWO defects, not
 * one: D1 = a missing instruction, upstream of every RA/scheduler model, and
 * D2 = the seat convention, which is only askable once D1 is fixed.
 * Eight one-object honest shapes fail to restore the load (split-read into
 * both arms, split-read in all three blocks, conditional expression, do-
 * while(0) around block 0 and around block 1, statement-order swap, u8-typed
 * value local, and an explicit fourth `q = &D_80106A73;` in block 1) -- cse
 * follows jumps at -O2, and a re-set of the SAME pseudo to a value it already
 * holds is rewritten to a register copy without invalidating the memory entry.
 * The RA search was then RE-RUN on a multiset-matching chassis (the dead
 * round-trip diagnostic, which classifies RA at 10/49) with the FULL blocks-0/1
 * disposition as the goal, and it still FORECLOSES -- so s23/s24's foreclosure
 * is upgraded, not voided. What restores the target's `lbu` at zero instruction
 * cost is specifically block 1 reading through a DIFFERENT pointer pseudo
 * (measured: y02, two objects, score 13, classify RA, still three %hi/%lo
 * pairs); two objects with block 1 still reading the first pointer (y01) is
 * score 21 and still PRE-RA. That is the s26 ruling question.
 *
 * (6) s26 (solver, continuation run) opened the SCHEDULER axis for the first
 * time in 26 sessions. tools/sched_solver extracts this TU at parity=True;
 * goalmap reports GOAL == OURS (identity) for all 11 blocks in both scheduler
 * passes; and perturb.py finds that EVERY vector reaching the target's
 * block-3 order (the address re-materialisation before block 1's store)
 * begins with `del_dep 56 <- 49`, with the single atom `del_dep 56 <- 49`
 * being the whole minimal vector in pass 2. That edge is the REG_DEP_ANTI
 * emitted at tools/gcc-2.7.2/sched.c:1738 because insn 49 USES and insn 56
 * SETS the same address pseudo, and schedule_block never releases an insn
 * with an unsatisfied LOG_LINK -- so the ordering triple is not a tie and no
 * statement move can reach it while one pseudo carries the address.
 * The obvious escape -- reach the byte by its plain symbol in the later
 * blocks, creating a fresh pseudo per use with no second declared object --
 * is false at the mechanism: CONSTANT_ADDRESS_P
 * (tools/gcc-2.7.2/config/mips/mips.h:2369) accepts SYMBOL_REF, so a direct
 * global access stays (mem (symbol_ref)) and allocates no address pseudo at
 * all; measured in h1's bytes as `lui` + `lbu ...%lo(sym)(...)` with the
 * %lo FOLDED and no addiu, 48 insns against the target's 49 unfolded
 * lui+addiu pairs. Five hybrid pointer/symbol bodies measure 16-28
 * (rejected/hybrid-h[1-5]-*.c). This body remains the best measured form.
 *
 * (7) s27 (forensics) turns s26's saturation argument into arithmetic. cse.c
 * keys a REG in the value table on its QUANTITY, not its number
 * (`hash += ((unsigned) REG << 7) + (unsigned) reg_qty[regno];`,
 * tools/gcc-2.7.2/cse.c:1905), so a new address quantity exists only after
 * setting the pseudo to an rtx cse cannot unify -- which is a fresh
 * lui+addiu materialisation. Ten bodies varying which of the four
 * byte-access groups re-assigns `q` give two exact identities: surviving
 * `lbu` == number of assignments to `q`, and build insns == 41 + 2 x that
 * number (1 set: 47/1/score 23; 2: 47/2/20; 3: 49/3/10 = this body; 4:
 * 51/4/12). The target needs FOUR loads at THREE pairs in 49 insns, which is
 * off that line -- so no assignment pattern of one pointer object reaches its
 * multiset. The complete cse input enumeration (volatile mem cse.c:1943,
 * CALL/UNSPEC_VOLATILE cse.c:1967, hard reg cse.c:1902, invalidate_memory
 * cse.c:7599, path boundary at a code label) is checked against the TARGET's
 * own stream -- `sb`, `lw`, `lbu`, no call, no store, and its earliest label
 * at insn 16 -- so the address quantity is the only live input. A zero-cost
 * fourth quantity needs a set whose source cse unifies (rewritten to a copy,
 * propagated away by flow) into a DIFFERENT pseudo: a second declared pointer
 * object, i.e. the standing ban.
 *
 * (8) s28 (forensics) re-measures this body on HEAD -- score 10, 49/49 insns,
 * classify PRE-RA with `ours only: nop` / `target only: lbu #,0(#)` -- and
 * makes the residual finite: the swap is 1:1, so the target's `lbu` at
 * 0x80034FB4 fills the load-delay slot of `lw $v0,0x20($a1)` that we fill with
 * a nop. Restoring block 1's reload is therefore INSTRUCTION-FREE; every route
 * measured to date costs +2 (a fourth address materialisation: 51 insns; a
 * CALL between the store and the read: 51 insns). s28 also completes s27's cse
 * input enumeration with two gates it had missed and closes both:
 * cse.c:7326 `in_libcall_block` (unreachable -- `*q &= 0xF8` contains no
 * libcall operation) and cse.c:7004-7027, the bitfield ZERO_EXTRACT SET_DEST
 * gate, which is unreachable on MIPS because mips.md:2901's `insv` expander
 * FAILs for any field that is not 32-bit byte-aligned (measured: bitfield
 * bodies b1/b1b at 51 insns / score 13, `zero_extract` count 0 in .rtl/.jump/
 * .cse). MEM_IN_STRUCT_P asymmetry between the store and the reload does not
 * defeat forwarding either. The one zero-cost invalidator known in 28 sessions
 * remains the dead pointer round-trip, whose gate s28 located in the RTL:
 * setting the address pseudo bumps `reg_tick` in `invalidate` (cse.c:1539),
 * which un-validates the `(mem:QI (reg 74))` entry the store recorded.
 *
 * (9) s29 (rederive) re-measures this body on HEAD -- score 10, 49/49 insns,
 * rules_dropped 0 -- and does not change a line of it. It banks the first full
 * instruction-by-instruction alignment (evidence.md "==== s29 (rederive) ===="):
 * blocks 2/3, the copy loop, the prologue and the epilogue are byte-exact, and
 * all ten points are in blocks 0-1. Re-auditing the closest banked form (the
 * s20/s25 dead round-trip, the only body whose multiset matches the target) on
 * the current chassis gives score 10 at 49 insns again -- and, aligned
 * instruction by instruction, shows that RESTORING BLOCK 1'S RELOAD IS WORTH
 * ZERO POINTS. The whole residual is the blocks-0/1 seat convention (7 insns)
 * plus the 3-insn lui/addiu/sb rotation at the block-1 join, both consequences
 * of one missing address allocno. Probe the ALLOCNO, not the reload.
 * s29 also read func_80035280 for the first time in 29 sessions -- this
 * function's inverse, same TU, same p[8] flag word. It holds &D_80106A73 in ONE
 * named u8 * local ($a1, materialised once, three surviving lbu) and DERIVES
 * its D_80106A70 pointer from it (`addiu $a2,$a1,-0x3`), so named pointer
 * locals are this code family's idiom for the byte and the plain-symbol reading
 * of the original is closed. Transplanting that idiom (`u8 *r = q - 3;` for the
 * copy loop) measures 19 at 50 insns and leaves blocks 0-1 BYTE-FOR-BYTE as
 * they are here: an address allocno that does not alias the flag byte does not
 * touch the contested seat.
 *
 * (10) s30 (rederive) re-measures this body on HEAD -- score 10, 49/49 insns,
 * rules_dropped 0 -- and again does not change a line of it. It spends the last
 * unspent sibling inheritance, CD_sync's FAKE-annotated F1 combine-foldable
 * chain-extender (owner ruling 2026-07-01), by assigning it to THIS function's
 * single object `q` (no second pointer object, so outside the standing ban):
 * `q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));` in
 * three placements, all 14 at 49 insns. The dumps say why: .combine folds the
 * SYMBOL_REF difference away completely (three plain
 * `(set (reg) (symbol_ref "D_80106A73"))`), .greg puts all three sets in
 * `(reg/v:SI 4 a0)` -- still ONE allocno -- and `diff base.s a2.s` is four
 * PROLOGUE/EPILOGUE lines and nothing else (frame 24 -> 32). The emitted body is
 * BIT-IDENTICAL to this one; the whole +4 is a phantom frame slot for the folded
 * intermediate. The reg_n_refs perturbation that is load-bearing in CD_sync is
 * inert here, so the F1 family cannot reach a body defect on this chassis.
 * s30 also closes the two remaining un-banked placements of sanctioned shapes:
 * do-while(0) wrapped JOINTLY over adjacent blocks (mask+block1 = 12 at 50;
 * block2+block3 = 10 at 49, an exact no-op on an already-exact region; all
 * three = 12 at 50), and m2c's INVERTED diamond (fresh decompile this session)
 * crossed with the pointer chassis for the first time -- 21 at 45 insns in both
 * the s32 and u8 spellings, because cse folds the duplicated `*q` read and the
 * select collapses, LOSING four instructions.
 *
 * (11) s31 (structural) re-measures this body on HEAD -- score 10, 49/49 insns,
 * rules_dropped 0 -- and does not change a line of it. It spends the structural
 * modality with DUMPS rather than scores: eleven bodies (block-local splits, a
 * named mask intermediate, a separate flag-word local, read-before-condition
 * re-association, one function-scope v/c pair shared by all three blocks, type
 * narrowing of `p`, an empty-else init-then-or spelling, and a hoist of block
 * 1's flag-word read above the mask). SEVEN of them tie at 10/49 and emit a
 * BIT-IDENTICAL instruction stream (`diff base.s vN.s` empty); the four that
 * change the code all move away (12 / 14 / 30 / 33). The .greg dumps show why:
 * the extra C objects create no allocno at all -- base, v5 and v7 print the same
 * nine allocnos, the same conflict graph and the same seats, and even the
 * five-allocno v8 body still puts the address object in $a0.
 * s31 also restates the residual one level lower than "one missing address
 * allocno": .greg prints `;; 74 conflicts: ... 2 3 29`, so the address allocno
 * is INELIGIBLE for $v1 before find_reg runs, because a local-alloc pseudo (the
 * block-0 QImode mask value) is seated there and local-alloc runs first. And it
 * CORRECTS s23: preferences for $v0/$v1 DO exist here (`77 preferences: 3`),
 * via global.c:1709-1713 mapping a copy operand through reg_renumber; what is
 * true is that allocno 74 is never a copy operand, so none can be recorded for
 * it without a second C object aliasing the byte.
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
