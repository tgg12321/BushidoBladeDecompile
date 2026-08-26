# Evidence bank — func_80061710

- == imported from memory/wip notes.md ==
# func_80061710 - WIP (register-rotation v0/v1 plateau; sibling of func_80061658)

## TL;DR (2026-06-22)
Identical plateau to `func_80061658`. HEAD pins `register s32 t asm("$2"); register
s32 mask asm("$3");` (0 regfix rules). Stripping both pins, honest masked floor =
**9** and 8 explored pure-C forms in this lane all land at >=9 (several worse,
11). The 9 diffs are ALL v0/v1 register-name swaps in the post-jal tail: target
uses v0 for the three arg0[] loads and v1 for the mask (lui/ori 0x10FF10);
sandbox uses v1 for the loads and v0 for the mask. Schedule is byte-identical;
this is purely an allocator preference flip the candidate could not influence.
`candidate.c` saved (PASS cheat-review) for the next agent to resume from.

## What target looks like (post-jal tail)
```
lw  v0, 0(s0)                      # t = arg0[0]
lui at, %hi(D_800F1140)
sw  v0, %lo(D_800F1140)(at)
lw  v0, 4(s0)                      # t = arg0[1]
lui v1, hi(0x10FF10)               # mask = ... (interleaved)
lui at, %hi(D_800F1144)
sw  v0, %lo(D_800F1144)(at)
lw  v0, 8(s0)                      # t = arg0[2]
ori v1, v1, lo(0x10FF10)
sw  v1, %gp_rel(D_800A3464)($gp)   # mask store before D_800F1148
lui at, %hi(D_800F1148)
sw  v0, %lo(D_800F1148)(at)
```
Sandbox (no pins) emits the exact same 12 insns with v0<->v1 swapped on every
load and on the mask. Masked Levenshtein = 9.

## Why pure-C levers don't flip it
GCC 2.7.2 local-alloc gives the LONGER-lived pseudo the lower-numbered reg.
The mask pseudo spans from `lui` (between the 2nd lw and the 2nd sw) to its
final store (between the 3rd lw and the 3rd sw). The t pseudo has 3 short
disjoint live ranges. mask is "longer," so mask -> $v0, t -> $v1. Target's
pinned form forces the opposite. We need a pure-C structure that either
shortens mask's effective live range or extends t's so the allocator preference
flips.

## Forms ruled out this session (all >= 9, none beat HEAD)
- decl order swap (`mask` before `t`) -> 9
- block-local `{ s32 t; s32 mask; ... }` scope after the call -> 9
- `mask = 0x10FF10;` placed right after `func_80060A68()` -> 9
- split mask: `mask = 0x100000; ... mask |= 0xFF10;` -> 9
- inline mask literal `D_800A3464 = 0x10FF10` (no `mask` var) -> 9
- inline t loads `D_800F1140 = arg0[0]` (no `t` var) -> floor 11
- `s32 mask = 0x10FF10;` initialiser at top -> 11 (+2 spill insns, needs callee-save)
- `*arg0 / *(arg0+N)` deref form vs `arg0[N]` -> identical asm

(Sibling 658's prior session also ruled out: split-t into t0/t1/t2 -> 11; mask after 3rd
load -> 9; inline loads (no t) -> 11. None of those repeat here.)

## Next hypotheses (genuinely untried)
- **Standalone cc1 `-da` dump on a minimal isolation TU** to see the actual
  `.greg` priority queue and which structural change in C flips mask's vs t's
  spill-cost ranking. Mechanical answer, not guesswork.
- **Directed permuter** seeded from `candidate.c` with a hint biasing the v0/v1
  allocation between the two pseudos.
- **Family-level resolution** covering func_80061658 / 710 / 617C8 / 618B4 /
  611A4 / 6133C - if all 6 plateau on the same mechanism, one lever (or one
  canonical/plateau decision) covers all siblings, see [[register-alloc-deep-dive]].

## Why blocked, not matched
COMPLETED-C requires 0 pins. Removing both pins raises the honest distance to
9 and no pure-C form explored in this lane reproduces target's v0/v1 rotation.
This is a genuine register-allocator preference plateau, not laziness.

---

## == grind s1 (recon), 2026-07-22 ==

