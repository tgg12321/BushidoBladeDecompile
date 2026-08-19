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
