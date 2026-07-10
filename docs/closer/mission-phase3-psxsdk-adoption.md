# CLOSER MISSION — Phase 3: PsyQ library adoption (SOTN standard)

The census (memory/closer/psyq-library-census.md + psyq-queue-hits.json)
proved 92 queue items are verbatim-linked Sony PsyQ 4.0 library functions.
Owner directive 2026-07-09: adopt the known-matching psxsdk C for these,
SOTN-standard, and complete them out of the queue through the real gate.

Read FIRST: memory/closer/psyq-queue-hits.json (the work list),
memory/closer/psyq-library-census.md, memory/closer/libcd-groundtruth.md
(method + tools: tmp/closer/psyq_lib.py, ground-truth LIBs in
tmp/closer/psyq/ — re-fetch from sozud/psy-q raw URLs if missing),
memory/closer/phase2-closeout.md if it exists (Phase 2 results),
docs/closer/rulings.md.

## SOTN standard, applied to this repo

1. **Source of truth for C**: sotn-decomp's psxsdk tree (github.com/Xeeynamo/
   sotn-decomp, src/psxsdk/ or include/psxsdk — explore the repo layout) is
   the primary reference — it is matched C for this exact library family and
   GCC 2.7.2-era toolchain. Secondary: sozud/psy-q-decomp. If a function has
   no public match: decompile it YOURSELF from the ground-truth object — you
   have the original bytes AND symbol names (psyq_lib.py XDEF/local tables);
   this is vastly easier than blind decomp. Never guess: the object is the
   oracle.
2. **Provenance headers**: every adopted function gets a comment:
   `/* PsyQ 4.0 <LIB> <module>: <SonyName> — verbatim-linked Sony object
   (census 2026-07-09); C ref: <source> */`
3. **Naming**: DO NOT mass-rename symbols — queue keys, regfix anchors, and
   grind ledgers reference the current names; renames would orphan them.
   Record the canonical mapping in memory/closer/sony-naming-map.md (func →
   Sony name, one line each, appending as you go). A dedicated rename pass
   can happen later with owner sign-off.
4. **Struct/volatile fidelity**: use the original declarations (e.g. volatile
   CD_intr, MMIO register pointers) as the reference source has them —
   original semantics, not coercions.

## Work discipline

- Work list: queue_hits.json entries, EXCLUDING:
  - `marionation_Exec` and `cpu_side_move_dir_4` (Phase 2 owns them — skip
    unless memory/closer/phase2-closeout.md says they are already committed
    at 0, in which case leave them alone anyway);
  - the ~26 "probable Sony, unproven" LIBSND-gap items (no ground truth yet —
    acquisition pending; do not adopt on faith).
- Priority order: (1) rule-free items ascending distance (40 items — pure
  transcription wins), (2) rule-carrying items ascending rule count. Batch by
  file/library so sibling declarations (shared statics/structs) land once,
  coherently.
- Per function:
  1. Get the reference C; adapt symbol names to this repo's externs.
  2. Splice into the correct src/<file>.c (replace the current body or the
     INCLUDE_ASM).
  3. `& tools/wteng.ps1 main sandbox <func> --disable all` — REQUIRED: 0.
     (--disable all = rules off = honest distance. The driver strips the
     actual rules afterward; you never touch regfix.txt/asmfix.txt.)
  4. If 0: add the func name to `completed_funcs` in your outcome JSON.
  5. If NOT 0: diff against the ground-truth object (compare_groundtruth.py
     method) — the delta tells you exactly what the C got wrong. Bank
     stubborn cases to memory/closer/candidates/ with the measured diff and
     move on; do NOT grind one function at the expense of the batch.
- Statics: library modules contain static helpers (get_cs, _addque2, ...)
  that queue items map to. A module's functions often only match as a set
  (shared statics) — adopt module-coherently.
- Data/rodata: if a function needs its module's static data relocated or
  redeclared, follow the census symbol map; verify data bytes against the
  object's .data/.rdata sections.
- Bank progress CONTINUOUSLY: update memory/closer/phase3-progress.md after
  every batch (done list, blocked list with reasons). Sessions get discarded
  if unproven — the banked memo is what survives.

## Outcome contract (extension)

Standard schema PLUS: `"completed_funcs": ["func1", "func2", ...]` — ONLY
functions you measured at sandbox 0 with --disable all THIS session, edits in
place in src/. The driver strips their rules, re-proves the full oracle,
runs each through `queue done` (the gate self-guards), and commits. A func
that fails the driver's gate stays queued — never pad this list.

Set result "progress" (or "candidate-ready" if you also closed a Phase 2
function). This mission is expected to take MULTIPLE sessions — complete as
many as cleanly possible, bank the rest with precise notes for the next
session.

## ADDENDUM 2026-07-10 — LIBSND hunt results (memory/closer/libsnd-hunt-report.md)

BB2's sound libs are a 4.0-lineage INTERIM Sony build (Jun-Sep 1997) not in
public archives. Consequences for this mission:
- **func_80085270 = _SsSndStop is now census-proven** (verbatim vs the
  Jun-06-1997 4.0 build, 118 words 100% masked) — ADD it to the work list;
  ground-truth object in tmp/libsnd_hunt/.
- The 9 near-proven items (SsUtKeyOnV, _SsVmKeyOffNow 1/27, SsUtGetDetVVol
  1/14, SsUtGetVVol 2/42, SsUtSetDetVVol, _SsVmGetSeqLVol/RVol 2/28,
  _SpuSetAnyVoice, _SsVmDoAllocate) are STRETCH targets only: identity is
  near-certain but our reference objects differ slightly (interim build).
  Adoption must reach sandbox-0 honestly against BB2's actual bytes — the
  reference C + close objects make this tractable, but do NOT force it;
  bank and move on if the interim delta resists.
- The remaining LIBSND-gap items stay excluded (unproven identity).
