---
name: canonical-gate-distance-not-evidence
paths: ["engine/canonical.py", "inline_asm_canonical.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "Large pure-C distance is NOT hand-asm evidence — it's just a large function with normal RA/scheduling drift to grind. A verdict of ASM-SUSPECT (or even ASM-STRUCTURAL) is the gate's BEST GUESS based on incomplete signal; keep pushing pure C. Don't give up and authorize canonical-asm without independent corroboration (scan_hand_coded >= POSSIBLE, S1/S2/S6/S7/S8 signals)."
metadata:
  type: rule
---

# Distance is not evidence — keep pushing pure C

## The lesson (2026-06-09 audit)

A canonical-asm audit of the 28-item `authorize` bucket found that **26 of
the 26 ASM-STRUCTURAL items had ZERO hand-coded signals**:

- `scan_hand_coded.py --single <func>`: 0/26 above LOW tier
- 0/26 fired S1 (uniform multu pacing), S2 (empty-body branch),
  S6 (BIOS jumptable), S7 (unsaved callee-save use), or S8
  (redundant mask before discarding shift)
- Manual asm read of samples: standard GCC-compiled C —
  prologue saves, GP-rel global loads, strength-reduced index math
  (`sll 4; addu; sll 2; ... = i*0x44C`), JAL+delay-slot, struct stores

They had been auto-routed to ASM-STRUCTURAL **purely on
`pure-C distance > 500`** — but at function sizes of 500–3000
instructions, accumulated RA/scheduling drift trivially exceeds the
threshold without the function being hand-written. The gate's heuristic
was confusing **"function is large"** with **"function is hand-asm."**

The gate was fixed (`engine/canonical.py`) to require BOTH distance > 500
AND `scan_hand_coded` tier >= POSSIBLE before routing to ASM-STRUCTURAL.
Without corroboration, large distance now demotes to ASM-SUSPECT
(bounded pure-C attempt, then PARK). The 26 rejected items were parked
with this audit's rationale.

## What this means for future agents

### When the gate routes a function to ASM-SUSPECT or ASM-STRUCTURAL

The verdict is the gate's **best guess** based on aggregate distance + a
secondary signal. It is NOT proof. Specifically:

- **ASM-SUSPECT** (distance > 50, no hand-coded signal, OR distance > 500
  with tier < POSSIBLE): the engine thinks pure-C might not close, but
  the workflow says **try anyway** with a bounded attempt. Keep pushing
  the levers in [[register-alloc-pure-c]], [[cross-jump-call-merge]],
  [[cross-jump-store-tail-merge]], [[defeat-licm-hoist-var-reuse]],
  [[split-read-defeats-hoist]], etc.
- **ASM-STRUCTURAL** (distance > 500 AND tier >= POSSIBLE): the engine
  has TWO converging signals. **Still not proof.** This is the threshold
  for "stop pure-C grinding and surface to user for canonical-asm
  sign-off", but the user's decision is still required
  ([[hand-coded-asm-recognition]]), and you can still be wrong about the
  tier. Lay out the per-function signals (which of S1/S2/S6/S7/S8
  actually fired, the asm signature) and let the user decide — don't
  pre-authorize.

### When you're tempted to think "this is canonical-asm"

Apply the same checklist the 2026-06-09 audit applied — before reaching
for the canonical-asm escape valve:

1. **Run the rigorous scanner.** `python3 tools/scan_hand_coded.py
   --single <func> --json`. Read the per-signal output. If tier is LOW
   or TIGHT_C with score <= 2, **the function is not strong evidence of
   hand-asm.** Keep pushing pure C.
2. **Read the asm.** Does it look like compiled C (standard prologue,
   GP-rel loads, strength-reduced index math, JAL+delay-slot patterns,
   struct field stores)? Or does it look hand-written
   ([[hand-coded-asm-recognition]] strong-signals section — uniform
   multu pacing, INT_MIN guard idioms, implausibly tight register
   packing, cluster behavior)?
3. **Check [[no-new-park-categories]].** The user policy on canonical-
   asm carve-outs is explicit: no new "X infrastructure" categories. The
   accepted carve-outs are jtbl-infra and GTE leaf wrappers — both
   "literally no C input produces target bytes" cases. "It's a hard
   function" is not in that bucket.
4. **Read [[difficult-is-not-impossible]].** The matching C exists.
   Stuck = unfinished work, never proven-impossible.
5. **Read [[no-compiler-divergence]].** The toolchain is frozen; the
   only variable is the C. "Reaching for asm" is never the answer to a
   register/scheduling diff.

### The slog-functions special case

Two functions in the queue carry distances > 2900 (`func_80023F08` =
2981, `func_80058580` = 2989, both per [[slog-kengo-dead-end]]). They
will be multi-session pure-C work. They are NOT canonical-asm —
slog-kengo-dead-end itself says: *"They'll need to be decompiled from
BB2 asm alone... expect a full asm-only decomp — likely multi-session
work even at COMPLETED-C budget."* Multi-session != canonical-asm.

## Cross-references for "keep pushing"

The cardinal rules that say "don't give up":

- [[difficult-is-not-impossible]] — the matching C exists; "wall" claims
  almost always wrong
- [[no-compiler-divergence]] — toolchain frozen; the variable is the C
- [[no-new-park-categories]] — no "X-infrastructure" carve-outs; the
  accepted no-C-form exceptions are exhaustive (jtbl-infra + GTE leaves)
- [[register-alloc-pure-c]] — the canonical RA-via-C-structure playbook
  (block-local var split, narrow integer type, loop-local precompute)
- [[hand-coded-asm-recognition]] — what STRONG evidence actually looks
  like (S1/S2/S6/S7/S8); requires user authorization, not agent
  inference

## The audit population (parked 2026-06-09)

26 functions across 7 src files. All currently `status=parked` with the
rejection reason: `"canonical-asm audit 2026-06-09: REJECTED..."`. If a
future pass identifies a real hand-coded signal that the scanner missed,
the function can be unparked and re-routed via `queue regen`. Until
then, they're INCOMPLETE pure-C work, awaiting prioritization.

Files affected:
- `text1b.c` (13): `calc_loc_mat_fw_80055B60`, `func_80058580`,
  `func_8005C8A8`, `func_8005D814`, `func_8005E54C`, `func_8005F1C8`,
  `func_8006A880`, `func_8006F97C`, `func_800720FC`,
  `motion_ShiftControl`, `replay_camera_attack`, `saTan4GaugeMain`,
  `single_game_SetStatusUpData`
- `code6cac.c` (4): `camera_set_target_zoom`, `func_800198D0`,
  `func_80022580`, `func_80023F08`
- `code6cac_b.c` (5): `calc_loc_mat_fw`, `calc_teasi_loc_fw`,
  `func_80029454`, `func_80030D7C`, `func_80034708`
- `code6cac_c_mid.c` (1): `func_8003993C`
- `code6cac_b2_post.c` (1): `special_camera_set_win_cam`
- `text1b_b.c` (1): `func_80079A30`
- `main.c` (1): `func_8008C464`

## Related

- [[hand-coded-asm-recognition]] — the rigorous evidence framework
  (5-signal scanner + tier classification)
- [[no-new-park-categories]] — the "no new infrastructure carve-outs"
  user policy
- [[difficult-is-not-impossible]] — the cardinal anti-give-up rule
- [[no-compiler-divergence]] — toolchain frozen, only the C varies
- [[gte-wrapper-misroute-park]] — the OPPOSITE bug (GTE leaves routed to
  active); fixed at the same time
- [[slog-kengo-dead-end]] — the two largest items are explicitly
  documented as multi-session pure-C work, not canonical-asm
