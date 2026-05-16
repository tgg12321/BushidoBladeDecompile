# Methodology

How `tools/propose_function_names.py` works.

## Inputs

- `asm/funcs/*.s` -- per-function disassembly (one file each)
- `named_syms.txt`, `symbol_addrs.txt` -- existing name -> addr table; anchors for call-graph and address-neighborhood evidence
- `kengo_matches.csv` -- output of `tools/kengo_match.py` (Kengo cross-reference)
- `kengo_name_decisions.csv` -- reviewed decisions (rename / demote / keep)
- `known_psyq_stdlib.txt` -- auto-detected PsyQ stdlib functions
- `undefined_syms_auto.txt` -- splat data symbols
- `asm/data/*.rodata*.s` -- string constants

## Evidence kinds

For each unnamed function the analyzer accumulates evidence rows; each row has a `kind`, a proposed `name`, and a `rank` (`high`/`medium`/`low`/`info`):

| kind | rank | what it detects |
|---|---|---|
| `bios_jumptable` | high | 3-4 insn body matching `addiu $tN,$zero,0xA0|0xB0|0xC0; jr $tN; addiu $t1,$zero,<idx>`. The `<idx>` is looked up in standard PsyQ/nocash A0/B0/C0 tables to propose the canonical BIOS name (e.g., `bios_FileRead`). |
| `syscall_wrapper` | high | Body contains a raw `syscall` or `break`. Address-suffixed because multiple wrappers can exist. |
| `psyq_idiom` | high | Tight `lbu`-`sb` pair-loop with stride+1, no `jal`, no jumptable -> `psyq_memcpy`. Store-only loop with no load and stride+1 -> `psyq_memset`. Both address-suffixed. |
| `data_as_code` | high | Function body is >=70% `lb $t0, neg_offset($zero)` with no actual logic -- a data table that splat misclassified as code. |
| `kengo_unique` | high | Kengo `name-unique` match with `|diff| <= 1` insns. Demoted to `medium` if multi-claimant. |
| `kengo_unique` | medium | Kengo `name-unique` / `name-callgraph` match with larger but still-small size diff. |
| `kengo_pattern` | medium | Kengo `callgraph` / `caller-unique` / `affinity-unique` match, or `seq-similarity` with `opseq_ratio >= 0.5`. |
| `sole_caller_path` | medium | Function has exactly one caller and that caller is trustworthy-named (not address-suffixed, not in known-bad cluster). Proposes `<caller>_helper_<addr>`. |
| `call_graph_cluster` | medium | Function has >=2 named callers and one subsystem prefix dominates (e.g., `cpu_*`). Proposes `<subsystem>_helper_<addr>`. |
| `kengo_pattern` | low | `seq-similarity` with `opseq_ratio` in 0.30-0.50 range. Always address-suffixed. |
| `kengo_size_only` | low | Kengo `size-only-unique` -- match was driven only by instruction count. Always address-suffixed. |
| `gte_op` | low | Body contains GTE atomic ops (`rtps`, `nclip`, `mvmva`, etc.). Proposes `gte_<top_op>_wrapper_<addr>`. |
| `string_adjacent` | low | Function loads a known string with a recognizable substring (e.g., `CHANBARA` -> `chanbara_*`). |
| `address_neighborhood` | low | >=2 functions at +-0x200 bytes share a trustworthy subsystem prefix. |
| `string_adjacent_info` | info | Function loads strings but no clear naming slug derives from them. Informational only -- doesn't count toward confidence. |

## Rolled-up confidence

- `high` if any one `high`-rank evidence fires OR >=2 `medium`-rank evidence rows agree
- `medium` if 1 `medium`-rank evidence row OR >=2 `low`-rank evidence rows
- `low` if 1 `low`-rank evidence row
- `none` if no evidence found

## Best-proposed-name selection

When multiple evidence rows propose names, the highest-rank evidence wins. Ties resolve by source-order in `analyze_one()`. The `evidence_detail_file` markdown lists ALL evidence rows so a reviewer can override the analyzer's pick.

## Known caveats (from prior triage)

Per `NAMING_TRIAGE_2026-05-12.md`:

- **Size-only Kengo false positives**: a Kengo `size-only-*` match is   weak. Five `katinuki_game_get_katinuki_max_num_*` renames were   applied in a prior pass and four of them turned out to be unrelated   GPU wrappers; one was `gpu_EnableDisplay` and one was   `gpu_DisableDisplay`. The analyzer **skips** this Kengo name entirely   and downgrades `size-only` to `low` with mandatory address-suffix.
- **Address-suffix indicates uncertainty**: a Kengo proposal with   `_<address>` suffix means even the reference is uncertain. The   analyzer reflects this by using suffixes for `seq-similarity` /   `size-only` / multi-claimant rows.
- **Trustworthy anchors**: the cluster-driving heuristics   (`call_graph_cluster`, `address_neighborhood`, `sole_caller_path`)   refuse to use address-suffixed names as anchors. Otherwise a stale   `katinuki_game_get_katinuki_max_num_80016868` could propagate naming   errors. Anchors also exclude the demoted clusters listed in   `KNOWN_BAD_ANCHOR_PREFIXES`.
- **Permanent out-of-scope categories** (`bios_or_syscall`,   `not_code_symbol`, `data_as_code`) are still in scope for NAMING   even though they're out of scope for decompilation. A BIOS wrapper   is a perfectly nameable function -- it just shouldn't be   attempted as pure C.

## Things the analyzer doesn't do

- It does not modify `named_syms.txt`, `symbol_addrs.txt`, or any   source files. Output is proposals only.
- It does not infer parameter types or return type. Those come from   caller-audit work, not naming.
- It does not propose names for `D_*` data symbols (see   `data_symbols_quick_wins.md` for a separate analysis).
- It does not run the build. SHA1 verification is the human's job.