**Baseline re-verified.** canonical = C (0 asm, total 46, distance 9). sandbox
--disable all on HEAD pinned form = 9 (46/46, cheat_asm_stripped 371, 0 rules).
Honest SANCTIONED pin-free floor = 9 (pure v0<->v1 rename). Confirms the WIP.

**Family placement (the decisive recon finding).** func_80061710 is a named
member of the `t=$2 / mask=$3` pin cluster (61710 / 617C8 / 618B4 / 611A4 /
6133C / 61658). Two siblings already at OWNER-ESCALATION (docs/grind/decisions.md):
- **func_800611A4** (2026-07-22) — NO pure-C-0 form exists (reachability wall);
  routed option (b) INCOMPLETE-owner-accepted. Root cause named at
  local-alloc.c:472 death-count routing gate + find_free_reg lowest-free-reg scan.
- **func_80061658** (2026-07-23) — HAS a bytes-proven pure-C-0 form via
  constant-staging `val = C; mask = val;`; Judge-FAILed (route-to-sign-off) as a
  first-reach coercion; escalated for owner SOTN census. Its escalation
  EXPLICITLY names func_80061710 as a cluster member the ruling closes in one pass.

**MEASURED THIS SESSION — 710 is the 658 subfamily, NOT the 611A4 wall.**
Tested the constant-staging form (pin-free, `val = 0x10FF10; mask = val;`) →
**sandbox 0, 46/46, 0 rules** (tmp/grind/func_80061710/s1/recon_measurements.txt).
So 710 DOES have a pure-C-0 form — the reachability-wall hypothesis is KILLED.
But the ONLY score-0 basin is that constant-staging coercion (same construct the
Judge FAILed for 658; no semantic purpose; local-alloc.c:472 copy-preference).

**Do-not-re-derive (exhausted on the two siblings, same 43/46-insn tail, same
v0<->v1 residual, same root cause):**
- structural: temp-split / block-local split / decl-order / mask-position / mask
  width / ptr-deref / no-temp inline — all >= 7, none reach committable 0
  (611A4 s2, 658 s2). Block-local split of the middle load → floor 7 (best
  SANCTIONED form on 658). mask-atomic-first (V7) → floor 6 on 611A4.
- walking-pointer `*p++` inverts RA direction but adds an addiu (insn count 44 !=
  target 43/46) — architecturally blocked (611A4 s2).
- permuter: 658 s4 found the val-staging basin (7 finds, sandbox 0); 658 s4b
  DENY-copy-source chassis (30,762 iters) produced NO score-0 → the score-0 basin
  is UNIQUELY the constant-staging copy. 611A4 s4/s5 (3 basins, ~128k iters) zero
  cheat-free sub-6.
- forensics: 611A4 s6/s7 named the mechanism at GCC source; cc1psx calibration
  compiled the same clean C to the same wall → not a compiler/hand-asm issue.
  scan_hand_coded LOW 0/8 on 611A4 (ordinary GCC output).

**Disposition (recon).** src restored to HEAD pinned form (oracle intact). The
best form (constant-staging, sandbox 0) saved to candidate.c, annotated
POLICY-BLOCKED — it is the pending-census coercion, not surfaced as a match. 710
is resolved by the same owner ruling pending under func_80061658's escalation.


- [s1] canonical func_80061710 = verdict C, 0 asm insns, total 46, distance 9.

- [s1] sandbox --disable all on HEAD pinned form = 9 (46/46, cheat_asm_stripped 371, 0 rules) — honest sanctioned pin-free floor = 9, pure v0<->v1 rename.

- [s1] sandbox --disable all on pin-free constant-staging form (val=0x10FF10; mask=val;) = 0 (46/46, 0 rules) — the ONLY score-0 basin; identical to sibling func_80061658.

- [s1] func_80061710 is a named member of the t=$2/mask=$3 cluster (61710/617C8/618B4/611A4/6133C/61658); func_80061658's OWNER-ESCALATION (2026-07-23) explicitly names 710 as a cluster member the pending SOTN census resolves in one pass.

- [s1] func_800611A4 (OWNER-ESCALATION 2026-07-22) = reachability wall, routed option (b) INCOMPLETE-owner-accepted; func_80061658 (OWNER-ESCALATION 2026-07-23) = has pure-C-0 constant-staging form, Judge-FAILed route-to-sign-off, pending owner SOTN census.

