# BB2 Decompilation Quality Audit — 2026-05-21

A from-scratch audit of decompilation **quality** (how each function matches),
computed directly from build artifacts + source, **not** from any cached
counts in memory files or `docs/STATUS.md`. Every number here is reproducible
from the snippet at the end.

## How this was measured

| Step | Source of truth |
|---|---|
| State | worktree `research-docs` @ `main` HEAD `e0ad2d7` |
| Build | `make` → **`OK: bb2 matches!`** (SHA1 `62efab4f…` = original `disc/SLUS_006.63`). Audit reflects a verified-matching binary. |
| Function set + C/asm attribution | ELF symbol tables of `build/src/*.o` via `mipsel-linux-gnu-readelf -sW` (typed `FUNC`), **not** source-text heuristics |
| regfix / asmfix / bridge sets | parsed from `regfix.txt`, `regfix_stage2.txt`, `asmfix.txt` (active, non-comment rules), keyed by function name |
| inline `__asm__` / register pins | `tools/classify_inline_asm.py` brace-depth parser, cross-checked with an independent pass; file-scope `__asm__("glabel …")` bodies counted separately |

## Key architectural finding (reframes "progress")

The link rule passes **no object files on the command line** — only `-T bb2.ld`.
`bb2.ld` enumerates inputs explicitly: it pulls `.text` **only** from
`build/src/*.o`, and pulls `build/asm/*.o` only for `.data`/`.rodata`/header.
The `asm/6CAC.s`, `text1b.s`, `text2.s` … text files (and `asm/funcs/*.s`) are
**reference-only and never linked.**

**Consequence: every function in the shipped binary is defined in a C source
file.** There is no "still in raw assembly, untouched" bucket. The real axis of
progress is the **quality** of each C definition — from pure C down to a bridge
that substitutes the original hand-disassembled bytes.

## Function universe

| Measure | Count |
|---|---:|
| `FUNC` symbols defined across linked `src/*.o` | 1,476 |
| – data labels typed `FUNC` (`D_*`/`jtbl_*`, excluded) | 17 |
| – address-suffixed alternate/duplicate entries (e.g. `name_8XXXXXXX`) | 68 |
| **= distinct primary functions (denominator below)** | **1,391** |
| (including alternate-entry symbols) | 1,459 |
| splat / `AGENTS.md` historical cite | 1,410 |
| `asm/funcs/*.s` reference files | 1,435 |

The 1,391 / 1,459 spread is entirely *how alternate entry points are counted*;
splat's 1,410 sits inside it. Percentages below are stable across the choice
(alternate entries are ~74% pure C, so they barely move the ratios).

## Quality partition (mutually exclusive, over 1,391 functions)

| Category | Count | Share |
|---|---:|---:|
| **Pure C** — 0 regfix, 0 asmfix, 0 pins, 0 inline asm | **778** | **55.9 %** |
| **Assisted C** — regfix / asmfix-surgical / register pins (no inline asm) | **230** | 16.5 % |
| **Inline `__asm__`** in body | **241** | 17.3 % |
| **Bridged** (`replace_with_asmfile` → original asm bytes) | **142** | 10.2 % |

A second framing of the same data:

| | Count | Share | Meaning |
|---|---:|---:|---|
| Genuine C logic (pure + assisted + C-with-asm-blocks) | 1,128 | 81.1 % | real decompiled C |
| Canonical hand-written asm (whole-function `glabel` bodies) | 121 | 8.7 % | original *was* asm — asm is the correct answer |
| Bridged (asm-backed, decomp incomplete) | 142 | 10.2 % | the genuine remaining "not yet C" work |

## The three buckets in detail

### 1. 100 % Pure C — **778 functions**
Clean C through the standard pipeline; appear in **none** of regfix, asmfix,
register-pin, or inline-asm sets (verified: 0 leakage).
*Caveat:* "pure" means no **per-function** cheat. These still pass through the
project's maspsx ASPSX-emulation pipeline — GP-relative addressing for ~308
functions (`sdata_funcs.txt`) and file-level `fix_lwl` / `multu_pad` /
`expand_lb` / rodata-align on a few files. That is faithful-toolchain behavior,
standard across PS1 decomps (SOTN, etc.), and is **not** counted as a cheat.

### 2. C with regfix / pins — **230 functions**
Real C bodies that match only with build-time assistance, no inline asm:

| Assistance (overlapping within the 230) | Funcs |
|---|---:|
| ≥1 active `regfix.txt` rule | 159 |
| register-asm pins (`register T x asm("$N")`) | 104 (65 with pins but no regfix) |
| asmfix surgical rules (label rename / splice) | 15 |

### 3. Inline `__asm__` — **241 functions**

| Sub-kind | Funcs |
|---|---:|
| whole-function asm body (`__asm__("glabel …")`) | 121 |
| in-function `__asm__` blocks | 120 |
| &nbsp;&nbsp;• canonical inline asm only (authentic GTE/BIOS/HW ops) | 10 |
| &nbsp;&nbsp;• mixed canonical + cheat-asm | 55 |
| &nbsp;&nbsp;• cheat-asm only (GPR scheduler/allocator workaround) | 55 |
| authorized in `inline_asm_canonical.txt` | 121 |

The project's own `classify_inline_asm.py` reports the **"SOTN gap"** —
non-canonical workaround debt — as **241 funcs / 1,411 cheat-asm instances**
(a different metric: cheat-asm `__asm__` blocks + register pins, excluding
canonical bodies; it coincidentally shares the 241 value).

### 4. Bridged — **142 functions**
`func: replace_with_asmfile "asm/funcs/func.s"` — the C body is compiled then
discarded, and the original hand-disassembled MIPS is substituted. 146 active
bridge rules total (142 → primary functions, 4 → alternate-entry symbols).
These are the clearest "remaining decomp work."

## Independent feature usage (overlapping counts)

| Feature | Functions |
|---|---:|
| active regfix rules | 210 |
| register-asm pins | 197 |
| asmfix surgical rules | 24 |
| bridged | 142 |
| any inline `__asm__` | 244 |

These overlap (a function can have regfix *and* inline asm). The mutually-
exclusive partition above assigns by worst-quality wins:
bridged > inline-asm > assisted > pure.

## Data-quality observations

- **3 functions have both an active bridge and an inline-`__asm__` body**
  (`SetPacketData`, `coli_HitPauseKatana_2`, `cpu_check_run_attack`). The bridge
  wins at build time, so they are counted as bridged; the inline-asm bodies are
  dead code — stale, removable.
- **1 canonical-list entry (`func_8008393C`)** doesn't resolve to a current
  function symbol (rename drift).
- `docs/STATUS.md` (2026-05-16) is stale: it cites ~148 bridged (now 142/146)
  and ~80 canonical inline-asm (now 121) and has no quality partition.

## Reproducing

```bash
# WSL, venv active, after `make` (needs build/src/*.o + build/bb2.elf):
python3 tools/decomp_quality_audit.py            # full partition
python3 tools/decomp_quality_audit.py --summary  # one-line headline
python3 tools/decomp_quality_audit.py --json     # machine-readable
python3 tools/classify_inline_asm.py             # inline-asm category detail
```

`tools/decomp_quality_audit.py` reads the compiled `build/src/*.o` symbol
tables (not source heuristics), so it reflects exactly what links into the
SHA1-matching binary. Re-run after any matching session to track movement of
the bridged → assisted → pure progression.
