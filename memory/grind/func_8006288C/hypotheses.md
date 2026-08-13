# Hypothesis ledger — func_8006288C

## Session 1 (recon, 2026-08-13) — floor 23 -> 2

### CONFIRMED
**H1 — the whole distance-23 gap is the global.c allocno-priority sort, and the
loop's C spelling is the lever that reorders it.**
Mechanism: GCC 2.7.2 `global.c:allocno_compare` sorts allocnos by
`floor_log2(nrefs)*nrefs/live_length`; MIPS `reg_alloc_order` then hands out
$3,$4,$5,$6,$7,$8,$9,$10,$11 down that order, so the sort IS the register
assignment. The loop spelling moves the latch block and the loop notes, which
changes every pseudo's `reg_live_length`, which permutes the sort.
Probe: instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_ALLOC_DEBUG=1`) priority
table + a 20-variant standalone sweep (`tmp/grind/func_8006288C/s1/sweep.py`).
Result: `do { mask = 1 << i; if (!(D_800A3460 & mask)) { ...; goto out; }
<latch> } while (i < 6);` gives **9/9 target registers**; sandbox distance
**23 -> 2**. `while (1) { ... break; ... }` is codegen-identical (9/9);
`for (i = 0; i < 6; i++)` is 7/9 (swaps `i`/`off_s32` between $3 and $4);
the old label + `goto loop_top` + else-latch form is 3/9.

### KILLED
**H2 — the destination writes are array indexing (`(&SYM)[i*3+k]`) and GCC's
strength reduction manufactures the +12/+8 induction variables.**
Probe: `tmp/grind/func_8006288C/s1/probe_b.c` compiled with the project cc1
flags (strength reduction is ON — `NO_SR_FILES` is empty in the Makefile).
Result: **92 instructions**, a 16-byte stack frame, and per-store index
recomputation (`sll`/`addu` chains, repeated `la` of each base). The target's
`lui $at,%hi(SYM); addu $at,$at,<off>; sw $v0,%lo(SYM)($at)` shape only comes
from an explicit byte-offset accumulator. KILLED; banked as
`rejected/array-index-form-explodes-to-92-insns.c`.

**H3 — the if-body statement order is a free lever for live lengths.**
Probe: sweep set 1 (`variants.py`), 6 permutations of the word-store group /
halfword-store group / `D_800A3460 |= mask` / `*flag_p = 0`.
Result: every permutation changes the cc1 instruction COUNT (43, 43, 43, 45)
away from the target's 42 — the body order is byte-load-bearing, not a free
allocation lever. KILLED; banked as
`rejected/ifbody-statement-reorder-breaks-42-insn-shape.c`.

**H4 — value-naming variants inside the loop move the allocation.**
Probe: sweep set 1 — explicit named flag-word local (`w = D_800A3460; ...
D_800A3460 = w | mask;`), no-mask-variable (inline `1 << i` at both uses),
and a staged temp for the third halfword.
Result: all three are **codegen-identical** to the base form (same 42
instructions, same registers). CSE canonicalises them to the same RTL.
KILLED as an allocation lever.

**H5 — the init-block statement order moves the allocation.**
Probe: sweep set 2 — 6 permutations (src pointers first / last, flag_p last,
off_s16 late, the `D_800F1138 = 1` store first).
Result: no permutation that keeps 42 instructions changes any register; the
two that did change registers (`g_flagp_last`, `g_off16_late`) moved `i` and
`off_s32` the WRONG way (1/9). KILLED.

**H6 — a `new_var = 1;` constant holder placed early gives the target's
prologue-slot-1 `li $t3,1`.**
Probe: sweep set 3 — `new_var = 1;` at each of the 8 init-block positions.
Result: every position **before** the `D_800F1138 = 1;` store makes cse fold
the two constant-1 materialisations (`sw $t3,D_800F1138`), dropping the
function to **41** instructions — the target has 42 with two separate `li`s.
Only the after-the-store position keeps 42, and there the scheduler parks
`li $t3,1` at prologue slot 5, not slot 1. KILLED; banked as
`rejected/const1-before-store-cse-folds-to-41-insns.c`.
(Note the current best form drops the holder entirely — `mask = 1 << i;` — so
the `li` is created by loop.c invariant hoisting after cse2 and no fold
happens; same 42 instructions, same slot-5 placement.)

## Live frontier for session 2

1. **The residual-2 is the prologue slot of `li $t3,1`.** Target emits it
   immediately after `move $a0,$zero`; we emit it after the four
   `la`/`move` init insns. Everything else — all 52 instructions, all 9
   register assignments — already matches. Mechanism to attack: the constant
   is created by loop.c's invariant hoist into the preheader (post-cse2, which
   is why it is not folded with the store's `1`), and cc1's first-pass
   scheduler (`sched.c`, `-fschedule-insns`) then places it at slot 5. Next
   probe: dump `probe.i.sched` / `probe.i.loop` for the current best form and
   read where insn for `(set reg 1)` is inserted and what its `INSN_PRIORITY`
   is relative to the `la`/`move` insns; then look for a C form whose init
   block gives the constant a longer dependence chain to the `sllv`.
2. **Cheap unexplored axis: the types of the offset/index locals.**
   `off_s16`/`off_s32`/`i` are all `s32`; `u32`/`s16` spellings change
   `allocno_size`/live ranges and cost nothing to sweep with the existing rig
   (add a set to `variants*.py`).
3. **Permuter.** At distance 2 with a 52/52 instruction-for-instruction shape
   this is an ideal short permuter run — build `target.o` from
   `asm/funcs/func_8006288C.s` + the permuter prelude so the function sits at
   offset 0 (see [[difficult-is-not-impossible]] §3), seed from the current
   best form, and run with `--stop-on-zero`.

## [s1] The entire distance-23 gap is register assignment, and the loop's C spelling is the lever: GCC 2.7.2 global.c allocno_compare sorts allocnos by floor_log2(nrefs)*nrefs/live_length and MIPS reg_alloc_order then hands out $3,$4,$5,$6,$7,$8,$9,$10,$11 down that order, so the sort IS the assignment; the loop spelling moves the latch block and loop notes and therefore every pseudo's reg_live_length.
- mechanism: global.c allocno_compare priority sort + MIPS reg_alloc_order; live_length is set by flow.c from the RTL block layout the loop spelling produces
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) priority table for the baseline, plus a 20-variant standalone cc1 sweep (tmp/grind/func_8006288C/s1/sweep.py) scoring each variant's variable->hard-register map against the target map extracted from asm/funcs/func_8006288C.s
- result: do { mask = 1 << i; if (!(D_800A3460 & mask)) { ...; goto out; } <latch> } while (i < 6); gives 9/9 target registers and sandbox --disable all distance 23 -> 2. while(1){...break;} is codegen-identical (9/9); for(i=0;i<6;i++) is 7/9; the previous label + goto loop_top + else-latch form was 3/9.
- verdict: CONFIRMED

## [s1] The destination writes are array indexing ((&SYM)[i*3+k]) and GCC's strength reduction manufactures the +12 / +8 induction variables that the target increments in the latch.
- mechanism: loop.c strength reduction creating givs from an i-scaled index
- probe: tmp/grind/func_8006288C/s1/probe_b.c compiled with the exact project cc1 flags (strength reduction is ON; NO_SR_FILES is empty in the Makefile)
- result: 92 instructions vs the target's 52, a 16-byte stack frame, and per-store index recomputation (sll/addu chains plus a repeated la of each base). The target's lui $at,%hi(SYM); addu $at,$at,<off>; sw $v0,%lo(SYM)($at) shape only comes from an explicit byte-offset accumulator, which is what the in-tree C already had.
- verdict: KILLED

## [s1] The statement order inside the taken-slot if-body is a free lever for shifting allocno live lengths.
- mechanism: moving a store changes each pseudo's last-use point and hence reg_live_length
- probe: Sweep set 1 (tmp/grind/func_8006288C/s1/variants.py): 6 permutations moving the word-store group, the halfword-store group, D_800A3460 |= mask, and *flag_p = 0
- result: Every permutation changes the cc1 instruction COUNT away from the target's 42 (43, 43, 43, 45). The body order is byte-load-bearing, not a free allocation lever.
- verdict: KILLED

## [s1] Renaming/staging values inside the loop (explicit flag-word local, no mask variable, staged temp for the third halfword) moves the allocation.
- mechanism: extra named intermediates changing allocno nrefs
- probe: Sweep set 1, variants v1_named_w / v5_no_mask / v6_stage_c
- result: All three are codegen-identical to the base form — same 42 instructions, same 9 registers. CSE canonicalises them to identical RTL, so nrefs never changes.
- verdict: KILLED

## [s1] The init-block statement order moves the allocation toward the target.
- mechanism: def points set the start of each live range
- probe: Sweep set 2: 6 init-order permutations (src pointers first / last, flag_p last, off_s16 late, D_800F1138 store first) with the loop held fixed
- result: No 42-instruction permutation changes any register; the two permutations that did change registers moved i and off_s32 the wrong way (1/9).
- verdict: KILLED

## [s1] A constant-holder local (new_var = 1;) placed early in the init block produces the target's prologue-slot-1 li $t3,1.
- mechanism: source position of the constant assignment setting its RTL emission slot
- probe: Sweep set 3: new_var = 1; at each of the 8 init-block positions, under the confirmed do/while shape
- result: Every position before the D_800F1138 = 1; store makes cse fold the two constant-1 materialisations (sw $t3,D_800F1138) and drops the function to 41 instructions; the target has 42 with two separate li insns. Only the after-the-store position keeps 42, and there the scheduler parks li $t3,1 at prologue slot 5, not slot 1. The best form therefore drops the holder entirely (mask = 1 << i), letting loop.c hoist the constant post-cse2 — same 42 instructions, same slot-5 placement.
- verdict: KILLED


## Session 2 (structural, 2026-08-13) - floor 2 -> 0

### CONFIRMED
**The closing lever - the constant holder's init-block position IS the
residual, and it becomes reachable once the `D_800F1138 = 1;` store is moved to
the FRONT of the init block.**
Mechanism: with a literal `1 << i` the shift base is loop-invariant, so loop.c
hoists its `(set reg 1)` into the TAIL of the preheader and sched.c's first
pass emits it at init-block slot 6. A holder assigned in the init block gives
that insn an early LUID instead. s1's H6 could not use an early holder because
the holder always preceded the store and cse folded the two constant-1s (41
insns). Putting the store FIRST keeps the holder after it (no fold, 42 insns)
while still giving it an early LUID.
Probe: sweep set 5 axis A (9 variants, init-order x holder-position) with the
new prologue-order scorer, then the winning form applied in-tree.
Result: `D_800F1138 = 1; i = 0; one = 1; flag_p = ...;` with `mask = one << i`
emits the target's init block exactly and the honest sandbox
(`--disable all`) prints **score 0 / 52 of 52 instructions**. Position is
exact: holder at index 1 inverts slots 1-2, holder at index 3 emits at slot 4.

### KILLED
**H7 - some ordering of the init block with a LITERAL `1 << i` (no constant
holder at all) can put the shift-base constant at the target's slot 2.**
Mechanism tested: statement order sets RTL LUID, which breaks ready-list ties
in sched.c's first pass.
Probe: sweep set 6 (`tmp/grind/func_8006288C/s2/variants6.py`, 10 variants) -
`D_800F1138 = 1;` at each of the 7 possible init-block positions, plus a
`while (1) { ... break; }` loop spelling, plus `i` initialised last, plus the
whole init block reversed.
Result: **all 10 park `addiu $t3,$zero,1` at init-block slot 6.** With a
literal the constant is not emitted from the init block at all - loop.c
creates it at the preheader tail - so no init-block ordering can reach it.
KILLED; banked as `rejected/literal-shift-base-parks-const-at-slot6.c`. This
is the lever-exhaustion evidence for the constant holder's FAKE annotation.

**H8 - the declared types of the loop scalars (`i`, `off_s32`, `off_s16`,
`mask`) have unexplored gradient (s1 frontier item 2).**
Mechanism claimed: `allocno_size` is a direct multiplier in
`global.c:allocno_compare`, and narrower types change flow.c live-range
bookkeeping.
Probe: sweep set 5 axis B - each scalar and then all of them re-declared
`unsigned int`, plus `short off_s16/off_s32` and `short i`.
Result: every `unsigned int` spelling is **codegen-identical** to the `s32`
base (same 9 registers, same instruction count, same init-block order) -
GCC 2.7.2 promotes all of them to SImode pseudos, so `allocno_size` is 1 for
every variant and the claimed multiplier never varies. `short off_s16/off_s32`
costs 2 extra instructions and drops to 7/9; `short i` costs 3 extra and drops
to 4/9. Axis closed as an allocation lever. KILLED.

## Live frontier after session 2

The function MATCHES in pure C (honest sandbox 0). The remaining work is not
a search problem:
1. **Rule retirement + oracle verify.** `regfix.txt` still carries the 6 rules
   ($5 <-> $8/$10/$7/$9/$6 plus `reorder 4,1,2,3 @ 1-4`) that were paperwork
   over the old register permutation. They must be retired
   (`retire func_8006288C`) and the full build re-verified against the oracle
   SHA1 before `queue done`. This session may not touch regfix.txt and did not.
2. **Layer-1 / layer-2 cheat-review of the `one` holder.** The one construct
   in the diff without independent semantic purpose is the constant holder
   (pre-existing in HEAD as `int new_var;`, moved/renamed/annotated here). It
   is claimed under the sanctioned constant-holder / opaque-arithmetic-variable
   family with the exhaustion evidence above; see `self_vet.md`.

## [s2] The `D_800F1138 = 1;` store must lead the init block for the constant holder to be placeable early: with a literal `1 << i` loop.c creates the shift base at the preheader tail (slot 6), and with the holder BEFORE the store cse folds the two constant-1 materialisations to 41 instructions. Store-first + holder-at-index-2 is the only cell that keeps 42 instructions AND gives the constant an early LUID.
- mechanism: loop.c invariant hoist (preheader tail) vs. init-block LUID + sched.c first-pass ready-list tie-break; cse2 constant folding across the two 1s
- probe: sweep set 5 axis A (tmp/grind/func_8006288C/s2/variants5.py) with the new prologue-order scorer in sweep2.py, then the winning form applied to src/text1b.c and measured with `sandbox func_8006288C --disable all`
- result: honest sandbox distance 0, build_insns 52 == target_insns 52, all 9 register assignments and the full init-block order reproduced
- verdict: CONFIRMED

## [s2] Some ordering of the init block with a literal `1 << i` and no constant-holder local can put the shift-base constant at the target's init-block slot 2.
- mechanism: statement order sets RTL LUID, which breaks ready-list ties in sched.c's first pass
- probe: sweep set 6 (tmp/grind/func_8006288C/s2/variants6.py) - 10 variants: the `D_800F1138 = 1;` store at each of the 7 possible init-block positions, a `while (1) { ... break; }` loop spelling, `i` initialised last, and the init block reversed
- result: all 10 park `addiu $t3,$zero,1` at init-block slot 6. With a literal the constant is never emitted from the init block at all - loop.c hoists it to the preheader tail - so no init-block ordering can reach slot 2. Banked as rejected/literal-shift-base-parks-const-at-slot6.c
- verdict: KILLED

## [s2] The declared types of the loop scalars (i, off_s32, off_s16, mask) still have unexplored allocation gradient via allocno_size and flow.c live ranges (s1 frontier item 2).
- mechanism: allocno_size is a direct multiplier in global.c allocno_compare; narrower types also change flow.c live-range bookkeeping
- probe: sweep set 5 axis B - each scalar and then all four re-declared `unsigned int`, plus `short off_s16/off_s32` and `short i`
- result: every unsigned spelling is codegen-identical to the s32 base (same registers, same instruction count, same init-block order) because GCC 2.7.2 promotes all of them to SImode pseudos, so allocno_size is 1 in every variant; `short off_s16/off_s32` costs 2 extra instructions and drops to 7/9 registers, `short i` costs 3 extra and drops to 4/9
- verdict: KILLED

## Session 3 (structural, 2026-08-13) — re-verification + self-vet format fix

No new search was needed and none was run: session 2 had already reached honest
sandbox 0, and its outcome was discarded by the driver for a MECHANICAL FORMAT
defect in `self_vet.md`, not for anything about the C. The validator
(`tools/grinder/grindlib.py:51`, regex
`^\s*SCOPE\s*:\s*["“](.+?)["”]\s*$`) requires each claimed family's SCOPE quote
to sit on a SINGLE physical line; s2 had wrapped both quotes across four lines
each, so zero of the two claims matched and the session was rejected with
"claims 2 sanctioned family/families but quotes only 0 verbatim SCOPE
sentence(s)".

What session 3 did:
1. Confirmed the driver had reverted s2's `src/text1b.c` edit (git status showed
   the ledger files dirty but `src/text1b.c` clean at HEAD's label+goto form).
2. Re-applied `memory/grind/func_8006288C/candidate.c` verbatim to
   `src/text1b.c` (do/while + `goto out`, store-first init block, `one` holder
   at init index 2 with its FAKE annotation).
3. Re-measured: `sandbox func_8006288C --disable all` -> `"score": 0`,
   `build_insns 52 == target_insns 52`, `rules_dropped: 6`. The 0 is reproduced
   in-tree THIS session, independently of s2.
4. Rewrote `self_vet.md`: same six answers, same two family claims, same
   prerequisite evidence, but each SCOPE quote de-wrapped onto one physical line
   (newlines -> single spaces, no other edit) and each PRECEDENT given as a
   literal file:line —
   `.claude/rules/named-local-fake-exception.md:12` for the constant-holder
   family and `.claude/rules/no-new-park-categories.md:175` for the opaque
   arithmetic variable family.
5. Verified the fix mechanically by calling `grindlib.validate_self_vet` on the
   repo directly: it now returns `(True, '')`.

LESSON FOR FUTURE SESSIONS ON ANY FUNCTION: the self-vet SCOPE line is
line-oriented. A rule file wraps its prose, so quoting "verbatim" means
de-wrapping the sentence onto one line in self_vet.md — a wrapped quote is
mechanically invisible to the validator no matter how faithful it is. State the
de-wrap explicitly next to the quote so the human reviewer can see nothing was
altered.

The live frontier is unchanged from session 2: the function MATCHES in pure C;
what remains is operator-side (retire the 6 regfix rules + oracle verify) and
the layer-1/layer-2 review of the `one` constant holder.
