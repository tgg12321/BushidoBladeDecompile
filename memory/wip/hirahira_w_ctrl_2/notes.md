# hirahira_w_ctrl_2 — WIP (current state 2026-08-05, round 1 / orientation)

`src/text1a_c.c:407`, stem **`text1a_c`** (not `text1a`). HEAD baseline re-measured:
honest pure-C distance **58**, 63 rules, `ASM-SUSPECT`, **118 insns vs target 122**.
Measure: `wsl bash tmp/csz/d.sh hirahira_w_ctrl_2 text1a_c`.

Signature: a 3x3 rotation-matrix builder — six `Judge[]` sine-table reads, ten `mult`s,
nine `a1[N]` halfword stores. No calls, no loop.

## FIRST: the source carries a pre-existing cheat

`src/text1a_c.c:449`:
```c
cosA = (s16)*(volatile u16 *)(&Judge[((s16)angA + 0x400) & 0xFFF]);
```
`*(volatile T *)&global` on a game-state global is in the **expanded cheat catalog**
([[inline-asm-policy]], 2026-05-31) — a volatile coercion, not a type-level fact. It must be
gone before this function can be COMPLETED-C, so it is part of the work, not the baseline.
Note the sandbox's `strip_cheat_asm` does **not** remove it (it is C, not `__asm__`), so the
honest 58 is measured *with the cheat still helping*. Expect the true starting distance to be
worse once it is removed — measure that first next session.

The cast's *shape* is the lead, though: "read as `u16`, then cast to `s16`" is exactly
`lhu` + `sll 16` + `sra 16`, which is what target emits at several sites.

## The halfword-type residual is a genuine C-level split, NOT an assembler gate

Target's six `Judge` loads (`asm/funcs/hirahira_w_ctrl_2.s`) are **mixed**:

| form | count | registers |
|---|---|---|
| `lh $r, %lo(Judge)($at)` | **2** | `$t2`, `$t1` (the first two loads) |
| `lhu $r, %lo(Judge)($at)` + `sll $r,16` + `sra $r,16` | **4** | `$t3`, `$a3`, `$a0`, `$v1` |

**maspsx's `--expand-lh` cannot produce this mix.** It is all-or-nothing per function
(`tools/maspsx/maspsx/__init__.py:993-1005` — gated only by `expand_lh_func_set`, and
`--expand-lh-funcs` is not even wired in the Makefile; only `EXPAND_LH_FILES`, currently
empty, and `--expand-lb-funcs` exist). Gating this function would expand **all six**. So the
2/4 split has to come from two different C expressions reading `Judge`.

The six reads in source order are `sinB`, `sinC`, `sinA`, `cosB`, `cosC`, `cosA`. The split is
**not** explained by:
- the index expression — `sinA` shares `Judge[ang & 0xFFF]` with `sinB`/`sinC` yet is not one
  of the two `lh`s;
- the declared destination type — `sinB`/`sinC`/`sinA`/`cosB`/`cosC` are all `s16` and only
  `cosA` is `s32`, which is a 5/1 split, not 2/4.

**That is the open question for round 2: what distinguishes the two `lh` sites from the four
`lhu`+sign-extend sites?** Map each target load to its source read first (the register
rotation makes this non-obvious — do it by index-expression provenance, not by position),
then find the one type spelling that yields 2 and 4. `Judge` is declared `extern s16 Judge[]`
at `src/text1a_c.c:406`; if a retype is the answer it must clear all four prongs of
[[header-type-correction-from-use-sites]] against every `Judge` use site tree-wide.

## The rest of the residual

- **Frame: ours 56, target 48 — we reserve 8 bytes MORE.** This is the *reverse* of the
  phantom-slot case (we have surplus locals, not a missing spill), so the
  [[phantom-slot-frame-lever]] recipe applies in the opposite direction: find the live value
  we are keeping that target rematerialises. 27 locals are declared; several are single-use
  products that may not need to be named.
- **A register rotation** over the multiply cluster (`$a2`/`$t3`/`$t1`/`$t2` and
  `$a3`/`$t2`), i.e. RENAME 25 in the triage. Expect it to follow the frame once that is
  fixed — treat it as downstream, not as its own target.
- Triage row for reference: RENAME 25, MISS 17, EXTRA 13, FRAME 6; class MIXED
  (type 30%, RA 34%).

## Next

1. **Remove the volatile cheat and re-measure** — that is the honest baseline.
2. Map target's 6 `Judge` loads to the 6 source reads and explain the 2/4 `lh`/`lhu` split.
3. Then the −8 frame (surplus locals), then the rotation.
4. **Do NOT commit.** The owner runs the gate.
