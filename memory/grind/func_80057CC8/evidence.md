# Evidence bank — func_80057CC8

- WIP rejected_form: {'form': 'explicit s32 offset local reused for both p computations', 'score': 4, 'reason': 'not a cheat, just worse — adds an insn vs the reassociated inline form'}

- WIP rejected_form: {'form': 'ternary/single-def prev_idx with explicit sidx local', 'score': 5, 'reason': 'not a cheat, worse than baseline-3'}

- WIP rejected_form: {'form': 'pointer arithmetic `table + (sidx<<1)` instead of byte-offset cast', 'score': 9, 'reason': 'not a cheat; the <<1 element-stride form changes codegen for the worse'}

- WIP rejected_form: {'form': 'prev_idx declared s32 with (u16) casts', 'score': 8, 'reason': 'not a cheat; worse'}

- WIP rejected_form: {'form': 'second p left as (table + offset) [HEAD form]', 'score': 9, 'reason': 'not a cheat; this is the un-reassociated baseline the candidate improves on'}

- WIP rejected_form: {'form': 'split-init-off-accumulation: `off = (((s32)(prev_idx<<16)>>16)<<2); off += (s32)table; p = (s16*)off;` (memory/wip/func_80057CC8/rejected/split-init-off-accumulation.c)', 'score': 0, 'reason': "REJECTED 2026-06-22 by user decision after layer-2 cheat-reviewer FAIL: (a) the broad split-init-accumulation sanction's SOTN evidence base did not survive verification (semantic-asymmetry misread of e_coffin.c, in-loop false-positive in dra/6BF64.c, semantic-purpose misread of e_bone_ark.c, naming-role failure of sp20), (b) the EXACT form was already rejected by layer-1 cheat-reviewer on 2026-06-16, and (c) the 2026-06-13 narrow sanction precedent (func_80049C24) may itself bear re-examination. Future workers MUST NOT re-attempt this form. The mechanism (refs-count bump on same-variable accumulation) is the kind of GCC-internals-only justification that the project's cheat-by-any-spelling posture forbids in the absence of clear SOTN-master evidence — which 2026-06-22 verification showed the project does not actually have for this specific construct."}

- == imported from memory/wip notes.md ==
# func_80057CC8 — WIP checkpoint (BLOCKED — continue search; not permanently parked)

## TL;DR (2026-06-22, post-adjudication)
Masked floor **9 -> 3** by reassociating both p-pointer adds from
`(table + offset)` to `(offset + table)` — pure C, no cheat. That is the
cleanest progress. The remaining 3-insn gap is a register-COALESCING decision
the masked sandbox cannot drive below 3, and the function additionally needs
its `asm("s3")` pin and all 7 regfix-subst rules to drop for COMPLETED-C.

**2026-06-22 ruling:** the split-init-off-accumulation form
(`off = (...); off += (s32)table;`) was proposed as a broad-family sanction
and **REJECTED** by user decision after layer-2 cheat-reviewer FAILed the
supporting SOTN-evidence base. Do NOT re-attempt that form. Card moves to
**blocked** (no-quit grind continues); not permanently parked.

## Resume steps
1. Apply `candidate.c` to `src/text1b.c` (replaces func_80057CC8, ~line 11840).
2. Confirm floor: `& tools/wteng.ps1 <id> sandbox func_80057CC8 --disable all` -> 3.
3. Iterate on the coalescing gap from THERE — see "Avenues still open" below.
4. Do NOT re-derive any of the rejected forms (see meta.json:rejected_forms).

## The remaining gap (the 3)
Target first `p`:  `addu v0,v0,a2` (offset+table, dest = the offset reg v0),
then `lh a0,0(v0)` / `lh a1,2(v0)`.
Ours:              `addu v1,v0,a2` (allocates a NEW reg v1), `lh ...0(v1)` etc.
=> GCC does not coalesce the offset temp into the pointer dest for the FIRST p.
The SECOND p coalesces correctly (v1->v1). The only structural asymmetry is
the `if ((s16)prev_idx < 0) { prev_idx = arg0[3]-1; }` reload: prev_idx has a
two-def (phi) live range, next_idx (set in a block) has a single def.

Also masked-out but real for the full SHA1: target holds prev_idx in `a0` and
table in `a2`; ours has them swapped. And `next_idx` must land in `s3`
naturally (the pin currently forces it).

## Ruled out (rejected_forms in meta.json — do not re-derive)
- explicit `s32 off`/`sidx` local reused for both p adds — floor 4
- ternary / single-def prev_idx with explicit sidx local — floor 5
- pointer arithmetic `table + (sidx<<1)` — floor 9
- prev_idx declared `s32` with `(u16)` casts — floor 8
- HEAD form (second p un-reassociated) — floor 9
- **`off = X; off += (s32)table;` split-init-accumulation — REJECTED**
  (rejected/split-init-off-accumulation.c). Was proposed for a broad-family
  sanction 2026-06-22; layer-2 cheat-reviewer FAILed the supporting SOTN
  evidence (mischaracterizations of e_coffin.c, dra/6BF64.c, e_bone_ark.c,
  sp20 — see meta.json:reviewer.evidence). User decision: NOT sanctioned;
  the EXACT form was already rejected by layer-1 on 2026-06-16, and
  retroactive sanction would be the self-sanctioning antipattern that
  [[review-discipline-before-commit]] was codified to prevent.