- [s1] Do-not-re-derive (exhausted on siblings, same tail/residual/root cause): structural splits >=7 (block-local middle-load -> 7; mask-atomic-first V7 -> 6; interleaved mask -> 9 wall); walking-pointer *p++ adds an addiu (insn count breaks); permuter deny-copy-source (658 s4b, 30,762 iters) -> NO score-0; cc1psx calibration hits the same wall; scan_hand_coded LOW 0/8.

- [s1] src restored to HEAD pinned form after measurement (main oracle intact). Best form (constant-staging, sandbox 0) saved to candidate.c annotated POLICY-BLOCKED; not surfaced as a match per no-new-park-categories 'vet before surfacing'.

## == grind s2 (STRUCTURAL modality), 2026-07-22 ==

The two sanctioned structural floors predicted from siblings (H2) were UNMEASURED on 710 until now. Measured both, plus diffed the V7 residual at instruction level. Artifact: tmp/grind/func_80061710/s2/structural_measurements.txt.

- [s2] Baseline re-confirmed: HEAD pinned form, sandbox --disable all = 9 (46/46, 0 rules).
- [s2] V7 mask-atomic-first (pin-free, mask store hoisted before the 3 loads) = sandbox 6 (46/46, 0 rules). CONFIRMS H2's sibling-611A4 prediction on 710. Best SANCTIONED (non-coercion, 0-rule) floor for 710 = 6. Saved rejected/v7-mask-atomic-first-floor6.c.
- [s2] Middle-load block-local split (2nd load in a { } scope) = sandbox 7 (46/46, 0 rules). CONFIRMS H2's sibling-658 prediction on 710. Saved rejected/middle-load-block-local-split-floor7.c.
- [s2] V7 residual diffed (objdump sandbox .o vs asm/text1b.s): the 6 is PURELY schedule. V7 emits mask as a LEADING BLOCK -> mask+all 3 loads reuse $v0 (register conflict freed); target INTERLEAVES the mask lui/ori/sw between the load/store pairs with loads=$v0, mask=$v1. Interleaved (target-schedule) forms make mask live-across-loads -> GCC 2.7.2 local-alloc gives the longer-lived mask the lower reg ($v0), loads get $v1 -> v0<->v1 wall = 9. Only the constant-staging copy-preference reaches interleave+mask=$v1 (=0, policy-blocked).
- [s2] STRUCTURAL AXIS MEASURED DEAD on 710: best sanctioned floor 6 (V7), no structural form reaches committable 0. Sanctioned floor improved 9->6 this session. src restored to HEAD pinned form (oracle re-verified = sandbox 9). No permuter/forensics run (H3 killed at family level; forensics done on 611A4 s6/s7).

- [s2] Baseline HEAD pinned form: sandbox --disable all = 9 (46/46, 0 rules); re-verified after src restore = 9 (oracle intact).

- [s2] V7 mask-atomic-first (pin-free, mask block hoisted before the 3 loads) = sandbox 6 (46/46, 0 rules) — new best sanctioned floor for 710 (was 9); confirms sibling 611A4 prediction. Saved memory/grind/func_80061710/rejected/v7-mask-atomic-first-floor6.c.

- [s2] Middle-load block-local split (2nd load in a { } scope) = sandbox 7 (46/46, 0 rules); confirms sibling 658 prediction. Saved memory/grind/func_80061710/rejected/middle-load-block-local-split-floor7.c.

- [s2] V7 residual diffed at instruction level (objdump tmp/sandbox/func_80061710/text1b.o vs asm/text1b.s): V7 emits mask+all 3 loads in $v0 as a leading block; target interleaves mask lui/ori/sw into the load/store web with loads=$v0, mask=$v1. The 6 diffs are purely the schedule (leading-block vs interleave).

- [s2] Mechanism (confirmed on 710): interleaved mask is live-across-loads -> local-alloc gives longer-lived pseudo the lower reg -> mask=$v0/loads=$v1 = v0<->v1 wall (9); mask-first frees the conflict but loses the interleave (6). Only the constant-staging copy-preference reaches interleave+mask=$v1 (=0, policy-blocked, candidate.c).

- [s2] src/text1b.c restored to HEAD pinned form after all measurements; git diff clean; oracle intact.

## == grind s3 (STRUCTURAL modality), 2026-07-22 ==

