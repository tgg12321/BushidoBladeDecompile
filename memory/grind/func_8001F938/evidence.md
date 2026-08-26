# Evidence bank — func_8001F938

## s1 recon (2026-07-23) — floor 11 -> 5 measured with clean pure-C levers
- HEAD honest floor CONFIRMED 11 (sandbox --disable all, 107==107, 13 rules).
- Instruction-level diff (build vs target) localizes the entire gap to TWO regions:
  KIND area (+0x6A: target has redundant `lhu $a1; andi $v1,$a1,0xFFFF` + uses
  $5/$3; HEAD reads straight into $3) and the +0x270 ternary.
- kind-split lever `u32 kind_full=lhu; kind=kind_full&0xFFFF` -> floor 6 (matches
  the whole KIND region). BORDERLINE construct, needs cheat-reviewer.
- kind-split + branch-sense flip (`if(probe>=4)raw_or_3=3; else raw_or_3=(u16)*p;`)
  -> floor 5. The branch-flip is unambiguously clean C.
- ROOT CAUSE of residual 5: target's ternary emits TWO unconditional same-address
  loads `lh $v0,0x270; lhu $v1,0x270`. GCC 2.7.2 CSE always merges same-address
  HImode reads to ONE load + in-register signedness materialization. Verified both
  orders: s16-first -> lh+move+andi; u16-first -> lhu+sll;sra (floor 10, worse).
  The 2nd `lhu` is UNREACHABLE in pure C -> irreducible >=1-insn ternary gap; the
  other residuals ($v0-vs-$v1 reg, andi-vs-sll) are all coupled to it.
- Fields at +0x6A/+0x270 are ordinary game-state entity-struct fields (entity base
  D_80101EC8, stride 0x44C, per sibling func_8001EEB4) — NOT MMIO/volatile.
- GRINDABLE (floor dropped this session) -> NOT owner-gated. Next: cheat-review the
  kind-split (H1); if clean, banked floor is 5-6.


- WIP rejected_form: {'slug': 'dual-type-probe-load', 'file': 'rejected/dual-type-probe-load.c', 'floor': 2, 'build_insns': 108, 'reviewer_verdict': 'FAIL', 'construct': 's32 probe = *((s16*)(arg0+0x270)); s32 v = *((u16*)(arg0+0x270)); if (probe>=4) v=3; t=v<<16;', 'why': 'Unconditional dual-typed read at same address — codegen-steering with no semantic purpose. Cheat-reviewer default-FAIL.'}

- == imported from memory/wip notes.md ==
# func_8001F938 (code6cac.c) — WIP, NEEDS-DECISION, dual-typed-read question

## TL;DR (2026-06-15)
Comprehensive 41-variant sweep this session. Discovered the pure-C lever set
that drops the honest sandbox floor from HEAD's 11 to **2** (with build_insns
108, off by 1 from target's 107) and to **4** with matched insn count. The
floor-2 candidate (saved in `rejected/dual-type-probe-load.c`) was **FAILED by
the adversarial cheat-reviewer** — the deciding construct is an unconditional
dual-typed read of the SAME address (`s16 probe` + `u16 v` at +0x270), judged
as codegen-steering with no semantic purpose. HEAD's pure-C body ALSO contains
a dual-typed read at +0x270, but in a guarded-ternary form. **Whether that
construct (already in HEAD) is sanctioned is the open question this session
surfaces.**

## What works
Two pure-C levers reduce the honest distance meaningfully:

