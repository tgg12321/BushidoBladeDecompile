# Evidence bank — func_80041BF4

- WIP rejected_form: {'form': 'rect coordinates named in s16 locals, passed to func_80048A7C instead of re-reading rect[0]/rect[1]', 'frame': 88, 'vars': 24, 'score': 51, 'reason': 'hits the EXACT target frame but adds 6 instructions (141 vs 135)'}

- WIP rejected_form: {'form': 'name the twice-read D_80094E08[sid] palette index (s16 / u8 / s32)', 'frame': 72, 'score': 48, 'reason': 'REMOVES the phantom and drops 5-6 insns - the two separate reads are load-bearing'}

- WIP rejected_form: {'form': 'sid + named tbl element reads', 'vars': 16, 'score': 53, 'reason': 'worse than sid alone'}

- WIP rejected_form: {'form': 's16 loop sentinel r', 'vars': 16, 'score': 56, 'reason': '+7 insns'}

- WIP rejected_form: {'form': 's16 xoff / yoff alone', 'vars': 8, 'score': 41, 'reason': 'inert'}

- WIP rejected_form: {'form': 'sid + rect_s16 combined', 'vars': 32, 'score': 78, 'reason': 'overshoots to three slots'}

- WIP rejected_form: {'form': 'sid + s16 g / s16 b / both', 'vars': 16, 'score': '55 / 61 / 61', 'reason': 'no extra slot, +4 to +9 insns'}

- WIP rejected_form: {'form': 'sid + s16 idx / s16 outer', 'vars': 16, 'score': '51 / 55', 'reason': 'no extra slot'}

- WIP rejected_form: {'form': 'sid + s16 xoff / s16 yoff / named sent / s16 rect-constant carrier', 'vars': 16, 'score': '47 / 47 / 47 / 54', 'reason': 'no extra slot'}

- WIP rejected_form: {'form': 'sid + named rect coordinate (rx or ry only)', 'vars': 24, 'score': 48, 'reason': 'reaches 3 slots at +3 insns but scores worse than s16 r'}

- WIP rejected_form: {'form': 'sid + u16 r with (s16) test cast', 'vars': 16, 'score': 46, 'reason': 'only 2 slots'}

- WIP rejected_form: {'form': 'sid + s16 r + explicit (s32) at the call / sentinel untouched', 'vars': 24, 'score': 40, 'reason': 'inert vs the candidate'}

- WIP rejected_form: {'form': '8 pure namings of existing sub-expressions (rect base ptr, (s32)rect arg, image src addr, palette table ptr, second sid copy, loop shift amount, s16 outer selector, func_8004881C result chain)', 'vars': 16, 'score': 47, 'reason': 'naming alone never adds a slot'}

- WIP rejected_form: {'form': 'u16 carriers on the two lhu tbl sites (pair / t0 only / t1 only / u16* walker)', 'vars': 16, 'score': '53 / 47 / 47 / 47', 'reason': 'no third slot'}

- WIP rejected_form: {'form': 's16 sent for the sentinel read; s16 want for the D_800A9A20 comparand; u16 pair + sentinel', 'vars': 16, 'score': '47 / 47 / 53', 'reason': 'no third slot'}

- WIP rejected_form: {'form': 'call-arg LUID: name both gpu_LoadImage args first in the loop block / only the source address / only the rect pointer', 'score': 29, 'reason': 'all inert on the corrected base'}

- WIP rejected_form: {'form': 'arg locals first + rect stores moved after them', 'score': 52, 'reason': 'loses the frame (vars back to 8)'}

- WIP rejected_form: {'form': 'for (;;) { sent = tbl[0]; if (sent < 0) break; ... } (s32 and s16 sentinel)', 'score': 69, 'reason': '131 insns; the while-with-assignment form is the matching one'}

- WIP rejected_form: {'form': 'sid + the corrected loop', 'vars': 32, 'score': 64, 'reason': 'overshoots to four slots'}

- WIP rejected_form: {'form': 'colour birth order b,r,g / g,b,r / r,b,g', 'score': '32 / 39 / 29', 'reason': 'does not reorder the allocation'}

- WIP rejected_form: {'form': 'outer if/else arm swap; yoff before xoff in both arms', 'score': '31 / 31', 'reason': 'no priority change'}

- WIP rejected_form: {'form': 'outer = outer + 1 long form; extra outer reference in each arm', 'score': 29, 'reason': 'inert; the arm-reference form is contrived and not kept'}

- WIP rejected_form: {'form': 's16 outer / s16 yoff', 'score': '34 / 29', 'reason': 's16 outer costs 4 insns; s16 yoff inert'}

- WIP rejected_form: {'form': 'for / while outer-loop spellings', 'score': 34, 'reason': 'pseudo numbering shifts; worse'}

- WIP rejected_form: {'form': 'do { ... } while (outer < 2)', 'score': 34, 'reason': 'moves priorities (5192/3870/5853) but into the wrong permutation yoff > outer > b'}

- WIP rejected_form: {'form': 'outer = outer + 1 long form; switch (outer) arm selection; if (outer != 2) loop-back', 'score': '29 / 29 / 30', 'reason': 'priorities byte-identical at 2000/2333/2325'}

- == imported from memory/wip notes.md ==
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


## [s1] 2026-08-19 — recon — FLOOR 29 -> 22; the rotation is SOLVED

