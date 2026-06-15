# func_800651F0 — WIP (text1b: copy 3-word table twice + set flags)

## TL;DR
**Floor LOWERED 13 -> 7** with a clean pure-C change (no cheats). HEAD "matches"
via two `register asm()` pins (`p asm("$3")`, `t asm("$2")`), a `volatile`
qualifier (volatile-coercion cheat), AND 3 regfix rules (reorder + 2x
`subst $4->$2`). candidate.c is the cheat-free t/u split at sandbox distance 7.
RESUME FROM candidate.c, not HEAD.

## What worked (the floor drop)
candidate.c removes ALL cheats (pins, volatile, the 3 regfix rules become
unnecessary) and splits the single reused temp into TWO block-locals: `t` for
the first read-pass (p[0],p[1],p[2]) and `u` for the second (p[0],p[1]). This is
Lever A ([[register-alloc-pure-c]] block-local split). It moved the first-pass
temp + the const-1 (`D_800F1114=1`) into v0 (matching target), dropping 13->7.

## Remaining 7 diffs = the SECOND read-pass temp `u` is in a0, target v0
Target reuses v0 for ALL temps (both passes) AND the const-1 — one register,
sequentially coalesced. Our GCC coalesces pass1-temp + const into v0 but leaves
the (non-overlapping) pass2-temp `u` in a0. GCC's local-alloc won't coalesce
the second short pseudo into v0.

## rejected_forms (measured-negative, do NOT re-derive)
- single temp `t` for both passes (v1/v12): 13 (const grabs v0, ALL temps a0).
- route const-1 through t or u (v2/v9/v10): SCRAMBLE to 34 (p flips to v0).
- third temp `w` for the const (v6): 7 (no change vs v5).
- swap which var is pass1 vs pass2 (v7): 7.
- decl `u` before `t` (v11): 7.
- `u` holds only p[2], t holds the rest (v8): 34 (scramble).
- move D_800F1114=1 to the end (v4): 26 (wrong store order).

## Avenues for next session (change MODALITY)
- decomp-permuter from candidate.c — the residual is a single short-pseudo
  coalescing miss (pass2 temp a0->v0); permuter's structural mutations are the
  documented modality for exactly this.
- cc1 -da greg dump to see why the pass2 pseudo won't coalesce into v0 (it's
  non-overlapping with pass1-temp + const), then a targeted live-range nudge.

## Floor
- HEAD honest distance: 13 (carries 2 pins + volatile + 3 regfix rules)
- candidate.c (v5): sandbox --disable all = 7, ZERO cheats. Floor LOWERED.
- NB: the 3 regfix rules in regfix.txt are HEAD-form-specific (instruction
  indices); they do NOT apply to candidate.c. On the eventual close, retire them.
