# CLOSER MISSION — Phase 1: External evidence sweep

155 grind sessions have exhausted the *internal* search space for
`marionation_Exec` (masked 4) and `cpu_side_move_dir_4` (masked 2). The
unknown is the original author's exact source idiom. This mission opens the
never-attempted external evidence axes. Four independent workstreams — run
all four, W4 FIRST (it gates an owner ruling); each has a kill criterion.
Negative results with measurements are deliverables, not failures.

## W4 — Structure provenance: is D_800A125C..D_800A1497 ONE object? (PRIORITY)

Ruling 1 in `docs/closer/rulings.md` is DEFERRED on exactly this question.
Answer it with evidence:

1. Map the address space: what symbols exist between 0x800A125C and
   0x800A1497 in the symbol tables / splat config / bb2.ld? An intervening
   unrelated symbol weakens the one-struct case; a clean gap of exactly
   142 s32 entries strengthens it.
2. Find every function in asm/ + src/ referencing D_800A125C, D_800A1494,
   D_800A1495, D_800A1496 (and any symbol in between). Does ANY code index
   from 125C across the 0x238 boundary, or treat the trio as table-relative?
   That is proof of one structure.
3. Check .data/.bss initialization and the sibling `saEft01Init`'s access
   pattern for the same evidence.
4. Deliver `memory/closer/structure-provenance.md` with a verdict:
   ONE-STRUCT (evidence), SEPARATE (evidence), or INCONCLUSIVE (what would
   decide it). If ONE-STRUCT: draft the merged declaration and dump-vet the
   honest member-access respelling of the csmd4 window against the p79
   refs>=5 requirement (s97/s98) — this could close the twin honestly.

Kill criterion: verdict memo written; if ONE-STRUCT, the respelling measured.

## W1 — Multi-build triangulation (prep + inventory)

Other compiles of ~the same source constrain the source shape:
- Bushido Blade 2 Japan release (check redump for catalog ID),
- BB2 demos/prototypes (check Hidden Palace / redump),
- Bushido Blade 1 (same developer Light Weight, ~1 year earlier, likely
  shared engine/library code — the `saEft*`/`marionation`/`cpu_side` family
  may exist there in an earlier compile).

Tasks:
1. Inventory what exists locally: `evidence/` dir, repo root, `archive/`,
   `Kengo/` (already known: PS2/EE, ruled non-transferable in csmd4 s18 —
   do not re-derive).
2. Web metadata research: enumerate the exact discs worth acquiring (catalog
   IDs, dates, any known demos/protos with debug leftovers or .SYM files).
   Write `docs/closer/acquisitions.md` — for each disc: what to extract,
   expected payoff, and a ready-to-run extraction recipe. DO NOT download
   commercial disc images.
3. Build the extraction tooling NOW so it runs the moment an image lands in
   `evidence/`: a byte-signature locator for the two windows. Strong anchors:
   the `debug_printf`/fmt string contents referenced by the windows (string
   xref), the distinctive table-lookup shape, prologue patterns. Test the
   locator by "finding" both functions in the local BB2 USA image
   (`Bushido Blade 2 (USA).bin`) — that validates it end-to-end. Save to
   `tmp/closer/locate_window.py` and document usage in the acquisitions doc.

Kill criterion: locator validated on the USA image + acquisitions doc written.

## W2 — Psy-Q SDK / era-corpus idiom mining

The residual windows are VSync-poll + debug-print blocks — textbook Psy-Q
sample idiom. If the devs cribbed SDK sample code, the literal source shape
may exist verbatim.

Tasks:
1. Check locally for Psy-Q SDK samples (tools/, archive/, common paths).
2. If absent, fetch the Psy-Q SDK sample corpus (archive.org mirrors are the
   standard source; SDK samples are fine to fetch — this project's toolchain
   is already Psy-Q-derived). ALSO consider: psn00bsdk/nugget re-creations,
   and any era open-source PSX code with FntPrint/VSync poll loops.
3. Grep the corpus for the window shape: VSync(-1) reads, poll-until-timeout
   loops, debug print with table-indexed args. Extract every distinct C
   spelling of the pattern.
4. Compile each candidate spelling through the project cc1 with the
   dump-first pre-vet against the named requirements:
   - csmd4: p79 (tbl) flow-time nrefs >= 5 pre-block-3, byte-neutral,
     livelen 148 preserved (see evidence.md s97).
   - marionation: the s39 fingerprint constraints for the do_timeout window.
5. Anything that moves the needle: sandbox-measure, bank to
   `memory/closer/candidates/`.

Kill criterion: corpus scanned, all distinct spellings dump-vetted, results
in `memory/closer/phase1-evidence.md`.

## W3 — Assembler-side provenance of marionation's region-3 nop

The region-3 residual is a delay-slot nop. The maspsx/aspsx axis has NEVER
been examined (grep both ledgers — zero mentions). Diagnostic only — the
canonical flags rule stays intact:

1. Determine who OWNS that nop: is it emitted by cc1 (in the .s) or inserted
   by maspsx (reorder/nop insertion pass)? Read the actual .s from the build
   for the region-3 window and diff against the final object.
2. If maspsx-inserted: study its insertion rule (tools/maspsx sources) and
   determine whether any HONEST C-side change alters the trigger condition.
   Note `.claude/rules/maspsx-label-nop-gate.md` and
   `maspsx-noreorder-stripping.md` — read them first; this may already be
   partially mapped.
3. If cc1-emitted: confirm the dbr-fill analysis (s51/s52) fully covers it
   and close the axis permanently with a one-paragraph proof.

Kill criterion: nop ownership named with evidence; axis either opened
(with a concrete lever) or closed permanently.

## Deliverables

- `memory/closer/phase1-evidence.md` — findings, measurements, kill records.
- `docs/closer/acquisitions.md` — the owner's shopping list (W1).
- `tmp/closer/locate_window.py` — validated extraction tool (W1).
- Outcome JSON to the path given below the mission text.

Read `docs/closer/rulings.md` FIRST — a fresh owner ruling on the
cross-symbol lever family is recorded there and affects what counts as a
legal candidate in W2.