## Avenues still open (next session)
- `-da` / ALLOCDBG dump from the candidate-floor-3 baseline to see WHY the
  first p's offset temp is not coalesced into the pointer dest while the
  second's is. The two-def vs single-def live-range asymmetry is the
  prime suspect.
- Force prev_idx to a single def so its live range matches next_idx's
  shape — without contriving named-intermediate aliases. Look at branchless
  forms: `prev_idx = ((s16)tmp < 0) ? arg0[3] - 1 : tmp;` and the family of
  conditional-init shapes; verify these don't trigger the cheat-by-spelling
  catalog (e.g. ternary that compiles to a cmov sequence is OK; ternary that
  forces an unused-arm computation may not be).
- Targeted permuter run from candidate.c base (CAVEAT: text1b.c is the
  highest-cascade file; ~50% apply-failure rate per
  docs/PERMUTER_PIPELINE.md and permuter-closability-evaluated memory).
- Investigate whether one of the COMPLETED-C sibling functions in the same
  text1b cluster (`func_8006133C`, `func_800613C8`, `func_80061454`,
  `func_800614E0` — pointer-walk + mask-last family) carries a structural
  asymmetry-fix lever that could transfer to this function's prev_idx case.

## Why blocked (not matched, not park)
COMPLETED-C needs 0 rules + 0 pins + SHA1==oracle. The 7 regfix-subst rules
rename exactly the registers this gap is about (a0<->a2, dest v0<->v1) and
the s3 pin forces next_idx; the explored pure-C forms do not make GCC
produce that allocation. Genuine register-allocation plateau. Not a
canonical-asm candidate (no hand-coded signals); not a sanctioned
pure-C-impossible plateau (the gap is RA-coalescing, which is
pure-C-reachable in principle — we just haven't found the lever).

Per user 2026-06-22: keep working it; not permanently parked.


- [s1] Floor 3 replays with candidate.c edits in src/text1b.c (offset+table form, no s3 pin) — sandbox --disable all reports score=3, rules_dropped=7, cheat_asm_stripped=395.

- [s1] Canonical gate: verdict=C, distance=3 (pure-C target, no ASM routing).

- [s1] Objdump of built .o localizes the 3 remaining diffs to the FIRST p's pointer add: our `addu v1,v0,a2` (b798) vs target `addu v0,v0,a2` (80057D54); cascades to `lh a0,0(v1)` / `lh a1,2(v1)` (b7ac/b7b0) vs target `lh a0,0(v0)` / `lh a1,2(v0)` (80057D68/80057D6C). The SECOND p already coalesces correctly in both (addu v1,v1,a0 at b7cc / 80057D88).

- [s1] Structural asymmetry (from ledger, unmeasured): prev_idx has a two-def live range from `if ((s16)prev_idx<0) { prev_idx = arg0[3]-1; }`; next_idx is set inside a block { ... = tmp; if(..) ...=0; } but its uses reach through a phi too — however next_idx's use flows through s3 (in target/pin); the pin masks this. Even with the pin removed, only the FIRST-p addu coalesces poorly, suggesting the phi shape at the prev_idx use-site is the differentiator.

- [s1] Rejected forms (do not re-derive): block-scope-alias-p1, duplicate-address-expr-pseudo-inline, separate-p1-p2-function-scope-aliases, split-init-off-accumulation, split-lh-px0-px1-locals. Also from evidence.md: explicit s32 off/sidx (floor=4), ternary+explicit sidx (floor=5), pointer-arith table+(sidx<<1) (floor=9), prev_idx s32 with (u16) casts (floor=8), un-reassociated HEAD form (floor=9).

- [s2] Candidate.c applied to src/text1b.c line 11837 (register asm("s3") pin removed, offset+table reassociation for both p adds). sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1 finding cleanly on current main HEAD.

- [s2] Objdump of candidate baseline localizes the 3-insn gap unchanged from s1: b798 `addu v1,v0,a2` (ours) vs 80057D54 `addu v0,v0,a2` (target); cascading to lh base regs at b7ac/b7b0 (v1) vs 80057D68/80057D6C (v0). Second p addu coalesces correctly in both (b7cc v1,v1,a0 vs 80057D88 v1,v1,a0).

- [s2] Two-def-phi asymmetry is NOT the discriminator. Mirroring next_idx's block-scoped `s32 tmp; = tmp; if(..) = 0;` shape onto prev_idx did NOT fix p1 coalescing — both variables are two-def yet only p1 addu misses. The real discriminator is deeper (probably shifted-offset pseudo lifetime across the next_idx bnez delay-slot scheduling window, or a reg_n_refs priority tiebreaker in global.c on the p1 pseudo).

- [s2] Statement re-ordering (moving prev_idx def, swapping IF blocks, hoisting p1 add) all failed. Target's shape is delay-slot-schedule-critical: swap made it floor 31; hoist p1 made it floor 12. The (prev_idx if-reload) -> (next_idx block with delay-slot fillable bnez) -> (p1 add reading a2=table and a0=prev_idx from the reload's phi) sequence is FIXED at the C level; the RA lever is inside that structural window.

- [s2] Type width (u16 vs s16) of prev_idx and shift-vs-cast spelling of the sign-extend chain are both folded identically by GCC — combine.c reduces them to the same pattern. Not levers.

- [s2] Sibling matched functions in text1b.c (func_8006133C, 800613C8, 80061454, 800614E0) are structurally DIFFERENT — they are s32* pointer-walk-with-post-increment (`D_800F1140 = *p++; ...`) with no phi-shape index. No transferable lever.