Mandate: structural. s2 already reported structural dead (floor 6); before rubber-stamping that per difficult-is-not-impossible, I swept the mask-position permutations NOT in the s2 rejected bank or WIP list. Four genuinely-untried positions measured, all pin-free. Artifact: tmp/grind/func_80061710/s3/structural_permutation_measurements.txt; rejected/mask-position-permutation-sweep-floor9.c.

- [s3] baseline pin-free re-confirmed = 9 (46/46, 0 rules).
- [s3] Form [C] mask split across load2 (mask=... after load1, D_800A3464=mask after load2's store) = sandbox 9.
- [s3] Form [A] mask atomic block after load1 = sandbox 9.
- [s3] Form [E] compute-early/store-late (mask computed before all loads, stored interleaved late) = sandbox 9.
- [s3] restored HEAD pinned form = 9 (oracle sanity; git diff clean).
- [s3] DECISIVE: only mask-dies-before-load1 (V7 leading block) reaches sub-9 (=6); EVERY mask position at/after load1 -> 9. The mask-position permutation space is now exhaustively mapped (V7=6, MLS=7, all four interleave/late positions=9, constant-staging copy=0/policy-blocked). Interleave (target schedule) and mask=$v1 are mutually exclusive in pure structure — confirmed independently on 710, not just inherited from siblings.
- [s3] STRUCTURAL AXIS re-confirmed DEAD with fresh 710 measurements. Floor unchanged at 6 sanctioned. No structural form reaches committable 0. Remaining disposition is the owner SOTN census (policy, not grind) under func_80061658's escalation which names 710. Next grind modality per ladder: escalation (file owner-gated citing the 658 census). src restored; oracle intact.

- [s3] s3 pin-free baseline re-confirmed = 9 (46/46, 0 rules); restored HEAD pinned form also = 9 (oracle sanity, git diff clean).

- [s3] Four genuinely-untried mask-position permutations measured on 710, none in the s2 bank/WIP: [C] mask split across load2 = 9, [A] mask atomic after load1 = 9, [E] compute-early/store-late = 9, [HEAD-posn] mask atomic after load2 = 9.

- [s3] Mask-position permutation space now exhaustively mapped: V7 leading block = 6 (unique sub-9 sanctioned form), middle-load block-local split = 7 (s2), all interleave/late positions = 9, constant-staging copy = 0 (policy-blocked coercion, candidate.c).

- [s3] Mechanism confirmed independently on 710 (not just inherited from siblings): interleave (target schedule) and mask=$v1 are mutually exclusive in pure structure because an interleaved mask is live-across the loads and GCC 2.7.2 local-alloc gives the longer-lived pseudo the lower-numbered reg.

- [s3] STRUCTURAL AXIS re-confirmed DEAD with fresh 710 measurements; sanctioned floor unchanged at 6. The sole 0-basin is the constant-staging copy-preference coercion, owner-gated under func_80061658's OWNER-ESCALATION (docs/grind/decisions.md 2026-07-23) which explicitly names func_80061710 as a cluster member the SOTN census closes in one pass.

## == grind s4 (PERMUTER modality), 2026-07-23 ==

Mandate: permuter. Prior permuter kills (H3) were inherited from sibling 658
s4/s4b; this session ran decomp-permuter NATIVELY on func_80061710 to confirm
whether any committable (non-cheat) score-0 basin exists. Two telemetried
campaigns (tools/permuter_campaign.py). Artifacts: tmp/grind/func_80061710/s4/
{permuter_summary.md, scores.txt, campaign1_pinfree.log, campaign2_denycopy.log,
permuter_score0_constant_staging_cheat.c}; rejected/permuter-constant-staging-cheat-s4.c.

- [s4] Workspace built full-TU (src/text1b.c) for correct codegen context; func
  region extracted vs target.o (asm/funcs/func_80061710.s, offset-0). Pin-free
  base reproduces the exact 9-line v0<->v1 tail residual (base 46 == target 46).
- [s4] Campaign 1 (pinfree-floor9, base_score 50, ~4574 iters): 4× score-0 finds
  (output-0-1..0-4), first at ~635 iters/15s. EVERY score-0 = the constant-staging
  coercion (`mask = (val = 0x10FF10);` or `val = 0x10FF10; ... mask = val;`) —
  the reused live local `val` staging the constant. IDENTICAL to the construct
  Judge-FAILed for func_80061658. Vetted per no-new-park-categories auto-search
  clause -> REJECTED, not surfaced.
- [s4] Campaign 2 (denycopy-blockscope-val, base_score 125, ~19k iters, ~23 min):
  `val` block-scoped inside each switch case so it is NOT a live copy source in
  the tail (decisive deny-copy-source test, replicating 658 s4b natively on 710).
  Best score 50 (pure tail v0<->v1 residual); scores 125/95/75/50; ZERO score-0;
  plateaued (>5 min no novel find at harvest --stop). With the copy source denied,
  NO score-0 form exists.
- [s4] PERMUTER AXIS MEASURED DEAD on 710 (native): the sole score-0 basin is the
  policy-blocked constant-staging cheat; denying the reused-live-local copy source
  eliminates all score-0 (matches 658 s4b, 30,762 iters). No committable permuter
  form exists. H3 (no clean permuter basin) now CONFIRMED natively on 710, not just
  inherited from siblings. src never modified (git clean; oracle intact — only
  temp copies tmp/perm_710_s4*_*.c preprocessed).
- [s4] Every grind-advanceable axis now measured dead on 710: structural (s2/s3 —
  V7 floor-6 best sanctioned, all interleave positions =9) + permuter (s4). Sole
  0-basin is the constant-staging coercion, owner-gated under func_80061658's
  OWNER-ESCALATION (docs/grind/decisions.md 2026-07-23, line ~1361) which names
  func_80061710 as a cluster member the SOTN census resolves in one pass. Filed a
  dedicated OWNER-ESCALATION for func_80061710 this session; returned owner-gated.

- [s4] s4 permuter workspace built full-TU (src/text1b.c) for correct codegen context; func region extracted vs target.o (asm/funcs/func_80061710.s, offset-0, .set noreorder r3000); pin-free base = 46 insns == target 46, diff is exactly the 9-line v0<->v1 tail rename.

- [s4] Campaign 1 (pinfree, ~4574 iters): 4x score-0, all the constant-staging coercion; vetted per no-new-park-categories auto-search clause -> REJECTED (cheat by any spelling, no semantic purpose, justified only by local-alloc.c:472 copy-preference), not surfaced.

- [s4] Campaign 2 (deny-copy-source, ~19k iters/~23 min): best 50, ZERO score-0, plateaued - the score-0 basin is UNIQUELY the constant-staging copy, confirmed natively on 710 (matches 658 s4b).

- [s4] Every grind-advanceable axis now dead on 710: structural (s2/s3 - V7 floor-6 best sanctioned; all interleave/late mask positions = 9) + permuter (s4 native). Sole 0-basin is the policy-blocked constant-staging cheat.

- [s4] func_80061658's OWNER-ESCALATION (docs/grind/decisions.md 2026-07-23) option (a) explicitly names func_80061710 as a cluster member its SOTN-master census resolves in one pass; sibling func_800611A4 (no pure-C-0 form) was ruled option (b) REFUSED/OWNER-ACCEPTED INCOMPLETE.

- [s4] src/text1b.c never modified this session (git clean, oracle intact); all edits were temp preprocessed copies (tmp/perm_710_s4*_*.c). Filed a dedicated OWNER-ESCALATION for func_80061710 this session.


## == grind s5 (SYNTHESIS modality), 2026-08-25 — **MATCHED, sandbox 0** ==

Mandate: synthesis (re-read the whole ledger, merge, reset the frontier). The
merged attack found the match. The v0<->v1 "architectural wall" that s1-s4 all
concluded was unreachable in sanctioned C is GONE — it was an artifact of the
TAIL SPELLING every prior session inherited from the WIP note, never a property
of the function.

**The decisive lead the ledger never used: func_8006156C.** The SessionStart
near-duplicate hint (similarity 0.644) names it and no session (s1-s4) ever
opened it. It is a COMPLETED-C sibling living 150 lines above 710 in the same
file (src/text1b.c:3296-3322) and its TARGET TAIL IS THE SAME SHAPE AS 710's:
three arg0[] loads in $v0 interleaved with a lui/ori constant in $v1 stored to
D_800A3464 (0xFF8080 there, 0x10FF10 here). It matches in pure C. Its tail is
spelled:

    p = arg0;
    D_800F1140 = *p++;  D_800F1144 = *p++;  D_800F1148 = *p;
    D_800A3464 = 0xFF8080;

i.e. a WALKING POINTER and an INLINE constant written LAST, with NO temp locals
at all. Every 710 session instead used `t = arg0[N]; D_800F114X = t;` plus a
named `mask` local — and THAT is what produced the v0<->v1 rename.

- [s5] **CORRECTION to a load-bearing s1 belief.** evidence.md s1 records
  "walking-pointer *p++ inverts RA direction but adds an addiu (insn count 44 !=
  target 43/46) — architecturally blocked (611A4 s2)". That is FALSE on 710's
  chassis: GCC 2.7.2 folds the three increments into 0/4/8($s0) offsets, exactly
  as target does, and the insn count stays 46. The claim was inherited from
  sibling 611A4 and never re-measured on 710 — it is the single assumption that
  kept the match hidden for four sessions.
