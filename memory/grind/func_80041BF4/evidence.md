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
