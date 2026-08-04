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
at the loop label. The m2c body conflated both into one `r`, misplacing the label
— what the two asmfix rules compensate for. `while ((sent = tbl[0]) >= 0)` puts
the read at the label and holds 135 insns. **41 → 29.** Also a semantic fix: the
old body passed `r` to `func_80048A7C`, so from iteration 2 on it passed the
previous `tbl[0]` instead of red. **The two asmfix rules should become
unnecessary** — verify at the gate.

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
**29 → 18** — first movement since round 7. Best is `xoff = 0x7F + outer;`: the
arm region then matches target *including* `move s4,zero`, the only divergence
being `li s5,128` → `addiu s5,s2,127`. (`yoff = outer - 1` is worse.)

### Why every such spelling is rejected — the fold/emit dichotomy

Round 10's "references fold" result **was** a then-arm finding; in the else arm
GCC 2.7.2 does not derive `outer == 1`, so a reference there is genuinely
unfoldable. That is precisely the problem. Measured, not argued
(`tmp/ra/prio.sh`, `tmp/ra/passcensus.sh`):

| else-arm spelling | nrefs | pri | rotation | bytes |
|---|---|---|---|---|
| `yoff = outer & 0` / `* 0` / `outer - outer` | 5 | 2000 | no | unchanged |
| `xoff = 0x7F + outer` / `0x80 * outer` | **6** | **2400** | **TARGET** | **+1 wrong insn** |

**A foldable reference is never born.** For `outer & 0` the pseudo-79 count is
**5 in the very first `.rtl` dump**, identical to baseline — the fold happens in
the front end before RTL expansion, so no pass "eats" it; it never exists. For
`xoff = 0x7F + outer` it is **6 from `.rtl` through `.lreg`**. A reference and an
instruction are the same object here.

**Target has nowhere to put a sixth reference.** Its only instructions touching
`outer`'s register in the loop are `move s2,zero` (65), `bnez s2` (66),
`addiu s2,s2,1` (112), `slti v0,s2,2` (113) — four instructions carrying exactly
the five refs we have. **Byte-neutrality and the extra ref are exclusive.**

**Route 2 is new** (`tmp/ra/stf_whatif.py`, a 2-atom solution): `b` livelen ≈75
**and** `yoff` ≈55 together also give the rotation. Not spellable — staging `b`
**adds references** (nrefs 7→9, pri 4500), and `yoff` needs a def to lengthen.

**Neither route is what the original did.** Target references `outer` at the same
four sites with the same span, hence the same priority 2000.

**And the reload escape is now MEASURED dead for the rotation** (Phase 5,
`tmp/ra/retry_survey.sh` via `BB2_FINDREG_DEBUG`'s `retry=` field). Retry *does*
fire here — pseudo 100 shows `calls=3` with **2 `retry=1` blocks**, 106/112 are
re-entered twice each — but **only on the `$t` trio 100/106/112**, exactly the
pseudos the simulator already mismatched. The three `$s` roles each show
`calls=1`, retry 0, so retry explains only the second cluster.

**cc1psx (the original PsyQ compiler) is not the escape.** On this candidate it
diverges *structurally* — 118 vs the fork's 116 instructions, inner loop not
rotated the same — but that is **further** from target, and the `$s` assignment
is unchanged (`b` in `$s2`). Contrast: on `func_8007C7A0`'s stream-exact body
cc1psx is byte-identical to the fork; on `hirahira_w_ctrl` its multiset is
identical (prologue scheduling only). No body tested reaches target's registers.

## Measured negative / inert (do not re-run)

- **Loop respellings (6 forms):** `for`/`while` forms (34); `do…while (outer < 2)`
  (34 — wrong permutation); long-form increment, `switch (outer)`, `if (outer != 2)`
  loop-back: all leave 2000/2333/2325 byte-identical. **Guard inversion** too.
- **Then-arm / outside-arm refs** (`idx = outer - outer`, `yoff = 0xF0 + outer`):
  folded, inert.
- **[[duplicated-statement-into-arms]] in BOTH arms** (`0xF0 * (outer ± 1)`):
  nrefs 7, pri 2641 — **wrong permutation** (`b`→`$s4`, `yoff`→`$s3`), and it
  perturbs `b`'s livelen 60→63.
- **Colour staging:** `b = a2; … b = (b << 12)/255;` (nrefs 9, pri 4500), all
  three channels, and `b` staged before the guards (livelen 77) — all raise
  `b`'s priority rather than lowering it.
- **Permuter — CLOSED** (operator-supervised). rot-cycle3: 75k iters, best 120;
  rot-cycle4 reseeded: 76k iters, best 105 — adds an empty `if ((!b)&&(!b)){}`,
  a dead-code cheat-form, REJECTED per [[no-new-park-categories]]. Basin flat.

## Resume here — banked at 29

Start from `candidate.c`. The rotation is fully pinned: both modeled routes
reproduce it in simulation, **neither is byte-neutral by construction**, target's
reference count proves the original used neither, **retry is measured not to
touch the `$s` trio**, and **cc1psx does not reach target either**. Do not re-run
loop respellings, the permuter, further `outer`-reference spellings, or a reload
extension aimed at the rotation. Remaining: an owner disposition (pseudos
100/106/112 are where retry lives — the only part a reload model could address).
**Instruments:** `tmp/stf*.py`, `tmp/stf_orphan.py`; `tmp/ra/*` — `prio.sh`,
`passcensus.sh`, `retry_survey.sh`, `stf_whatif.py`, `psx.sh`, `psxdiff.py`.
