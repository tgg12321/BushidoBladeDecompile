# func_80045B68 — hypotheses tried and killed (manual lane, 2026-09-24)

Base for every row below is the banked `candidate.c` (score **22**, 302/302
instructions) unless stated. Scores are `engine sandbox --disable all`.

## The one open question

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

- "Base holder" variants where the holder is a variable CSE cannot fold away
  because it is independently live (`prev`/`last` doubling as the byte base,
  or a holder assigned **before** the `func_80044F50` call so the call stops
  referencing `hdr` — that is the only spelling that reaches 6 refs).
  Generated as `tmp/f45b68/vB{a,b,c,d}.c`, unmeasured.
- decomp-permuter. Needs a workspace built against the **current** flags
  (`-mel -msoft-float`, `--prefill-label-funcs`); `tools/mar_perm_workspace.sh`
  is a stale per-function example and must not be copied verbatim.
