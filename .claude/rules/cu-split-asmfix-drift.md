---
name: cu-split-asmfix-drift
paths: ["asmfix.txt", "src/text1b.c"]
description: "CU splits break asmfix rules whose `rename \\".L<N>\\" \\".L<addr>\\"` patterns depend on GCC's monolithic-CU codegen. GCC re-emits different code (not just renumbered labels) when a function compiles in a smaller CU."
metadata:
  type: recipe
---


When splitting a monolithic src/*.c into two CUs, asmfix rules of the form
`rename ".L<N>" ".L<8XXXXXXX>"` are NOT just suffering label renumbering —
GCC actually emits **different machine code** for the affected functions
when their CU shrinks.

**Why:** A new CU means a different `.text` section layout, different
file-scope symbol set, and potentially different register-allocation
decisions in GCC 2.7.2. For some functions (typically those with
heavy register pressure or complex scheduling), this means the function
size changes by ±4 bytes. Labels then aren't where the asmfix rule's
address-encoded targets expect them.

**Detected 2026-05-17 (text1b.c at 0x8005B43C):** `cu_split_analyzer.py`
correctly identified the strongest cohesion boundary (depth 78%, exactly
matching SUBSYSTEM_MAP_2026-05-12.md). Mechanical split (text1b_a2.c)
linked after `fix-label-drift`, but binary was +8 bytes. Two functions —
`func_8006BD28` and `func_80070C70` — each grew by +4 bytes because GCC's
codegen shifted, and the precision asmfix `delete_between`/`insert_before`
rules anchored to address-encoded labels couldn't find their anchors.

`tools/fix_asmfix_label_drift.py` (built during the attempt) tries to
remap labels by byte offset within the function. For func_80070C70 it
matched 1 of 2 anchors. For func_8006BD28 it matched 0 of 7 — because the
underlying codegen itself shifted, the byte offsets no longer match.

**Why:** HISTORY.md `2026-05-15` records two prior CU sandwich split
attempts (func_80077B30 / func_8006B578) reverted same-day for sibling
regressions — almost certainly the same root cause.

**How to apply:**

1. Before attempting a CU split, audit asmfix.txt for rules in the proposed
   "after" CU that use `rename ".L<N>" ".L<8XXXXXXX>"` patterns. Each such
   rule is a risk: the function might compile to a different size in the
   new CU.

2. For the split to work safely, **either**:
   - Convert affected asmfix rules to be codegen-invariant (use regex
     patterns matching instruction shape, not label name)
   - OR: pick a split point where the post-split CU has **zero**
     address-encoded asmfix renames in the functions it contains
   - OR: be prepared to re-derive those asmfix rules from scratch using
     the new GCC output

3. `cu_split_analyzer.py` now loads `asmfix.txt` + `regfix.txt` and
   annotates every candidate split with a **fragility score** per side
   (asmfix label rule = 10pt, asmfix slice anchor = 5pt, regfix hardcoded
   `.L<N>` = 1pt). A split is **SAFE** only when both sides score 0. Top
   cohesion candidates often score >0; that's the early warning.

4. The build's `+N bytes` size MISMATCH (where N is a small even number like
   ±4, ±8) after a clean structural split is the signature of this trap.
   `dc.sh fix-asmfix-drift` doesn't detect it because rename rules silently
   no-op when their pattern doesn't match.

## text1b.c specifically (2026-05-17 analysis)

**Four fragile-rule functions block every cohesion-aligned split of text1b.c.**
Pre-requisite for any cohesion-driven split: retire these in pure C.

| Function | Address | Fragility score | Notes |
|---|---|---:|---|
| `func_80056CB8` | 0x80056CB8 | 30 | 2 asmfix label rules + 2 slice anchors |
| `func_80069AE4` | 0x80069AE4 | 1 | Just one regfix `.L825` hardcoded label |
| `func_8006BD28` | 0x8006BD28 | **80** | 7 asmfix label rules + 2 slice anchors + 28 regfix rules — the worst |
| `func_80070C70` | 0x80070C70 | 30 | 2 asmfix label rules + 2 slice anchors |

These occupy the file's middle. Fragility-free regions are:
- `0x80047ED0 – 0x80056CB8` (59 KB head, ~120 funcs)
- `0x80070C70 – 0x80077B30` (27 KB tail, ~30 funcs)

The 4 strongest cohesion candidates (`0x80052C28`, `0x80055948`, `0x8005B43C`,
`0x8005C614`) ALL fall inside the fragile zone. **None is SAFE.**

A "late split" (somewhere in the 27 KB tail region) is the lowest-risk test
of the split mechanism, but the organizational win is small (text1b.c keeps
~94% of its functions). Don't bother unless validating the mechanism for a
future larger split is the goal.

## Project-direction note

Per [[minimize-regfix]] and [[community-standard]],
the project goal is to reduce regfix/asmfix toward zero. CU splits that
**add new fragile-rule patterns** (option A from the 2026-05-17 conversation —
regenerating asmfix rules with asmfix_slice.py for the new CU) work tactically
but entrench debt. **Option D (retire the bridge first, then split)** aligns
with project direction. **Option C (stay monolithic) is a defensible default**
until the regfix/asmfix burden shrinks broadly.

**Related:**
- [[cu-split-evidence-required]] — CU split without binary evidence is a
  workaround
- [[minimize-regfix]] — regfix/asmfix is debt
- [[community-standard]] — community zero-regfix bar
- [[naming-worktree]] — structural changes vs naming
