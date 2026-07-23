# Evidence bank — func_800617C8

## s1 (recon) — WALL REFUTED, pure-C floor 9 -> 0

- **Honest floor 0 reached in pure C** via the walking-pointer form. sandbox
  --disable all == 0. Zero regfix/asmfix, zero cheat-asm, zero volatile.
- The inherited "v0/v1 swap wall" (WIP: "proven unreachable via cc1psx
  calibration") was derived ONLY from `arg0[i]` INDEXING forms. It was never
  tested against the plain WALKING-POINTER form.
- **Mechanism of the wall:** post-call block does temp1/temp2/temp3 loads +
  stores to D_800F1140/44/48, plus a mask store D_800A3464 = 0xC06013. Mask
  0xC06013 needs lui+ori (2 insns). Target: load temps -> $v0, mask -> $v1,
  with the mask lui/ori/store scheduled into the load-delay regions (mask store
  emitted before the final temp store). Indexing form allocates the reverse
  (temp -> $v1, mask -> $v0) -> 9 diffs.
- **The lever:** walking pointer `p = arg0; D_800F1140 = *p++; D_800F1144 =
  *p++; D_800F1148 = *p; D_800A3464 = 0xC06013;`. The three fresh single-use
  loads (vs one reused 3-web temp) change allocno priority so temps take $v0 and
  the mask takes $v1 — matching target. cc1's first-pass scheduler defers the
  mask lui+ori+store into temp3's load-delay region on its own. NO staging local,
  NO reorder needed.
- **Sibling proof:** func_800618B4 (same file, same cluster shape, one extra arg)
  is committed COMPLETED-C using exactly this walking-pointer form (mask store
  after all loads). Verified sandbox == 0 on it this session. Only structural
  delta to 617C8 is the mask constant (0xFF0000 = single lui vs 0xC06013 = lui+ori)
  and one-arg vs two-arg prologue.
- Objdump of the score-0 build matches asm/funcs/func_800617C8.s exactly in the
  post-call region (temps $v0, mask $v1, mask store before final temp store).

## Intermediate measurements (this session)
- indexing form, pins removed: floor 9 (the wall). rejected/indexing-form-floor9-wall.c
- walking ptr, mask store BEFORE final store, no t3: floor 5 (mask lui hoisted
  too early by scheduler).
- walking ptr + t3 staging + mid-seq mask store: 0 but flagged codegen-only by
  layer-1 reviewer; UNNECESSARY. rejected/t3-staging-reorder.c
- walking ptr, mask store AFTER all loads (natural sibling form): **0**. CANDIDATE.

## Family implication (NOT acted on — out of my mandate)
The whole text1b render-state cluster (func_800611A4, func_80061250, func_80061454,
func_800614E0, func_8006156C, func_80061658, func_80061710) shares the identical
pin + post-call mask/load shape and was declared the same "wall". The
walking-pointer lever likely retires the family. func_800611A4 is currently
owner-escalated (docs/grind/decisions.md 2026-07-22) on this exact swap — this
finding is direct counter-evidence that the swap IS pure-C-reachable via the
walking-pointer form. A future session on those functions should try the lever
before honoring the escalation.

## Prior (imported from memory/wip notes.md) — superseded by s1
- WIP claimed floor 9, all 9 diffs a v0<->v1 swap, unreachable in pure C.
  REFUTED s1: reachable via walking-pointer form.
