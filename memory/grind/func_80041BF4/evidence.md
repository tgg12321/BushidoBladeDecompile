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

## [s2] 2026-08-19 — structural — FLOOR 22 -> 17; the LICM/symbol-fold mechanism is NAMED

Chassis check at dispatch: candidate.c (s1 form) re-measured **22** on this
chassis, 135/135 insns, canonical verdict C. Ledger floor confirmed, not stale.

### The mechanism, read from dumps (not inferred)
`pwsh tools/grinder/dump.ps1 func_80041BF4` -> tmp/grind/func_80041BF4/dumps/.
The `.loop` dump carries loop.c's own decision log:

    Loop from 175 to 272: 37 real insns.
      Insn 217: regno 138 (life 1), move-insn savings 1  moved to 319
      Insn 222: regno 139 (life 1), move-insn savings 1  moved to 321
      Insn 227: regno 141 (life 1), move-insn savings 1  moved to 323
    Loop from 126 to 288: 61 real insns.
      Insn 163: regno 124 (life 2), move-insn savings 1  moved to 325
      Insn 319: regno 138 (life 43), ... halved since already moved  moved to 327
      ... (321 -> 329, 323 -> 331)

- regno 124 = `(set r124 (symbol_ref "D_80094DF0"))`, hoisted out of the OUTER
  loop into its preheader by `move_movables` (loop.c:1631, the
  `threshold * savings * m->lifetime >= insn_count` test; threshold =
  `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`, loop.c:532 — measured to
  sit in [37,82] on this chassis, so ANY life-1 invariant in a <=37-insn
  call-carrying loop is hoisted).
- Hoisting moves the def to a DIFFERENT basic block from its use, so `combine`
  (which runs after loop) can no longer fold the symbol into the MEM. That is
  the entire cause of residual item 1: ours emitted
  `lui t0,%hi; addiu t0,t0,%lo; addu v0,v0,t0; lw s0,0(v0)` where target has
  `lui at,%hi(D_80094DF0); addu at,at,v0; lw s0,%lo(D_80094DF0)(at)`.
- WHY there was a separable `(set reg symbol_ref)` at all: `.rtl` insn 163/165/
  166/168 show that for the scale-4 array index `D_80094DF0[k]` expand emits a
  standalone symbol load PLUS an explicit `(set r127 (plus r126 r124))` add.
  For the scale-1 array `D_80094E08[k]` (same function, two lines up) expand
  keeps the sum un-emitted and produces `(mem (plus (reg) (symbol_ref)))`
  directly — which is why that access already matched. The MULT node from the
  scale-4 index is what forces the sum out of EXPAND_SUM into a real insn.
- loop.c's own escape (the `reg_single_usage` substitute-and-delete path,
  loop.c:735-767) cannot fire for the array-index form: substituting the
  symbol into `(set r127 (plus r126 SYM))` yields an invalid MIPS `addsi3`
  (operand 2 must be `arith_operand`), so `validate_replace_rtx` fails and the
  insn becomes a movable instead. It DOES fire when the use is the MEM itself.
- Sibling func_80041AC8 (same TU, byte-identical expression, MATCHED) gets
  target's form from the plain array-index spelling only because it is not in
  a loop: nothing hoists, so combine folds the symbol in-block. The s1 note
  "loop-context divergence, not expression spelling" was HALF right — it is a
  loop-context divergence that IS repairable by expression spelling.

### The fix (22 -> 17)
    tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
Byte-pointer arithmetic removes the MULT from the address tree, so expand
keeps the address as an un-emitted sum, no `(set reg symbol_ref)` insn exists,
loop.c has nothing to hoist, and the load prints target's `%hi/%lo`-indexed
form. `.loop` after the change no longer lists regno 124. 135/135 insns.

Spelling sweep (all 135 insns): array index `D_80094DF0[k]` = 22 (baseline);
`*(D_80094DF0 + k)` = 22; `(s16*)*(s32*)((u8*)D_80094DF0 + (k*4))` = 22;
`*(s16**)((u8*)D_80094DF0 + (k<<2))` = **17**;
`*(s16**)((s32)D_80094DF0 + (k<<2))` = **17**.
So the win needs BOTH the byte-granular base cast AND the explicit `<< 2`;
writing the scale as `* 4` on a `(s32 *)` deref reintroduces the MULT and the
hoist.

### Residual 17 — one family, the same LICM mechanism one level in
`.loop` on the 17-form still shows regnos 136 `(const_int 16)`, 137
`(const_int 1)` and 139 `(symbol_ref D_800A9A24)` hoisted out of the inner
loop (37 insns) and then out of the outer. reload rematerializes all three
(REG_EQUIV), so the instruction COUNT is unaffected (135/135) but:
  - the rematerialized scratch is `$t0` where target uses `$v0`/`$a1`;
  - the LoadImage arg-setup block (`addiu a0,sp,24` + the D_800A9A24 la +
    `addu a1,...`) lands mid-body instead of at the top of the loop body;
  - the y-coordinate temp takes `$v0` (target `$v1`), which is what leaves
    `$v0` unavailable for the two rematerialized constants;
  - riders: `sll a1,s1,5` (target `sll v0,s1,5`), `li v1,1 / bne v0,v1`
    (target `li t0,1 / bne v0,t0`).
Full opcode-normalized diff: tmp/grind/func_80041BF4/s2/diff17.txt
(normalizer tmp/grind/func_80041BF4/s2/norm2.py, disasm ours.txt).

### [s2] KILLED — inner-loop body spelling does not touch the residual
Thirteen forms measured, ALL 17 at 135 insns (i.e. exactly inert), except one
that is worse: name the LoadImage source in a block-local; drop the `u8*` cast
(`(s32)&D_800A9A24 + off`); move `tbl += 2` after the constant stores; fold
`idx++` into the shift; assign the computed source address back into `off`;
commute `rect[0]`'s add; commute `rect[1]`'s add; commute both; name the x
read in a block-local; name the y read in a block-local; swap the
`rect[2]`/`rect[3]` order. WORSE: hoisting `rect[2]/rect[3]` above the two
coordinate stores = 46 at 139 insns (rejected/rect-const-stores-before-coords.c).
CONCLUSION: the residual is NOT an expression-shape problem in the inner body;
it is the inner-loop LICM hoist of the three invariants plus the register
consequences of the remat. Do not re-run inner-body respellings.

- [s2] loop.c decision log lives in the .loop dump ("Insn N: regno R (life L), move-insn savings S  moved to M" / "not desirable") — read it instead of guessing whether LICM fired
- [s2] threshold in loop.c:532/1631 measured to lie in [37,82] on this chassis (a life-1 savings-1 invariant hoists from a 37-insn call-carrying loop but is "not desirable" in an 83-insn one), so no realistic BB2 inner loop is big enough to defeat the hoist by size — the movable has to not be RECORDED
- [s2] a hoisted invariant is usually instruction-count-NEUTRAL because reload rematerializes it from REG_EQUIV; what it costs is the scratch register choice and the schedule position, which is exactly the shape of this function's residual 17
- [s2] scale-1 array indexing folds the symbol into the MEM at expand; scale-N (N>1) does not, because the MULT node forces the address sum out of EXPAND_SUM into a real add insn — byte-pointer arithmetic with an explicit shift restores the scale-1 behaviour

- [s2] Chassis check: the s1 candidate re-measured 22 on this chassis (135/135 insns, canonical verdict C) before any edit - the ledger floor was current, not stale.

- [s2] loop.c writes its own decision log into the .loop dump ('Insn N: regno R (life L), move-insn savings S  moved to M' and 'not desirable'). Reading it names LICM in one grep instead of inferring it from asm shape.

- [s2] On this chassis loop.c's move_movables threshold (loop.c:532, (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)) measures into [37,82]: a life-1 savings-1 invariant is hoisted out of a 37-insn call-carrying loop but is 'not desirable' in an 83-insn one. No realistic BB2 inner loop is large enough to defeat the hoist by size, so the movable has to be prevented from being RECORDED, not made undesirable.

- [s2] A hoisted invariant is normally instruction-count-NEUTRAL, because reload rematerializes it from its REG_EQUIV note at the use. What the hoist actually costs is (a) combine's ability to fold a symbol_ref into a MEM (def and use end up in different basic blocks) and (b) the scratch register / schedule slot the rematerialized copy gets. Both cost shapes are present in this function.

- [s2] Expand folds a symbol into (mem (plus (reg) (symbol_ref))) for a scale-1 array index but not for scale-4: the MULT node forces the address sum out of EXPAND_SUM into a real add insn. Byte-pointer arithmetic with an explicit shift restores the scale-1 behaviour. This is a general BB2 lever for any array-of-word load inside a loop.

