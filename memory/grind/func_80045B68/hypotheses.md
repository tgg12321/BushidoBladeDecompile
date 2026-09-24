# func_80045B68 — hypotheses tried and killed (manual lane, 2026-09-24)

Base for every row below is the banked `candidate.c` (score **22**, 302/302
instructions) unless stated. Scores are `engine sandbox --disable all`.

## The one open question (SOLVED 2026-09-24 — see "Killed" below)

`cnt` is the same variable as block 3's switch counter `n`. That merge made
`global_alloc` reach the merged pseudo before `hdr`, so `hdr` took `$s1` and
`arg1` kept `$s0` — the target's assignment. Distance 22 → 9, and register
allocation is now **exact** (0 operand-only hunks). The original analysis is
kept below because its numbers are what made the merge predictable.

### Superseded: why hdr outranked arg1 before the merge

`global_alloc` must reach **arg1/cnt before hdr**. It doesn't:

```
ALLOCDBG func_80045B68  ord=10 pseudo=87 hardreg=16 nrefs=9 livelen=24 pri=11250   <- hdr  -> $s0
ALLOCDBG func_80045B68  ord=15 pseudo=73 hardreg=17 nrefs=5 livelen=20 pri= 5000   <- arg1 -> $s1
```
(instrumented cc1 = `tools/gcc-2.7.2/cc1`, `BB2_ALLOC_DEBUG=1`; **not** the
build's `build/cc1` — see [[instrumented-cc1-location]]. Driver:
`tmp/f45b68/allocdbg.sh`.)

`allocno_compare` (tools/gcc-2.7.2/global.c) is
`floor_log2(n_refs) * n_refs / live_length * 10000`, ties broken by **lower
allocno number** — and arg1's pseudo (73) is lower than hdr's (87), so a tie
is a win. MIPS defines no `REG_ALLOC_ORDER`, so allocation walks registers
ascending and whoever is reached first takes `$s0`.

Numeric thresholds for the flip (any one suffices):

| change | value | result |
|---|---|---|
| hdr nrefs 9 → 6 (livelen 24) | 5000 | tie → arg1 wins on allocno |
| hdr nrefs 9 → 5 | 4166 | arg1 wins outright |
| hdr livelen 24 → ≥55 (nrefs 9) | ≤4909 | arg1 wins outright |
| arg1 nrefs 5 → 8 (livelen 20) | 12000 | arg1 wins outright |
| arg1 livelen 20 → ≤8 (nrefs 5) | ≥12500 | arg1 wins outright |

hdr's 9 refs are: 2 defs (`= p`, `= arg3`), 1 call arg, 3 subscripts
(`hdr[1]`, `hdr[0]`, `&hdr[cnt]` — `hdr[cnt-1]` is already folded to
`-4(&hdr[cnt])`), 3 additions. So a "base holder" for the three additions is
the only family that can reach 6, and only if CSE does not propagate the
holder back into `hdr`.

## Killed — measured neutral or worse

| Spelling | Score | Note |
|---|---|---|
| `hb = (s32)hdr` base holder for the three additions | 22 | CSE propagates the copy back; hdr's refs unchanged |
| `hdr` declared as `s32`, subscripts via `((s32 *)hdr)[k]` | 22 | same value ⇒ same pseudo |
| `hdr` declared first / last among locals | 22 / 22 | allocno numbering is not the tiebreak here — priorities do not tie |
| hdr selection as `if (arg3 == 0) … else …` | 25 | worse |
| hdr selection as straight-line `hdr = p; if (arg3) hdr = arg3;` | 22 | neutral |
| no `cnt` variable at all (CSE-derived `hdr[0]`) | 26 | worse |
| separate `cnt` local instead of reusing `arg1` (3 placements) | 26 | worse — the arg1 reuse is load-bearing, and `local_alloc` does **not** pick arg1 up even when its range is single-block |
| `cnt = hdr[0]` before vs after `last = …` | 22 / 22 | statement order does not drive the header load order; the scheduler does |
| fill loop as `i = 31; do {…} while (--i >= 0);` | 22 | neutral |
| block-5 init order `i = 0; y = 0;` | 23 | worse than `y = 0; i = 0;` |

## Killed earlier (superseded by the banked form)

- Block 2 with a plain `s16 *` deref (`*r != -2` / `*r >= 0`): emits **both**
  `lh` and `lhu` for the same address. The target emits one `lhu` whose
  `sll …,16` is shared by the `!= -2` compare and the sign test — that only
  happens through a `u16` holder variable. Worth 47 points.
- Block 2 walking `D_800993FC` with its own pointer: leaves the table base
  where the source put it. The target has it in the loop **preheader**, which
  is where `loop.c` strength reduction puts a giv's initial value — so the
  source indexes `D_800993FC[i]`.
- Block 2's walker as a *separate* variable from blocks 4/5's `sp18` walker:
  costs 30 points. It is one variable; the pseudo crosses `func_800433E4`, so
  it is callee-saved, which is the target's `move $s0,$fp`. Splitting it also
  moved every spill reload from `$t0` to `$t1`.

## Not yet tried

- **The frontier is item 1** (see below): find the C shape whose `dl` chain is
  no deeper than its `last` chain at sched1. Everything else in the function is
  byte-exact.
