# Evidence bank — func_8001B748

- == imported from memory/wip notes.md ==
# DispPracticeMenuTex_A (src/code6cac.c) — WIP checkpoint

**Floor: 32. RA component is VALIDATED UNREACHABLE; only the SCHED component is open.**
Wiring: `asmfix.txt` `DispPracticeMenuTex_A: replace_with_asmfile "asm/funcs/DispPracticeMenuTex_A.s"`.
Body is cheat-clean (`volatile_cheat_count` 0).

## Residual (via `tools/pairdiff.py code6cac DispPracticeMenuTex_A`)

Two components, per `goal_from_tgt.py classify`:

1. **RA** — a uniform `$v0`/`$v1` exchange: `$v0->$v1` x30, `$v1->$v0` x4, `$a0->$v0` x2.
   Every "delta then round-shift" block puts the intermediate in `$v0` for us and `$v1` for
   target. Representative (asm 112-119):
   ```
   ours   : sra v0,v0,0xc | lh v1,0(gp) | subu v0,v0,a0 | slti v1,v1,11 | bgez v0 | ...
   target : sra v1,v0,0xc | lh v0,0(gp) | subu v1,v1,a0 | slti v0,v0,11 | bgez v1 | ...
   ```
2. **SCHED** — 1 nop. Untouched, and the ONLY part not yet ruled on.

## RA component — UNREACHABLE, with evidence

Attribution closed via `goal --scope` (`tools/ra_solver/pseudo_scope.py`, narrows by
`.lreg` block scope): `$v1->$v0` narrows 27 holders to **UNIQUE pseudo 100**; `$v0->$v1`
narrows 85 to 3 (`83, 84, 98`); `$a0->$v0` narrows 5 to 3.

Derived goal `{100: $v0}` and all three full-exchange variants (`+{83|84|98: $v1}`) run
through `inverse.py global` at depth 2: **all four NEGATIVE**, with 5-17 preference atoms
FORECLOSED — `prune_preferences` (global.c:897) strips `$v0` preferences from allocnos that
cross calls, and these do.

So the exchange is not produced by refs, live span, birth order, conflicts, preferences or
calls-crossed. **Further spelling searches on the RA component are provably zero-yield.**
Next move is INSTRUMENTATION, not spelling: the local-alloc SUGGESTED-REGISTER pass
(`qty_phys_copy_sugg` / `qty_phys_sugg`), reported-but-not-scored today.

