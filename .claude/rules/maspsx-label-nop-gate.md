---
name: maspsx-label-nop-gate
paths: ["tools/maspsx/**"]
description: "RETIRED 2026-09-14. The maspsx .L-label load-delay nop is now emitted GLOBALLY with the same $at/$gp expansion guards the non-label path always had; maspsx_label_nop_funcs.txt and --label-nop-funcs are DELETED. The per-function scoping never protected ASPSX fidelity — it contained a missing-guard bug in the .L arm. Do NOT re-introduce a per-function gate for this."
metadata:
  type: reference
---

# maspsx `.L`-label load-delay nop — RETIRED (mechanism deleted 2026-09-14)

> **This rule is now a retirement record.** There is no
> `maspsx_label_nop_funcs.txt`, no `--label-nop-funcs` flag, and nothing to opt
> into. If a function looks like it is missing a load-delay nop across a `.L`
> label, the gate already fires for it — the bug is elsewhere. **Do not re-add a
> per-function list.**

## What the mechanism was

maspsx's `is_label()` matches `$L`-prefix locals, but this GCC fork emits `.L`,
so a load whose result is consumed across a `.L` merge label wasn't seen as a
load-delay hazard and lost its nop. The target has that nop (ASPSX emitted it).
The fork handled `.L` in a dedicated block with two arms:

- **jalr-consumer** (`lw $rN; .L:; jalr $rN`) — always global, never gated.
- **load / branch / store-value consumer** — gated per function by
  `maspsx_label_nop_funcs.txt` (9 entries at retirement: SpuFree, CD_getsector,
  func_80060E04, func_80040594, _spu_init, func_80027640, func_80022F34,
  func_8007526C, _spu_gcSPU).

## Why it was per-function — and why that reason was wrong

The stated reason (this rule's previous text and [[maspsx-gate-lists]] endgame
item 1) was that emitting the nop shifts maspsx output indices, breaking other
functions' **index-anchored regfix/asmfix rules** (the `saTan4FireDisp .L279
already defined` cascade). That justification retired with the rule system
itself — `regfix.txt` / `asmfix.txt` absent since 2026-08-25
([[rules-to-zero-complete]]).

**Tested 2026-09-14.** Removing the per-function condition alone → **oracle
RED** (`912917be…`). Exactly one object changed (`code6cac_c_ab.o`) and exactly
one function gained an instruction: `func_8003ACB8`, +1 spurious nop. The other
7 "changed" functions in that object were pure downstream address shifts
(identical instruction counts, branch targets +4).

The site, and the actual bug:

```
lh  $v0, SYM          # load into $v0
.L<n>:                # merge label
sh  $v0, SYM2         # consumer -- expands to `lui $at,%hi; sh $v0,%lo($at)`
```

`line_loads_from_reg` returns True because `$v0` is the store's **value**
operand (the "a bit of a lie" case). But `sh $v0,SYM2` expands through `$at`,
and the generated `lui $at` **fills the load-delay slot by itself** — no nop is
required, and ASPSX emitted none.

The non-label path already knew this: it guards with `uses_at()` / `_uses_gp()` /
`nop_at_expansion`. **The `.L` arm never applied those guards** and emitted
unconditionally. That was harmless only because per-function scoping kept it to
9 functions, none of which had an `$at`-expanding consumer. The list was
containing a bug, not protecting fidelity.

## The fix that landed

Mirror the non-label path's guards into the `.L` arm, then make the arm global:

```python
if line_loads_from_reg(after_label, r_dest):
    nop_required = False
    if not uses_at(after_label):                        nop_required = True
    if self._uses_gp(after_label):                      nop_required = True
    if uses_at(after_label) and self.nop_at_expansion:  nop_required = True
    if nop_required:
        ...keep the merge label, emit the nop...
```

Verified: oracle GREEN (`62efab4f…`) with the arm global **and** all 9 entries
removed **and** the list file, CLI flag, Makefile/buildconfig plumbing and
engine bookkeeping deleted. maspsx's own suite: 135 tests, the same 2
pre-existing failures before and after (unrelated `ori`/`addiu` immediate
expansion for a different ASPSX version). `engine test` 379 passed / 0 failed.
`check_completion_integrity` gate-dependent completions 12 → 3.

## Evidence that ASPSX was uniform

A read-only survey of all 1,436 target functions (`tmp/label_nop_survey.py`,
2026-09-14) found **33** `load → label → consumer` sites in the original binary
and **zero** lacking the nop. Consumers vary widely (`lw`, `sh`, `sw`, `jalr`,
`beqz`, `andi`, `addu`, …) — what a mechanical assembler rule looks like.

**Caveat worth keeping:** that survey is *necessary but not sufficient*. It
measures the TARGET's shape, while the gate fires on OUR output's label
placement — a site can exist in our build with no counterpart label in the
target (splat only emits labels at branch targets). `func_8003ACB8` was exactly
that case, and the survey was structurally blind to it. A target-side census
can never clear a gate change on its own; only a full build against the oracle
can.

## If a load-delay nop looks wrong today

The gate is global. A missing or spurious nop across a `.L` label is a **maspsx
bug** to diagnose and fix in `tools/maspsx/` (bug-fix scope is permitted by
[[no-compiler-divergence]] §2) — never a new opt-in list. Adding a per-function
gate to paper over it re-creates exactly the failure this retirement fixed.

## Related
- [[maspsx-gate-lists]] — the four remaining gate lists and their adjudication
- [[maspsx-is-label-dot-prefix]] — the underlying `$L` vs `.L` blind spot
- [[no-compiler-divergence]] — bug-fix scope on maspsx is allowed; global
  behavior changes need owner sign-off (given for this one, 2026-09-14)
- [[rules-to-zero-complete]] — the regfix/asmfix retirement that made the
  original per-function justification moot