- [s5] **V8 (6156C tail transplant, constant inlined in each switch case) =
  sandbox 12**, and the residual MOVED: the tail (target insns 30-41) became
  BYTE-EXACT (loads $v0, mask $v1, interleaved) and all 12 diffs relocated into
  the switch head, where the 0x21000E/F constant and the D_800F115C byte pointer
  had their $v0/$v1 roles swapped vs target. First proof the tail wall is not
  architectural. rejected/v8-walktail-but-inline-const-head-floor12.c.
- [s5] **V9a (s1 switch head with `val` + 6156C tail) = sandbox 0.**
- [s5] **V9b (constant stored first inside each case, no `val`) = sandbox 12** —
  the head's $v0/$v1 roles swap again.
  rejected/v9b-const-store-first-in-case-floor12.c.
- [s5] **V9c (arms `goto` a shared block) = sandbox 0.**
- [s5] **V9d/FINAL (arms select `val`+`q`, `default: goto done;`, ONE shared
  trailing block `*q = 0; D_800F1180 = (s32)q; *v1 = val;`, then the 6156C tail)
  = sandbox 0 (46/46, 0 rules, 0 pins, 0 inline asm).** This is the cleanest
  spelling and it mirrors target's own control flow: target shares that block at
  .L8006176C and jumps past it for the default case.