1. **kind-split** — `u32 kind_full = lhu; u32 kind = kind_full & 0xFFFFU;`
   provokes target's `lhu $a1, 0x6A($a0); andi $v1, $a1, 0xFFFF; ...` shape
   (target's redundant andi after lhu). Drops 5 distance vs HEAD when applied.
   Reviewer noted this as codegen-steering but secondary; treated as dual-use
   with split-init-accumulation.

2. **preload + conditional override + uniform shift** —
   `s32 v = *((u16*)(+0x270)); if (probe>=4) v=3; t = v<<16;` provokes target's
   `addiu $v1,$zero,3; sll $v0,$v1,16` else-branch shape (preventing GCC's
   lui-fold of `3 << 16`). Reuses `v` across both arms so the constant doesn't
   fold. Drops another 2-3 distance.

Combined: floor 2 with bi=108.

## The reviewer FAIL
Reviewer's deciding evidence:
> `s32 probe = *((s16 *)(arg0 + 0x270)); s32 v = *((u16 *)(arg0 + 0x270));` —
> Both load the same address with different sign types. `probe` is used only
> for the `>= 4` branch guard; `v` is used for the subsequent shift arithmetic.
> A single variable with one load would produce identical program semantics.
> The split exists to produce distinct RTL nodes (signed vs. unsigned memory
> access) to influence GCC's instruction selection. Not on the SOTN-accepted
> list. Test 1 (semantic purpose) and Test 2 (human-programmer) both fail.

Verdict: FAIL. Saved to `rejected/dual-type-probe-load.c`.

## The wrinkle — HEAD already has it
HEAD's pure-C body for the saturating ternary is:
```c
s32 probe = *((s16 *)(arg0 + 0x270));
s32 raw_or_3 = (probe < 4) ? (s32)*((u16 *)(arg0 + 0x270)) : 3;
idx = ((raw_or_3 << 16) >> 15);
```
This is **also** a dual-typed read at +0x270 — just in a guarded-ternary form
(the `(u16)` read only executes when `probe < 4`). The reviewer flagged the
unconditional split-into-separate-variables form but did not opine on the
HEAD guarded form. If the construct is fundamentally a cheat, HEAD's pure-C
body is ALSO suspect — the 13 regfix rules just paper over GCC's actual emission
of that source. If the construct is sanctioned (since it's already in main
and this function was "retired" with it 2026-05-14), then the v36 form can be
shipped as candidate.c with the 13 rules retired.

## Levers tried that DIDN'T help

- **Single-read variant** (`s32 v = *((s16*)(+0x270)); if (v >= 4) v = 3; t =
  v<<16;`) — scored 8, bi=104. Removes too many insns; target really has both
  the lh and the lhu.
- **Volatile-qualified u16 read** — scored 5. Volatile changes load semantics
  and emits extra work elsewhere.
- **Two-pointer alias** (`const s16 *p_s; const u16 *p_u; *p_s; *p_u;`) —
  same score as the single-pointer form. GCC's 2.7.2 CSE still merges.
- **Memory union at +0x270** — scored 4 (no better than v30); union didn't
  unlock additional savings.
- **Shift inside each if/else branch** with const else (no preload) — scored
  4 or 5; the `else` branch's `3 << 16` always folds to `lui`.

## Resume steps
1. **READ THIS FIRST.** The deciding question is the dual-typed read.
2. If user sanctions the dual-typed-read pattern for this function (it's
   already in HEAD), apply `rejected/dual-type-probe-load.c` to
   `src/code6cac.c`'s `func_8001F938`, retire the 13 regfix rules with `retire
   func_8001F938`, and verify oracle. The candidate has bi=108 (1 extra insn:
   the `andi $v1,$v0,0xFFFF` that materializes u16 from the s16-loaded $v0
   because GCC CSE'd the two reads). That extra insn is the LAST remaining
   gap; without it bytes would match. New lever idea: try a memory clobber
   that forces a second load — but that itself may be a cheat.
3. If user rejects the dual-typed read, this function is structurally not
   pure-C-closable under GCC 2.7.2 (CSE merges the two reads) and warrants
   either parking-with-evidence or canonical-asm authorization for the ternary
   region.

## Pointers
- `rejected/dual-type-probe-load.c` — the floor-2 form + full reviewer rationale
- `.claude/rules/inline-asm-policy.md`, `.claude/rules/codegen-technique-index.md`
- `memory/feedback/split-init-accumulation-sanctioned.md` — the closest
  sanctioned analogue
- 27-commit history: `git log --all -- src/code6cac.c | grep F938` (and the
  2026-05-14 "retire ... un-pin stabilizer" commits that left these rules).


- [s1] HEAD honest floor confirmed 11 (sandbox --disable all, 107==107 insns, 13 rules dropped).

- [s1] Instruction-diff localizes the entire 11-gap to two regions: KIND (+0x6A) and the +0x270 saturating-ternary.

- [s1] kind-split lever alone: floor 6; kind-split + branch-flip (clean): floor 5, build_insns 108.

- [s1] Residual 5 after clean levers all trace to ONE cause: target's ternary has two unconditional loads `lh $v0,0x270; lhu $v1,0x270`; GCC 2.7.2 CSE always keeps ONE same-address HImode load and materializes the other signedness in-register (sign->zero via andi, zero->sign via sll;sra).

- [s1] Verified both read orders: s16-first -> lh+move+andi; u16-first -> lhu+sll;sra (floor 10). No pure-C spelling produced two loads.

- [s1] The prior floor-2 form (rejected/dual-type-probe-load.c, reviewer-FAILED) also did NOT produce two loads; its win over floor-5 was register/branch shape from the unconditional preload steering.

- [s1] Fields +0x6A/+0x270 are ordinary game-state entity-struct fields (entity base D_80101EC8, stride 0x44C, per sibling func_8001EEB4) - not MMIO/volatile.

- [s1] Function is GRINDABLE (floor dropped this session) - NOT owner-gated.

## s2 structural (2026-07-23) — floor-5 was NOT clean; honest clean floor is 8; dual-read -> ruling
- [s2] Applied candidate (kind-split + branch-flip + guarded dual-typed read): sandbox floor 5, build_insns 108 CONFIRMED.
- [s2] Instruction diff pins the ENTIRE residual to .L8001FA60 (+0x270 clamp/index). All other regions byte-match. Coupled fixpoint: target's `lhu $v1,0x270` (2nd same-address load, also fills lh's load-delay slot) is the SINGLE lever; if emitted, all 5 residual insns collapse (nop, move, andi->sll, li v0->v1, sll reg). Build CSE-merges to one `lh` + `move`+`andi`.
- [s2] union { s16 s; u16 u; } at +0x270: floor 5, still ONE load. GCC 2.7.2 CSE merges union same-offset dual-typed reads too. KILLED (frontier item 2 union sub-probe).
- [s2] CLEAN single-read form (else arm reuses `probe`, no re-read): floor 8, build_insns 105. This is the honest floor using ONLY reviewer-PASSED constructs. The s1 "floor 5 clean levers" was mischaracterized — 5 always relied on the dual-typed read in the else arm.
- [s2] Fresh adversarial cheat-reviewer: kind-split = PASS (split-init family); guarded dual-typed read = FAIL (Tests 1/3/5, no semantic purpose — s16 and u16 provably equal in-range; only justification is "GCC CSE would merge"). Reviewer routes CSE-defeat-via-dual-typed-access to the OWNER as a NEW borderline-construct family needing SOTN evidence + ruling. (tmp/grind/func_8001F938/s2/cheat_reviewer_verdict.txt)
- [s2] Frontier item 2 EXHAUSTED: no semantically-purposeful pure-C construct keeps two loads. The two target loads are adjacent with NO intervening op (no legit memory-invalidation); +0x270 is an ordinary u16 game-state accumulator (func_80027438 `*(u16*)(a0+0x270)+=a2`), no dual-view field/union member/MMIO. Every two-load source is either CSE-merged (union/alias/single-read) or the reviewer-FAILED dual-typed read.
- [s2] => Frontier item 3 active: the target PROVABLY contains a construct (adjacent lh+lhu same address) the reviewer's Test-1 rejects. RULING-REQUEST emitted. src/ left at the CLEAN floor-8 form (no cheat in tree). candidate.c = clean floor-8 form.

## s2 structural (2026-07-23, DECISIVE update) — DISTANCE-0 FORM FOUND; structural fold-defeat KILLED
- [s2] Re-derived the WHOLE gap from full disasm (independent, not handoff): src at HEAD=floor 11; clean floor-8 form (kind-split+branch-flip+single-read) build_insns 105 confirmed. Entire residual is .L8001FA60 (+0x270). Region .L8001FA98 (+0x27E, the SIBLING index) ALREADY byte-matches with `sll,1` because its value is a SUM (vv0+vv1) GCC can't range-bound -> no fold. Region .L8001FA60 folds `(x<<16)>>15`->`<<1` because its value is a single sign-extended s16 (GCC knows the high bits).
- [s2] KEY PROOF: `(X<<16)>>15` keeps ONLY the low 16 bits before shifting, so it is signedness-INSENSITIVE to its result — s16 and u16 reads of +0x270 give IDENTICAL idx for ALL values (not just in-range). Confirms the dual read has ZERO observable effect; it only changes which load GCC keeps. Independently corroborates the reviewer's cheat verdict.
- [s2] STRUCTURAL fold-defeat probes (NEW, both KILLED): P1 `idx = raw_or_3 * 2` (literal, vs the <<16>>15 idiom) -> floor 8, build 105 (fold is spelling-invariant). P2b redundant register mask `raw_or_3 = probe & 0xFFFF` -> floor 8, build 105 — GCC ELIMINATES the mask (subsumed by <<16) then folds on probe's sign-extension knowledge. So NO register-level construct can force the opaque shape; only a second TYPED memory view of the field works. This closes the "redundant register mask" spelling that s1/s2 never tested (they only tested memory-level dual reads).
- [s2] *** DISTANCE-0 FORM FOUND *** (first ever; prior best was floor 2 / bi 108, never a true match): single u16 read + `(s16)` value cast — `s32 u=*(u16*)(a0+0x270); if((s16)u>=4)raw=3; else raw=u; idx=(raw<<16)>>15;` -> sandbox score 0, build_insns 107 == target. Disasm confirms GCC emits `lh $v0,0x270; lhu $v1,0x270; slti; sll $v1,16; li v1,3; sll v1,16; sra 15` = target .L8001FA60 byte-for-byte. This is ONE C dereference + a value cast, not two dereferences. PROVES the function is pure-C byte-closable.
- [s2] Field type: sibling clean func_80027438 (code6cac_b.c:234) accesses +0x270/+0x272/+0x26C/+0x26E as `*(u16*)`. BUT func_8001F938's target reads +0x26E/+0x272 as `lh` (signed) and indexes `sum*2`=`<<1` cleanly; only +0x270 is dual-read. +0x270 is a struct-OFFSET access (no named global / no header typedef to correct — the F2 "retype globals" remedy does NOT apply). Target's compare is signed (slti/lh); a u16 field signed-compared in C emits sltiu, so the `(s16)` cast is unavoidable -> genuine dual-view requirement, not a mistyped-global cleanup.
- [s2] Fresh adversarial cheat-reviewer on the distance-0 form: FAIL (Tests 1/2/3/4/5). "Third respelling of the same signedness-split CSE-defeat construct pre-banned in judge_constraints; F2 SOTN census (2026-07-01) already came back NOT ESTABLISHED; no ruling has landed." next_action = owner ruling-request OR canonical-asm authorization for the region. (verdict inline in outcome; form saved rejected/signed-cast-single-read.c)
- [s2] CONCLUSION: structural is EXHAUSTED for reaching < floor 8 cleanly. The +0x270 region inherently needs signed-compare + unsigned-index of ONE field; a single C value cannot be simultaneously known-signed (slti) and unsigned-opaque (no fold) without giving GCC two typed views = the reviewer-FAILED signedness-split. Not a search gap — a dichotomy. => RULING-REQUEST: a distance-0 pure-C form provably exists but only in the pre-banned F2 family (census NOT ESTABLISHED); owner must rule sanction-family / canonical-asm-authorize-region / keep-INCOMPLETE.

## s2 structural (2026-07-23, continued) — u16-local-PHI fold-defeat: new register-level spelling, KILLED (banned family)
- [s2c] Baseline re-confirmed: clean floor 8, build_insns 105 (src had drifted back to HEAD's guarded dual-read + 13 rules; re-applied candidate.c clean form first).
- [s2c] NEW PROBE (never measured before): `u16 raw_or_3;` (branch-PHI of {3, (u16)probe}) + `idx=((s32)raw_or_3<<16)>>15` -> sandbox floor 4, build_insns 106. The branch-PHI blocks GCC back-substitution so the u16 truncation is NOT subsumed by the <<16, defeating the fold -> unfolded sll16;sra15 emitted.
- [s2c] *** CORRECTS the s2 P2b over-claim *** "NO register-level construct can force the opaque shape; only a second TYPED memory view works" is FALSE. That claim only tested the NON-branched `raw_or_3 = probe & 0xFFFF` (GCC eliminates+folds -> floor 8). A branch-PHI narrow-TYPE truncation IS a register-level construct that defeats the fold (floor 4). The correct statement: register-level fold-defeat EXISTS but is still the signedness-split family (no semantic purpose).
- [s2c] Floor 4 (not 0): one dereference (lh for probe); u16 truncation materializes in-register (andi) where target has a 2nd memory load (lhu). Strictly DOMINATED by the known distance-0 memory dual-read (signed-cast-single-read.c) — a worse cheat, not a new clean path.
- [s2c] Fresh cheat-reviewer FAIL (Tests 1/2/3/5). Algebraically verified zero observable effect: ((s32)(u16)probe<<16)>>15 == (probe<<16)>>15 for ALL probe (the <<16>>15 idiom discards bits above bit15 before re-sign-extend). 5th spelling of the pre-banned +0x270 signedness-split/CSE-defeat family. (tmp/grind/func_8001F938/s2/cheat_reviewer_u16local.txt; form saved rejected/u16-local-phi-truncation.c)
- [s2c] NET: eliminates the u16-local-truncation spelling from future search; confirms the dichotomy holds at the register level too (every fold-defeat = the banned family). src/ kept at clean floor-8. Structural axis remains EXHAUSTED for a clean sub-8; the standing owner-ruling (family banned pending SOTN evidence; function stays INCOMPLETE, search continues) is unchanged.

- [s2] Baseline re-confirmed: clean floor-8 form (kind-split + branch-flip + single s16 read) = sandbox score 8, build_insns 105 (target 107). src/ had drifted back to HEAD's guarded dual-read + 13 rules; re-applied candidate.c clean form.

- [s2] NEW measurement: u16-local-PHI truncation `u16 raw_or_3` (branch-PHI) + `((s32)raw_or_3<<16)>>15` -> sandbox floor 4, build_insns 106. The branch-PHI blocks GCC back-substitution so the truncation defeats the (raw<<16)>>15 -> raw*2 fold.

- [s2] Corrects the s2 P2b over-claim: register-level constructs CAN force the opaque unfolded shape (via a branch-PHI narrow-type truncation) — s2 had only tested the non-branched `& 0xFFFF` register mask.

- [s2] Floor 4 (not 0): the u16 truncation materializes in-register (andi) where target keeps a SECOND memory load (lhu) at +0x270; strictly dominated by the known distance-0 memory dual-read cheat (rejected/signed-cast-single-read.c).

- [s2] Fresh cheat-reviewer FAIL: algebraically verified ((s32)(u16)probe<<16)>>15 == (probe<<16)>>15 for every probe (the <<16>>15 idiom discards bits above bit15 before re-sign-extending). The u16 type has zero observable effect; sole purpose is defeating GCC's fold = 5th spelling of the pre-banned +0x270 signedness-split/CSE-defeat family (guarded ternary / unconditional split / union / two-pointer / single-u16-read+(s16)cast already FAILED).

- [s2] Dichotomy re-confirmed at the register level: every fold-defeat construct (memory dual-read OR register-level narrow-type PHI) belongs to the banned signedness-split family; the ONLY clean forms fold to raw*2 (floor 8). No clean sub-8 lever exists.

- [s2] src/ left at the clean floor-8 form; func_8001F938 stays INCOMPLETE (search continues, per the standing owner ruling — family banned pending NEW SOTN-master-branch evidence).

## s3 structural (2026-07-23) — pure UNSIGNED single-read: fold-defeat WITHOUT a dual view; ledger over-claim corrected; dichotomy re-proven
- [s3] Independently re-derived the +0x270 residual from raw target asm (asm/funcs/func_8001F938.s, NOT handoff): .L8001FA60 = `lh v0,0x270; lhu v1,0x270; slti v0,v0,4; bnez; sll v0,v1,16(delay); addiu v1,0,3; sll v0,v1,16; sra v0,v0,15` — signed load drives the compare, UNSIGNED load drives the index, index UNFOLDED. Sibling .L8001FA98 (+0x27E) matches cleanly with a single `sll ,1` because its operand is a genuine SUM (vv0+vv1) written `*2` in C — no dual read there. Confirms the +0x270 dual-read asymmetry is intrinsic to that block.
- [s3] Baseline re-confirmed THIS session (own measurement): clean floor-8 form (kind-split + branch-flip + single s16 read) = sandbox score 8, build_insns 105 (target 107). src/ had drifted back to HEAD's guarded dual-read (reviewer-FAILED) + non-split kind; re-applied the clean candidate.c form first.
- [s3] NEW PROBE (never measured; NOT in the rejected bank): pure UNSIGNED single read `u32 probe=*(u16*)(a0+0x270); if(probe>=4U)raw=3; else raw=probe; idx=(raw<<16)>>15;` -> sandbox floor 6, build_insns 106. Disasm: ONE `lhu 0x270`, `sltiu v0,v1,4`, `sll v0,v1,16; sll; sra v0,v0,15` — the fold is DEFEATED (unfolded sll16;sra15 matching target) by a SINGLE typed read, no dual view / cast / split / branch-PHI. (tmp/grind/func_8001F938/s3/region_0x270_dichotomy.txt)
- [s3] *** CORRECTS the s2/s2c over-claim *** "NO single-typed read defeats the fold; only a second TYPED memory view works." FALSE. A plain unsigned read defeats it (floor 6). Mechanism: GCC cannot prove a u16-typed value (bit15 may be set) fits signed-16, so `(x<<16)>>15 != x<<1` for it -> no fold. The signed s16 form folds (s16 provably fits) -> floor 8. So read-signedness alone toggles the fold.
- [s3] NOT a candidate / KILLED: the unsigned read forfeits target's SIGNED compare — it emits `sltiu`, target has `slti` (semantically different for field>=0x8000: signed<0<4 uses field, unsigned>=4 uses 3). Floor 6 has BOTH the missing 2nd load AND the wrong compare; cannot reach 0 without adding a signed view = the pre-banned dual read. Strictly dominated by the floor-0 signed-cast-single-read.c.
- [s3] DICHOTOMY re-proven from a fresh angle: a single typed read of +0x270 delivers {signed => fold => floor 8} XOR {unsigned => unfold-but-sltiu => floor 6}, never both. Target needs signed-compare AND unsigned/opaque-index of ONE field simultaneously; that requires two typed views = the pre-banned signedness-split family (already owner-ruled FAIL twice: decisions.md 2026-07-23 10:19 + 10:46). Structural axis remains EXHAUSTED for a clean sub-8; no owner-escalation/park state (owner directed "keep INCOMPLETE, search continues"). Next needed axis is NON-structural: the SOTN signedness-split-family census. src/ kept at the clean floor-8 form; candidate.c unchanged.

- [s3] Independently re-derived (from raw asm/funcs/func_8001F938.s, not handoff) that target .L8001FA60 = `lh v0,0x270; lhu v1,0x270; slti v0,v0,4; bnez; sll v0,v1,16(delay); addiu v1,0,3; sll v0,v1,16; sra v0,v0,15`: the SIGNED load drives the compare and the UNSIGNED load drives the unfolded index.

- [s3] Sibling .L8001FA98 (+0x27E) byte-matches with a single `sll ,1` because its operand is a genuine sum (vv0+vv1) written `*2` in C — no dual read — confirming the +0x270 dual-read asymmetry is intrinsic to that block, not a global codegen quirk.

- [s3] Baseline this session (own measurement): clean floor-8 form = sandbox score 8, build_insns 105 (target 107). src/ had drifted back to HEAD's reviewer-FAILED guarded dual-read + non-split kind; re-applied clean candidate.c form.

- [s3] NEW: pure unsigned single read -> sandbox floor 6, build_insns 106; disasm confirms fold defeated with ONE lhu (no dual view), but compare is sltiu (target: slti).

- [s3] Dichotomy re-proven from a fresh angle: a single typed read of +0x270 gives {signed=fold=floor 8} XOR {unsigned=unfold-but-sltiu=floor 6}, never both. Target needs signed-compare AND unsigned/opaque-index of ONE field simultaneously = two typed views = the pre-banned signedness-split family.

- [s3] Two owner ruling-requests already stand for this function (docs/grind/decisions.md 2026-07-23 10:19 and 10:46), both FAIL on (a) sanction-family and (b) canonical-asm-authorize; disposition (c) keep INCOMPLETE, search continues, NOT parked. So this is neither owner-gated nor a new ruling-request.

- [s3] src/ left at the clean floor-8 form (candidate.c unchanged); no cheat in tree; 13 regfix rules remain (function INCOMPLETE).

## s4 permuter (2026-07-23) — clean-floor8 chassis plateaus at score 320 over ~95k iters; permuter modality exhausted; frame is a coupled byproduct, not a clean lever
- [s4] Built a decomp-permuter workspace (tmp/grind/func_8001F938/s4/ws) using a MINIMAL self-contained base.c (int typedefs + func_8001F938 only — the fn uses ONLY raw `*(T*)(arg0+off)` casts, no globals/calls/structs, so isolated compile == full-TU compile; verified base=105 insns, target=107, same as `sandbox --disable all` build_insns 105). Full-TU base.c was unusable (pycparser can't parse code6cac.c's `GameObj` typedef). target.o from asm/funcs/func_8001F938.s at offset 0; --stack-diffs ON (target has a real 8-byte frame diff). Base permuter score 615 (mostly branch-address displacement noise from the 2-insn length gap).
- [s4] Campaign ran ~95k iterations (~40 min wall) from the clean floor-8 chassis. Best score reached 415 early, then 320 at ~11 min, and NEVER improved past 320 across the remaining ~85k iters. Hard plateau; fresh-seed window exhausted -> harvest --stop. (tmp/grind/func_8001F938/s4/ws/campaign.log + output-320-*)
- [s4] The best find (320) is NOT a match and reached 320 ONLY by injecting TWO cheats: (1) `volatile short pad;` — a dead no-semantic-purpose local named "pad" that forces target's 8-byte frame (addiu sp,sp,-8/+8); (2) `raw_or_3 = raw_or_3;` dead self-assign (Lever-D) in the else arm. Both pure codegen-coercion; rejected per cheat catalog. Saved rejected/permuter-volatile-pad-frame.c.
- [s4] KEY NEW FINDING: even WITH both cheats the score is 320 — the +0x270 fold gap is UNTOUCHED (single s16 read still folds (raw<<16)>>15 -> raw*2). The permuter NEVER generated the banned dual-typed read (the only distance-0 form) in ~95k iters. Confirms from a fresh (random-search) angle that no clean sub-8 lever exists and the dual-read is a narrow, rarely-generated mutation. The target's 8-byte frame is a COUPLED byproduct of the dual-read's spill slot (the distance-0 signed-cast form already had 107 insns incl. frame) — NOT an independent clean lever; the permuter could only manufacture it via the volatile-pad cheat, which buys nothing toward a match.
- [s4] NET: permuter modality is EXHAUSTED for a clean sub-8 on this function. Structural (s1-s3) + permuter (s4) axes both dead. src/ kept at clean floor-8 (sandbox re-confirmed score 8, build_insns 105 after the campaign). candidate.c unchanged. Only the non-structural SOTN signedness-split-family evidence census remains (owner-gated); two owner ruling-requests already stand (decisions.md 2026-07-23 10:19 + 10:46) FAILing sanction-family and canonical-asm-authorize, disposition "keep INCOMPLETE, search continues".

- [s4] Permuter workspace validated: minimal self-contained base.c compiles func_8001F938 to 105 insns (== `sandbox --disable all` build_insns 105); target.o 107 insns; permuter base score 615 (mostly branch-address displacement noise from the 2-insn length gap).

- [s4] Campaign (pid 406) ran ~95k iterations / ~40 min from the clean floor-8 chassis; best score frozen at 320 for the final ~85k iters. Hard plateau; fresh-seed window exhausted -> harvest --stop (0-find harvest is the data point).

- [s4] Best find (output-320-*) is a REJECTED cheat: `volatile short pad;` (dead no-purpose local named 'pad', forces the 8-byte frame) + `raw_or_3 = raw_or_3;` (dead self-assign). Rejected per cheat catalog; saved rejected/permuter-volatile-pad-frame.c.

- [s4] KEY: even with both cheats the score is 320 (fold gap untouched) and the randomizer never produced the banned dual-typed read in ~95k iters -> confirms from a fresh random-search angle that no clean sub-8 lever exists; the s1-s3 signedness-split dichotomy holds.

- [s4] The target's 8-byte frame is a COUPLED byproduct of the dual-read's spill slot (the known distance-0 signed-cast form already had 107 insns incl. frame), NOT an independent clean lever; the permuter could only manufacture the frame via the volatile-pad cheat, which buys nothing toward a byte match.

- [s4] src/ re-confirmed at the clean floor-8 form after the campaign: sandbox score 8, build_insns 105, target 107, 13 rules dropped. No cheat in tree; function stays INCOMPLETE.

## s5 permuter (2026-07-23) — fresh-seed from the untested unsigned floor-6 basin: plateau 505 / 36k iters, +0x270 crux untouched; permuter modality exhausted from BOTH basins
- [s5] Baseline re-confirmed (own measurement): clean floor-8 form (kind-split + branch-flip + single s16 read) = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. src/ had drifted back to HEAD's guarded dual-read (reviewer-FAILED) + non-split kind; re-applied the clean candidate.c form first.
- [s5] Built a STRUCTURALLY-DIFFERENT permuter chassis (tmp/grind/func_8001F938/s5/ws) that s4 never explored: the s3 unsigned-read floor-6 form `u32 probe=*(u16*)(a0+0x270); if(probe>=4U)raw=3; else raw=probe; idx=(raw<<16)>>15;`. Verified base.c compiles to 106 insns; reused s4 target.o at offset 0; --stack-diffs ON; base permuter score 705.
- [s5] Campaign pid 405 ran 36,031 iterations / ~19 min. Best score plateaued at 505 (from base 705) and stayed FLAT for the entire window (5 stale samples / ~7 min continuous; observer loop saw best=505 from t=5s to campaign death). Fresh-seed window exhausted -> harvest --stop.
- [s5] KEY: the best 505 form left the +0x270 crux UNTOUCHED (still the plain unsigned read with sltiu; fold defeated but signed compare forfeited). The 505 gain over 705 traces entirely to unrelated branch-address/frame scheduling noise, NOT to closing the +0x270 gap. The randomizer NEVER restored the signed compare and NEVER generated the banned dual-typed read (the only distance-0 form). Same result as s4 from a SECOND, genuinely-different basin. Saved rejected/permuter-unsigned-basin.c.
- [s5] The unsigned floor-6 basin plateaus WORSE (505) than s4's floor-8 signed basin (320) because the wrong-compare (sltiu) is baked into the chassis and no clean mutation removes it -> re-confirms the signedness-split dichotomy from a fresh angle: single typed read gives {signed=fold=floor8} XOR {unsigned=unfold-but-sltiu=floor6}, never both.
- [s5] NET: permuter modality is now EXHAUSTED from BOTH available clean basins (s4 floor-8 signed / s5 floor-6 unsigned); no third structurally-distinct clean chassis exists to reseed (the floor-4 u16-local-PHI chassis is itself a banned-family form). Structural (s1-s3) + permuter (s4-s5) axes all dead for a clean sub-8. Only the non-structural SOTN signedness-split census (owner; F2 2026-07-01 = NOT ESTABLISHED, owner FAILed twice) remains. src/ kept at clean floor-8; function stays INCOMPLETE, search continues (owner disposition, not parked).

- [s5] Baseline re-confirmed THIS session (own measurement): clean floor-8 form (kind-split + branch-flip + single s16 read) = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. src/ had drifted back to HEAD's guarded dual-read (reviewer-FAILED) + non-split kind; re-applied the clean candidate.c form first.

- [s5] s5 permuter workspace: unsigned floor-6 chassis base.c compiled to 106 insns (matches the s3 floor-6 measurement); target.o 107 insns at offset 0; base permuter score 705.

- [s5] Campaign pid 405 ran 36,031 iterations / ~19 min from the unsigned floor-6 chassis; best score frozen at 505 for the full window. Hard plateau; fresh-seed window exhausted -> harvest --stop (a 0-find harvest of a genuinely-different basin is the data point).

- [s5] KEY: the best 505 form left the +0x270 region byte-for-byte structurally identical to the seeded unsigned-read chassis (== already-banked rejected/unsigned-single-read.c) -- the permuter improved OTHER regions but could not close the crux, never restored the signed compare, never produced the dual read. Confirms from a SECOND random-search basin (s4 was the floor-8 basin) that no clean sub-8 lever exists and the dual-read is not reachable by mutation.

- [s5] The unsigned floor-6 basin plateaus WORSE (505) than the floor-8 signed basin (s4: 320) because the wrong-compare (sltiu) is baked into the chassis and no clean mutation removes it -- strictly re-confirms the signedness-split dichotomy: a single typed read gives {signed=fold=floor8} XOR {unsigned=unfold-but-sltiu=floor6}, never both; both demand two typed views = the pre-banned family.

- [s5] Two owner rulings already stand for this function (docs/grind/decisions.md 2026-07-23 10:19 + 10:46), both FAIL on (a) sanction the signedness-split family and (b) canonical-asm-authorize .L8001FA60; disposition (c) keep INCOMPLETE, search continues (NOT parked). So this is neither owner-gated nor a new ruling-request.

- [s5] src/ left at the clean floor-8 form (candidate.c unchanged); no cheat in tree; 13 regfix rules remain (function INCOMPLETE). Campaign confirmed stopped (observer saw pid death; harvest --stop succeeded).

## s6 forensics (2026-07-23) — divergence named to the exact GCC pass: combine, gated on num_sign_bit_copies==16; ledger "CSE" claim corrected
- [s6] Instrumented cc1 (tools/gcc-2.7.2/build/cc1, -O2 -G0, dumps -dr -dj -ds -dc -dl -dg -dR -df) on two self-contained variants (isolated==full-TU per s4): A_clean (clean floor-8, single s16 read) and C_dist0 (distance-0, u16 read + (s16) cast). A_clean +0x270 = ONE lh + FOLDED sll,1; C_dist0 +0x270 = lh;lhu + UNFOLDED sll16;sra15 == target .L8001FA60 byte-for-byte.
- [s6] FINDING 1: the index fold (raw<<16)>>15 -> raw<<1 is a COMBINE event, not cse. RTL (pre-combine) has the pair as two insns in BOTH variants (A rtl:581 ashift16 + :586 ashiftrt15; C rtl:585/:590). After combine: A collapses to `(ashift (reg/v 125) 1) 181{ashlsi3}` (folded); C keeps `(ashift (reg/v 124) 16) 181{ashlsi3}` + `(ashiftrt (reg 128) 15) 191{ashrsi3}` (unfolded). cse/jump/flow dumps show the pair intact in both -> collapse is uniquely combine.
- [s6] FINDING 1 mechanism (the exact combine decision variable): combine simplify_shift_const rewrites (ashiftrt (ashift X 16) 15)->(ashift X 1) iff num_sign_bit_copies(X) > 16. Signed load (lh=sign_extend mem) -> 17 sign copies -> FOLD -> floor-8 divergence. Unsigned load (lhu=zero_extend mem) -> exactly 16 (bit15 significant) -> FOLD REFUSED -> target's unfolded shape. The PHI of {const 3, else-arm} keeps the min sign-copies (3 has >16), so the else-arm read signedness alone toggles the fold at the ==16 boundary. This IS the s1-s5 signedness dichotomy, mechanized: a single typed read gives 17(fold, wrong index) XOR 16(unfold, but sltiu), never both.
- [s6] FINDING 2: the target's SECOND same-address load (lh;lhu) is ALSO a combine event, not a cse merge. mem-ref count at +0x270 (const_int 624) in C_dist0 per pass: rtl=1 jump=1 cse=1 flow=1 | combine=2 lreg=2 greg=2 sched2=2. combine turns the (s16)u compare operand — sign_extend-via-shifts of the zero-extended lhu — into a direct (sign_extend:SI (mem:HI)) second lh (cheaper than shifting). One C deref + a signedness cast -> two memory loads, manufactured by combine.
- [s6] LEDGER CORRECTION (verify-opus-handoff-claims): s1/s2/candidate.c say "GCC 2.7.2 CSE ALWAYS merges two same-address HImode reads into ONE load." Dumps show cse keeps a single ref in every clean form (only one source deref exists; nothing for cse to merge) and it is COMBINE, not cse, that both folds the clean index and manufactures the dual load. Empirical dichotomy stands; the "CSE-merge" framing was imprecise. Disposition unchanged: the fold-refusal target needs is reachable only via an unsigned/dual view = pre-banned signedness-split family (owner FAILed twice); keep INCOMPLETE, search continues. src/ untouched; candidate.c = clean floor-8. Artifacts: tmp/grind/func_8001F938/s6/ (FINDINGS.md + A_clean/C_dist0 .i.{rtl,cse,combine,flow,...} dumps + .s).

- [s6] cc1 dump execution order (GCC 2.7.2): rtl -> jump -> cse -> flow -> combine -> sched -> lreg -> greg -> sched2.

- [s6] A_clean (single s16 read) +0x270: lh $2,624; move $3,$2; slt $2,$3,4; bne; sll $2,$3,1(delay); li $3,3; sll $2,$3,1 -> ONE load, index FOLDED to *2 (floor 8).

- [s6] C_dist0 (u16 read + (s16) cast) +0x270: lh $2,624; lhu $3,624; slt $2,$2,4; bne; sll $2,$3,16(delay); li $3,3; sll $2,$3,16; sra $2,$2,15 -> TWO loads, index UNFOLDED == target .L8001FA60 byte-for-byte (distance 0).

- [s6] FINDING 1: index fold is a combine event. A_clean.i.combine:488 `(ashift (reg/v 125) 1) 181{ashlsi3}` (folded); C_dist0.i.combine:493 `(ashift (reg/v 124) 16)` + :499 `(ashiftrt (reg 128) 15) 191{ashrsi3}` (unfolded). Pair intact in rtl/cse/jump/flow for both.

- [s6] FINDING 1 decision variable: combine simplify_shift_const folds (ashiftrt (ashift X 16) 15)->(ashift X 1) iff num_sign_bit_copies(X)>16. lh=17 (fold), lhu=16 (refuse). The read signedness of the else-arm alone toggles the fold at the ==16 boundary.

- [s6] FINDING 2: the 2nd same-address load is combine, not cse. mem-ref count at +0x270 per pass in C_dist0: rtl=1 cse=1 flow=1 | combine=2 greg=2. combine rewrites sign_extend-via-shifts of the zero-extended lhu into a direct sign_extend(mem)=2nd lh.

- [s6] LEDGER CORRECTION: 'CSE merges the two same-address loads' (s1/s2/candidate.c header) is imprecise — cse keeps a single ref in every clean form; combine both folds the clean index and manufactures the dual load. Empirical dichotomy stands; disposition unchanged.

- [s6] Sibling .L8001FA98 (+0x27E) folds to sll,1 in both variants because its C source is a literal *2 (RTL enters combine already as (ashift X 1)); no fold decision is involved there — confirming the +0x270 asymmetry is intrinsic to that block's operand provenance.

- [s6] src/code6cac.c untouched (HEAD, git clean); candidate.c unchanged (clean floor-8 form).

## s7 forensics (2026-07-23) — fold gate refined to the SHIFT-OPERAND sign-copies (not load signedness); distance-0 additionally needs a 2nd MEMORY load
- [s7] Independently reproduced s6's instrumented-cc1 dumps (fresh recompile; verify-opus-handoff-claims). CONFIRMED s6 FINDING 1 (fold = combine event: A_clean collapses to `(ashift (reg/v:SI 125) 1)`; C_dist0 keeps `(ashift 124 16)+(ashiftrt 128 15)`; cse/jump/flow keep the pair in both) and FINDING 2 (mem-ref count at const_int 624: C_dist0 = 1 through cse/flow, 2 at combine → 2nd load is a combine byproduct, not cse). Artifacts tmp/grind/func_8001F938/s7/.
- [s7] NEW control B_u16phi (signed `s16` read + `u16`-typed branch-PHI raw_or_3): emits a SIGNED `lh` (mem-count stays 1, byte-identical load to A_clean) yet the index UNFOLDS — combine operand `(ashift:SI (subreg:SI (reg/v:HI 125) 0) 16)` is a HImode subreg = 16 sign copies → fold refused. asm: `lh $2,624; move $3,$2; slt; bne; sll $3,16; sra 15` = floor 4 (== s2c u16-local-phi-truncation).
- [s7] *** REFINES/CORRECTS the s6 framing *** s6 read the boundary as "signed LOAD→17→fold; unsigned LOAD→16→no fold," implying the load type is the lever. B_u16phi decouples them: a SIGNED load that UNFOLDS. The true decision variable is num_sign_bit_copies of the SHIFT OPERAND as combine sees it after its own simplifications; the read signedness only sets it indirectly. Confirmed a 3rd way by D_andmask.
- [s7] NEW control D_andmask (signed read + `probe & 0xFFFF` in the branch else-arm): operand `(and:SI (reg 122) 0xFFFF)` = 16 copies (AND 0xFFFF forces bits31..16=0) → also UNFOLDS with a SINGLE `lh` + `andi` (floor-4 class). CLARIFIES s2 P2b: P2b's "&0xFFFF folds→floor 8" was the NON-branched inline form where combine has the whole and→ashift→ashiftrt chain in one block and drops the redundant AND, recovering probe's 17 copies → fold. The BRANCHED else-arm mask (across a PHI join) keeps the AND → 16 copies → unfold. Placement decides; no contradiction, distinct construct. Banked rejected/branched-and-mask-register-fold-defeat.c.
- [s7] *** DECISIVE THEOREM (dump-proven) *** Unfolding the index (16-copy operand) is reachable at REGISTER level with a single signed load (B via move, D via andi) → but that is floor 4. Target's byte-exact block is `lh;lhu;…;sll16`: the 16-copy operand must be a SECOND MEMORY LOAD (`lhu`). combine only MANUFACTURES that 2nd load from a source giving it a SECOND TYPED MEMORY VIEW of +0x270 (the `(s16)u` dual read → C rewrites sign-extend-via-shifts of the lhu into a direct 2nd `sign_extend(mem:HI)`=lh). A register-level 16-copy value (move/andi) can NEVER become the 2nd lhu. So distance-0 requires BOTH (i) 16-copy operand (unfold) AND (ii) that operand delivered as a 2nd memory load — jointly satisfiable ONLY by a 2nd typed memory view = the pre-banned signedness-split family. (i) alone caps at floor 4. This is the pass-level "why the shipped bytes required two typed memory views" proof for the SOTN census.
- [s7] Disposition UNCHANGED: no clean sub-8 lever; structural+permuter+forensics axes all dead. Owner FAILed the family twice (decisions.md 2026-07-23 10:19+10:46; "keep INCOMPLETE, search continues"). Only remaining sanctioned axis = the non-structural SOTN-master-branch signedness-split-family evidence census → owner (escalation modality). src/ untouched (HEAD, git clean); candidate.c unchanged (clean floor-8 form). Artifacts: tmp/grind/func_8001F938/s7/{FINDINGS.md, A_clean/C_dist0/B_u16phi/D_andmask .c/.s/.i.*}.

- [s7] Independent recompile reproduces s6: fold is a combine event (A_clean folds (ashift reg/v:SI 125 1); C_dist0 keeps the sll16/ashiftrt15 pair; cse/jump/flow keep the pair in both), and the 2nd same-address load in C_dist0 appears at combine (const_int-624 mem-ref count 1->2), not at cse.

- [s7] B_u16phi (signed lh read + u16-typed branch-PHI index) emits a SIGNED lh with a single memory load yet UNFOLDS the index (operand (subreg:SI (reg/v:HI 125) 0) = 16 sign copies). This decouples fold-refusal from load signedness -> the fold gate is num_sign_bit_copies of the SHIFT OPERAND, refining s6's 'load signedness' framing.

- [s7] D_andmask (signed read + branched probe & 0xFFFF) also unfolds with a single lh + andi (operand (and reg 0xFFFF) = 16 copies). Clarifies s2 P2b: the &0xFFFF register mask folds ONLY in the non-branched inline form (combine drops the redundant AND, recovering probe's 17 copies); across a PHI join the AND survives -> 16 copies -> unfold. No contradiction; distinct construct. Banked rejected/branched-and-mask-register-fold-defeat.c.

- [s7] DECISIVE THEOREM: distance-0 requires BOTH (i) a 16-copy shift operand (unfold) AND (ii) that operand delivered as a 2nd MEMORY load (target's lhu). Register-level fold-defeat (B via move, D via andi) satisfies (i) only -> floor 4. combine manufactures the 2nd memory load solely from a 2nd typed memory view (the (s16)u dual read). So (i)+(ii) are jointly reachable ONLY via the pre-banned signedness-split family -> pass-level proof the shipped bytes required two typed memory views of +0x270.

- [s7] Disposition unchanged: no clean sub-8 lever; structural (s1-s3) + permuter (s4-s5) + forensics (s6-s7) axes all dead. Owner FAILed the signedness-split family twice (docs/grind/decisions.md 2026-07-23 10:19 + 10:46; disposition 'keep INCOMPLETE, search continues, NOT parked'). src/code6cac.c untouched (HEAD, git clean); candidate.c unchanged (clean floor-8 form).

## s8 rederive (2026-07-23) — fresh m2c + decomp.me corpus census + Kengo: crux is structure-invariant; func_8009AA68 transplant KILLED; census strengthened
- [s8] Baseline re-confirmed (own measurement): clean floor-8 chassis (kind-split + branch-flip + single s16 read) = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. src had drifted back to HEAD's guarded dual-read + non-split kind; applied clean candidate.c form first, reverted to HEAD at session end (tree clean).
- [s8] FRESH m2c (tools/m2c/m2c.py --valid-syntax; tmp/grind/func_8001F938/s8/m2c_fresh.c): an INDEPENDENT decompiler reconstructs the +0x270 block with TWO typed views — `(u16)*(s16*)(+0x270)<<16` for the index AND `*(s16*)(+0x270)>=4` for the compare. Overall shape differs from candidate.c (switch + nested-if) but the +0x270 crux (two typed views) is INVARIANT. Corroborates the s7 combine theorem from a fresh angle: the dual view is intrinsic to the target bytes, not an artifact of one C shape.
- [s8] decomp.me whole-function similarity search: top match 0.082 (func_8009E4B0) — no structurally-similar gcc2.7.2 function; the damage-scaling clamp idiom is game-specific. Corpus clamp-idiom `(x<<16)>>15` scan: 3 hits, NONE clean (scores 5/4000/110200). No clean transplant.
- [s8] *** DUAL-LOAD CENSUS (3754-scratch gcc2.7.2 corpus) *** 31 scratches have adjacent same-address dual typed loads; 2 are CLEAN (score 0, community-matched): (a) CW0dj func_8009AA68 — `lh;lhu` of a single `s16 angle` field from ordinary pure C (`angle<0x1000` compare -> lh; `angle+0x800` stored to s16 -> lhu); (b) V5c4K func_800A3320 — `lw;lhu` of offset 0, a genuine word+halfword multi-field read (no union/casts). BOTH clean dual-loads are sign-INSENSITIVE (narrowing store / multi-width field). NEITHER is a redundant signed/unsigned split of a sign-SENSITIVE value with no semantic purpose. Corpus has NO clean precedent for func_8001F938's construct -> consistent with the F2 SOTN census (NOT ESTABLISHED). (tmp/grind/func_8001F938/s8/census_dualload.md, scan_dualload.py)
- [s8] Kengo transplant DEAD: kengo_matches.csv maps func_8001F938 -> md_game_mode_data_restart_init at confidence `size-only-ambiguous` (107==107 insns only, 24 candidates, opseq_ratio 0.24). A coincidental instruction-count match, not a semantic sibling; name unrelated to damage-scaling. No usable transplant.
- [s8] *** func_8009AA68 TRANSPLANT MEASURED + KILLED *** Applied the func_8009AA68 shape to the +0x270 block (separate `*(s16*)(+0x270)` re-reads in compare and else-arm, no reused local): sandbox score 9, build_insns 104 (WORSE than clean floor 8). Disasm: ONE `lh v1,624`; the two re-reads CSE-merged; combine FOLDED (raw<<16)>>15 -> `sll,1`. WHY it cannot transfer: `(field<<16)>>15` is sign-SENSITIVE (bit15 -> result sign; s3: signed=fold/floor8, unsigned=sltiu/floor6), so a signed-compare view cannot ALSO serve the index via lhu — unlike func_8009AA68's `angle+0x800->sh` narrowing store which is sign-INSENSITIVE (only low 16 bits survive, GCC free to zero-extend). The two views are non-interchangeable here -> the 2nd load is only reachable via a programmer-authored 2nd typed memory view = pre-banned signedness-split family. Re-derives the s3 dichotomy / s7 theorem from the transplant angle. (memory/grind/func_8001F938/rejected/reread-narrowing-mirror.c)
- [s8] NET: rederive EXHAUSTED for a clean sub-8. The overall function structure is orthogonal to the +0x270 crux (m2c reproduces the crux from a different shape; the transplant measurement confirms it). Disposition unchanged: no clean sub-8 lever; structural (s1-s3) + permuter (s4-s5) + forensics (s6-s7) + rederive (s8) axes all dead. Owner FAILed the signedness-split family twice (decisions.md 2026-07-23 10:19 + 10:46; "keep INCOMPLETE, search continues"). Only remaining sanctioned axis = the non-structural SOTN-master-branch signedness-split-family census (escalation modality); s8 adds an independent-decompiler corroboration + a gcc2.7.2 corpus data point (sign-insensitive dual-loads exist clean; sign-SENSITIVE redundant split does not). src/ untouched (HEAD, git clean); candidate.c unchanged (clean floor-8 form). Artifacts: tmp/grind/func_8001F938/s8/{m2c_fresh.c, census_dualload.md, scan_corpus.py, scan_dualload.py, extract_clean.py}.

- [s8] Baseline re-confirmed this session: clean floor-8 chassis (kind-split + branch-flip + single s16 read) = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. src reverted to HEAD (git clean) at session end; candidate.c unchanged.

- [s8] Fresh m2c (tools/m2c/m2c.py --valid-syntax) independently reconstructs the +0x270 block as `(u16)*(s16*)(+0x270)<<16` for the index AND `*(s16*)(+0x270)>=4` for the compare -- two typed views -- despite a totally different overall shape (switch + nested-if). The dual view is intrinsic to the target bytes, not an artifact of candidate.c's structure. Corroborates the s7 combine theorem from an independent tool.

- [s8] decomp.me dual-load census over 3754 gcc2.7.2 scratches: 31 have adjacent same-address dual typed loads; only 2 are CLEAN (score 0, community-matched) -- CW0dj func_8009AA68 (`lh;lhu` of a single s16 field via a NARROWING store, sign-insensitive) and V5c4K func_800A3320 (`lw;lhu`, a genuine word+halfword multi-field read, sign-insensitive). NEITHER is a redundant signed/unsigned split of a sign-SENSITIVE value with no semantic purpose; the corpus has NO clean precedent for func_8001F938's construct (consistent with the F2 SOTN census: NOT ESTABLISHED).

- [s8] The func_8009AA68 transplant, measured on +0x270, scored 9 / bi 104 (worse than floor 8): ONE signed load, re-reads CSE-merged, index folded to sll,1. The distinguishing fact: func_8001F938's `(field<<16)>>15` is sign-SENSITIVE (bit15 -> result sign), so a signed-compare view cannot also serve the index via lhu; func_8009AA68's narrowing second use is sign-INSENSITIVE. This re-derives the s3 dichotomy / s7 theorem from the transplant angle.

- [s8] Kengo transplant dead: func_8001F938 -> md_game_mode_data_restart_init at confidence size-only-ambiguous (107==107 insns only, 24 candidates, opseq_ratio 0.24); a coincidental instruction-count match, not a semantic sibling; name unrelated to damage-scaling.

- [s8] Rederive axis EXHAUSTED for a clean sub-8. Structural (s1-s3) + permuter (s4-s5) + forensics (s6-s7) + rederive (s8) all dead. Disposition unchanged: no clean sub-8 lever; owner FAILed the signedness-split family twice (docs/grind/decisions.md 2026-07-23 10:19 + 10:46; keep INCOMPLETE, search continues, NOT parked).

## s9 rederive (2026-07-23) — BB2-INTERNAL field-model census: +0x270 is a single u16 accumulator; genuine-dual-view-field + header-type-correction escapes KILLED
- [s9] Baseline re-confirmed (own measurement): clean floor-8 form (kind-split + branch-flip + single s16 read) applied to src = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. Reverted src to HEAD (git clean) at session end; candidate.c unchanged.
- [s9] NEW rederive angle vs s8 (s8 used EXTERNAL corpora only: decomp.me similarity, 3754-scratch dual-load census, Kengo). s9 = a BB2-INTERNAL write-site census of the accessed fields, to test the last rederive escape: is +0x270 a genuine dual-view / multi-width / union field (like s8's clean corpus precedents func_800A3320 word+halfword and func_8009AA68 narrowing store) that would give the signed+unsigned dual read a real semantic purpose and lift the ban?
- [s9] CENSUS RESULT: +0x26E/+0x270/+0x272 are SINGLE u16 accumulator fields. Write sites (src/code6cac_b.c func_80027438, the per-status-byte damage accumulator): `*(u16*)(a0+0x272)+=a2;`, `*(u16*)(a0+0x270)+=a2;`, `*(u16*)(a0+0x26E)+=a2;`. Address-taken at src/text1a.c:142 `*(s32*)((u8*)a1+0xC)=(s32)a0+0x270;` (table base into a descriptor). NO union, NO multi-width, NO overlapping field.
- [s9] => genuine-dual-view-field escape KILLED with BB2-internal evidence. Target .L8001FA60 reads the identical single u16 field BOTH signed (`lh`->slti compare) AND unsigned (`lhu`->sll index); the two roles differ ONLY in signedness of interpretation of ONE field. Unlike func_800A3320 (two DISTINCT sub-fields of DISTINCT widths) and func_8009AA68 (a sign-INSENSITIVE narrowing store), func_8001F938's dual read is sign-SENSITIVE with no second field to justify it -> squarely the pre-banned signedness-split family, not a clean multi-purpose access. Corroborates s8's external-corpus finding from an INDEPENDENT (internal) source.
- [s9] COROLLARY: header-type-correction-from-use-sites is INAPPLICABLE (KILLED). The census proves the field is genuinely u16, but reading it consistently u16 forfeits target's SIGNED compare — s3 measured the pure-unsigned read at floor 6 (emits sltiu; target has slti). Target's compare is intrinsically signed AND its index intrinsically unsigned OF THE SAME FIELD; no single type serves both roles. header-type-correction is a one-extern-edit remedy; this needs two simultaneous typed views of one field = the banned construct. So "use the correct type" cannot close it.
- [s9] NET: rederive axis EXHAUSTED (s8 external corpora + s9 internal field model). All four grindable axes dead: structural (s1-s3) + permuter (s4-s5) + forensics (s6-s7) + rederive (s8-s9). Disposition unchanged: no clean sub-8 lever; owner FAILed the signedness-split family twice (decisions.md 2026-07-23 10:19 + 10:46; "keep INCOMPLETE, search continues, NOT parked"). Only the non-structural SOTN-master-branch signedness-split census remains (escalation modality). src/ kept at HEAD (git clean); candidate.c unchanged (clean floor-8 form). Artifact: tmp/grind/func_8001F938/s9/field_model_census.md.

- [s9] s9 baseline (own measurement): clean floor-8 form (kind-split + branch-flip + single s16 read) = sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped. src reverted to HEAD (git clean) at session end; candidate.c unchanged.

- [s9] BB2-internal write-site census: +0x26E/+0x270/+0x272 are single u16 accumulator fields -- src/code6cac_b.c func_80027438 writes `*(u16*)(a0+0x270)+=a2`, `+0x272`, `+0x26E`; src/text1a.c:142 stores +0x270's address as a table base. No union, no multi-width, no overlapping field.

- [s9] Target .L8001FA60 reads the identical single u16 +0x270 field BOTH signed (lh -> slti compare) AND unsigned (lhu -> sll index) -- a programmer-authored dual-typed read of ONE field, sign-SENSITIVE, with no second field to justify it.

- [s9] Distinguished from s8's two CLEAN gcc2.7.2 corpus dual-loads: func_800A3320 (lw;lhu word+halfword = two distinct sub-fields) and func_8009AA68 (lh;lhu via a sign-INSENSITIVE narrowing store). Both are legitimately multi-purpose; func_8001F938's is a redundant signed/unsigned split of one field = the pre-banned signedness-split family. The BB2-internal census independently corroborates s8's external-corpus conclusion.

- [s9] header-type-correction inapplicable: field genuinely u16, but consistent-u16 read yields floor 6 (sltiu, wrong compare per s3) -- signed compare and unsigned index of one field cannot both be served by any single type.

- [s9] All four grindable axes now dead: structural (s1-s3) + permuter (s4-s5) + forensics (s6-s7) + rederive (s8 external corpora + s9 internal field model). Only the non-structural SOTN-master-branch signedness-split census remains (escalation modality). Owner FAILed the family twice (docs/grind/decisions.md 2026-07-23 10:19 + 10:46; disposition 'keep INCOMPLETE, search continues, NOT parked').

- [s10] s10 live re-measurement: HEAD src reads honest floor 11; the clean floor-8 chassis (kind-split + branch-flip + single s16 read, = candidate.c) reads sandbox --disable all score 8, build_insns 105, target 107, 13 rules dropped.

- [s10] scan_hand_coded.py --single func_8001F938 = tier LOW, score 0/8, no strong hand-coded indicators (no S1 multu, S2 empty-branch, S3 spills, S6 BIOS jumptable) -> canonical-asm authorization gate FAILS.

- [s10] Byte-match is held by 13 regfix rules (regfix.txt lines 638-656: 4x $3<->$5 swaps + andi insert for +0x6A kind region; subst/delete/insert_label patch reconstructing target's +0x270 dual-load .L8001FA60), 0 asmfix, 0 cheat-asm.

- [s10] Endgame-lock AND-gate 2 FAILS: the distance-0 pure-C form is the signedness-split/redundant dual-typed-read CSE-defeat family (one +0x270 field read signed for slti compare AND unsigned for sll index, provably identical program behavior); F2 SOTN-master-branch census (2026-07-01) = NOT ESTABLISHED, no-new-park-categories.md names 'redundant width casts (F2)' as unsupported.

- [s10] Exhaustion: 9 sessions, floor flat at 8 since s2; structural (s1-s3), permuter (s4 ~95k iters/floor-8 basin + s5 36k iters/floor-6 basin), forensics (s6-s7 combine simplify_shift_const pass-level proof), rederive (s8 m2c+corpus+transplant, s9 BB2-internal field census) all measured dead; two prior ruling-requests (2026-07-23 10:19 + 10:46) answered FAIL.

- [s10] Filed the formal OWNER-ESCALATION entry at docs/grind/decisions.md line 1474 ('## 2026-07-23 — func_8001F938 — **OWNER-ESCALATION**') stating both endgame-lock AND-gates and the exhaustion; src reverted to HEAD (git clean); candidate.c unchanged (clean floor-8 form).

## s11 (escalation modality, 2026-08-25) — MATCHED. Honest floor 8 -> 0; full-build oracle verified.

- [s11] CHASSIS RE-MEASURE: with `memory/grind/func_8001F938/candidate.c` (s10 clean form)
  applied to `src/code6cac.c`, `sandbox --disable all` = 8, build_insns 105, target_insns 107,
  rules_dropped 0. The s10 ledger line "the byte-match is held by 13 regfix rules" is STALE —
  post asm-until-matched the function ships as `INCLUDE_ASM` with 0 regfix/0 asmfix/0 cheat-asm,
  so nothing "held" a match; the only artefact was the honest floor.

- [s11] OWNER DIRECTIVE DISCHARGED (solver modality). `inverse_compose.py classify` is
  UNUSABLE as-is for an asm-until-matched function: it builds its "target" stream from
  `src/` + regfix, so with C in src/ it compares the honest build to itself and reports
  "FIRST DIVERGENCE: IDENTICAL". The honest classification, done by hand on opcode multisets
  (aliases folded: bnez/beqz<->bne/beq, sltiu<->sltu-imm, addiu<->addu-imm, jr<->j $ra):
  honest 105 vs target 107, target +1 `lhu` +1 `sra` +1 `addiu`, honest +1 `nop`.
  Multisets DIFFER => PRE-RA by the classifier's own funnel rule => ra_solver and
  sched_solver are FORECLOSED for this residual. Recorded so no future session re-runs them.

- [s11] THE WALL WAS A SEARCH GAP, NOT A DICHOTOMY. Target's `.L8001FA60`:
      lh   $v0,0x270($a0)   ; lhu  $v1,0x270($a0)
      slti $v0,$v0,4        ; bnez $v0,.L8001FA7C
       sll $v0,$v1,16       ; addiu $v1,$zero,3 ; sll $v0,$v1,16
      .L8001FA7C: sra $v0,$v0,15
  s1-s10 all held the clamped value in an `s32` and spelled the scale `(raw << 16) >> 15`,
  which made the two loads look like they required a second TYPED memory view (the pre-banned
  signedness-split family; s7 "theorem"). Putting the value in a `s16` HImode LOCAL instead
  produces both loads from ONE dereference of ONE type:
      s16 raw_or_3 = *((s16 *)(arg0 + 0x270));
      if (raw_or_3 >= 4) { raw_or_3 = 3; }
      idx = raw_or_3 * 2;
  cc1 materialises the HImode pseudo with `lhu` ($v1) and supplies the sign-extended SImode
  operand the `>= 4` compare needs with a separate `lh` ($v0). `* 2` on the HImode pseudo is
  emitted as `sll 16 ; sra 15` (the HImode subreg carries exactly 16 sign-bit copies, so the
  s7 combine gate refuses the fold), and reorg steals the `sll` into the branch delay slot as
  in target. MEASURED: sandbox score **0**, build_insns 107 == target 107; normalised asm diff
  vs `asm/funcs/func_8001F938.s` empty except local label names; `verify-oracle` exit 0
  (full build+link SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa).

- [s11] The 8-byte frame is NOT a byproduct of the banned dual-read (s4's claim). It is the
  `s16` local's own stack slot: `.frame $sp,8` appears as soon as the HImode local exists, and
  `addiu $sp,$sp,-8` lands in the first `beq`'s delay slot exactly as in target. s4's
  `volatile short pad` was solving a problem that ordinary typing solves.

- [s11] KILLED: `s32 three = 3;` constant-holder as a fold lever — cse folds the plain
  constant local away before combine; floor unchanged at 8 / 105 insns.

- [s11] MEASURED LADDER (all sandbox --disable all, clean chassis otherwise):
  s10 form (s32 probe, `(raw<<16)>>15`) 8/105 | + constant holder 8/105 | `s32 probe` +
  `s16 raw_or_3` PHI + `*2` 4/106 | `s16 raw_or_3` written in both arms from duplicate signed
  reads 2/108 | `s16 raw_or_3` from one read, clamped in place 0/107.
  For the 0x6A kind-split: `(u16)kind_full` cast 1/107 (emits `move`, target has `andi`);
  single `u16 kind` for everything 16/106; no mask at all 16/106; second `*(u16*)` read into a
  `u16` local 16/106. Only `kind_full` raw + `kind = kind_full & 0xFFFFU` reaches 0.

- [s11] DISPOSITION NOTE (why no escalation packet was filed even though the modality was
  `escalation`): before the lever was found, both endgame-lock AND-gates still FAILed
  (scan_hand_coded tier LOW per s10; the F2 SOTN census for the signedness-split family is
  NOT ESTABLISHED). Under the owner's 2026-08-24 second ruling
  (`.claude/rules/escalation-not-parked.md`, AUTO-REJECT class) a packet whose YES would grant
  a coercion family with no in-hand SOTN precedent, or override the canonical-asm evidence bar,
  is PRE-DECIDED NO and MUST NOT be filed — such a function "stays ACTIVE and keeps grinding
  under standing policy". That is exactly what this session did, and the grind found the match.
