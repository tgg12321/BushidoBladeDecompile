# saTan4FireDisp — WIP (current state 2026-08-04, else-arm round)

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

Frame and instruction count are exactly target's. The residual 29 is a 3-cycle
callee-save rotation — ours `s2`/`s3`/`s4` for `b`/`yoff`/`outer` where target
has `s3`/`s4`/`s2` — plus the scheduling that rides on it.

## The structural fix (round 7) — the whole win

Target keeps the **red channel** and the **loop sentinel** in *different*
registers (colours `$s7`/`$s6`/`$s3`, sentinel `$v0`) and reads `tbl[0]` **twice**
at the loop label. The m2c body conflated both into one `r`, putting the label in
the wrong place — what the two asmfix rules compensate for.
`while ((sent = tbl[0]) >= 0)` puts the read at the label and holds 135 insns.
**41 → 29.** Also a semantic correction: the old body passed `r` to
`func_80048A7C`, so from iteration 2 on it passed the previous `tbl[0]` instead
of red. **Expected side effect:** the two asmfix rules should become unnecessary.

## ra_solver + else-arm rounds (2026-08-04) — the rotation is understood

Model extracted; simulate matches the dump **20/23**. The three mismatches
(pseudos 100/106/112, all `$t`-registers) are the documented reload spill-retry
divergence — the README's known case. **All three `$s` roles match exactly**:
78 = `b` → `$s2`, 82 = `yoff` → `$s3`, 79 = `outer` → `$s4`.

**`perturb.py` (spec `{78:19, 82:20, 79:18}`), 200 single atoms — exactly three
hits, all priority lifts on pseudo 79:** `refs+1`, `refs+2`, `live-8`. **No
conflict, preference or birth-order atom reaches the target**, so the round-10
priority framing was right and is now exhaustive at single-atom depth.

**Route 1 (refs+1) is validated end-to-end.** Probes in the **else arm** lift
pseudo 79 to pri 2400 and produce **exactly the target rotation**, score
**29 → 18** — first movement since round 7. Best is `xoff = 0x7F + outer;`
(`S_E5`): the arm region then matches target *including* `move s4,zero`, the only
divergence being `li s5,128` → `addiu s5,s2,127`. (`0x80 * outer` is equivalent;
`yoff = outer - 1` is worse — it breaks `move s4,zero` itself.)

### Why every such spelling is rejected — the fold/emit dichotomy

The else-arm idea is sound in one respect: round 10's "references fold" result
**was** a then-arm finding, and in the else arm GCC 2.7.2 does not derive
`outer == 1` from a two-pass loop, so a reference there is genuinely unfoldable.
That is precisely the problem. Measured, not argued (`tmp/ra/prio.sh` for nrefs,
`tmp/ra/passcensus.sh` for the per-pass census):

| else-arm spelling | nrefs | pri | rotation | bytes |
|---|---|---|---|---|
| `yoff = outer & 0` / `* 0` / `outer - outer` | 5 | 2000 | no | unchanged |
| `xoff = 0x7F + outer` / `0x80 * outer` | **6** | **2400** | **TARGET** | **+1 wrong insn** |

**A foldable reference is never born.** The pass census counts pseudo-79
appearances per `-da` dump: for `outer & 0` it is **5 in the very first `.rtl`
dump**, identical to baseline — the fold happens in the front end before RTL
expansion, so no pass "eats" it; it never exists. For `xoff = 0x7F + outer` it is
**6 from `.rtl` through `.lreg`**. A reference and an instruction are the same
object here; no pass elides one and keeps the other.

**Target has nowhere to put a sixth reference.** Its only instructions touching
`outer`'s register in the loop region are `move s2,zero` (65), `bnez s2` (66),
`addiu s2,s2,1` (112), `slti v0,s2,2` (113) — four instructions carrying exactly
the five refs we have. (`sw`/`move s2,a2`/`sll`/`lw` at 6/7/40/130 belong to the
blue-channel value sharing the register earlier.) **Byte-neutrality and the extra
reference are mutually exclusive.**

**Route 2 is new** (`tmp/ra/stf_whatif.py`, a 2-atom solution the single-atom
search misses): `b` livelen ≈75 **and** `yoff` ≈55 together also give the
rotation. Not spellable — staging `b` **adds references** (nrefs 7→9, pri 4500),
and `yoff` cannot be lengthened without a def.

**Neither route is what the original did.** Target references `outer` at the same
four sites with the same span, hence the same priority 2000 — so the original
reached the rotation with no priority difference at all. Combined with this being
the one function whose dump carries reload-retry effects, the final allocation is
most likely decided **post-reload**, outside the modeled path.

## Measured negative / inert (do not re-run)

- **Loop respellings (6 forms):** `for`/`while` outer forms (34);
  `do { … } while (outer < 2)` (34 — wrong permutation `yoff > outer > b`);
  long-form increment, `switch (outer)`, `if (outer != 2)` loop-back: all leave
  2000/2333/2325 byte-identical. **Guard inversion**: byte-identical.
- **Then-arm / outside-arm references** (`idx = outer - outer`, round 9's
  `yoff = 0xF0 + outer`): folded, inert.
- **[[duplicated-statement-into-arms]] in BOTH arms** (`0xF0 * (outer ± 1)`):
  nrefs 7, pri 2641 — **wrong permutation** (`b`→`$s4`, `yoff`→`$s3`), and it
  perturbs `b`'s livelen 60→63.
- **Colour staging:** `b = a2; … b = (b << 12)/255;` (nrefs 9, pri 4500); all
  three channels staged; `b` staged before the early-return guards (livelen 77,
  pri 3506). All raise `b`'s priority rather than lowering it.
- **Permuter — CLOSED** (operator-supervised). rot-cycle3: 75k iters, best 120
  (`new_var2` constant-holder + split-shift); rot-cycle4 reseeded: 76k iters,
  best 105 — adds an empty `if ((!b)&&(!b)){}`, a dead-code cheat-form, REJECTED
  per [[no-new-park-categories]]. Basin flat.

## Resume here — banked at 29

Start from `candidate.c`. The rotation's mechanism is fully pinned: both modeled
routes reproduce it in simulation, **neither is byte-neutral by construction**,
and target's own reference count proves the original used neither. Do not re-run
loop respellings, the permuter, or any further `outer`-reference spelling — the
fold/emit dichotomy closes that family. The next legitimate move is extending
`ra_solver` past reload (this is the known retry case) or an owner disposition.

## Instruments

`tmp/stf*.py`, `tmp/stf_orphan.py`, `tmp/rtldump.sh`, `tmp/frame_probe.sh`;
`tmp/ra/*` (generic harness) — `prio.sh` (allocdbg rows), `passcensus.sh`
(per-pass pseudo census), `window.sh` (listing window), `stf_whatif.py`.
