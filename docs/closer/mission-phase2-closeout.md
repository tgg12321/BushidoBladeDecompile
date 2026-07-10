# CLOSER MISSION — Phase 2: Close-out on the libcd identity

Phase 1 (memory/closer/phase1-evidence.md) proved the twins are PsyQ libcd
bios.c v1.86 `CD_sync` / `CD_ready`, recovered the original volatile-Intr
semantics, dropped marionation to floor 2 at 179/179 parity (P6 candidate),
and ruled the csmd4 h5 form a fakematch. This mission drives both functions
to an honest close. Three workstreams, priority order below.

Read FIRST: docs/closer/rulings.md, memory/closer/phase1-evidence.md,
memory/closer/structure-provenance.md, memory/closer/libcd-identity.md,
memory/closer/candidates/marionation_p6_volatile1496.c, and
tmp/closer/sotn_bios.c (re-fetch from sotn-decomp if missing).

## W1 — marionation_Exec (= CD_ready) → 0 (PRIORITY)

Start from the P6 chassis (179/179, masked 2). Two named residuals:

1. **Branch-destination pair**: two `beqz (a1==0)` guards branch to the FIRST
   copy of the duplicated `j; move v0,a2` tail; target branches to the SECOND.
   Known lever family: `.claude/rules/cross-jump-store-tail-merge.md` label-
   direction steering — mix exit forms / move `done:` / reorder the return
   paths so cross-jump picks the other copy. Read the rule fully first.
2. **The sll/addu transposition** in the do_timeout window — the SAME shape as
   csmd4's residual (the twins share this knot). The s33 qty-tie machinery now
   applies to a ONE-pair residual on a parity chassis; re-run the dump-vet
   with the volatile chassis in place (volatile changes scheduling freedom —
   prior KILLs were measured on the non-volatile chassis and do NOT bind).

**Volatile-scope discipline (owner stance: no exceptions just because it's
hard):**
- The original declares the WHOLE Intr struct volatile. The faithful form is
  full-volatile (P5, masked 8). FIRST try to drive P5 → ≤2 (its +6 over P6 is
  window scheduling — the volatile chassis changes which wraps/orderings are
  reachable; measure, don't assume).
- P6 (only idx_1496 volatile) is a hybrid: two non-volatile views of a
  volatile object. Treat it as a fallback, not the answer. If P5 cannot reach
  ≤2 and P6 reaches 0, present BOTH with measurements as a ruling question
  (result: "ruling-request") — do not self-certify the hybrid.
- On whichever chassis wins: re-derive which existing FAKE wraps become
  REMOVABLE (volatile provides ordering/weighting some wraps coerced). The
  minimal-FAKE form is what the Judge will pass.

If sandbox reaches 0: leave edits in src/, result "candidate-ready" — the
driver byte-verifies. Bank every measured form (candidates/ or rejected/).

## W2 — cpu_side_move_dir_4 (= CD_sync) honest respell

Replace the fakematch h5 initializer using the v1.86 idiom. Phase 1 lessons:
- Uniform volatile (P2/P3/P4: 38/28/28) does NOT reproduce the codegen; the
  CD_ready P6 lesson says volatility is expressed on SOME accesses (the
  original materializes pointer values — target stages both &Intr and
  &Intr+1, reads `ready` two different ways).
- Enumerate the {volatile-effective vs pointer-cached} assignment over
  csmd4's five Intr access sites (window sync read, window ready read,
  callback-arm reads, tail sync write). Dump-first pre-vet each against the
  s97 requirements: p79 flow-time nrefs >= 5 pre-block-3, livelen 148.
- Only sandbox-measure survivors of the dump gate.
- Target: an honest form at ≤2 (parity chassis), replacing h5. If the honest
  family bottoms out above that, record the floor with the partition evidence
  (which access spelling produces which nrefs) — that is the input Phase 3
  needs for the minimal-FAKE ruling under Ruling 2.

## W3 — PsyQ LIBCD.LIB ground truth

Fetch PsyQ SDK archives (sanctioned corpus — archive.org mirrors; versions
3.6/4.0-era should bracket bios.c v1.86). Extract LIBCD.LIB → BIOS.OBJ:
1. Object format is likely SN Systems .OBJ (psyq-obj-parser exists in the
   community; or extract with PSYLIB2/psylib tooling — build what you need in
   tmp/closer/).
2. objdiff CD_sync/CD_ready bytes vs the BB2 windows (locator:
   tmp/closer/locate_window.py). Outcomes that matter:
   - identical bytes → the SDK object IS the ground truth; any remaining
     source question reduces to "what C produces this object" with symbols.
   - different bytes → measure the delta; it brackets the v1.77→v1.86 source
     diff and Lightweight's compile flags.
3. If BIOS.OBJ carries symbols/relocs, extract the static layout ordering —
   it independently confirms the W4 structure map.

Kill criterion: LIBCD.LIB located+parsed with a byte-diff verdict, or a
precise acquisition blocker written to docs/closer/acquisitions.md.

## Deliverables

- memory/closer/phase2-closeout.md — measurements, kill records, verdicts.
- Updated candidates/ and rejected/ banks.
- If either function hits 0: candidate-ready with edits in place.
- Outcome JSON to the path given below the mission text.