- [s2] The matched sibling func_80041AC8 in the same TU emits target's %hi/%lo-indexed form from the plain array-index spelling only because it is not inside a loop - nothing hoists, so combine folds in-block. s1's 'loop-context divergence, not expression spelling' was half right: it is a loop-context divergence that IS repairable by expression spelling.

- [s2] Residual 17 is a single family: .loop on the 17-form still hoists regno 136 (const_int 16 -> rect[2]), 137 (const_int 1 -> rect[3]) and 139 (symbol_ref D_800A9A24 -> the LoadImage source address) out of the inner loop and then out of the outer loop. Count-neutral, but the remat scratch is $t0 where target uses $v0/$a1, the y-coordinate temp takes $v0 where target takes $v1, and the LoadImage arg-setup block loses its top-of-body schedule slot.

- [s2] src/text1a_post.c was REVERTED to HEAD at session end (s1 precedent): the 33 regfix + 2 asmfix rules anchor on the old instruction stream and the do-while renumbers cc1 {lbl#N} slots, so applying the candidate stays a ledger-only operation until the function closes and the rules retire in the same change. The 17-form lives in memory/grind/func_80041BF4/candidate.c.

---

## [s3] 2026-08-19 — structural modality — floor 17 -> 13

### Chassis
HEAD floor re-measured at dispatch with the s2 `candidate.c` applied to
`src/text1a_post.c`: **17 at 135/135 insns, frame 88** — identical to the
ledger's recorded floor, so every s1/s2 spelling conclusion is still valid on
this chassis.

### Tooling built this session (reusable)
- `tmp/grind/func_80041BF4/s3/sbs.py` — normalized side-by-side divergence
  report built on `engine.score.normalized_insns` against
  `build/src/text1a_post.o`. This is the right tool for this function: a raw
  `objdump` diff is drowned in pseudo-op/format noise (`move` vs `addu ...,zero`,
  decimal vs hex, `fp` vs `s8`), and `engine.score` already normalizes exactly
  what the scorer normalizes. Prints only the differing insns plus their index.
- `tmp/grind/func_80041BF4/s3/sweep.py` + `setbase.py` — splice a variant body
  into `src/text1a_post.c`, run `engine.cli sandbox --disable all`, harvest
  `score`/`build_insns`, restore the base. Sweeps every `v_*.c` in the scratch
  dir in one invocation, so a 14-form round costs ONE turn.

### The 17 -> 13 win: `tbl += 2` is a scheduling barrier where it stood
At floor 17 the divergence was six groups; four of them were pure ORDERING —
target issues the LoadImage argument-setup block (`addiu a0,sp,24`, the
`lui/addiu` of `D_800A9A24`, `addu a1`) as the FIRST thing in the loop body,
right after `addiu s1,s1,1`, while ours issued it in the middle, after the
rect[0] store and the y-coordinate load.

`.greg` on the floor-17 form shows why: the pointer bump

    (insn 210 (set (reg/v:SI 16 s0) (plus (reg/v:SI 16 s0) (const_int 4)))
              (insn_list:REG_DEP_ANTI 201 (nil)))

carries an anti-dependence against insn 201, the y-coordinate load
`(set (reg:HI 133) (mem (plus (reg/v 83) (const_int 2))))`, and it sat directly
between the coordinate work and the argument-setup chain (insns 227 / 343 / 229).
sched1 could not lift the arg-setup above it. Moving `tbl += 2;` out of the
middle of the body — to immediately after `DrawSync(0)` — takes the
anti-dependence out from between them.

Result: **17 -> 13 at 135/135 insns, and the ordering divergence goes to ZERO.**
The side-by-side now reports only register renames; target insns 82..118 and
ours occupy identical positions. Moving the bump further along the tail (before
`DrawSync`, or after the `func_80048A7C` call) is equally good at 13; leaving it
mid-body is 17. This is the same class of finding as s2's LICM result — a
statement whose PLACEMENT, not whose content, was the whole divergence.

### The residual 13, named to a single line of local-alloc.c
Every one of the thirteen differing insns is downstream of ONE allocation
decision. From `.lreg` on the floor-13 form:

    Register 129 used 6 times across 4 insns in block 10; GR_REGS or none.
    ;; Register 129 in 5.                                  <- $a1
    (insn 229 (set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139))))

`off` is pseudo 129, block-local to the loop body, and it dies in an insn whose
DEST IS THE HARD ARGUMENT REGISTER `$a1`. local-alloc.c:1240-1300 walks that
insn's source operands calling `combine_regs (operand, recog_operand[0])`;
`combine_regs` at local-alloc.c:1884-1896 sees a hard `setreg` and
*unconditionally* records `a1` in `qty_phys_sugg[reg_qty[129]]`, then returns 0.
Two consequences worth writing down, because both were probed and both matter:
  - returning 0 means the operand loop does NOT break, so the suggestion is
    recorded no matter which operand position `off` occupies. Commuting the
    address add was measured and is exactly inert (13), as predicted.
  - `find_free_reg`'s `just_try_suggested` pass (local-alloc.c:2208-2215) then
    restricts the candidate set to the suggestion, so `off` gets `$a1`.

With `$a1` taken, reload — which must rematerialize the three loop-hoisted
invariants that global.c left unallocated (regno 136 = `(const_int 16)`,
137 = `(const_int 1)`, 139 = `(symbol_ref D_800A9A24)`; the `.loop` movable log
is unchanged from s2 and still hoists all three out of the inner loop and then
out of the outer loop) — has nothing better than `$t0` for all three. Target
instead has `off` in `$v0`, i.e. it had NO suggestion and `find_free_reg` simply
took the first register in `REG_ALLOC_ORDER`.

The y-temp rename (`lhu v0,2(s0)` vs target `lhu v1,2(s0)`, and the matching
`addu`/`sh` pair) is a knock-on of the same theft, and the trailing
`li v1,1 / bne v0,v1` vs target `li t0,1 / bne v0,t0` is its mirror image
outside both loops: ours ALLOCATES the constant where target rematerializes it.

### Structural axes measured DEAD this session
- **Declaration order — completely inert.** Twelve permutations of the twelve
  locals on the floor-17 base (sent first, rect first, tbl/idx before the
  offsets, idx before tbl, sent before tbl, rect after outer, yoff before xoff,
  fp_ptr last, new_var last, new_var deleted, `off` at function scope in two
  positions) all scored exactly 17/135; three more on the floor-13 base all
  scored exactly 13/135. This axis is closed for this function.
- **Inner-body expression shape — closed.** Twenty further forms on the
  floor-13 base (listed in hypotheses.md) produce fourteen exact ties and six
  regressions; nothing improves. Combined with s2's thirteen forms that is
  33 measured inner-body spellings with a single winner (`tbl += 2` placement),
  and that winner was a STATEMENT MOVE, not a respelling.

### What the next session should do
The whole residual is one register: deny `$a1` to `off`. `find_free_reg`
excludes any hard reg that appears in `regs_live_at[ins]` for
`ins` in `[born_index, dead_index)` (local-alloc.c:2170), and the suggestion
path is skipped entirely for a pseudo with `reg_qty < 0`
(local-alloc.c:1826) — i.e. one that is NOT block-local, which is handed to
global.c and allocated from `REG_ALLOC_ORDER` (`$v0` first, which is exactly
target). So the two live levers are (a) make `$a1` live somewhere inside
`off`'s range, or (b) move `off`'s birth across the basic-block boundary at the
top of the loop body. Both are outside the body, which is precisely the region
s3 could not reach. The instrumented cc1 carries a `BB2_SUGG_DEBUG` env hook in
`find_free_reg` that prints the `used` and `first_used` hard-reg sets per call —
use it to confirm a candidate form in one build rather than inferring from the
score.

- [s3] Chassis re-measured at dispatch: the s2 candidate applied to src/text1a_post.c scores exactly 17 at 135/135 insns, frame 88 - identical to the ledger's recorded floor, so all s1/s2 spelling conclusions remain valid on this chassis.

- [s3] New floor is 13 at 135/135 insns, carried by ONE structural change on top of the s2 form: `tbl += 2;` moved from the middle of the inner-loop body to immediately after `DrawSync(0)`.

- [s3] After that move the inner-loop SCHEDULE is byte-for-byte target's: the normalized side-by-side reports zero ordering divergence, and all thirteen remaining differences are register renames at identical instruction positions.

