# CLOSER MISSION — Phase 4: naming integrity (Kengo false-positive audit)

The Kengo-derived names misled 155 grind sessions (cpu_side_move_dir_4 was
CD_sync; single_game_getEnemyCharId was ratan2). Owner directive 2026-07-10:
audit and repair the naming layer. THIS MISSION RUNS ONLY AFTER the Phase 3
adoption list is exhausted and Phase 5 (twins) is closed — renames while
adoptions are in flight would collide.

Read FIRST: memory/closer/sony-naming-map.md (accumulated Sony identities),
memory/closer/psyq-library-census.md + psyq-queue-hits.json,
kengo_matches.csv + kengo_name_decisions.csv (how each name was assigned),
tools/rename_funcs.py (the rename tool — READ it fully; renames must update
queue keys, regfix/asmfix anchors, named_syms.txt, symbol_addrs.txt, ledger
references TOGETHER or they orphan).

## W1 — Sony truth pass (mechanical)

Every census-identified function gets its Sony name. Use tools/rename_funcs.py
per function (or batch mode if it has one); verify after EACH batch:
1. `git grep <oldname>` residue = only historical docs/ledgers (acceptable)
   — never src/, engine/queue.json, regfix.txt, asmfix.txt anchors.
2. Full build byte-verify (`verify-oracle --rebuild`) — renames must be
   codegen-neutral. Any drift = STOP and revert the batch.
Static library-internal helpers get their Sony local names with a
`/* PsyQ static */` marker. Keep memory/closer/sony-naming-map.md as the
authoritative old→new record.

## W2 — Kengo audit (evidence-tiered)

For the ~87 kengo-tagged names NOT covered by W1 (grep `kengo:` provenance
tags in src/*.c; join with kengo_matches.csv):
1. Re-score each match with evidence the original round lacked:
   - Sony-span exclusion (must not be inside any census library region);
   - string xrefs (does the function reference strings consistent with the
     name's domain?);
   - call-graph consistency (do its callers/callees' IDENTITIES — not just
     names — support the match? Beware circular confirmation from other
     Kengo names);
   - opseq_ratio / n_candidates / callee_overlap from kengo_matches.csv.
2. Verdict per name: KEEP (evidence beyond size), DEMOTE (size-only or
   contradicted → rename back to func_<addr>, keeping the old name in a
   comment `/* kengo-guess (demoted <date>): <name> */`), or CORRECT (a
   better identity is provable — document evidence like the census did).
3. Ledger: memory/closer/naming-audit.md — one line per name with verdict +
   evidence. Demotions also go through tools/rename_funcs.py with the same
   two verification steps as W1.

## W3 — the institutional rule (proposal only)

Draft (do NOT install — .claude/rules/ is owner-gated) a proposed rule file
content in memory/closer/proposed-rule-name-provenance.md:
- names are hypotheses, not evidence; every non-ground-truth name carries a
  provenance tag (kengo:TIER / sony:census / manual:evidence);
- any session about to spend >1 hour on a function MUST run the 5-minute
  identity check first (string xrefs, RCS ids, census-span membership,
  cross-ref against known library corpora);
- naming rounds require the W1/W2 verification discipline.
The operator reviews and installs it.

## Outcome

completed_funcs is NOT used here (no queue completions expected — renames
don't complete items). Result "progress" with the audit ledger; list any
identity CORRECTIONS that reclassify queue items (those are census-grade
findings and may unlock adoptions — flag them loudly in the headline).