- [s5] **The pointer alias is load-bearing, and measured so.** Removing
  `s32 *v1 = (s32 *)&D_800F116C;` in favour of the direct-global spelling
  (`D_800A3468 = (s32)&D_800F116C; ... D_800F116C = val;`) = **sandbox 5**
  (rejected/v9e-no-pointer-alias-direct-global-floor5.c). The alias keeps
  &D_800F116C in ONE pseudo, allocated to $a0 and live across the switch,
  instead of being re-materialised per use. Kept, with the mandatory
  `/* FAKE: ... */` annotation per .claude/rules/pointer-alias-fake-exception.md;
  the identical declaration is already in the COMPLETED-C sibling at
  src/text1b.c:3297.
- [s5] **NO constant-staging anywhere in the final form.** The REFUSED F1 family
  ("constant-staging through a REUSED LIVE local", survey WEAK,
  docs/grind/borderline.md 2026-08-18) is absent by construction: the tail has no
  temp at all, and `val` is fresh per arm, never reused, and dead before the call
  to func_80060A68(). H1's entire premise (that the match requires the refused
  family) is KILLED.
- [s5] **Policy state corrected in the ledger.** The s1-s4 frontier said 710 was
  "awaiting the owner SOTN census under func_80061658's escalation". That census
  RAN and came back WEAK — F1 REMAINS REFUSED (docs/grind/borderline.md
  2026-08-18) — and owner ruling 2026-08-24 (escalation-not-parked) kicked 710
  back to active grinding. There was never anything left to wait for; the correct
  move was exactly this one, a fresh look at the in-repo sibling.
- [s5] src/text1b.c carries the matched C in place of the INCLUDE_ASM line;
  self_vet.md written; candidate.c updated to the matched form.
- [s5] **CROSS-FUNCTION LEAD (high value, untested).** The whole t=$2/mask=$3
  cluster — func_80061658, func_800617C8, func_800618B4, func_800611A4,
  func_8006133C — shares this exact tail shape and the same v0<->v1 residual, and
  their ledgers rest on the SAME false "walking pointer adds an addiu" premise.
  func_800611A4 was ruled OWNER-ACCEPTED INCOMPLETE (2026-07-22) and
  func_80061658 escalated (2026-07-23) on the belief that only the refused F1
  family reaches 0. The 6156C tail transplant should be re-measured on every one
  of them before any of those dispositions is honoured. This may close five more
  functions.
