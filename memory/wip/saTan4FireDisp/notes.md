# saTan4FireDisp — WIP (current state 2026-08-06)

`src/text1a.c:1176`. Fire effect: validates a stage/player id, converts three
colour channels to Q12, then a two-pass outer loop over a coordinate table
uploading 16x1 strips via `gpu_LoadImage` + `func_80048A7C`. 33 regfix + 2
asmfix rules (the asmfix pair moves a `{lbl#6}` label before an `lh`/`lhu`).

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
at the loop label. The m2c body conflated both into one `r`, misplacing the label
— what the two asmfix rules compensate for. `while ((sent = tbl[0]) >= 0)` puts
the read at the label and holds 135 insns. **41 → 29.** Also a semantic fix: the
old body passed `r` to `func_80048A7C`, so from iteration 2 on it passed the
previous `tbl[0]`. **The two asmfix rules should become unnecessary** — verify
at the gate.

## The rotation is fully understood, and every route to it is closed

`ra_solver` reproduces this function's allocation **17/17 exactly** (2026-08-06;
see "allocation model" below). All three `$s` roles are already right:
78 = `b` → `$s2`, 82 = `yoff` → `$s3`, 79 = `outer` → `$s4`.

**`perturb.py` (spec `{78:19, 82:20, 79:18}`), 200 single atoms — exactly three
hits, all priority lifts on pseudo 79:** `refs+1`, `refs+2`, `live-8`. **No
conflict, preference or birth-order atom reaches the target**, so the priority
framing is exhaustive at single-atom depth. **Route 1 (refs+1) is validated
end-to-end** — probes in the **else arm** lift pseudo 79 to pri 2400 and produce
**exactly the target rotation**, score **29 → 18**. Best is
`xoff = 0x7F + outer;`: the arm region then matches target *including*
`move s4,zero`, the only divergence being `li s5,128` → `addiu s5,s2,127`.

### Why every such spelling is rejected — the fold/emit dichotomy

Measured, not argued (`tmp/ra/prio.sh`, `tmp/ra/passcensus.sh`):

| else-arm spelling | nrefs | pri | rotation | bytes |
|---|---|---|---|---|
| `yoff = outer & 0` / `* 0` / `outer - outer` | 5 | 2000 | no | unchanged |
| `xoff = 0x7F + outer` / `0x80 * outer` | **6** | **2400** | **TARGET** | **+1 wrong insn** |

**A foldable reference is never born.** For `outer & 0` the pseudo-79 count is
**5 in the very first `.rtl` dump** — the fold happens in the front end before
RTL expansion, so no pass "eats" it. For `xoff = 0x7F + outer` it is **6 from
`.rtl` through `.lreg`**. A reference and an instruction are the same object.
**Target has nowhere to put a sixth reference.** Its only instructions touching
`outer`'s register in the loop are `move s2,zero` (65), `bnez s2` (66),
`addiu s2,s2,1` (112), `slti v0,s2,2` (113) — four instructions carrying exactly
the five refs we have. **Byte-neutrality and the extra ref are exclusive.**

**Route 2** (`tmp/ra/stf_whatif.py`, 2 atoms): `b` livelen ≈75 **and** `yoff`
≈55 together also give the rotation. Not spellable — staging `b` **adds**
references (nrefs 7→9, pri 4500), and `yoff` needs a def to lengthen.

**Neither route is what the original did.** Target references `outer` at the same
four sites with the same span, hence the same priority 2000. **cc1psx is not the
escape either**: on this candidate it diverges *structurally* (118 vs the fork's
116 insns, inner loop rotated differently) — further from target, `b` still `$s2`.

## The allocation model — complete as of 2026-08-06 (Campaign 7)

Earlier notes recorded "three mismatches (pseudos 100/106/112) = the reload
spill-retry divergence". That was **two thirds a simulator artifact**:

* All three are `mulhi` pseudos — the three `channel / 255` conversions. GCC
  emits `(truncate:SI (lshiftrt:DI (mult:DI …) 32))` for division by a constant;
  `extract.py`'s MD-class regex read only the outermost operator, saw
  `truncate`, and typed them GR_REGS. Only the highest-priority one gets `$hi`;
  the other two lose it to a conflict and fall back to GR_REGS via the alternate
  class (global.c:585). **They were never divergent.**
* `simulate.py` also compared against `.greg` dispositions, which cc1 writes
  *after* reload — post-retry values against a pre-reload simulation.
* **Exactly one** pseudo is retry-affected: 99, `$hi` → `$t1`. reload pass 1
  needs `GR_REGS n=1` + `HI_REG n=1`, spills `$t0` and `$hi`; evicting 99 sends
  it to the lowest free GR register.

`reload_sim.py --target text1a saTan4FireDisp 99 8` gives its only lever: `$t0`
would have to stop being a spill register. **None of this touches the `$s`
rotation** — the reload escape is *modelled* dead, not just measured dead.
Details: `memory/wip/_reload_solver_2026-08-06.md`.

## Measured negative / inert (do not re-run)

- **Loop respellings (6 forms):** `for`/`while` (34); `do…while (outer < 2)`
  (34 — wrong permutation); long-form increment, `switch (outer)`, `if (outer
  != 2)` loop-back: all leave 2000/2333/2325 byte-identical. **Guard inversion**
  too.
- **Then-arm / outside-arm refs** (`idx = outer - outer`, `yoff = 0xF0+outer`): inert.
- **[[duplicated-statement-into-arms]] in BOTH arms** (`0xF0 * (outer ± 1)`):
  nrefs 7, pri 2641 — **wrong permutation** (`b`→`$s4`, `yoff`→`$s3`), and it
  perturbs `b`'s livelen 60→63.
- **Colour staging:** `b = a2; … b = (b << 12)/255;` (nrefs 9, pri 4500), all
  three channels, and `b` staged before the guards (livelen 77) — all raise
  `b`'s priority rather than lowering it.
- **Permuter — CLOSED** (operator-supervised). rot-cycle3 75k iters best 120;
  rot-cycle4 reseeded 76k iters best 105 — adds an empty `if ((!b)&&(!b)){}`,
  a dead-code cheat-form, REJECTED per [[no-new-park-categories]]. Basin flat.

## Resume here — banked at 29

Start from `candidate.c`. The rotation is fully pinned: both modeled routes
reproduce it in simulation, **neither is byte-neutral by construction**, target's
reference count proves the original used neither, **the reload/retry path is
modelled and does not reach the `$s` trio**, and **cc1psx does not reach target
either**. Do not re-run loop respellings, the permuter, further `outer`-reference
spellings, or any reload work aimed at the rotation. Remaining: an owner
disposition. **Instruments:** `tmp/stf*.py`, `tmp/stf_orphan.py`; `tmp/ra/*` —
`prio.sh`, `passcensus.sh`, `retry_survey.sh`, `stf_whatif.py`, `psx.sh`,
`psxdiff.py`; `tools/ra_solver/reload_sim.py`.