- decomp-permuter. Needs a workspace built against the **current** flags
  (`-mel -msoft-float`, `--prefill-label-funcs`); `tools/mar_perm_workspace.sh`
  is a stale per-function example and must not be copied verbatim. Note the
  residual is a *scheduling* difference, so the permuter has to find a
  structural restatement, not a register nudge.

The "base holder" family listed here earlier (`tmp/f45b68/vB*.c`) is **moot** —
it existed only to lower `hdr`'s allocation priority, and the `cnt`/`n` merge
solved that outright.

---

# Residual at 9 — the three placement items (2026-09-24)

All three are one instruction in a different slot of the *same* basic block.
No register differs anywhere in the function.

## Item 1 (5 of the 9 points) — ROOT-CAUSED, not closable by statement order

Target loads `hdr[1]` before `hdr[0]`; we do the reverse, and the following
four instructions mirror accordingly. Everything from the `lw hdr[n]` on is
identical.

Per-pass RTL dumps (`tmp/f45b68/rtldump.sh`, cc1 `-da`) locate the decision
exactly:

```
rtl       50:LOAD hdr[1] | 51:srl | 52:sll | 54:addu | 57:LOAD hdr[0] | 61:sll | 63:addu | 65:LOAD hdr[n] ...
combine   50:LOAD hdr[1] | 51:srl | 52:sll | 54:addu | 57:LOAD hdr[0] | 61:sll | 63:addu | 65:LOAD hdr[n] ...
sched     57:LOAD hdr[0] | 61:sll | 63:addu | 65:LOAD hdr[n] | 66:srl | 67:sll | 50:LOAD hdr[1] | 69:addu ...
```

- `rtl`/`combine` preserve **source order** — expand is not the problem.
- **`sched1` reorders**, hoisting the whole `hdr[0]`→`dl` chain ahead of the
  `hdr[1]`→`last` chain. `sched2` then interleaves them, and whichever chain
  `sched1` put first leads in the final code.

`sched1` picks by `INSN_PRIORITY`, which in GCC 2.7.2 (`tools/gcc-2.7.2/sched.c`,
`priority()`) walks **`LOG_LINKS`** — i.e. depth from the block start, not to
the end. The two chains are `lw→sll→addu→lw→srl→sll→addu` (7 deep, `dl`) and
`lw→srl→sll→addu` (4 deep, `last`), so the `dl` chain wins on depth. That is a
property of the *dependency graph*, not of statement order — which is why the
order is invariant under every spelling measured:

| spelling | result |
|---|---|
| `last` (hdr[1]) assigned first | hdr[0] emitted first |
| `n = hdr[0]` assigned first | hdr[0] emitted first |
| `dl` assigned before `last` | hdr[0] emitted first |
| named temp for `hdr[1]`, read after `hdr[0]` | hdr[0] emitted first |
| named temp for `hdr[n]`, read before `hdr[1]` | hdr[0] emitted first |
| both reads into temps, either order | hdr[0] emitted first |
| `n = *hdr` spelling | hdr[0] emitted first |
| `hdr[1]` read through a second pointer | hdr[0] emitted first |
| `hdr[1]` read last of all (vF1) | **47** — badly worse, +3 insns |

**Consequence:** to flip this, the `last` chain must be at least as deep as the
`dl` chain at sched1 time. With the emitted instruction count already exact at
302/302, no extra dependent operation can be added. So the target's C for this
block is structurally different from ours in a way not yet identified — most
likely `dl`'s dependence on `n` is shorter there, or the block is split. That
is the frontier; it is **not** a register or scheduling-lever problem.

## Item 2 (2 points) — closable only with a flagged construct

The fill value `-1` is a loop invariant that `loop.c move_movables` hoists via
`emit_insn_before(…, loop_start)`, which lands it **after** `i = 31`. The
target has it **before**, so there it was never a movable.

A named local (`s16 fill = -1;` before the loop) reproduces the target exactly
and scores **7** (`tmp/f45b68/vF3.c`). That is a *constant-holder* — the
`func_80041988` class (`.claude/rules/no-new-park-categories.md`,
named-local-fake-exception, SOTN `s16 three = 3;`). **Deliberately NOT adopted:**
it does not reach 0, and adding a construct that needs an owner-cited
justification for a partial improvement is strictly worse than banking honestly.
It should only be revisited as part of a form that actually byte-matches.

Measured neutral: `do {…} while (--i >= 0)`, `while (i >= 0) {… i--;}`,
descending pointer walk.

## Item 3 (2 points)

Target's `y = 0` sits in the loop preheader **after** the four hoisted `li`
constants; ours sits before the guard test. Writing the loop with an explicit
guard (`if (*q != -2) { y = 0; do {…} while (…); }`, `tmp/f45b68/vDc.c`) moves
it from slot 196 to 200 — closer, still not 204, and the score does not change.
Landing after the movables means the insn was inserted *after* `move_movables`
ran, which in `loop.c` is what `strength_reduce`'s `emit_iv_add_mult` does for
a giv's initial value. But `y` increments **conditionally** (only for
non-negative entries), so it cannot be a giv of this loop — that route is
closed, and the mechanism is still unexplained.

Measured neutral: `y = 0` before/after `q = sp18`, before/after `i = 0`.
