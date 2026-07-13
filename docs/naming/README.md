# Naming Proposal Database

Evidence-backed function-name proposals for the unnamed `func_XXXXXXXX`
functions in Bushido Blade 2. The initial 1217-function analyzer pass has
been fully resolved as of 2026-07-13 (see `proposals_audit_2026-07-13.md`);
this dir stays as the analyzer's regenerable output for future passes and
as the archive of applied/superseded proposals.

## Current state (2026-07-13)

- **High confidence:** 0 pending (all 10 resolved).
- **Medium confidence:** 0 pending (all 111 resolved).
- **Low confidence still pending:** 23 (weak-evidence `_local_`/`_func_` stubs;
  these need more caller context to firm up).
- Full audit: `proposals_audit_2026-07-13.md`
- Resolved rows: `proposals_resolved.csv`

## Files

- `proposals.csv` -- canonical machine-readable proposal table
  - one row per originally-unnamed function
  - schema: address, current_name, proposed_name, confidence, evidence_summary, evidence_detail_file
- `proposals_high_confidence.md` -- review table for `high`-tier proposals
- `proposals_medium_confidence.md` -- review table for `medium`-tier proposals
- `proposals_resolved.csv` -- rows that have already been applied (or applied under a different name)
- `proposals_audit_2026-07-13.md` -- audit result: applied-as-proposed / applied-differently / still-pending counts
- `psyq_library_matches.md` -- focused view on PsyQ stdlib / BIOS jumptable / syscall wrapper proposals
- `subsystem_clusters.md` -- call-graph cluster analysis (which subsystem do unnamed funcs belong to)
- `data_symbols_quick_wins.md` -- naming hints for `D_*` undefined symbols based on access pattern
- `methodology.md` -- analyzer design + evidence kinds + scoring + known caveats
- `evidence/<func_name>.md` -- per-function evidence detail file

## How to apply a proposal

Open `proposals_high_confidence.md` for the lowest-risk batch. For each row you want to apply:

1. Read `evidence/<func>.md` to verify the evidence stands up.
2. Spot-check the disassembly snippet matches the proposed semantics.
3. Add `<proposed_name> = 0x<addr>;` to `named_syms.txt`.
4. Grep and replace `func_XXXXXXXX` references throughout `src/*.c` with the new name.
5. Verify: `& tools/wteng.ps1 main verify-oracle --rebuild` (in PowerShell). Expect SHA1 unchanged — a name change is symbol-level only. Do NOT run `make setup` — `bb2.ld` is hand-maintained (see CLAUDE.md).
6. Commit one batch (e.g., all BIOS wrappers, all PsyQ memcpy/memset) at a time.

The existing `tools/apply_kengo_names.py` knows how to apply Kengo-named proposals in lockstep (named_syms.txt + src/*.c). For BIOS / PsyQ / syscall proposals the asm rename lands via the linker's symbol table (both old and new names resolve to the same address); update `src/*.c` references so the C-level reader sees the new name. Do NOT touch `symbol_addrs.txt` or `undefined_syms_auto.txt` manually unless you know the splat tooling.

## Regenerating

```
python3 tools/propose_function_names.py            # rebuild proposals.csv + evidence/
python3 tools/render_naming_docs.py                # rebuild markdown views
python3 tools/diff_naming_proposals.py OLD.csv NEW.csv  # show diff vs prior run
```

Inputs the analyzer reads:

- `asm/funcs/*.s` -- raw function disassembly
- `named_syms.txt`, `symbol_addrs.txt` -- already-named functions and globals
- `kengo_matches.csv` -- Kengo PS2 cross-reference (from `tools/kengo_match.py`)
- `kengo_name_decisions.csv` -- reviewed decisions to suppress demoted names
- `known_psyq_stdlib.txt` -- existing PsyQ idiom scan
- `undefined_syms_auto.txt` -- splat-auto-detected data symbols
- `asm/data/*.rodata*.s` -- string constants for `string_adjacent` evidence

## Important caveats

- **Size-only Kengo matches were the #1 source of false-positive renames** in the prior pass (`katinuki_game_get_katinuki_max_num_*`, 5 functions). The analyzer downgrades these to `low` and address-suffixes them.
- **Address-suffixed names indicate uncertainty.** A proposed `foo_8001ABCD` is the analyzer saying "this is the best candidate but I can't promise it's a true match." When applying, prefer dropping the suffix only AFTER body inspection confirms semantics.
- **Per-function evidence files** are the single source of truth for why the proposal exists. Read them.
