# Hypothesis ledger — saSeMain_80045600

## s1 (recon, 2026-07-21)

- H1 "s16 local for the id compare creates an orphanable HImode pseudo" — KILLED.
  Mechanism: reload stale-ref phantom (alter_reg on combine-deleted pseudo).
  Probe: vars= gradient, tmp/.../s1/v0_baseline.c. Result: vars=0 — single-use extension folds into
  the lh (mem-fold path decrements refs). Form kept anyway (byte-neutral, natural spelling).

- H2 "HImode arithmetic intermediate (sub/xor) orphans via the paradoxical-subreg shift fold" — KILLED.
  Probes v2 (sub) / v11-v13 (xor family): the HImode op is EMITTED (+2 insns), vars=0. combine does not
  fold (eq (xor a b) 0) → (eq a b) in GCC 2.7.2, and non-bitwise (sub) never qualifies.

- H3 "two uses of the s16 local prevent the mem-fold and force a foldable reg-form extension" —
  KILLED in the two-SImode-COMPARE-uses spelling (v3: CSE unifies pre-flow, vars=0). NOT yet killed for
  mixed-kind dual use (compare + index/call-arg) — see F1; func_800493E4 is a live in-tree witness of
  exactly that firing at zero cost.

- H4 "s16 first param" — KILLED (v4: promoted subreg, no pseudo; also would need caller-side probes).

- H5 "the phantom fires in this function at all" — CONFIRMED (v1 positive control: witness graft →
  vars=8/frame 0x20; costs +11 insns as grafted).

- H6 "args=24 via 5-arg call explains the frame instead of vars=8" — KILLED by inspection: a 5-arg call
  requires a 5th-arg stack store; target has zero sp stores besides ra.

## Frontier (for s2+)

- F1 (TOP): RTL forensics on the two ZERO-COST in-tree witnesses — func_800493E4 (dual-use u8:
  compare + array index) and func_80030900 (lh as direct call arg; u16 truncating RMW). cc1 -da lreg/greg
  dumps: identify the exact orphaned pseudo and the pass path (which combine deletion skips ref
  accounting), then enumerate which consumer shapes fire. Derive a transplantable spelling for a
  function whose ONLY narrow site is `lh; beq` against an s32 param.
- F2: probe grid over consumer-shape respellings of saSeMain's compare that stay byte-neutral:
  e.g. `s16 cur` consumed once by the compare and once by a NON-COMPARE fold-away use (index-like or
  arg-like) built from the function's real semantics; pointer-walk respellings (`a2 = a3 + 8` s16-elem
  arithmetic); found-block store respelled through the s16* base. Use s1's probe.py (vars= gradient) +
  sandbox for byte checks.
- F3: if F1 shows the path needs an insn the target lacks → the phantom is unreachable at zero cost;
  then the remaining axes are the endgame-lock species (compare func_80049A2C: aggregate-only +8 slot,
  owner-gated 2026-07-20). Do NOT go there until F1/F2 are measured dead.

## [s1] A single s16 local for the lh id compare orphans an HImode pseudo (phantom slot)
- mechanism: reload alter_reg stale-ref after combine deletes the redundant sign-extension
- probe: vars= gradient via tmp/grind/saSeMain_80045600/s1/probe.py (v0)
- result: vars=0 — single-use extension takes the mem-fold path (refs decremented); byte-neutral, kept in candidate
- verdict: KILLED

## [s1] HImode xor/sub compare intermediates orphan via the paradoxical-subreg shift fold at zero cost
- mechanism: combine folding (eq (xor a b) 0) -> (eq a b) post-flow
- probe: v2/v11/v12/v13
- result: xor/sub EMITTED (+2 insns), vars=0 — GCC 2.7.2 combine does not do this fold
- verdict: KILLED

## [s1] Two SImode compare uses of the s16 local block the mem-fold and orphan the pseudo
- mechanism: multi-use def not foldable into a single use
- probe: v3
- result: CSE unifies the extension pre-flow; vars=0. Mixed-kind dual use (compare+index) NOT yet killed — func_800493E4 is a live zero-cost witness of that shape
- verdict: KILLED

## [s1] s16 first parameter creates an orphanable pseudo
- mechanism: param narrow type
- probe: v4
- result: promoted subreg, no pseudo exists; vars=0
- verdict: KILLED

## [s1] The phantom-slot mechanism fires in this function/TU at all
- mechanism: tslLineG5Init trigger grafted (two s16 locals, HImode bitwise, two low-bit consumers)
- probe: v1 positive control
- result: vars=8, frame 0x20 — fires, but +11 emitted insns as grafted
- verdict: CONFIRMED

## [s1] args=24 via a 5-arg call explains frame 0x20 instead of vars=8
- mechanism: outgoing-args region growth
- probe: frame equation + target store audit
- result: a 5th arg requires a sw 16($sp) the target lacks; only jal InitFadePanel exists
- verdict: KILLED
