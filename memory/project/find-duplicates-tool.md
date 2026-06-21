---
name: find-duplicates-tool
description: tools/find_duplicates.py — SOTN-style cross-function near-clone finder ported to BB2; writes tmp/duplicates.txt + tmp/duplicates_leads.txt (INCOMPLETE -> highest-similarity COMPLETED-C candidate) for jump-starting matching by analogy.
metadata:
  type: project
---

# `tools/find_duplicates.py` — cross-function near-clone finder

Port of [`sotn-decomp/tools/dups`](https://github.com/Xeeynamo/sotn-decomp/tree/master/tools/dups)
(Rust) to Python. BB2's tooling is Python-heavy and the algorithm is small
enough that a pure-Python implementation (with length-banded pruning and an
early-exit Levenshtein) runs the full 1419-function grid in ~18 s on the
project laptop, so no Rust dependency was worth taking on.

## Algorithm

For each per-function `.s` file under `asm/funcs/`:

1. Parse the third hex word on each splat instruction line (the
   little-endian instruction bytes, byte-swapped to MIPS big-endian-numeric).
2. Reduce the instruction stream to its **opcode-class key** — one byte per
   instruction = `(op >> 26) & 0x3F`, i.e. the MIPS primary opcode field.
   Operand differences (registers, immediates, branch targets, GP-rel
   offsets, `%hi`/`%lo` data references) are normalised out.
3. Compute pairwise Levenshtein similarity on the keys with a length-band
   filter (`min/max >= threshold` is a necessary condition for a hit) and
   row-min early exit when the distance budget is exceeded. Default
   threshold 0.85 (looser than SOTN's 0.90 — we want leads, not just
   exact dups).

The trivial `jr $ra; nop` body is skipped (SOTN does the same).

## Outputs

Always under `tmp/` (root-cleanliness):

| Report | What it contains |
|---|---|
| `tmp/duplicates.txt` | All pairs at or above the threshold, sorted by descending similarity. Useful for spotting sibling clusters across the whole codebase. |
| `tmp/duplicates_leads.txt` | The actionable view for matching work — for every INCOMPLETE function (member of `engine/queue.json`), the single highest-similarity COMPLETED-C candidate (zero-rule, not in `inline_asm_canonical.txt`). "Here is a near-clone whose C is known — try its shape." |

Each row: `func_A ~= func_B (similarity 0.93, lens 42/44)`. Files have a
header recording the parameters used.

## Running

```powershell
# From the repo root (PowerShell). Pure Python in the WSL venv:
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && python3 tools/find_duplicates.py --threshold 0.85'
```

Flags:

- `--threshold <0..1>` — similarity cutoff (default 0.85)
- `--limit N` — cap rows per report (default 200)
- `--asm-dir <path>` — override `asm/funcs/`
- `--out-all` / `--out-leads` — override output paths
- `--quiet` — suppress progress on stderr

## Caveats

- The opcode-class key loses operand information, so it over-matches on
  short BIOS-trampoline-shaped bodies (2-3 insns). The all-pairs report
  is dominated by these in the long tail; the leads report side-steps
  most of them by requiring an INCOMPLETE/COMPLETED-C split.
- A 1.000-similarity pair across two glabels at the SAME VRAM is an
  alias, not a sibling — the matcher will (correctly) flag it as a copy
  candidate; the worker just renames.
- Read-only: never edits `src/`, `asm/`, or build inputs. Outputs are
  gitignored (`tmp/`).
