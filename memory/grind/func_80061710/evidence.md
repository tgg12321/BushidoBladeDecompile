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