- [s3] The thirteen residual insns are: `lui/addiu` of D_800A9A24 into $t0 instead of $a1 and the consequent `addu a1,a1,t0` instead of target's `addu a1,v0,a1`; `li t0,16 / sh t0,28(sp) / li t0,1 / sh t0,30(sp)` instead of the same four with $v0; the y-coordinate temp in $v0 instead of $v1 (`lhu`, `addu`, `sh`); `sll a1,s1,5` instead of `sll v0,s1,5` in the loop-bottom delay slot; and `li v1,1 / bne v0,v1` instead of `li t0,1 / bne v0,t0` after both loops.

- [s3] Named to the source line: `off` is pseudo 129, block-local, and .lreg says `;; Register 129 in 5.` ($a1). It is given that hard reg by combine_regs' qty_phys_sugg path (local-alloc.c:1884-1896) because it dies in an insn whose DEST is the hard argument register $a1. combine_regs returns 0 there, so the operand loop does not break and the suggestion is recorded regardless of operand position - which is why commuting the address add is measured exactly inert.

- [s3] Because combine_regs is skipped entirely for a pseudo with reg_qty < 0 (local-alloc.c:1826) - i.e. one that is NOT block-local - a form in which `off` is born outside the loop body's basic block would be handed to global.c and allocated from REG_ALLOC_ORDER, whose first GR entry is $v0: exactly target's assignment.

- [s3] find_free_reg also excludes any hard reg present in regs_live_at[ins] for ins in [born_index, dead_index) (local-alloc.c:2170), so making $a1 live somewhere inside `off`'s range is the second independent way to deny the suggestion.

- [s3] loop.c's hoist of regnos 136/137/139 out of the 37-insn inner loop and then out of the 59-insn outer loop is UNCHANGED from s2 and is not itself the problem: it is count-neutral (135/135) because reload rematerializes all three via REG_EQUIV. The gate is `threshold * savings * lifetime >= insn_count` (loop.c:1631) with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) (loop.c:532), which is ~61 against an insn_count of 37 - unreachable from C without changing the loop's instruction count. The frontier item is the REGISTER, not the hoist.

- [s3] loop.c's substitute-and-delete escape (loop.c:735-767) cannot fire for any of the three: it requires validate_replace_rtx to succeed at the single use, and neither `(plus reg (symbol_ref))` as an addsi3 operand nor `(set (mem:HI) (const_int))` as a MIPS halfword store is a valid insn.

- [s3] Declaration order is COMPLETELY DEAD for this function: fifteen permutations across two chassis, every one byte-identical to its base. Do not spend a session on it.

- [s3] Inner-body expression shape is closed: 33 measured spellings across s2 and s3 with exactly one winner, and that winner was a statement MOVE rather than a respelling.

- [s3] Reusable tooling banked in tmp/grind/func_80041BF4/s3/: sbs.py (normalized side-by-side built on engine.score.normalized_insns - a raw objdump diff is unusable here, drowned in pseudo-op and format noise) and sweep.py + setbase.py, which measure every v_*.c variant in the scratch dir in one invocation, so a 14-form round costs one turn.


## [s4] permuter modality � floor 13 -> 11

Workspace: `tmp/grind/func_80041BF4/s4/ws` (hand-built, full-TU compile with a
per-function extraction so the permuter metric is the real per-function diff).
It replicates the production pipeline for this file exactly � `-G8` (text1a_post
is in GP_FILES), `-mel`, prologue_fix, maspsx with MASPSX_FLAGS_GP, the
`.align 3 -> .align 2` rodata sed, multu_pad � and its base build reproduces the
known s3 residual insn-for-insn, so finds are chassis-faithful.
Campaign: 21428 iterations / 714 s / 6 jobs, base weighted score 80, TWO novel
finds (output-75-1 at 75 after 6.7 s, output-70-1 at 70 after 31.8 s), harvested
and stopped in-session.

FLOOR: 13 -> 11 at 135/135 insns, from output-70-1 re-measured in the real
chassis: an opaque constant holder for the trailing `func_8003E2A0() == 1` test.
It converts our `li v1,1 / bne v0,v1` into target's `li t0,1 / bne v0,t0` and
changes nothing else. It is a `/* FAKE */`-annotated constant-holder-family
construct and is NOT yet vetted by a cheat-reviewer or the Judge.

Controls measured EXACTLY INERT at 13 (so the holder is currently the only known
form, and the item is not reachable by ordinary C): the call result named in a
block-local `rc`; named in a function-scope `rc`; the Yoda spelling
`1 == func_8003E2A0()`. All three banked in rejected/.

The other novel find (output-75-1, the function-scope `u8 *` symbol base) is
EXACTLY INERT at 13 in the real chassis � its permuter-metric gain was an
artifact of the permuter's own weighted scorer.

TWO HYPOTHESES KILLED WITH DIRECT COMPILER WITNESSES (details in hypotheses.md):

1. s3's frontier item 1 � "deny $a1 to `off` by making it non-block-local so
   local-alloc's combine_regs suggestion path bails" � is FALSIFIED. Two
   cross-block forms were built and checked against the instrumented cc1's
   `BB2_SUGG_DEBUG` per-qty table: in both, block 10 loses its `sugg=5,` entry
   entirely (baseline: `blk=10 qty=0 reg1=129 birth=6 death=12 refs=6 nsugg=1
   sugg=5,`). Both are WORSE (19 and 17), and `off` is STILL in $a1 in the
   objdump. global.c re-derives the same preference independently via
   record_one_conflict -> hard_reg_preferences (global.c:1728/1747), consumed by
   find_reg at global.c:1133-1140. The lever is therefore a LIVE-RANGE CONFLICT
   on $a1, not a preference.

2. s3's frontier item 2 � "the trailing `li v1,1` is the mirror image of item 1;
   one allocno-ordering shift would flip both; do not tune it independently" �
   is FALSIFIED. It closed on its own and bought nothing for item 1.

Artifacts: tmp/grind/func_80041BF4/s4/ (mkws.sh, measure.ps1, sugg.sh, cmp.sh,
sugg_base.txt, sugg_vA.txt, sugg_vB.txt, ws/campaign.log, ws/output-*).

- [s4] FLOOR 13 -> 11 at 135/135 insns. The improvement is one construct: a `/* FAKE */`-annotated `int one = 1;` constant holder consumed by the trailing `func_8003E2A0() == one` test. It is NOT vetted — no cheat-reviewer, no Judge has seen it — and the outcome is deliberately `progress`, not `candidate-ready`.

- [s4] A chassis-faithful permuter workspace for this function now exists at tmp/grind/func_80041BF4/s4/ws (built by tmp/grind/func_80041BF4/s4/mkws.sh): full-TU compile with per-function extraction, -G8 (text1a_post is in GP_FILES), -mel, prologue_fix, MASPSX_FLAGS_GP, the .align 3 -> .align 2 rodata sed, multu_pad. Its base build reproduces the known s3 residual insn-for-insn. Reuse it rather than rebuilding.

- [s4] Campaign telemetry: 21428 iterations / 714 s / 6 jobs, base weighted score 80, two novel finds (75 at 6.7 s, 70 at 31.8 s), harvested and stopped in-session. Consistent with the fresh-seed rule: this basin yielded within the first 30 seconds and produced nothing further in the remaining ~11 minutes.

- [s4] The instrumented compiler is tools/gcc-2.7.2/cc1 (NOT tools/gcc-2.7.2/build/cc1, which the Makefile uses and which has no instrumentation). BB2_SUGG_DEBUG=1 prints a `SUGGDBG-QTY` row per qty per block at block-alloc time and a `SUGGDBG-FFR` row per find_free_reg call; BB2_QTY_DEBUG=1 adds `QTYDBG-SUGG` rows showing which reg each suggested qty actually got. This is the fastest way to settle an allocation question in ONE build.

- [s4] Baseline block-10 suggestion table (floor-13 form): `blk=10 qty=0 reg1=129 birth=6 death=12 refs=6 nsugg=1 sugg=5,` — pseudo 129 (`off`) is the ONLY qty in the entire function carrying a plain arithmetic suggestion, and it suggests $a1.

- [s4] `off` is routed to $a1 by TWO independent mechanisms, not one: local-alloc's combine_regs -> qty_phys_sugg (local-alloc.c:1857-1896) and global.c's record_one_conflict -> hard_reg_preferences (global.c:1728/1747, consumed at global.c:1133-1140). Both fire from the same fact — `off` is a direct register source of a set whose destination is hard reg $a1 — so a C form that only changes WHICH allocator handles `off` changes nothing.