Chassis check: HEAD form (goto outer loop, conflated sentinel) = 41; banked
29-form re-measured 29 on this chassis (canonical verdict C, 135/135 insns).

### The allocation model, completed and QUANTIFIED
global.c `allocno_compare` (tools/gcc-2.7.2/global.c:635):
`pri = floor_log2(refs) * refs / live_length * 10000 * size`; ties broken by
LOWER pseudo number. `refs` = REG_N_REFS is **loop-depth-weighted by flow.c**
(each mention counts `loop_depth`, where depth comes from NOTE_INSN_LOOP
notes — REAL loop constructs only; goto-loops add no depth). Every measured
number in the ledger fits this formula exactly:
  - 29-form: b 2*7/60=2333 > yoff 2*5/43=2325 > outer 2*5/50=2000
    -> allocation order b,yoff,outer -> s2,s3,s4 (the wrong rotation).
    yoff's "extra" 2 refs = the `do { yoff=0; } while (0);` wrap putting the
    else-def one loop level deeper; the inner `while` puts the read at depth 2.
  - the old do-while-outer attempt's 5192/3870/5853: outer 3*9/52 (9 = 1+2+4+2
    weighted), b 3*8/62 (sw at depth 3), yoff 3*8/41 (else-def at depth 3 via
    the do-while(0) wrap + read at depth 3). The wrap is what made yoff win
    and flipped the rotation the WRONG way — the ledger's "wrong permutation"
    was caused by a leftover construct, not by the do-while-outer idea itself.
- .greg dispositions of the 29-form: 72->s0 74->s2 (param homes; a2's home
  ALREADY owns s2 and b was merely SHARING it), 75->fp, 76->s7, 77->s6,
  81->s5, 83->s0, 84->s1 — i.e. everything except the trio already matched
  target. Target differs only in WHICH of {b,yoff,outer} shares s2 with the
  dead a2 home.

### The fix (candidate.c): outer as REAL `do {...} while (outer < 2);`
+ REMOVE the do-while(0) wrap. Measured: rotation flips to target's exact
allocation (a2 home->s2, outer->s2, b->s3, yoff->s4, xoff->s5). Score 22.
Frame/insns still 135/135. This kills the "rotation modelled-exhausted /
owner disposition" conclusion — the s0..s7+fp allocation is now fully matched.

### KILLED this session
- a2-param-reuse-as-outer (rejected/a2-param-reuse-as-outer.c): 34. Merged
  pseudo pri ~1628 (7 refs / ~86 len over two disjoint phases) allocates last
  -> s4 + prologue save-order perturbation.

### Residual 22 (opcode-normalized diff, tmp/grind/func_80041BF4/s1/diff22.txt)
1. D_80094DF0 load in-loop: ours `lui t0,%hi; addiu t0,%lo; addu v0,v0,t0;
   lw s0,0(v0)`; target `lui at,%hi; addu at,at,v0; lw s0,%lo(at)`.
   Sibling func_80041AC8 (same TU, same expression, matched, NOT in a loop)
   emits target's form -> loop-context divergence (loop.c invariant handling
   / CSE of the symbol address under loop notes), not expression spelling.
2. LoadImage arg setup (addiu a0,sp,24 + the D_800A9A24 address chain)
   scheduled after the `lhu tbl[1]` read; target before. Probably cascades
   from (1) — a1 IS the D_800A9A24 chain.
3. Caller-save renames riding on (1)/(2): idx<<5 lands in a1 (target v0);
   the func_8003E2A0()==1 compare constant in v1 (target t0); lhu into v0
   (target v1).
NOTE: src/text1a_post.c REVERTED to HEAD at session end — the 33 regfix +
2 asmfix rules are keyed to the old instruction stream and the do-while
renumbers cc1 `{lbl#N}` slots (brief §2 fragile-anchor warning); applying
the candidate stays a ledger-only operation until the function closes and
the rules can retire in the same change.

- [s1] global.c:635 allocno_compare formula verified against every ledger number: pri = floor_log2(refs)*refs/live_length*10000, ties to lower pseudo number; refs are loop-depth-weighted by flow.c

- [s1] the ledger's old do-while-outer 'wrong permutation yoff > outer > b (5192/3870/5853)' was caused by the leftover do-while(0) yoff wrap (else-def at depth 3), not by the do-while-outer idea — the modelled-exhausted/owner-disposition conclusion for the rotation is DEAD

- [s1] .greg of the 29-form: a2's param home (pseudo 74) already owned s2; the trio question was only which of {b,yoff,outer} SHARES s2 with the dead home

- [s1] residual 22 is non-rotation: (1) in-loop D_80094DF0 load uses full-address-in-t0 vs target's %hi/%lo indexed form (sibling func_80041AC8, same TU + expression + matched, emits target's form outside a loop -> loop-context divergence); (2) LoadImage arg setup scheduled after the lhu vs before (likely cascades from 1); (3) v0/v1/t0 renames riding on those

- [s1] duplicates scan: no near-clone of func_80041BF4 at 0.5 threshold in tmp/duplicates_leads.txt

- [s1] src/text1a_post.c reverted to HEAD at session end: the 33 regfix + 2 asmfix rules anchor on the old instruction stream ({lbl#N} slot-ordinal anchors, brief fragile-carrier warning); the 22-form lives in memory/grind/func_80041BF4/candidate.c
