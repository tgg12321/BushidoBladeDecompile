# Naming Proposal Database

Evidence-backed function-name proposals for the 1217 unnamed `func_XXXXXXXX` functions in Bushido Blade 2.

## Files

- `proposals.csv` -- canonical machine-readable proposal table
  - one row per still-unnamed function
  - schema: address, current_name, proposed_name, confidence, evidence_summary, evidence_detail_file
- `proposals_high_confidence.md` -- review table for `high`-tier proposals
- `proposals_medium_confidence.md` -- review table for `medium`-tier proposals
- `psyq_library_matches.md` -- focused view on PsyQ stdlib / BIOS jumptable / syscall wrapper proposals
- `subsystem_clusters.md` -- call-graph cluster analysis (which subsystem do unnamed funcs belong to)
- `data_symbols_quick_wins.md` -- mechanical width-based naming hints for `D_*` undefined symbols
- `data_evidence/<D_addr>.md` -- per-symbol prose evidence for hand-reviewed data names (the deeper version of the same work)
- `methodology.md` -- analyzer design + evidence kinds + scoring + known caveats
- `evidence/<func_name>.md` -- per-function evidence detail file

## How to use

Open `proposals_high_confidence.md` for the lowest-risk batch. For each row you want to apply:

1. Read `evidence/<func>.md` to verify the evidence stands up.
2. Spot-check the disassembly snippet matches the proposed semantics.
3. Add `<proposed_name> = 0x<addr>;` to `named_syms.txt`.
4. Run `make setup && make` (in WSL); verify SHA1 unchanged.
5. Commit one batch (e.g., all BIOS wrappers, all PsyQ memcpy/memset) at a time.

The existing `tools/apply_kengo_names.py` knows how to apply Kengo-named proposals in lockstep (named_syms.txt + src/*.c). For BIOS / PsyQ / syscall proposals the asm rename happens via `make setup` after editing `named_syms.txt`. Do NOT touch `symbol_addrs.txt` or `undefined_syms_auto.txt` manually unless you know the splat tooling.

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

## Important caveats (from NAMING_TRIAGE_2026-05-12.md)

- **Size-only Kengo matches were the #1 source of false-positive renames** in   the prior pass (`katinuki_game_get_katinuki_max_num_*`, 5 functions). The   analyzer downgrades these to `low` and address-suffixes them.
- **Address-suffixed names indicate uncertainty.** A proposed `foo_8001ABCD`   is the analyzer saying "this is the best candidate but I can't promise   it's a true match." When applying, prefer dropping the suffix only AFTER   body inspection confirms semantics.
- **Per-function evidence files** are the single source of truth for why   the proposal exists. Read them.