Honesty note: `{100: $v0}` is one side of an exchange and the faithfulness guard warns a
partial goal can flatter a result. Here the result is NEGATIVE and the component is
NECESSARY (pseudo 100 must reach `$v0` for target's `lh v0,0(gp)`), so it stands; the three
full-exchange variants were run precisely to close that gap.

## SCHED component (the 1 nop) — ruling: NOT DERIVABLE, and not an independent lever

**Where it is.** Target index 207, immediately after `lw a0,24(s0)` at 206 — it is that
load's delay slot:
```
target : addu v1,v1,v0 | sra v1,v1,0xc | lw a0,24(s0) | nop | subu v0,v1,a0
ours   : addu v0,v1,v0 | sra v0,v0,0xc |              ...  | subu v0,v0,a0
```
Target loads `dst+0x18` LATE and eats the delay; we hoist the load to index 197 so no nop
is needed. The nop is a CONSEQUENCE of the load placement, not a separate phenomenon.

**Mechanically NOT DERIVABLE today (measured, not inferred).** `inverse_sched`'s
`--goal-from-target` reads target's order through `goalmap.asm_body`, probed directly for
this function:
```
honest  code6cac.hon.s: OK, 228 insns
target  code6cac.tgt.s: KeyError: 'DispPracticeMenuTex_A not found in code6cac.tgt.s'
```
The block IS present, as `glabel DispPracticeMenuTex_A` + `/* off addr bytes */` lines —
the exact form `asm_body` skips. The `--goal-order` / `--goal-before` alternatives need RTL
insn UIDs, which needs the four-hop object -> hon.s -> cc1.s -> dbr-uid chain that was
deliberately not built. Note the RA side escaped this because register substitutions are
read off ALIGNED PAIRS; the scheduler side is keyed by UID and cannot avoid it.

**Not an independent lever either way.** The load placement is in the same block-3/4 region
whose RA exchange is validated UNREACHABLE, and a source reorder to defer the load was
measured NOT to move it (the scheduler re-hoists). So even with a derivable goal, this is
coupled to a residual already ruled out — not a standalone opportunity.

**Verified honest** (per the corrected doctrine — measured, not assumed): the function is
NOT rules-free (it still carries `asmfix.txt:50 replace_with_asmfile`, `rules_dropped: 1`),
but the model is honest regardless, because that rule is a post-cc1 TEXT substitution and
cannot reach the RTL. The body itself carries zero cheat constructs: `volatile_cheat_count`
0, zero detector hits inside the body span, and zero occurrences of `__asm__`, `asm(`,
`register `, or `volatile`. The file-wide `cheat_asm_stripped: 139` is siblings.

**Full diagnosis is now complete: every residual component of this function is ruled on.**

## Measured negatives — do NOT re-run

| lever | result |
|---|---|
| remove dead self-assign `inv_s1 = inv_s1;` | inert (32 -> 32); removed anyway, it was catalog debt |
| move `D_800A3310 = 0;` above `new_var` | store moved index 64 -> 53 (target wants 56); 32 -> 32 |
| reorder to load `dst+0x18` later | scheduler re-hoists it; 32 -> 32 |
| local-alloc vectors on **block 12** | wrong block entirely — block 12 is `asm[174:181]`; the contested hunk is block 3 (`asm[98:118]`) |
| `inverse.py global`, 4 goals, depth 2 | all NEGATIVE + foreclosures (above) |

The block-12 error is worth remembering: the contested exchange is **global-vs-local** (the
`use_high` value is block-local; the `dx` chain crosses into the next block, so it is a
global allocno). `inverse.py local` cannot express it — it permutes quantities inside one
block and one participant is not one.

## Tooling

- `tools/pairdiff.py code6cac DispPracticeMenuTex_A` — the scored object-level diff.
- `tools/blockmap.py code6cac DispPracticeMenuTex_A <idx>` — asm index -> basic block.
- `tools/ra_solver/goal_from_tgt.py classify|goal --scope` — verdict + attribution.
- `inverse_compose.py classify` does NOT work here (guarded since f68a9115): its
  `replace_with_asmfile` target is disassembly text it cannot read.

## Stash disposition — `stash@{0}` "nearmatch drafts: SetPacketData + DispPracticeMenuTex_A"

Two uncommitted drafts, held OUT of the tree because both functions still carry
`replace_with_asmfile` wiring, so their edits shift maspsx indices for sibling index-based
regfix rules in the same TU. Neither can be committed without a full-build verify.

| file | draft | worth |
|---|---|---|
| `src/code6cac.c` | removes the dead self-assign `inv_s1 = inv_s1;` from this function | **score-neutral** (32 -> 32). Pure catalog-debt cleanup; it would block `queue done` but buys no distance. Safe to drop and redo if the stash is ever lost. |
| `src/main.c` | SetPacketData cheat removal — redundant volatile casts/aliases, `la` inline asm replaced by honest `&D_800F1AE2`, `register asm("s4")` pin, param-alias local | **34 -> 27, the real value.** Losing it costs a re-derivation; the `la`-to-honest-C step (30 -> 27) is the non-obvious one. |

Recovery if the stash is lost: both are reconstructible from this checkpoint and
`memory/wip/SetPacketData/notes.md`, which record every step with its measurement.
SetPacketData cannot reach COMPLETED-C regardless until the owner grants the D_800F1AEC
IRQ-touched volatile carve-out (`volatile s32 *loop_flag` is load-bearing, 27 -> 39
without it).


## s1 addendum (recon, 2026-08-19) — chassis re-confirmation + dump facts

- `canonical func_8001B748`: verdict **C**, total 231, distance 32 ("pure-C target").
- `sandbox func_8001B748 --disable all`: **score 32**, build_insns 230 vs target 231,
  rules_dropped 1 (the asmfix replace_with_asmfile), cheat_asm_stripped 137 (siblings).
  Floor identical to the imported WIP ledger ⇒ chassis unchanged; all banked verdicts
  above remain spendable as-is.
- Pass dumps regenerated to tmp/grind/func_8001B748/dumps/ (dump.ps1). `.greg`
  "Register dispositions" for func_8001B748: **100 in 3, 83 in 2, 84 in 2, 98 in 2** —
  direct disposition-level confirmation of the $v0/$v1 exchange (target wants 100 in 2).
- Vanilla `.lreg` contains NO suggested-register output for this function — the
  qty_phys_copy_sugg probe (top frontier lever) requires the instrumented cc1
  (tools/gcc-2.7.2/cc1, BB2_*_DEBUG hooks), not the standard -da dump set.
- tmp/duplicates_leads.txt: no entry for func_8001B748 — no sibling/duplicate analog.
- Note: dump.ps1's cc1 run printed pre-existing parse noise for OTHER functions in the TU
  (conflicting func_80017FA0 decls; `parse error before GameObj` at src lines 780/1095);
  func_8001B748's own section compiled and is present in every dump. Not caused by, and
  not affecting, this function's body.

- [s1] func_8001B748 IS DispPracticeMenuTex_A (src/code6cac.c:943, 81-line body); rule = asmfix.txt:50 replace_with_asmfile, no text anchor.

- [s1] Honest floor THIS session: 32 (sandbox --disable all; 230 built vs 231 target insns). Canonical verdict C.

- [s1] .greg Register dispositions confirm the ledger's RA exchange: pseudo 100 in hard reg 3 ($v1) where target needs $v0; pseudos 83/84/98 in reg 2 ($v0). Dumps in tmp/grind/func_8001B748/dumps/.

- [s1] Inherited and still binding: inverse.py global NEGATIVE on all 4 exchange goals with preference foreclosure (prune_preferences global.c:897); sched 1-nop residual is a consequence of the RA-coupled load placement; measured-inert list in evidence.md must not be re-run.

- [s1] Vanilla .lreg has no suggested-register output; the qty_phys_copy_sugg mechanism can only be observed via the instrumented cc1 at tools/gcc-2.7.2/cc1 (BB2_*_DEBUG).

## s2 addendum (structural, 2026-08-19) — FLOOR 32 -> 14

**Headline: the s1 ledger's "RA component is VALIDATED UNREACHABLE" conclusion is WRONG at the
structural level and is hereby retracted.** The `ra_solver`/`inverse.py global` NEGATIVE result
proved only that the exchange is unreachable *within the goal formulation those tools express*
(refs / live span / birth order / conflicts / preferences / calls-crossed on the EXISTING pseudo
set). It said nothing about spellings that CHANGE THE PSEUDO SET. Ordinary C-level
named-intermediate and variable-reuse restructuring moved the honest floor 32 -> 14 in one
session, closing every one of the $v0/$v1 exchange hunks that the solver had declared foreclosed.
Future sessions must NOT treat a NEGATIVE `inverse.py` run as an axis kill.

### Winning structural chain (each step measured with `sandbox --disable all`)

| step | change | floor |
|---|---|---|
| baseline (s1 form, in src) | — | 32 |
| V2 | delete the `cur = new_var;` / `dy = cur;` copy chain in the early-exit arm; write `*(dst+8) = new_var >> 12;` directly | **30** |
| V5 | + delete the dead `inv_s1 = inv_s1;` self-assign (catalog debt; independently inert) | 30 |
| W3 | + ONE SHARED named intermediate `t` for all three deltas: `t = <expr> >> 12; dX = t - cur;` | **22** |
| T6 | + tail sum through `t`: `t = target + (inv_s1 * 0x2EE0); dx = (t >> 12) - cur;` | **21** |
| U3 | + split the tail shift in place and load `cur` after it: `t = t >> 12; cur = *(s32*)(dst+0x18); dx = t - cur;` | **19** |
| D5 | + a DISTINCT local `dd` (not the reused `dx`) for the final delta | **14** (with plain-multiply arms) |

`candidate.c` is the D5 form with plain-multiply arms (= variant `g_target.c`), floor **14**,
in place in `src/code6cac.c` at the end of this session.

### Residual at floor 14 (from `tools/pairdiff.py code6cac func_8001B748`)
1. **store placement, 2 insns** — `sh zero,0(gp)` (`D_800A3310 = 0;`) ours idx 64, target idx 56.
2. **arms chain register, ~10 insns** — the `frac_s1 * 0x7D0` / `* 0x1F4` synth-mult chain runs in
   `$v0` for us and `$v1` for target (target reuses the destination register from the first step).
3. **tail load/sra order + 1 nop, ~2 insns** — target emits `sra v1,v1,0xc` then `lw a0,24(s0)`
   plus a maspsx load-delay `nop`; ours emits the `lw` one slot earlier and fills the branch delay
   with `sra v0,v1,0x4`, so we build 230 insns vs target's 231.

### The 4-insn form and why it is NOT the candidate (READ THIS BEFORE RE-DERIVING IT)
Hand-decomposing the arm multiply into an in-place shift/add accumulator —
`target = frac_s1 << 5; target = target - frac_s1; target = target << 2;
target = target + frac_s1; target = target << 4;` (and `<< 2` in the else arm) — reaches
**floor 4**, closing residuals (2) and (3) above and leaving only the 2-insn store hunk plus the
tail load/sra swap. It is banked at
`memory/grind/func_8001B748/rejected/reg-n-refs-chain-extender-arms-F1-unexhausted.c`.

It is NOT submitted, and this is a policy call, not a measurement gap. A write-count gradient was
measured on an otherwise identical body:

| writes to `target` in the arms | spelling | floor |
|---|---|---|
| 1 | `target = frac_s1 * 0x7D0;` | 14 |
| 2 | `target = frac_s1 * 0x7D; target = target << 4;` | 12 |
| 3 | `target = frac_s1 * 0x7D; target <<= 2; target <<= 2;` | 12 |
| 5 | full shift/add decomposition | 4 |

The score is a function of how many times the variable is WRITTEN, not of what is computed — every
row computes the identical value and GCC's `synth_mult` already emits exactly this shift/add chain
for `* 0x7D0`. That is the signature of the **combine-foldable chain-extender that bumps
`reg_n_refs`** — the F1 family (sanctioned by owner ruling 2026-07-01 as a FAKE-annotated LAST
RESORT only). Its three prerequisites are (a) the full modality ladder demonstrably spent, (b) a
named GCC-pass mechanism, (c) the `/* FAKE: ... */` annotation. Prerequisite (a) plainly FAILS:
this is session 2, the modality ladder has barely started, and the honest floor dropped 18 points
THIS session. It also fails the plain human-programmer test — nobody writes a five-statement
shift chain to multiply by 2000. Do not submit it; do not respell it. It is recorded here as a
measured upper bound on what the arms hunk is worth (10 insns) and as the mechanism identification
for residual (2).

### Measured negatives banked this session (do NOT re-run)
| lever | result |
|---|---|
| declaration-position sweep for `t`, all 12 positions in the decl block | fully INERT (22 at every position) |
| statement-position sweep for `D_800A3310 = 0;` in the early-exit arm, all 9 slots | positions 0-4 INERT (22); 5,6,7 regress to 27/26/26; 8 gives 24 at 231 insns |
| split-init accumulation of `new_var` to reposition the gp store (`new_var = frac*a8; D_800A3310 = 0; new_var += inv_frac*b8;`) | REGRESSION on every base measured: 32->34, 30->43, 4->17 |
| THREE DISTINCT intermediates `t0/t1/t2` for the three deltas | 30 (no gain) — the win requires ONE SHARED reused `t`, not per-delta temps |
| ONE named intermediate for only dz, or only dy | 30 each (no gain) — must be all three sharing `t` |
| hoisting `inv_s1 * 0x2EE0` into a local before or after the arms (`w` or `t`) | catastrophic: 33 / 36 / 58 |
| swapping the tail addend order `((inv_s1*0x2EE0) + target)` | INERT |
| swapping the arm multiply operand order `0x7D0 * frac_s1` | INERT |
| tail statement-order permutations (load first / load mid / single-shift form / `>>=` shorthand / `target` as accumulator) | all 4-9 on the C1 base, all >= 14 on the honest base; no honest gain past D5 |
| distinct local `cc` for the tail load; inline duplicated read + RMW store of `dst+0x18` | INERT at 4 (and at 14) — the load hoist is NOT source-order controllable |
| inverting the arm condition (`if (!use_high)`), ternary form | 15 / 14 — no gain |
| reusing `t` / `dd` / `dx` / `dy` / `dz` / `v` / `new_var` / a fresh `tg` as the arm value instead of `target` | 17/22/17/22/22/17/19/17 — all WORSE than reusing `target` (14) |

### Tooling note
`tools/pairdiff.py` compares `tmp/sandbox/<func>/<stem>.o` against `build/src/<stem>.o`. It reads a
STALE sandbox object unless you run `sandbox --disable all` AFTER installing the edit — a `make`
alone does not refresh it (and `make` always reports "OK: bb2 matches!" here because the
`replace_with_asmfile` asmfix rule substitutes target asm for this function). Also note the
function is `func_8001B748` to pairdiff, not `DispPracticeMenuTex_A`. Run
`sandbox` first, then `pairdiff`, or you will diff the previous variant and mis-attribute the result.

- [s2] Honest floor THIS session moved 32 -> 14 (`sandbox func_8001B748 --disable all`, 230 built vs 231 target insns, rules_dropped 1). The 14-floor form is IN PLACE in src/code6cac.c and saved to memory/grind/func_8001B748/candidate.c.

- [s2] The s1 ledger's headline conclusion - 'RA component is VALIDATED UNREACHABLE; further spelling searches on the RA component are provably zero-yield' - is RETRACTED. inverse.py global's NEGATIVE verdict is scoped to the goal formulation it can express over the existing pseudo set; it says nothing about spellings that change the pseudo set. A NEGATIVE inverse.py run must not be banked as an axis kill again.

- [s2] Winning chain, each step measured: V2 delete the `cur = new_var;` / `dy = cur;` copy chain (32->30); V5 also delete the dead `inv_s1 = inv_s1;` self-assign (independently inert, catalog debt); W3 ONE SHARED named intermediate `t` for all three deltas (30->22); T6 route the tail sum through `t` (22->21); U3 split the tail shift in place and load `cur` after it (21->19); D5 a DISTINCT local `dd` for the final delta (19->14).

- [s2] Residual at floor 14 has exactly three components (tools/pairdiff.py code6cac func_8001B748): (1) `sh zero,0(gp)` at ours idx 64 vs target idx 56 = 2 insns; (2) the `frac_s1 * 0x7D0` / `* 0x1F4` synth-mult chain running in $v0 instead of $v1 = ~10 insns; (3) the tail `lw a0,24(s0)` / `sra v1,v1,0xc` swap plus the missing maspsx load-delay nop = ~2 insns.

- [s2] MECHANISM IDENTIFIED for residual (2): the score is a function of how many times the arm destination variable is WRITTEN - 1 write = 14, 2 writes = 12, 3 writes = 12, 5 writes = 4 - while every spelling computes the identical value and GCC's synth_mult already emits that exact shift/add chain for `* 0x7D0`. That is the combine-foldable chain-extender that bumps reg_n_refs (the F1 family).

- [s2] The 5-write form reaches floor 4 and is banked at memory/grind/func_8001B748/rejected/reg-n-refs-chain-extender-arms-F1-unexhausted.c. It is deliberately NOT submitted: F1 is a FAKE-annotated LAST RESORT requiring (a) a demonstrably spent modality ladder, (b) a named GCC pass, (c) the annotation. Prerequisite (a) fails outright - this is session 2, the ladder has barely started, and the honest floor dropped 18 points this session. It also fails the plain human-programmer test (nobody writes a five-statement shift chain to multiply by 2000). It is recorded only as a measured 10-insn upper bound on residual (2) and as the mechanism identification.

- [s2] Every construct in the in-place candidate sits in a FROZEN-list sanctioned family: variable reuse for codegen control (the shared `t`, reusing `target` across both if/else pairs), named-intermediate declaration order (`t`, `dd`), and split-init accumulation (`t = t >> 12;`). The session also DELETED three inherited constructs (`cur = new_var;`, `dy = cur;`, `inv_s1 = inv_s1;`), two of which were load-bearing debt costing 2 insns.

- [s2] Negative results banked so this session's search space is never re-walked: declaration position of `t` inert at all 12 slots; `D_800A3310 = 0;` statement position inert at slots 0-4 and regressive at 5-8; split-init of `new_var` regressive on all three bases; three distinct per-delta intermediates give 30 (only the shared one wins); hoisting `inv_s1 * 0x2EE0` into a local is catastrophic (33 / 36 / 58); tail addend order and arm multiply operand order both inert; distinct `cc` local and inline duplicated-read RMW for the tail load both inert; inverted condition 15 and ternary 14; every alternative holder for the arm value (t/dd/dx/dy/dz/v/new_var/fresh tg) scores 17-22 versus 14 for reusing `target`.

- [s2] TOOLING GOTCHA worth a session to anyone who hits it: tools/pairdiff.py diffs tmp/sandbox/<func>/<stem>.o against build/src/<stem>.o and silently reads a STALE sandbox object unless `sandbox --disable all` is re-run AFTER installing the edit. A bare `make` does not refresh it, and `make` always prints 'OK: bb2 matches!' for this function because the asmfix replace_with_asmfile rule substitutes target asm. Also: pairdiff wants `func_8001B748`, not `DispPracticeMenuTex_A`.

## s3 addendum (structural, 2026-08-19) — FLOOR 14 -> 2

**Headline: two sanctioned structural moves closed BOTH open register/scheduler residuals.
The remaining gap is a single 2-insn hunk: the `sh zero,0(gp)` placement.**

### Chassis at dispatch
src/code6cac.c held the s1 form (floor 32), NOT the s2 candidate — the driver had not
committed s2's edit. s2's `candidate.c` was re-installed and re-measured: **floor 14**,
230 built vs 231 target insns, `rules_dropped: 1`. Ledger floor confirmed, every s2 verdict
spendable.

### THE MECHANISM, read out of the compiler source and confirmed with the instrumented cc1
Residual (2) — the arm synth-mult chain living in `$v0` where target wants `$v1` — was
attributed EXACTLY, and the attribution kills the s2 "reg_n_refs / chain-extender" framing:

1. `tools/gcc-2.7.2/local-alloc.c:combine_regs` refuses to tie the chain temp to the
   destination pseudo because of this guard (verbatim):
   `/* Can't combine if SREG is not a register we can allocate.  */ || (sreg >= FIRST_PSEUDO_REGISTER && reg_qty[sreg] == -1)`
   `reg_qty[sreg] == -1` means "not local to this block". The arm-destination variable is
   assigned in the arm and read after the join, so it is ALWAYS multi-block and the tie can
   NEVER fire for any spelling that keeps a shared destination variable.
2. With the tie impossible, the chain is its own block-local quantity. `find_free_reg`
   (local-alloc.c:2135) scans hard regs in `reg_alloc_order`; **MIPS defines no
   `REG_ALLOC_ORDER`**, so the scan is plain 0,1,2,... and `$v0` (=2) wins whenever it is
   free and there is no suggestion.
3. Instrumented cc1 (`BB2_SUGG_DEBUG=1 tools/gcc-2.7.2/cc1`, log at
   `tmp/grind/func_8001B748/s3/sugg.log`) prints, for the true arm (block 14):
   `SUGGDBG-QTY func=func_8001B748 blk=14 qty=0 reg1=234 birth=2 death=10 refs=8 ... ncopysugg=0 nsugg=0 copysugg= sugg=`
   `SUGGDBG-FFR qty=0 class=1 mode=4 jts=0 acc=0 born=2 dead=10 used=0,1,26,...,67 first_used=<same>`
   `used` contains NEITHER 2 nor 3 — **both `$v0` and `$v1` were free and there were zero
   suggestions**, so `$v0` was taken purely by scan order. The s2 write-count gradient
   (1 write=14, 2=12, 3=12, 5 writes=4) is therefore NOT a `reg_n_refs` priority effect at
   all: each extra write moves one more chain step OUT of the block-local temp quantity and
   INTO the global destination pseudo, which global-alloc puts in `$v1`.

**Consequence for policy:** the F1 chain-extender form was never the mechanism this residual
needed; it was a crude way of emptying the local quantity. The real fix is structural and is
described next. The banked `reg-n-refs-chain-extender-arms-F1-unexhausted.c` stays rejected
and is now also SUPERSEDED — the honest form beats it (2 vs 4).

### Winning structural chain this session (each step measured with `sandbox --disable all`)

| step | change | floor |
|---|---|---|
| s2 candidate re-installed | — | 14 |
| **G** | duplicate the tail addend into BOTH arms: `if (use_high) t = (frac_s1*0x7D0) + (inv_s1*0x2EE0); else t = (frac_s1*0x1F4) + (inv_s1*0x2EE0);` and delete the shared `t = target + (inv_s1*0x2EE0);` | **7** |
| **L_t** | + the FIRST `use_high` if/else also writes the shared `t` instead of `target` (the arm-value hunk had simply moved one level up: ours `addiu v0,v0,128`, target `addiu v1,v0,128`) | **4** |
| **N** | + delete the now-unused `s32 target;` declaration (score-neutral, removes dead-local debt) | 4 |
| **T4** | + duplicate the `>> 12` into both arms too: `t = ((frac_s1*0x7D0) + (inv_s1*0x2EE0)) >> 12;` — this puts the `sra` ahead of the `lw a0,24(s0)` exactly as target has it, and buys the missing maspsx load-delay `nop`: **231 built insns, first time we match target's insn COUNT** | **2** |

`candidate.c` is the T4 form (floor **2**), in place in `src/code6cac.c` at end of session.
Why G works: `jump2` cross-jumping (post-reload) merges the identical duplicated tails back
into ONE copy, so the duplication costs nothing in the output while giving the arms their own
in-block destination for the chain.

### Residual at floor 2 — ONE hunk, 2 insns
`tools/pairdiff.py code6cac func_8001B748`:

    @@ ours[56:56] -> target[56:57]  (insert)     +  56  sh zero,0(gp)
    @@ ours[64:65] -> target[65:65]  (delete)     -  64  sh zero,0(gp)

`D_800A3310 = 0;` in the early-exit arm: target issues it at index 56 (immediately after
`mult t0,v0`), we issue it at 64 (after the `(frac_s1*0x9C4)+(inv_s1*0x2710)` synth-mult
chain). Everything else in the function is byte-identical, insn count matches (231/231).

### Measured negatives banked this session (do NOT re-run)
| lever | result |
|---|---|
| defer the consumer of the `func_8001A4F0` result past the arms, to keep hard `$v0` live across them (A/B/C) | 23 / 26 / 102 — and pairdiff CONFIRMS the arm chain stayed in `$v0`: the pseudo copy happens at the call, so hard `$v0` never lives across the arm block. The "make `$v0` busy" route is DEAD. |
| `t = target; t = t + (inv_s1*0x2EE0);` split accumulation | 14 (inert) |
| swap the `dst+0x14` / `dst+0x12` store order | 16 |
| ternary form of the second arm | 14 (inert) |
| `target = frac_s1*0x1F4; if (use_high) target = target*4;` (single chain + scale) | 21 @ 225 insns |
| merge the second arm's multiply into the FIRST `use_high` if/else (longer live range) | 123 @ 224 insns — catastrophic |
| duplicate the `- dst[0x10]` subtraction into the first if/else's arms | 13 @ 232 insns |
| holder sweep for the FIRST if/else value on the G base (`t`/`dx` = 4; `dd`/`dy`/`dz`/`v`/`new_var` = 7; `cur` = 41) | only `t` and `dx` win; `t` chosen (already the shared intermediate) |
| **85-variant single-statement-move sweep of the whole early-exit arm** (every statement to every legal position, `new_var` def-before-use enforced) on the T4 base | **minimum is 2, reached by 19 distinct orders; nothing below.** Statement order CANNOT move the `sh zero,0(gp)`. This supersedes and generalises s2's 9-slot gp-store sweep. |
| named intermediate for the `dst+0x18` early-exit store (`t = ...; *(dst+0x18) = t >> 12;`) | 9 |
| named intermediates for the first two early-exit stores | 6 |
| swap the addends of the `dst+0x18` early-exit expression | 12 |
| swap the multiply operand order there (`0x9C4 * frac_s1`) | 2 (inert) |
| delete `new_var` and spell the `dst+8` early-exit store like its siblings | 20 @ 232 insns — **`new_var` is load-bearing, not debt** |

### Constructs in the floor-2 candidate (all FROZEN-list sanctioned families)
- **duplicated statement into branch arms** — the `(inv_s1 * 0x2EE0)` addend and the `>> 12`
  appear in both arms; `jump2` cross-jumping merges them, so the output carries one copy.
- **variable reuse for codegen control** — one shared `t` carries all three delta
  intermediates, the first `use_high` value, and the tail sum.
- **named-intermediate declaration order** — `t`, `dd`.
There is no dead code, no dead store, no self-assign, no volatile, no `__asm__`, no register
pin, and the `s32 target;` declaration was DELETED once it became unused. Every construct is
something a human writing this interpolation routine would plausibly write.

### Tooling notes
- `tools/pairdiff.py` must be run through WSL (`bash tools/wsl.sh 'python3 tools/pairdiff.py ...'`);
  the Windows Python cannot find `objdump`. And it still reads a STALE sandbox object unless
  `sandbox --disable all` is re-run after installing the edit (s2's gotcha, re-confirmed).
- Variant harness for this function: `tmp/grind/func_8001B748/s3/install.py <body.c>` splices a
  body file into `src/code6cac.c` at the function's line span; `tmp/grind/func_8001B748/s3/run.ps1
  -Files <list>` installs + sandboxes each and prints `score`/`build_insns`. 100+ variants were
  measured this session at roughly 4 s each.
- Instrumented-cc1 recipe: `tmp/grind/func_8001B748/s3/sugg.sh` (cpp -> `BB2_SUGG_DEBUG=1
  tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -mel -mcpu=3000 -mips1 ...`). Grep the log for
  `func=func_8001B748` and the `SUGGDBG-FFR` line that FOLLOWS the block's `SUGGDBG-QTY` line.
  Do NOT `grep -A3` the whole log — the FFR lines are enormous.

- [s3] Honest floor THIS session moved 14 -> 2 (`sandbox func_8001B748 --disable all`, 231 built vs 231 target insns, rules_dropped 1). The floor-2 form is IN PLACE in src/code6cac.c and saved to memory/grind/func_8001B748/candidate.c.

- [s3] MECHANISM for the $v0/$v1 arm-chain residual, read out of tools/gcc-2.7.2/local-alloc.c and confirmed with the instrumented cc1: combine_regs cannot tie the synth-mult chain temp to the arm's destination pseudo because that pseudo is multi-block (`reg_qty[sreg] == -1` guard), so the chain is its own block-local quantity; MIPS defines no REG_ALLOC_ORDER, so find_free_reg takes $v0 by plain scan order. BB2_SUGG_DEBUG shows blk=14 qty=0 with ncopysugg=0 nsugg=0 and `used` containing neither 2 nor 3 — both registers free, zero suggestions.

- [s3] That attribution RETRACTS s2's characterisation of the write-count gradient as a reg_n_refs / F1 chain-extender effect. Extra writes did not raise an allocno priority; they moved chain steps out of the block-local temp quantity into the global destination pseudo. The honest structural fix (duplicate the addend into both arms) beats the 5-write F1 form outright: floor 2 vs floor 4.

- [s3] Duplicating the tail addend AND the `>> 12` into both use_high arms is free in the output because jump2 cross-jumping (post-reload) merges the identical tails back into one copy. This is the sanctioned duplicated-statement-into-arms family and it closed BOTH the arm-register residual (14->7) and the tail lw/sra + missing-nop residual (4->2, and it is what finally made build_insns 231 == target 231).

- [s3] The first use_high if/else had the SAME divergence one level up (ours `addiu v0,v0,128`, target `addiu v1,v0,128`); routing its value through the shared `t` instead of a dedicated `target` closed it (7->4) and made `s32 target;` dead, so the declaration was deleted.

- [s3] The "make hard $v0 live across the arm block" route is DEAD: deferring the consumer of the func_8001A4F0 return value past the arms scores 23/26/102 and pairdiff confirms the chain stayed in $v0 — the hard-reg copy happens at the call, so hard $v0 never spans the arm block.

- [s3] The remaining 2-insn hunk is `sh zero,0(gp)` (`D_800A3310 = 0;`) at ours index 64 vs target 56 in the early-exit arm. An 85-variant sweep moving EVERY statement of that arm to EVERY legal position bottoms out at 2 (19 distinct orders tie), so source statement order cannot move it. Expression-shape variants for the same block (named intermediates, addend swap, operand swap, dropping new_var) score 9 / 6 / 12 / 2 / 20 — none better.

- [s3] `new_var` in the early-exit arm is LOAD-BEARING, not catalog debt: deleting it and spelling the `dst+8` store like its siblings costs 18 points (2 -> 20, 232 insns).

- [s3] Chassis at dispatch: src/code6cac.c held the s1 form (floor 32), not the s2 candidate; re-installing memory/grind/func_8001B748/candidate.c re-measured the ledger floor of 14 exactly (230 built vs 231 target insns, rules_dropped 1).

- [s3] Honest floor THIS session: 2 (sandbox func_8001B748 --disable all), with 231 built insns == 231 target insns for the first time in this function's history. The floor-2 form is in place in src/code6cac.c and saved to memory/grind/func_8001B748/candidate.c.

- [s3] Winning chain, each step measured: G duplicate the tail addend into both use_high arms (14 -> 7); L_t route the first use_high if/else through the shared t (7 -> 4); N delete the now-unused 's32 target;' declaration (4, score-neutral); T4 duplicate the >>12 into both arms (4 -> 2, and 230 -> 231 insns).

- [s3] MECHANISM for the $v0/$v1 arm-chain residual, read out of tools/gcc-2.7.2/local-alloc.c and confirmed with the instrumented cc1: combine_regs cannot tie the chain temp to a multi-block destination pseudo (reg_qty[sreg] == -1 guard), so the chain is its own block-local quantity, and with MIPS defining no REG_ALLOC_ORDER find_free_reg takes $v0 by plain scan order. BB2_SUGG_DEBUG confirms ncopysugg=0, nsugg=0 and both $v0 and $v1 free in 'used'.

- [s3] s2's characterisation of the write-count gradient as a reg_n_refs / F1 chain-extender effect is RETRACTED. The honest structural form beats the banked 5-write F1 form outright (floor 2 vs floor 4), so that rejected form is now superseded as well as unsanctioned.

- [s3] The duplication is free in the output because jump2 cross-jumping (post-reload) merges the identical duplicated tails back into a single copy - the arms gain their own in-block destination without paying for a second chain.

- [s3] Remaining residual is ONE hunk of 2 insns: sh zero,0(gp) (D_800A3310 = 0;) issued at our index 64 vs target 56 in the early-exit arm, i.e. target places it immediately after 'mult t0,v0' and ahead of the (frac_s1*0x9C4)+(inv_s1*0x2710) synth-mult chain. Everything else in the function is byte-identical.

- [s3] Dead axes banked so they are never re-walked: deferring the call-result consumer to free $v0 (23/26/102, chain confirmed still in $v0); t = target then t += ... split accumulation (14, inert); dst+0x14 / dst+0x12 store swap (16); ternary second arm (14); single chain plus scale-in-arm (21 @ 225); merging the second arm into the first if/else (123 @ 224); duplicating the subtraction into the first if/else arms (13 @ 232); the exhaustive 85-variant statement-move sweep (min 2, 19 ties).

- [s3] new_var in the early-exit arm is LOAD-BEARING, not catalog debt: deleting it and spelling the dst+8 store like its siblings costs 18 points (2 -> 20, 232 insns).

- [s3] Constructs in the floor-2 candidate, all FROZEN-list sanctioned families: duplicated statement into branch arms; variable reuse for codegen control (one shared t); named-intermediate declaration order (t, dd). No dead code, no dead store, no self-assign, no volatile, no __asm__, no register pin; the dead 's32 target;' declaration was removed.

- [s3] Tooling: tools/pairdiff.py must be run under WSL (bash tools/wsl.sh) because the Windows Python cannot find objdump, and it still reads a stale sandbox object unless sandbox --disable all is re-run after installing the edit.

## == s4 (permuter modality) ==

- **Chassis re-measured at session start: honest floor 2 @ 231/231 insns**, with `memory/grind/func_8001B748/candidate.c` applied verbatim to `src/code6cac.c` (the file had drifted back to an older s2-era body carrying `inv_s1 = inv_s1;` and a `cur`/`dy` relay; the s3 candidate replaced it). `sandbox func_8001B748 --disable all` = `{"score": 2, "target_insns": 231, "build_insns": 231}`.

- **A STANDALONE permuter workspace reproduces the full-TU residual exactly.** `tmp/grind/func_8001B748/s4/setup_ws.sh <ws> <base.c>` builds target.o from `asm/funcs/func_8001B748.s` + `tools/decomp-permuter/prelude.inc` and writes the current-pipeline `compile.sh` (cc1 with `-mel`, prologue_fix, maspsx 2.34 with all seven gate lists, multu_pad). base.c is 10 lines of typedefs + four externs (`D_800A3310` u16, `D_800A3748` s8, `D_80101EC8` u8, `func_8001A4F0`) + the candidate body. Validation: objdump-diff of the compiled base against target.o = 231 vs 231 insns and exactly two differing lines, the `sh zero,0(gp)` displacement — identical to the engine sandbox's score of 2. This is a REUSABLE fast harness: `tmp/grind/func_8001B748/s4/score.py <body.c>...` scores an arbitrary body in ~2 s with the same metric as `sandbox --disable all`, without a full-TU build.
  - Gotcha for the next session: `compile.sh` `cd`s to the repo root, so it must be invoked with repo-root-relative paths (`./tmp/perm_.../compile.sh tmp/.../base.c x out.o`), not from inside the workspace directory.
  - `tmp/grind/func_8001B748/run_dump.sh` (the s3 dump script) is currently BROKEN on the full TU: `src/code6cac.c:780/1096: parse error before 'GameObj'` under its CPP defines. Dump against the standalone base.c instead — `tools/gcc-2.7.2/build/cc1 ... -da -dumpbase tmp/grind/func_8001B748/s4/dumps/b tmp/grind/func_8001B748/s4/base.c` — which is validated byte-equivalent for this function.

- **Permuter is measured DEAD on this residual.** Three fresh-seed campaigns, ~45 min and ~35k iterations total, all at permuter base_score 60 (= exactly one reordering at the permuter's weighting): chassis A (floor-2 candidate, 22 min, 11 novel outputs), chassis B (if/else respelling, 18.5 min, 5 outputs), chassis C (a drifted equal-score output of A re-seeded, 3.7 min / 11,130 iterations, 0 outputs). **Every single find scored exactly 60 — no improvement of any size.** The permuter does relocate `D_800A3310 = 0;` freely (chassis A's output-60-11 hoists it to the first statement of the arm) with no effect on the emitted store position, independently corroborating s3's 85-variant hand sweep.

- **Control-flow spelling of the early-exit arm is INERT.** if/else (no early return), `do { ... } while (0)` wrap, `goto end` with a shared end label, and an inverted `goto cont` guard ALL measure score=2 @ 231 insns — byte-identical output to the candidate. s3 frontier hypothesis #2 (change which basic block the store lives in via control-flow) is retired.

- **PASS ATTRIBUTION, now exact: SCHED2, sched.c:2461, insn 116 vs insn 140, LUID tie-break.** The store is insn UID 116; the last insn of the 0x9C4/0x2710 synth-mult chain is UID 140. In `.greg` — sched2's INPUT — insn 116 is already in TARGET's position (`531 mflo -> 116 sh zero,gp -> 119 sh18 -> 122/124 sh16 -> 127 sh20 -> 131..140 chain`). `.sched2` re-emits it as `131..140, 109, 116, 119, 531, 127`. So sched2 alone creates the divergence; sched1, local-alloc and global-alloc all deliver target's order. `BB2_RANK_DEBUG=1 tools/gcc-2.7.2/cc1` (the INSTRUMENTED cc1 — not `build/cc1`) yields a 145-line rank log containing exactly ONE decision touching the store: `RANKDBG last=119 y=116 cls=3 x=140 cls2=3 val=0`. INSN_PRIORITY is equal (the comparator returned 0 before reaching the class test) and the class is 3/3 (both independent of / latency-1 from `last_scheduled_insn` 119), so the whole divergence rests on `INSN_LUID (116) - INSN_LUID (140)` at `tools/gcc-2.7.2/sched.c:2461`. sched.c schedules the block BACKWARDS, so 116 winning the ready pick is exactly what sinks it below the chain in the final stream.

- **The obvious LUID lever does not exist at source level.** Computing the 0x9C4/0x2710 expression into an existing local earlier in the arm (six variants: `t` at top / before the gp store / after it, plus `dd`, `cur`, `dx` at top) scores 8/8/8/18/12/8 @ 231 insns. Pairdiff of the best shows the `sh zero,0(gp)` hunk COMPLETELY UNCHANGED plus four new divergences in the dst+0x10/0x12 region. sched1 normalises the chain back below the three halfword stores regardless of source position, so `.greg` order (and hence sched2's LUIDs) never changes.

- [s4] Chassis re-measured at session start: honest floor 2 @ 231 built insns vs 231 target insns, after applying memory/grind/func_8001B748/candidate.c verbatim to src/code6cac.c. src had drifted back to an older s2-era body (carrying `inv_s1 = inv_s1;` and a cur/dy relay); the s3 candidate replaced it and is what src/ holds now.

- [s4] A STANDALONE permuter workspace reproduces the full-TU sandbox residual EXACTLY: 10 lines of typedefs + four externs (D_800A3310 u16, D_800A3748 s8, D_80101EC8 u8, func_8001A4F0) + the candidate body, compiled through the current pipeline (cc1 -mel, prologue_fix, maspsx 2.34 with all seven gate lists, multu_pad), objdump-diffs against target.o at 231 vs 231 insns with exactly two differing lines. tmp/grind/func_8001B748/s4/setup_ws.sh builds such a workspace for any base.c; tmp/grind/func_8001B748/s4/score.py scores an arbitrary body in ~2 s with the same metric as `sandbox --disable all`. REUSE THIS - it is roughly two orders of magnitude faster than a full-TU sandbox run.

- [s4] Harness gotcha: the generated compile.sh cd's to the repo root, so it must be invoked with repo-root-relative paths (./tmp/perm_X/compile.sh tmp/.../base.c x out.o), never from inside the workspace directory.

- [s4] tmp/grind/func_8001B748/run_dump.sh (the s3 full-TU dump script) is currently BROKEN: `src/code6cac.c:780/1096: parse error before 'GameObj'` under its CPP defines. Dump against the standalone base.c instead (validated byte-equivalent for this function): tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -da -dumpbase tmp/grind/func_8001B748/s4/dumps/b tmp/grind/func_8001B748/s4/base.c.

- [s4] Permuter is measured DEAD on this residual: 3 fresh-seed campaigns, ~45 min, ~35k iterations, base_score 60 on all three, 16 novel outputs total, every single one scoring exactly 60. All three harvested with --stop; `permuter_campaign.py status` reports 0 live campaigns and 0 stale registry entries at session end.

- [s4] The permuter freely relocates `D_800A3310 = 0;` within the arm (chassis A output-60-11 makes it the first statement) with no effect on the emitted `sh zero,0(gp)` position - an independent random-search confirmation of s3's exhaustive 85-order hand sweep.

- [s4] Control-flow spelling of the early-exit arm is inert: if/else, do{...}while(0), goto-end, and inverted-guard all score 2 @ 231 insns, byte-identical to the candidate. s3 frontier hypothesis #2 is retired.

- [s4] EXACT DECISION POINT: tools/gcc-2.7.2/sched.c:2461, sched2 pass, ready-list comparison between insn UID 116 (sh zero,0(gp)) and insn UID 140 (the last insn of the 0x9C4/0x2710 synth-mult chain), with last_scheduled_insn = 119. INSN_PRIORITY equal, rank class 3 vs 3, so the result is INSN_LUID(116) - INSN_LUID(140) < 0. sched.c schedules backwards, so 116 wins the pick and is emitted after the chain.

- [s4] sched2's INPUT (.greg) already holds TARGET's order for the store - 531 mflo, 116 sh zero,gp, 119 sh18, 122/124 sh16, 127 sh20, then 131..140. Everything upstream of sched2 (sched1, local-alloc, global-alloc) is therefore already correct for this hunk; only sched2 diverges.

- [s4] The obvious source-level LUID lever does not exist: six early-computation variants of the 0x9C4/0x2710 expression score 8/8/8/18/12/8 @ 231 insns, and the best one leaves the sh zero,0(gp) hunk completely unchanged while adding four new divergences elsewhere. sched1 normalises the chain back below the three halfword stores no matter where the C source computes it.
