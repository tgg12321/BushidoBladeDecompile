# saTan4FireDisp — WIP (current state 2026-08-04, ra_solver round)

`src/text1a.c:1176`. Fire effect: validates a stage/player id, converts three
colour channels to Q12, then a two-pass outer loop over a coordinate table
uploading 16x1 strips via `gpu_LoadImage` + `func_80048A7C`. 33 regfix + 2
asmfix rules (the asmfix pair moves a `{lbl#6}` label before an `lh`/`lhu`).

## Where it stands

| | score | frame | vars | insns |
|---|---|---|---|---|
| committed HEAD | 41 | 72 | 8 | 135 |
| **`candidate.c`** | **29** | **88** | **24** | **135** |
| target | 0 | 88 | 24 | 135 |

Frame and instruction count are both exactly target's. The residual 29 is a
3-cycle callee-save rotation (ours `s2`/`s3`/`s4` for `b`/`yoff`/`outer` where
target has `s3`/`s4`/`s2`) plus the scheduling that rides on it.

## The structural fix (round 7) — the whole win

Target keeps the **red channel** and the **loop sentinel** in *different*
registers (colours from `$s7`/`$s6`/`$s3`, sentinel in `$v0`) and reads `tbl[0]`
**twice** at the loop label. The m2c body conflated both into one `r`, putting
the label in the wrong place — what the two asmfix rules compensate for. Writing
`while ((sent = tbl[0]) >= 0)` puts the read at the label as target does and
holds 135 insns. **41 → 29.** Also a semantic correction: the old body passed `r`
to `func_80048A7C`, so from iteration 2 on it passed the previous `tbl[0]`
instead of red. **Expected side effect:** the two asmfix rules should become
unnecessary — verify at the completion gate.

## ra_solver round (2026-08-04) — the rotation is now understood

Model extracted; simulate matches the dump **20/23**. The three mismatches
(pseudos 100/106/112, all `$t`-registers) are the documented reload spill-retry
divergence — this is the README's known case. **All three `$s`-register roles
match exactly**, so the part that matters is faithful: 78 = `b` → `$s2`,
82 = `yoff` → `$s3`, 79 = `outer` → `$s4`.

**`perturb.py` (spec `{78:19, 82:20, 79:18}`), 200 single atoms — exactly three
hits, all priority lifts on pseudo 79:** `refs+1`, `refs+2`, `live-8`. **No
conflict, preference or birth-order atom reaches the target.** That closes the
"maybe the priority framing is the wrong model" question for this function: the
round-10 framing was right and is now proven exhaustive at single-atom depth.

**Route 1 validated end-to-end for the first time.** A probe realizing `refs+1`
(`yoff = outer - 1;` in the else arm — DIAGNOSTIC, not a candidate) lifts pseudo
79 from pri 2000 to 2400 and produces **exactly the target rotation**
(`b`→`$s3`, `yoff`→`$s4`, `outer`→`$s2`), moving the score **29 → 18** — the
first movement since round 7. Rejected as a candidate: it is a coercion (the
value is the constant 0, spelled through a variable that happens to hold 1), and
it emits `addiu s4,s2,-1` where **target emits `move s4,zero`** — provably not
the original's code, so it can never reach 0 by that route.

**Route 2 is new** (`tmp/ra/stf_whatif.py`, a 2-atom solution the single-atom
search misses): `b` livelen ≈75 **and** `yoff` livelen ≈55 together also give the
target rotation. Not spellable either — staging `b` from the parameter
(`b = a2; … b = (b << 12) / 255;`) lengthens it but **adds references**
(nrefs 7→9, pri 4500), moving priority the wrong way; and `yoff` cannot be
lengthened without a def, since both its defs are inside the guard arms.

**The decisive new evidence: neither route is what the original did.** Target
references `outer` at exactly the same four sites as we do — `move s2,zero` (65),
`bnez s2` (66), `addiu s2,s2,1` (112), `slti v0,s2,2` (113) — i.e. the same
nrefs and the same span, hence the same priority 2000. So the original reached
the rotation without any priority difference at all. Combined with this being the
one function whose dump carries reload-retry effects, the likeliest explanation is
that the final allocation is decided **post-reload**, outside the modeled path.

**Second cluster revealed.** With the rotation fixed (score 18), the remaining
diffs are a *different* group: the `gpu_LoadImage` argument setup (target
`addiu a0,sp,24` / `lui a1` / `addu a1,v0,a1` where ours uses `$t0`) plus
`sll v0,s1,0x5` vs `sll a1,s1,0x5` and `li t0,1` vs `li v1,1` — precisely
pseudos 100/106/112, the reload-retry trio. So the rotation is worth ~11 points
and closing it alone does **not** reach 0.

## Measured negative / inert (do not re-run)

- **Loop respellings (6 forms):** `for`/`while` outer forms (34);
  `do { … } while (outer < 2)` (34 — moves priorities but into the wrong
  permutation `yoff > outer > b`); long-form increment, `switch (outer)` arm
  selection, `if (outer != 2)` loop-back: all leave 2000/2333/2325 byte-identical.
- **Guard inversion** (`if (outer != 0)` with arms swapped): priorities
  byte-identical to the candidate.
- **A reference to `outer` outside the arms** (`idx = outer - outer;`): folded,
  inert — confirms [[duplicated-statement-into-arms]] cannot supply the sixth
  reference here, because the arms are guarded by `outer == 0` so GCC constant-
  folds `outer` inside them.
- **Colour staging:** `b = a2; … b = (b << 12)/255;` (nrefs 9, pri 4500);
  all three channels staged (same, and `outer` moves to `$s6`); `b` staged before
  the early-return guards (livelen 77 but nrefs 9, pri 3506). All raise `b`'s
  priority rather than lowering it.
- **Permuter — CLOSED** (2026-08-04, operator-supervised). Two campaigns on a
  clean single-function pair: rot-cycle3 from candidate-29, 75k iters, 186 finds,
  best 120 (introduces a `new_var2` constant-holder + split-shift respelling);
  rot-cycle4 reseeded from that find, 76k iters, 5 finds, best 105 — the 105 find
  adds an empty duplicated-condition `if ((!b)&&(!b)){}`, a dead-code cheat-form,
  REJECTED per [[no-new-park-categories]]. Basin flat; campaign stopped inside
  the session (no orphan).

## Resume here — banked at 29

Start from `candidate.c`. The rotation's mechanism is now pinned: both modeled
routes work in simulation, neither is spellable, and **target's own reference
count proves the original used neither**. The next legitimate move is either an
extension of `ra_solver` past reload (this function is the known retry case), or
an owner-level disposition. Do not re-run loop respellings or the permuter.

Screen anything touching the frame with `tmp/stf_orphan.py`.

## Instruments

`tmp/stf*.py`, `tmp/stf_orphan.py` (unallocated-pseudo detector),
`tmp/rtldump.sh`, `tmp/orphan_probe.py`, `tmp/frame_probe.sh`;
`tmp/ra/*` (generic apply / probe / prio / model harness),
`tmp/ra/stf_whatif.py` (the two-route table).
