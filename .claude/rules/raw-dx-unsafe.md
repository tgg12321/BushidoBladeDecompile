---
name: raw-dx-unsafe
paths: ["src/*.c", "named_syms.txt"]
description: "Promoting raw D_X identifiers in src/*.c to semantic names can cascade-break unrelated functions in GCC 2.7.2 + PsyQ — even when the linker resolves both names to the same address"
metadata:
  type: naming
---


After completing the placeholder-refinement addendum traces (recent_naming_findings.md §11-22), an attempt to "promote" raw `D_XXXXXXXX` references in src/*.c to their new semantic names produced **catastrophic cascade failures** in unrelated functions.

## What happened (2026-05-17)

Tried to promote 22 recently-renamed symbols (snd_data cluster, winanim cluster, motion_ex pool B, alarm cluster) from `D_X` form to semantic names across 5 src/*.c files (55 total replacements).

Result: **4376 instructions across 949 functions diverged from SHA1.**

Many of the diverged functions had NO direct connection to the renamed symbols — e.g., `bb2_memset`, `cdrom_BcdToFrames`, `stage_*`, `sys_*`, `camera_*`. Total binary size shrank by 292 bytes.

## Bisection result

Reduced to a single rename: `D_8008EB1C` → `g_winanim_event_subtable_eb1c` at one line in `src/code6cac.c:2746` (a single C source line change).

Still broke the build catastrophically. Even adding an explicit `extern s32 g_winanim_event_subtable_eb1c;` declaration before the use did not fix it.

`dc.sh verify func_800224E0` (the function containing the rename) showed 33 of 34 instructions different — the function was **completely re-generated** by GCC, not just shifted.

## Why this happens

The symbol `D_8008EB1C` has a `dlabel D_8008EB1C` definition in `asm/data/7D920.data.s`. Splat's data file emits the symbol as a global. My rename added `g_winanim_event_subtable_eb1c = 0x8008EB1C` to `named_syms.txt`.

Both names coexist in the linker symbol table pointing at the same address — **this is fine for linking**. But GCC 2.7.2 + PsyQ (`cc1psx`-equivalent) appears to make different codegen decisions based on symbol name *length* or *content*, which then triggers the well-known PsyQ "label drift" cascade: changes to `.L<N>` numbering in one function shift the entire `.text` layout, and unrelated functions diverge as a result.

This is consistent with the project's existing `reference/quick-reference.md` warnings about "label drift after match" and `dc.sh fix-label-drift` — small symbol changes propagate broadly.

## What's SAFE

- **Renaming in `named_syms.txt` only** is always safe. The linker resolves multiple names → same address with no codegen impact.
- **Renaming raw D_X refs in src/*.c IS safe IF the symbol has NO `dlabel` definition in `asm/data/`** (i.e., it's a pure BSS symbol with no splat-generated declaration). Example: `D_800A26D4` → `g_alarm_secondary_cb_ptr` succeeded (single rename in main.c, BSS-only symbol).
- **Adding `extern <type>` declarations in src/ for BSS-only symbols** is probably safe (untested but lower risk).

## What's NOT safe

- **Renaming any `D_X` in src/*.c that has a `dlabel D_X` in asm/data/**. The rename cascades into broad codegen divergence.
- **Adding `extern` typed declarations for symbols with `dlabel`s** — possibly same risk.
- **Function renames** (`func_XXXXXXXX` → semantic name) — likely same risk if the function definition or extern decl is in src/*.c. Untested.

## Procedure for safe promotion

Before promoting any `D_X` ref in src/:

```bash
grep "dlabel D_XXXXXXXX" asm/data/*.s
```

If a `dlabel` exists → **DO NOT PROMOTE**. Leave the source as `D_X`; the semantic name lives only in `named_syms.txt`.
If no `dlabel` → safe to promote. Verify with clean rebuild + SHA1 check.

For BSS-only symbols (no asm/data definition), the rename is symbol-only and the linker resolves identically.

## Implication for naming work

Naming work that touches src/*.c is **fundamentally more risky** than naming work that only touches `named_syms.txt`. The named_syms.txt approach (used throughout the placeholder-refinement sprint) is the correct minimum-risk path.

This explains why the project has historically kept the `D_X` form in src/ even after the symbol got a semantic name — the `D_X` form is build-stable, the semantic name is documentation-stable, and the linker reconciles both.

Future "raw-D_X promotion" passes should be opportunistic and per-symbol-checked against the `dlabel` test, not bulk.

## Cross-references

- [[naming-worktree]] — why naming work stays in a worktree
- [[quick-reference]] — label-drift and `fix-label-drift` machinery
- [[crlf-via-edit-write]] — another build-stability hazard
- `docs/engine/recent_naming_findings.md` §11-22 — the cluster traces that prompted the (failed) promotion attempt
