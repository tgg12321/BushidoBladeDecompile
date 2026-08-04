# saTan4FireDisp — WIP (current state 2026-08-04, round 10)

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

**Frame and instruction count are both exactly target's.** The residual 29 is a
3-cycle callee-save rotation (ours `s2`/`s3`/`s4` where target has
`s3`/`s4`/`s2`) plus the scheduling that rides on it.

## The structural fix (round 7) — the whole win

Target keeps the **red channel** and the **loop sentinel** in *different*
registers (colours from `$s7`/`$s6`/`$s3`, sentinel in `$v0`) and reads
`tbl[0]` **twice** at the loop label (`lh` for the test, `lhu` for the next
`rect[0]`). The m2c body conflated both into one `r`, putting the label in the
wrong place — exactly what the two asmfix rules compensate for. Writing
`while ((sent = tbl[0]) >= 0)` with the sentinel as its own variable puts the
read at the label as target does, supplies the third phantom slot, and holds
135 insns. **41 → 29.** Also a **semantic correction**: the old body passed `r`
to `func_80048A7C`, so from iteration 2 on it passed the previous `tbl[0]`
instead of red. **Expected side effect:** the two asmfix rules should become
unnecessary — verify at the completion gate.

## Round 10 — ALLOCDBG numbers: the gap is exactly quantified

Instrumented cc1 (`tools/gcc-2.7.2/cc1`, env `BB2_ALLOC_DEBUG=1
BB2_PRIO_DEBUG=1`; driver `tmp/allocdbg.sh`, table extractor
`tmp/allocpick.py`), candidate applied:

| ord | pseudo | reg | nrefs | livelen | pri | role |
|---|---|---|---|---|---|---|
| 8 | 78 | `$s2` | 7 | 60 | **2333** | `b` |
| 9 | 82 | `$s3` | 5 | 43 | **2325** | `yoff` |
| 10 | 79 | `$s4` | 5 | 50 | **2000** | `outer` |

`pri = floor_log2(nrefs) * nrefs / livelen * 10000`, verified against all three.

**`b > yoff` already holds (2333 > 2325) — the only requirement is
`outer > 2333`.** At livelen 50 that needs `floor_log2(n)*n > 11.67`:

- **nrefs 6** → `2*6/50*10000 = 2400` ✓ (one extra reference is exactly enough)
- or livelen ≤ 42 at nrefs 5 → `2*5/42*10000 = 2380` ✓ (shorten the live range)

So the required lift is ~17%, and one unfoldable reference to `outer` would do it.

**Loop respellings (6 forms), none delivers it:** `for` and `while` outer-loop
forms (34, and the pseudo numbering shifts); `do { … } while (outer < 2)` (34 —
it *does* move the priorities to `outer` 5192 / `b` 3870 / `yoff` 5853, but into
the **wrong** permutation `yoff > outer > b`); long-form increment,
`switch (outer)` arm selection, and `if (outer != 2)` loop-back all leave the
three priorities **byte-identical** at 2000 / 2333 / 2325.

**Why the sanctioned ref-lift cannot reach it here:**
[[duplicated-statement-into-arms]] needs a real statement mentioning `outer`
duplicated into both arms — but the arms are guarded by `outer == 0`, so GCC
folds `outer` to a constant in the then-arm and the reference never reaches
`reg_n_refs`. That is why round 9's `yoff = 0xF0 + outer` probe was inert. No
byte-neutral construct in this body adds an *unfoldable* sixth reference.

## Resume here — banked at 29, with the number to beat

Start from `candidate.c` (29). The task is now a one-line spec:

> **raise pseudo 79 (`outer`) from pri 2000 to ≥ 2334** — one more unfoldable
> reference at livelen ~50, or a live range shortened to ≤ 42 — **without
> disturbing `b` (2333) or `yoff` (2325).**

Next rung is the **directed permuter** (step 3 of [[register-alloc-pure-c]]):
clean single-function target at offset 0, `PERM_*` directed mode,
`tools/permuter_campaign.py` for telemetry + the fresh-seed stopping rule
(~20-30k iters per [[permuter-fresh-seed-discipline]]). **Not started here** —
per [[grinder-permuter-orphan-stop-gate]] a detached campaign must be
waited on and harvested inside the same session, and there was not enough
session left to supervise one. Do not launch it without that budget.

Screen anything touching the frame with `tmp/stf_orphan.py`.

## Instruments

`tmp/stf*.py` (rounds 1-9), **`tmp/stf_orphan.py`** (unallocated-pseudo
detector), `tmp/stf_apply.py`, `tmp/rtldump.sh`, `tmp/orphan_probe.py`,
`tmp/frame_probe.sh`, `tmp/sbs.sh`.

## Permuter rung — CLOSED (2026-08-04, operator-supervised)

Two campaigns on a clean single-function pair (135/135, offset 0):
rot-cycle3 from candidate-29: 75k iters, 186 finds, best 120 — the find
introduces new_var2=0xF0 (constant-holder, SOTN new_var convention) + a
split-shift respelling ((a0<<5)<<7)/255; vettable but nonzero.
rot-cycle4 reseeded from the 120 find: 76k iters, 5 finds, best 105 — the
105 find adds an EMPTY duplicated-condition if ((!b)&&(!b)){} = dead-code
cheat-form; REJECTED per no-new-park-categories (recorded, not surfaced as a
candidate). Basin flat; campaign stopped inside the session (no orphan).
The permuter rung is exhausted. BANKED at 29; every ladder rung has run.
Remaining path per endgame policy: the s2/s3/s4 rotation needs a construct
nobody has found — or an owner-level disposition.