- [s4] The residual 11 is now entirely the `off`-in-$a1 allocation and its downstream renames: `lui/addiu t0 + addu a1,a1,t0` vs target `lui/addiu a1 + addu a1,v0,a1`; `lhu v0` vs `lhu v1` for the y temp; `li t0,16 / li t0,1` vs `li v0,16 / li v0,1`; `sll a1,s1,5` vs `sll v0,s1,5`. Zero ordering divergence remains (s3's schedule result holds).

- [s4] Target's own shape puts the SYMBOL value in $a1 and `off` in $v0. Since the symbol pseudo (139) is currently hoisted out of both loops by loop.c and left unallocated by global.c, it never competes for $a1 at all — making it block-local to the loop body is the one untried route by which it could take $a1 first and push `off` down REG_ALLOC_ORDER to $v0.

## [s5] permuter modality — floor UNCHANGED at 11 (135/135 insns), three hypotheses killed

- [s5] CHASSIS RE-MEASURED at dispatch: applying memory/grind/func_80041BF4/candidate.c to src/text1a_post.c gives `sandbox --disable all` = 11 at 135/135 insns, confirming the s4 ledger floor on today's HEAD. Nothing in the chassis moved between s4 and s5.

- [s5] KILLED — naming the D_800A9A24 base in a BLOCK-LOCAL pointer inside the loop body is 5 instructions WORSE (47 at 140 insns), in BOTH declaration orders relative to `off` (v1: pointer after `off`; v4: pointer before `off` — identical results). The normalized objdump names the mechanism: the named pointer is loop-invariant, loop.c hoists it to a whole-function pseudo needing a callee-saved register, and the added pressure evicts `fp_ptr` from $s8 to the stack — frame 96 vs target's 88, `sw v0,32(sp)` / `lh v1,8(v0)` where target has `move s8,v0` / `lh v1,8(s8)`. This closes s4's frontier item 1 as stated: you cannot make pseudo 139 a short-lived in-loop competitor for $a1 by NAMING it, because any C name for a loop-invariant address lengthens its live range instead of shortening it.

- [s5] KILLED — interposing a named address intermediate between `off` and the hard-$a1 LoadImage argument is EXACTLY INERT at 11/135 in all three spellings tried: `s32 addr = (s32)&D_800A9A24 + off` (v5), `u8 *addr = (u8 *)&D_800A9A24 + off` (v6), and the temp-free integer form `(s32)&D_800A9A24 + off` (v2). The copy is coalesced before allocation, so `off` is still a direct register source of the set into $a1 in the RTL both allocators see, and the s4-measured preference survives untouched.

- [s5] KILLED — random permuter sampling is SPENT for this function across three structurally distinct chassis. s5 ran two fresh chassis-faithful campaigns via tools/permuter_campaign.py (telemetry recorded): campaign A `s5-intaddr` on the integer-arithmetic address chassis (6 jobs, 20303 iterations / 664 s, base weighted 70) and campaign B `s5-ptrtemp` on the pointer-temp chassis (8 jobs, 18966 iterations / 563 s, base weighted 70). Each returned exactly ONE find, both inside the first 10 seconds (7.4 s and 9.9 s), then nothing across the remaining ~9 minutes — the fresh-seed exhaustion signature. Both were harvested and STOPPED in-session. Re-measured in the real chassis, both finds are EXACTLY INERT at 11/135: A's is the function-scope `s32 sym_base = (s32)&D_800A9A24;` (the s32 twin of the `u8 *` form s4 already killed), B's is the artifact spelling `((u8 *)(&D_800A9A24)) - (-off)`. Combined with s4's 21428 iterations this is 60k+ iterations over three chassis with zero byte-moving finds beyond the s4 constant holder.

- [s5] Reusable infrastructure: tmp/grind/func_80041BF4/s5/mkwsA.sh and mkwsB.sh build chassis-faithful permuter workspaces from whatever is currently in src/text1a_post.c (derived from the s4 recipe); tmp/grind/func_80041BF4/s5/apply.py splices a candidate .c body into src/text1a_post.c (LF-safe, drops the candidate's header comment so no prose lands in src); tmp/grind/func_80041BF4/s5/cmp.sh produces the normalized objdump diff against target.o. All three are the fastest way to re-run this function's measurement loop.

- [s5] Seven forms banked in rejected/ this session: symbol-base-block-local-in-loop-body-plus5-insns.c, symbol-base-block-local-declared-before-off-plus5-insns.c, loadimage-address-integer-arithmetic-inert.c, loadimage-address-named-s32-temp-inert.c, loadimage-address-named-pointer-temp-inert.c, symbol-base-function-scope-s32-inert.c, loadimage-address-minus-negated-off-inert.c.

- [s5] CHASSIS RE-MEASURED at dispatch: applying memory/grind/func_80041BF4/candidate.c to src/text1a_post.c gives sandbox --disable all = 11 at 135/135 insns on today's HEAD, confirming the s4 ledger floor. Nothing in the chassis moved between s4 and s5.

- [s5] Naming the D_800A9A24 base in a block-local pointer inside the loop body costs exactly 5 instructions (47 at 140 insns) in BOTH declaration orders relative to `off`, because the named pointer is loop-invariant: loop.c hoists it to a whole-function pseudo needing a callee-saved register and the pressure evicts fp_ptr from $s8 to the stack (frame 96 vs target's 88).

- [s5] Interposing a named address intermediate between `off` and the $a1 argument is exactly inert at 11/135 in all three spellings tried (s32 temp, u8* temp, temp-free integer arithmetic) — the copy is coalesced before allocation and the $a1 preference relation on `off` survives.

- [s5] Permuter telemetry s5: campaign A 20303 iterations / 664 s / 6 jobs, one find at 7.4 s; campaign B 18966 iterations / 563 s / 8 jobs, one find at 9.9 s. Both stopped in-session. Combined with s4's 21428 iterations that is 60k+ iterations across THREE structurally distinct chassis with zero byte-moving finds beyond the s4 constant holder.

- [s5] Reusable infra now exists at tmp/grind/func_80041BF4/s5/: mkwsA.sh / mkwsB.sh build chassis-faithful permuter workspaces from whatever is in src/text1a_post.c; apply.py splices a candidate .c body into src (LF-safe, strips the candidate header comment so no prose lands in src); cmp.sh emits the normalized objdump diff against target.o.

- [s5] The residual 11 is unchanged in kind from s4: `off` (pseudo 129) in $a1 plus four downstream renames, with zero ordering divergence. The banked floor-11 constant holder for the trailing `== 1` test remains UNVETTED and was NOT spent this session.

## [s6] forensics modality — floor UNCHANGED at 11 (135/135), three hypotheses killed, the frontier item numerically sharpened

- [s6] CHASSIS RE-MEASURED at dispatch AND at the end of the session: applying
  memory/grind/func_80041BF4/candidate.c to src/text1a_post.c gives
  `sandbox --disable all` = 11 at 135/135 insns, frame 88. The dispatch brief
  reported "measurement unavailable"; the real number is 11, identical to the s4/s5
  ledger floor. Nothing in the chassis moved.

- [s6] PASS ATTRIBUTION CORRECTED — the pass that creates the divergence-producing
  fact is **combine.c**, not local-alloc/global.c (which are only its consumers).
  Read from the -da dumps in tmp/grind/func_80041BF4/dumps/:
  `.flow` still has TWO insns — `(insn 225 (set (reg:SI 139) (plus (reg/v:SI 130)
  (reg:SI 140))))` and `(insn 229 (set (reg:SI 5 a1) (reg:SI 139)))`;
  `.combine` has ONE — `(insn 229 (set (reg:SI 5 a1) (plus (reg/v:SI 130)
  (reg:SI 140))))`, insn 225 deleted. So combine.c propagates the address sum
  directly into the hard-register argument set, and THAT is what makes `off`
  (pseudo 130) a direct register source of a set whose destination is $a1 —
  the single fact both allocators consume. Every prior session attributed this to
  local-alloc.c/global.c.

- [s6] Pseudo map on the floor-11 chassis (differs from the s3/s4 numbering — always
  re-read it, never quote the old numbers): 130 = `off`, 140 = the D_800A9A24
  symbol_ref pseudo (hoisted out of BOTH loops by loop.c as insn 327, left
  unallocated, rematerialized by reload via its REG_EQUIV), 139 = the pre-combine
  address sum (deleted by combine), 84 = `idx`, 142/145 = the func_80048A7C
  arg temps.

- [s6] Instrumented-cc1 measurement on the floor-11 chassis (tools/gcc-2.7.2/cc1,
  BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1; raw in tmp/grind/func_80041BF4/s6/sugg_base.txt):
    SUGGDBG-QTY blk=10 qty=0 reg1=130 birth=6 death=12 refs=6 nsugg=1 sugg=5, ncopysugg=0
    SUGGDBG-FFR qty=0 class=1 jts=1 born=6 dead=12 used=0,1,4,26..67
    QTYDBG-SUGG blk=10 ord=5 qty=0 reg1=130 got=5
  `used` for `off` does NOT contain hard reg 5, so the $a1 suggestion is honored on
  the just_try_suggested pass. The only other suggested qtys in block 10 are
  reg 142 (copysugg $a0, birth 36) and reg 145 (copysugg $a1, birth 38) — the
  func_80048A7C argument temps — and their ranges are disjoint from `off`'s, which
  is why they do not block it.

- [s6] KILLED — the "make $a1 LIVE inside `off`'s range" escape (s3 frontier item 1,
  restated by s4 as "the remaining escape is a CONFLICT, not a preference").
  find_free_reg's conflict scan is `for (ins = born_index; ins < dead_index; ins++)`
  (local-alloc.c:2170) and `off`'s measured range is [6,12) = exactly three RTL
  insns: 189 (`sll` computing off), 192 (`idx++`), 227 (`a0 = fp+24`). The insn that
  sets $a1 IS the death insn (index 12) and the scan is half-open, so it is excluded
  by construction. Closing the escape therefore requires either (a) inserting a real
  $a1-touching instruction between the shift and the LoadImage argument setup — target
  has no such instruction, so any such form is >= +1 insn — or (b) extending `off`'s
  death past insn 229, which necessarily puts it across the LoadImage call, flipping
  `used` from fixed_reg_set to call_used_reg_set (local-alloc.c:2166-2168) and
  excluding $v0 as well as $a1. Both ends are closed; this axis is dead.

- [s6] KILLED — the global.c operand-order escape, and with it the mechanism behind
  s4's "global.c re-derives the same preference independently". global.c's
  set_preference (global.c:1680) begins
  `if (GET_RTX_FORMAT (GET_CODE (src))[0] == 'e') src = XEXP (src, 0), copy = 0;`
  — for a PLUS it looks at the FIRST operand ONLY. So in any cross-block regime the
  hard-reg preference attaches to whichever pseudo is operand 0 of the plus, and
  putting the symbol pseudo there would move the preference off `off`. MEASURED: the
  RTL operand order is NOT C-controllable. Writing `off + (u8 *)&D_800A9A24` instead
  of `(u8 *)&D_800A9A24 + off` yields a byte-identical .lreg — still
  `(plus:SI (reg/v:SI 130) (reg:SI 140))` and still `;; Register 130 in 5.` The
  symbol is force_reg'd by expand into its own pseudo in a separate preceding insn
  (visible as insn 223 in the .cse dump, before loop.c hoists it), so `off` is always
  operand 0 of the sum. Banked as
  rejected/loadimage-address-commuted-c-order-does-not-reach-rtl.c.

- [s6] Local-alloc is operand-order-IMMUNE for the same question, independently:
  block_alloc's tying loop (local-alloc.c:1240-1298) scans i = 1..n_operands and
  combine_regs RETURNS 0 on the hard-reg-suggestion path (local-alloc.c:1880-1882),
  so `win` never breaks the loop and every register operand of the plus is offered a
  suggestion. Pseudo 140 escapes only because it is hoisted and therefore has
  reg_qty < 0, which combine_regs rejects at local-alloc.c:1826.

- [s6] MECHANISM for why every s5 named-intermediate form measured exactly inert:
  combine_regs ties two pseudos only when
  `(already_dead || find_regno_note (insn, REG_DEAD, ureg))` (local-alloc.c:1917).
  `off` always carries a REG_DEAD note at the address sum, so an interposed
  intermediate is tied into `off`'s quantity, and a quantity's suggestions are shared
  by every register in it. Naming the sum can therefore never separate `off` from the
  $a1 suggestion.

- [s6] FRONTIER ITEM SHARPENED WITH NUMBERS — "make pseudo 140 block-local so it
  competes for $a1" is now known to WIN if it can be achieved. block_alloc orders
  quantities with qty_sugg_compare (local-alloc.c): fewer suggestions first
  (`sugg = ncopysugg ? ncopysugg : nsugg * FIRST_PSEUDO_REGISTER`), then
  `pri = floor_log2(n_refs) * n_refs * size / (death - birth) * 10000`, higher first.
  A block-local 140 would carry nsugg=1 (tie with `off` on the first key) and its
  refs would be weighted the same way `off`'s are (measured refs=6 for two textual
  references, i.e. flow.c's loop-depth weighting at depth 3), giving
  pri = floor_log2(6)*6/2 * 10000 = 60000 against `off`'s
  floor_log2(6)*6/6 * 10000 = 20000. 140 would be allocated FIRST, take $a1, and
  `off` would then fail the just_try_suggested pass and fall to the plain
  REG_ALLOC_ORDER pass whose first free register (used = {0,1,4,5,26..67}) is $v0 —
  which is target's assignment exactly, and it also reproduces target's
  `lui $a1 / addiu $a1 / addu $a1,$v0,$a1` operand shape. The ONLY thing standing in
  the way is loop.c's hoist of insn 327; everything downstream of un-hoisting is now
  measured or derived rather than hoped for.

- [s6] Artifacts: tmp/grind/func_80041BF4/s6/ (ex.sh dump-region extractor, sugg.sh
  instrumented-cc1 runner, rtlord.sh per-variant .lreg operand-order dumper,
  sugg_base.txt, loop.txt, lreg.txt, combine.txt, flow.txt, cse.txt,
  lreg_commute.txt, v_commute.c) plus the full -da set in
  tmp/grind/func_80041BF4/dumps/.

- [s6] Chassis re-measured at dispatch AND again at the end of the session: memory/grind/func_80041BF4/candidate.c applied to src/text1a_post.c scores `sandbox --disable all` = 11 at 135/135 insns, frame 88. The dispatch brief reported 'measurement unavailable'; the real HEAD floor is 11, identical to the s4/s5 ledger.

- [s6] Pass attribution for this function is now READ rather than guessed: combine.c merges the address-sum copy into the hard-$a1 argument set (.flow has two insns, .combine has one). local-alloc.c and global.c consume that fact; they do not create it.

- [s6] Pseudo map on the floor-11 chassis (DIFFERENT from the s3/s4 numbering — always re-read it): 130 = `off`, 140 = the D_800A9A24 symbol_ref pseudo (hoisted out of both loops by loop.c as insn 327, left unallocated, rematerialized by reload via REG_EQUIV), 139 = the pre-combine address sum (deleted by combine), 84 = `idx`, 142/145 = the func_80048A7C argument temps.

- [s6] Measured block-10 suggestion table on the floor-11 chassis: qty0 reg1=130 birth=6 death=12 refs=6 nsugg=1 sugg={5} ncopysugg=0, allocated at ord=5, got=5. The only other suggested qtys in that block are reg 142 (copysugg $a0, birth 36) and reg 145 (copysugg $a1, birth 38) — the func_80048A7C argument temps — and their ranges are disjoint from `off`'s, which is why they do not block it.

- [s6] find_free_reg's conflict set for `off` came back used={0,1,4,26..67} — hard reg 5 is absent, so the $a1 suggestion is honored on the first (just_try_suggested) pass.

- [s6] `off`'s live range is exactly three RTL insns — 189 (`sll` computing off), 192 (`idx++`), 227 (`a0 = fp+24`) — and find_free_reg's scan `for (ins = born_index; ins < dead_index; ins++)` is half-open, so the $a1-setting insn is excluded by construction. The conflict route is therefore closed at both ends.

- [s6] The RTL operand order of the address sum is NOT C-controllable: `off + (u8 *)&D_800A9A24` and `(u8 *)&D_800A9A24 + off` produce byte-identical .lreg — `(plus:SI (reg/v:SI 130) (reg:SI 140))` and `;; Register 130 in 5.` The symbol is force_reg'd by expand into its own pseudo in a separate preceding insn (insn 223 in .cse), so `off` is always operand 0.

- [s6] Both allocators are operand-order-immune for this question: local-alloc's block_alloc scans every register operand (combine_regs returns 0 on the hard-reg-suggestion path at local-alloc.c:1880-1882, so `win` never breaks the loop), and global.c's order is fixed by the expand-time RTL shape just measured. Pseudo 140 escapes the suggestion only because it is hoisted and therefore has reg_qty < 0, rejected at local-alloc.c:1826.

- [s6] MECHANISM for s5's inert named-intermediate results: combine_regs ties two pseudos only when `(already_dead || find_regno_note (insn, REG_DEAD, ureg))` (local-alloc.c:1917). `off` always carries a REG_DEAD note at the address sum, so any interposed intermediate is tied into `off`'s quantity, and a quantity's suggestions are shared by every register in it. Naming the sum can never separate `off` from the $a1 suggestion.

- [s6] The surviving frontier item is now numerically PROVEN sufficient rather than hoped for: a block-local symbol pseudo beats `off` on qty_sugg_compare's priority key (60000 vs the measured 20000) after tying on the suggestion-count key, takes $a1, and pushes `off` to $v0 — target's exact allocation.

- [s6] loop.c's move_movables per-movable log is NOT in the -da dump set that tools/grinder/dump.ps1 produces (loop_dump_stream is the separate -dL stream). It is the single highest-value unread artifact left for this function.

- [s6] One form banked this session: rejected/loadimage-address-commuted-c-order-does-not-reach-rtl.c (the C operand order of the address sum does not reach the RTL).

## [s7] forensics modality — floor UNCHANGED at 11 (135/135, frame 88); the loop.c hoist frontier item is KILLED with numbers, and s6's sufficiency derivation is now MEASURED

- [s7] CHASSIS RE-MEASURED at dispatch AND at the end of the session: candidate.c
  applied to src/text1a_post.c gives `sandbox --disable all` = 11 at 135/135 insns,
  frame 88. The dispatch brief again said "measurement unavailable"; the real HEAD
  floor is 11. Nothing in the chassis moved between s5, s6 and s7.

- [s7] CORRECTION to an s6 ledger claim: loop.c's move_movables per-movable log IS
  in the -da dump set. It is in `tmp/grind/func_80041BF4/dumps/text1a_post.loop`
  (11 `savings` lines across the TU; func_80041BF4 occupies lines 2246-2963). s6
  recorded it as "the single highest-value unread artifact"; it was already on disk.
  Read it with `grep -n "real insns|savings" <dump>.loop`.

- [s7] The full movable log for func_80041BF4 on the floor-11 chassis:
    Loop from 174 to 271: 37 real insns.        <- INNER loop
      Insn 213: regno 137 (life 1), move-insn savings 1  moved to 317
      Insn 218: regno 138 (life 1), move-insn savings 1  moved to 319
      Insn 223: regno 140 (life 1), move-insn savings 1  moved to 321   <- the symbol
    Loop from 129 to 287: 59 real insns.        <- OUTER loop
      Insn 321: regno 140 (life 43), ... halved since already moved  moved to 327
  So the symbol pseudo is the THIRD movable moved out of the inner loop, and the
  outer-loop move is forced by already_moved[] (set at loop.c:1909).

- [s7] THE GATE, read from source: loop.c:1631
  `if (already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count || ...)`
  with `m->savings = n_times_used[regno]` (loop.c:793), `m->lifetime = luid(last use)
  - luid(first use)`, `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`
  (loop.c:532), and `threshold -= 3` after EVERY moved movable (loop.c:1719/1904).
  For the symbol movable savings = 1 and lifetime = 1 — both already at their MINIMUM,
  and both make hoisting MORE likely if increased. already_moved and m->forces only
  ever FORCE a move. **insn_count is the only field in the gate whose sign can help.**

- [s7] THRESHOLD MEASURED, not assumed: a synthetic TU (tmp/grind/func_80041BF4/s7/
  gen_thr.py + gen_thr2.py + thr.sh/thr2.sh) with one call-containing loop and a single
  `(set (reg) (symbol_ref))` movable was bisected on loop size. 61 real insns -> "moved";
  63 real insns -> "not desirable". So **threshold = 61 or 62** for a call-containing
  loop under this cc1 configuration (i.e. n_non_fixed_regs = 60 or 61). An independent
  in-compile datapoint agrees: func_80041EB0's 83-insn loop declines its FIRST movable.

- [s7] THE PRICE OF SUPPRESSING THE HOIST in the real chassis: the symbol movable is
  tested with threshold already decremented twice (137, 138 moved first), i.e. 55 or 56,
  against insn_count = 37. To make the gate fail the INNER loop must reach >= 56 real RTL
  insns — **+19 insns on a 37-insn loop, inside a 135-insn function** — or 7 further
  movables must be moved ahead of it (3 threshold points each). Measured in situ by
  padding the inner loop body (tmp/grind/func_80041BF4/s7/pad.py): pad=8 -> 46 insns,
  still moved; pad=16 -> 54 insns, **"not desirable"**; pad=24 -> 62 insns, not desirable.

- [s7] MEASURED (not derived): s6's frontier claim is CORRECT. At pad=16 the symbol
  pseudo (156) is left in the inner loop and takes **hard reg 5 = $a1** (`;; Register 156
  in 5.`), displacing `off` (130) off $a1. With BOTH loops goto-spelled so loop.c sees no
  loop in this function at all, the .lreg reads `;; Register 137 in 5.` (symbol -> $a1)
  and `;; Register 127 in 2.` (`off` -> **$v0**) — **target's exact allocation**, and the
  address insn is `(set (reg:SI 5 a1) (plus (reg/v:SI 127) (reg:SI 137)))`. The
  qty_sugg_compare priority story s6 derived is therefore confirmed end-to-end.

- [s7] KILLED — the goto-spelled loop route, the ONLY C-level way found to suppress the
  hoist. Two forms measured:
    * inner loop goto-spelled, outer left as the real do-while: **46 at 136 insns**.
      loop.c then sees ONE loop (the outer, 54 real insns) and STILL hoists the symbol —
      54 < the effective threshold 55/56, missing by TWO insns.
    * both loops goto-spelled: **43 at 136 insns**, allocation exactly target's, but the
      frame collapses to **80 vs target's 88** and the callee-save rotation is lost
      (s4/s5/s6/s7 permuted vs target), plus a `nop` where target has `lhu v1,0(s0)`.
  Banked as rejected/goto-spelled-inner-loop-plus1-insn-still-hoisted.c and
  rejected/goto-spelled-both-loops-target-alloc-but-frame-80.c.

- [s7] MECHANISM MEASURED — loop-invariant work written INSIDE a loop is
  instruction-count-neutral and is the one thing that raises insn_count for free:
  tmp/grind/func_80041BF4/s7/licm_out.c vs licm_in.c (identical but for where
  `t = (a0 << 12) / 255;` is written) compile to the same instructions in a different
  order/allocation, while the in-loop spelling raises insn_count 13 -> 20 and adds SEVEN
  extra movables ahead of the symbol movable (-21 threshold). That is exactly the size of
  lever this function needs.

- [s7] ...and it is REFUTED FOR THIS FUNCTION BY TARGET'S OWN BYTES. A hoisted in-loop
  invariant lands in the loop PREHEADER. In target (objdump of
  tmp/grind/func_80041BF4/s5/wsA/target.o) the three /255 conversions sit at 0x78-0xd8,
  BEFORE the `jal func_800486FC` at 0xd8, whereas a hoisted in-loop conversion would have
  to land after the func_8004881C block (0xe0-0xfc). Target's outer-loop preheader is bare
  (`move s2,zero` at 0x100) and its inner-loop preheader is bare (`j 1ac / move s1,zero`
  at 0x140-0x144) — **there is no hoisted invariant computation anywhere in target's
  preheaders**, so the original source had no extra loop-invariant work inside either loop.

- [s7] KILLED — using DEAD invariant work to raise insn_count. Four dead `(aN << k) / c`
  computations written into the inner loop leave the loop dump's insn_count at exactly 37:
  cse.c deletes dead pseudo sets BEFORE loop.c ever scans the loop. Banked as
  rejected/dead-invariant-pad-deleted-by-cse-before-loop.c.

- [s7] NET: on the 135-insn / frame-88 chassis, loop.c's hoist of the D_800A9A24 symbol
  pseudo is NOT suppressible by any C form — the gate's only usable field is insn_count,
  raising it requires either +19 real insns (bytes) or in-loop invariant work (refuted by
  target's preheaders) or dead work (deleted before loop.c), and the loop-note-removal
  route costs the frame and one instruction. With s3 (declaration order, body shape),
  s4/s5 (permuter, naming), s6 (live-range conflict, operand order) and now s7 (the hoist)
  all closed, **the register-allocation axis for this function is exhausted**; the residual
  must now be attacked by rederiving the body from target's asm.

- [s7] Artifacts: tmp/grind/func_80041BF4/s7/ — loopdump.sh (movable-log dumper for the
  current src), gen_thr.py/gen_thr2.py/thr.sh/thr2.sh (synthetic threshold bisection),
  pad.py/padrun.sh (in-situ inner-loop padding), licm_out.c/licm_in.c/licm.sh (the
  in-loop-invariant byte-neutrality measurement), v_goto.c, v_goto2.c, v_invpad.c,
  d_base.loop, d_p8/p16/p24.loop, d_goto.loop, d_goto2.loop, lreg_p16.txt,
  lreg_goto2.txt.

- [s7] Chassis re-measured at dispatch and again at the end of the session: candidate.c applied to src/text1a_post.c scores sandbox --disable all = 11 at 135/135 insns, frame 88. The dispatch brief said 'measurement unavailable'; the real HEAD floor is 11, unchanged since s4.

- [s7] CORRECTION to the s6 ledger: loop.c's move_movables per-movable log IS in the -da dump set - tmp/grind/func_80041BF4/dumps/text1a_post.loop, 11 'savings' lines across the TU, func_80041BF4 at lines 2246-2963. s6 recorded it as the highest-value UNREAD artifact; it was already on disk.

- [s7] The movable log for this function: inner loop 174->271 is 37 real insns and moves regno 137 (insn 213), 138 (insn 218) and 140 (insn 223, the D_800A9A24 symbol) in that order; the outer loop 129->287 is 59 real insns and re-moves all three under already_moved[] with insn_count halved.

- [s7] The desirability gate is loop.c:1631 `already_moved[regno] || (threshold*savings*m->lifetime) >= insn_count`; m->savings = n_times_used[regno] (loop.c:793), threshold = (loop_has_call ? 1 : 2)*(1 + n_non_fixed_regs) (loop.c:532), threshold -= 3 after each moved movable (loop.c:1719 and 1904).

- [s7] Threshold MEASURED by synthetic bisection under this exact cc1 configuration: a call-containing loop of 61 real insns still moves its first savings-1/life-1 symbol movable; 63 real insns declines it. threshold = 61 or 62 (n_non_fixed_regs = 60 or 61). Independent agreement in the same compile: func_80041EB0's 83-insn loop declines its FIRST movable.

- [s7] Because the symbol movable is third, its effective threshold is 55-56 against insn_count 37: suppressing the hoist needs +19 real RTL insns in the inner loop of a 135-instruction function. In-situ padding confirms the flip point (pad=8 -> 46 insns moved; pad=16 -> 54 insns 'not desirable').

- [s7] MEASURED, not derived: with the hoist suppressed the symbol pseudo takes hard reg 5 ($a1) and `off` takes hard reg 2 ($v0) - target's exact assignment - in the goto-spelled chassis where loop.c sees no loop at all.

- [s7] Loop-invariant work written INSIDE a loop is instruction-count-neutral (licm_in.c vs licm_out.c compile to the same instructions in a different order/allocation) and is the only free way to raise insn_count: it added 7 insn_count and SEVEN extra movables (-21 threshold) in the synthetic. It is refuted for this function by target's bare preheaders.

- [s7] Dead invariant work cannot raise insn_count: four dead (aN<<k)/c computations placed in the inner loop left the loop dump's insn_count at exactly 37, because cse.c deletes dead pseudo sets before loop.c scans the loop.

- [s7] Three forms banked in rejected/ this session: goto-spelled-inner-loop-plus1-insn-still-hoisted.c (46 at 136), goto-spelled-both-loops-target-alloc-but-frame-80.c (43 at 136, frame 80 vs 88), dead-invariant-pad-deleted-by-cse-before-loop.c (measurement probe, not a proposed form).

- [s7] NET: with s3 (declaration order, body shape), s4/s5 (permuter, naming, named intermediates), s6 (live-range conflict, operand order) and now s7 (the loop.c hoist) all closed, the register-allocation axis for func_80041BF4 is exhausted. The residual 11 must now be attacked by re-deriving the body, not by more allocation work.

---

## [s8] REDERIVE session (2026-08-19) - floor 11 held; frontier item 1 KILLED by a cc1 crash; two dead constructs deleted from the candidate

Chassis re-measurement at session start: applying s7's banked `candidate.c` to
`src/text1a_post.c` reproduces **score 11 at 135/135 insns** on today's HEAD.
The ledger floor of 11 is therefore chassis-current, and every conclusion below
is measured against it.

### E-s8-1 - the full 11-instruction residual is ONE register swap plus its cascade
Instruction-by-instruction diff of the sandbox object against
`asm/funcs/func_80041BF4.s` (both 135 insns; artifacts
`tmp/grind/func_80041BF4/s8/mine.txt` and `target.txt`): the prologue, the three
`/255` divides, the `func_800486FC` branch, the `if (outer == 0)` arms, the
table reload, the loop-back test, the epilogue and every stack offset are
already byte-identical. All differences are inside the inner-loop body plus
the trailing test:

| idx | target | s8 candidate |
|---|---|---|
| 84-85 | `lui $a1,%hi(D_800A9A24)` / `addiu $a1,$a1,%lo` | `lui $t0` / `addiu $t0` |
| 86 | `addu $a1, $v0, $a1` | `addu $a1, $a1, $t0` |
| 89 | `lhu $v1, 0x2($s0)` | `lhu $v0, 2($s0)` |
| 91-94 | `li $v0,0x10` / `sh $v0` / `li $v0,1` / `sh $v0` | `li $t0,...` / `sh $t0,...` |
| 95/97 | `addu $v1,$v1,$s4` / `sh $v1,0x1A($sp)` | `addu $v0,$v0,$s4` / `sh $v0` |
| 110 | `sll $v0, $s1, 5` | `sll $a1, $s1, 5` |
| 117 | `addiu $t0, $zero, 0x1` | matches only because of the `one` holder |

i.e. target has **the symbol in `$a1` and `off` in `$v0`**; we have **`off` in
`$a1` and the symbol re-materialised into `$t0`**. Everything else in the loop
is that cascade.

### E-s8-2 - the hoist is now READ, not inferred (.greg / .lreg dumps)
`pwsh tools/grinder/dump.ps1 func_80041BF4` on the s8 candidate:

- `.lreg` insn **327** is `(set (reg:SI 140) (symbol_ref/v:SI ("D_800A9A24")))`
  carrying `REG_EQUIV`, and it sits **before `(note 129 ... NOTE_INSN_LOOP_BEG)`**:
  loop.c hoisted it out of BOTH loops, to ahead of the OUTER loop, next to the
  two other REG_EQUIV movables `(set (reg:HI 137) (const_int 16))` and
  `(set (reg:HI 138) (const_int 1))` (insns 323 / 325 - the `rect[2]` / `rect[3]`
  constants).
- `.greg` for func_80041BF4 lists `;; 140 conflicts: 75 76 77 78 79 80 81 82 83
  84 86 127 137 138 140 2 4 5 6 7 29` - **hard regs 4 5 6 7 are `$a0`-`$a3`**.
  Because the hoist makes reg 140 live across the whole outer loop (hence across
  `LoadImage` / `DrawSync` / `func_80048A7C`), it structurally conflicts with
  `$a1` and can never be allocated there.
- `off` is **reg 130** and it is NOT in global.c's `;; 24 regs to allocate`
  list - local-alloc keeps it and hands it `$a1` off the hard-reg suggestion on
  `.lreg` insn **229** = `(set (reg:SI 5 a1) (plus (reg/v:SI 130) (reg:SI 140)))`.

This confirms s6/s7's attribution from the dumps themselves: the hoist is the
sole cause and everything downstream is forced.

### E-s8-3 - KILL (mechanical): the array / scaled-pointer rederive family SEGFAULTS cc1
Frontier item 1's flagship probe was "declare D_800A9A24 as an array type and
index it, so the address is a single `(plus (symbol) (mult))` tree". Three
spellings were tried:

    LoadImage((s32)rect, (s32)((u16 *)&D_800A9A24 + (idx << 4)));       /* u16 stride 0x10 */
    LoadImage((s32)rect, (s32)((u8 (*)[32])&D_800A9A24)[idx - 1]);      /* array of 32-byte rows */
    LoadImage((s32)rect, (s32)&((u16 (*)[16])&D_800A9A24)[idx - 1][0]); /* array of 16 u16 rows */

**All three make GCC 2.7.2 `cc1` terminate with SIGSEGV (exit 139)** on
`src/text1a_post.c`. The sandbox surfaces this as `C build failed` with a
cascade of `.size expression ... does not evaluate to a constant` assembler
errors (the truncated asm stream) - that is the signature to recognise.
Flag bisection on the identical preprocessed input
(`tmp/grind/func_80041BF4/s8/v04.i`, script `crashtest.sh`):

| flag | result |
|---|---|
| (none) | **139 (SIGSEGV)** |
| `-fno-strength-reduce` | no segfault |
| `-fno-schedule-insns` | 139 (SIGSEGV) |
| `-fno-schedule-insns2` | 139 (SIGSEGV) |
| `-fno-rerun-cse-after-loop` | 139 (SIGSEGV) |

The crash is therefore in **loop.c's strength reduction**, triggered by a scaled
(element size > 1) pointer add on a loop-variant index against this symbol
inside the inner loop. `CC_FLAGS` is frozen, so this family is **unavailable**,
not merely worse. It is also the wrong shape on the evidence: target
strength-reduces nothing here - it re-emits `sll $v0,$s1,5` and re-materialises
the symbol on every iteration, which is the signature of an UNSCALED
byte-pointer add (`(u8 *)&D_800A9A24 + off`), i.e. exactly the form the
candidate already uses. Corroboration from the same file: the sibling
`func_80041AC8` DOES walk a `u16 *` by `+= 0x10` and compiles to a biv
(`.loop:1895` - `Biv 74 initialized at insn 119: initial value
(symbol_ref ("D_800A9A24"))`), which func_80041BF4's target asm plainly is not.

### E-s8-4 - WIN (cheat-surface, not floor): two candidate constructs are EXACTLY INERT and are now deleted

- **`int new_var; new_var = 5;`** - an UNANNOTATED opaque constant-holder used
  as the shift amount in `idx << new_var`, carried since s0. Measured worth
  **zero**: literal `idx << 5` scores 11 at 135 insns, and `idx * 32` also
  scores 11 at 135 insns. It was a standing layer-1 liability (a constant-holder
  with no `/* FAKE */` annotation and no lever-exhaustion record) that bought
  nothing. Deleted from `candidate.c`.
- **`s32 sent;` with `while ((sent = tbl[0]) >= 0)`** - measured worth zero:
  plain `while (tbl[0] >= 0)` scores 11 at 135 insns. Deleted.

The s8 candidate therefore carries exactly ONE non-ordinary construct (the
`one` constant-holder), down from three.

### E-s8-5 - the `one` holder is an INDEPENDENT 2-instruction divergence, not a cascade of the loop residual

| chassis | with `one` holder | with literal `1` |
|---|---|---|
| while / do-while (the candidate) | 11 @ 135 | 13 @ 135 |
| goto-spelled (s7 `v_goto2`) | 43 @ 136 | 45 @ 136 |

Fixing the loop allocation will NOT fix the trailing test for free. The
divergence is `li $v1,1 / bne $v0,$v1` (ours) vs `li $t0,1 / bne $v0,$t0`
(target) - a reload scratch-register choice for the rematerialised `const_int 1`.

### E-s8-6 - direct confirmation that the goto chassis already has target's ENTIRE loop body
Disassembling the goto chassis WITHOUT the holder (`g2_goto2_lit1`, 45 @ 136),
the inner-loop body is target's register-for-register:

    addiu s1,s1,1 / addiu a0,sp,24 / lui a1 / addiu a1 / addu a1,v0,a1
    addu v0,v1,s8 / sh v0,24(sp) / lhu v1,2(s0) / addiu s0,s0,4
    li v0,16 / sh v0,28(sp) / li v0,1 / sh v0,30(sp)

Every one of E-s8-1's loop rows is fixed there (`addu a1,v0,a1`, `lhu v1`,
`li v0,16`, `li v0,1`). Its remaining cost is entirely OUTSIDE the loop body:
frame 80 vs 88, the callee-save rotation (`$s8` where target has `$s5`), one
extra instruction, and the same trailing-test divergence. That is exactly the
problem class s1 solved in the while basin, and it makes frontier item 2 the
strongest remaining lead by a wide margin.

### E-s8-7 - inert rederive spellings (all measured 11 @ 135, no effect)
`u32 off` - `&((u8 *)&D_800A9A24)[off]` - `(char *)&D_800A9A24 + off` -
`off` declared at function scope instead of block scope -
`rect[3] = 1;` stored before `rect[2] = 0x10;` -
`LoadImage((s32)&rect[0], ...)` instead of `(s32)rect` -
outer loop spelled `for (outer = 0; outer < 2; outer++)` instead of do-while -
`rect` declared as `struct { s16 x, y, w, h; }` and passed as `&rect` or
`&rect.x` (the PsyQ-idiomatic RECT rederive - exactly inert).

### E-s8-8 - worse rederive spellings (banked in rejected/)
`idx++` moved to after `DrawSync()` / `tbl += 2` gives 16 @ 137 - naming the
LoadImage source address in a block-local declared BEFORE the rect stores gives
18 @ 138 - `for (idx = 0; tbl[0] >= 0; idx++)` gives 16 @ 137.

### E-s8-9 - tooling note
`m2c` is NOT installed in `.venv` on this machine (`No module named m2c`), so
the "fresh m2c decompile" leg of the rederive modality is unavailable. It is
also redundant here: the build is already 135/135 insns with byte-identical
control flow, so a fresh m2c pass has no structural information left to add -
the instruction-level derivation in E-s8-1 is strictly stronger.

- [s8] Chassis re-measured at session start: s7's banked candidate.c applied to src/text1a_post.c reproduces score 11 at 135/135 insns on today's HEAD, so the ledger floor of 11 is chassis-current.

- [s8] The build is already 135/135 insns with byte-identical prologue, /255 divides, func_800486FC branch, if (outer == 0) arms, table reload, loop-back test, epilogue and every stack offset. All 11 differing instructions live in the inner-loop body plus the trailing == 1 test.

- [s8] Target's loop allocation is: symbol in $a1 (reloaded straight into the argument destination), off in $v0, y in $v1, rect constants in $v0. Ours is: off in $a1, symbol re-materialised into $t0, y in $v0, rect constants in $t0.

- [s8] GCC 2.7.2 cc1 SEGFAULTS (exit 139) on src/text1a_post.c for every scaled-pointer / array-typed spelling of the D_800A9A24 address inside the inner loop; -fno-strength-reduce is the only flag that avoids it, so the crash is in loop.c strength reduction and the family is unavailable on the frozen chassis.

- [s8] The sandbox reports that cc1 crash as 'C build failed' with a cascade of '.size expression for <func> does not evaluate to a constant' assembler errors - a truncated asm stream, not a C syntax error. Recognise this signature rather than debugging the C.

- [s8] The `int new_var = 5;` shift holder and the `s32 sent;` loop-condition temp are BOTH measured worth exactly zero (11 at 135 with or without) and have been deleted from candidate.c. The candidate now carries exactly one non-ordinary construct (the `one` constant-holder) instead of three.

- [s8] The `one` constant-holder is worth exactly 2 in BOTH basins (11 vs 13 in the while chassis, 43 vs 45 in the goto chassis), so it is an independent divergence, not a cascade of the loop residual. It remains UNVETTED and its FAKE prerequisites remain unmet (synthesis modality is untried, so the ladder is not spent). Do not spend it.

- [s8] s7's goto-spelled chassis WITHOUT the holder (45 at 136) emits the inner-loop body register-for-register identical to target: addiu s1,s1,1 / addiu a0,sp,24 / lui a1 / addiu a1 / addu a1,v0,a1 / addu v0,v1,s8 / sh v0,24(sp) / lhu v1,2(s0) / addiu s0,s0,4 / li v0,16 / sh v0,28(sp) / li v0,1 / sh v0,30(sp). Its entire remaining cost is OUTSIDE the loop: frame 80 vs 88, the $s8-where-target-has-$s5 callee-save rotation, one extra instruction, and the independent trailing test.

- [s8] The sibling func_80041AC8 in the same file walks a u16 * by += 0x10 over the same D_800A9A24 base and compiles to a biv (.loop:1895, 'Biv 74 initialized at insn 119: initial value (symbol_ref ("D_800A9A24"))'). func_80041BF4's target asm has no biv and no walking pointer - it re-materialises the symbol and re-shifts idx every iteration - so the two functions were NOT written with the same address idiom.

- [s8] m2c is not installed in .venv on this machine ('No module named m2c'), so the fresh-m2c leg of rederive is unavailable; it is also redundant at 135/135 insns with byte-identical control flow.
