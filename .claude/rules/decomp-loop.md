---
name: decomp-loop
paths: [".claude/rules/decomp-loop.md"]
description: "The manual-path per-function decomp loop (queue → canonical → sandbox → edit → retire → done), WIP checkpoints (memory/wip/<func>/), the near-duplicate-lead shortcut, and the sandbox-vs-build/ reference gotcha. Condensed spine lives in CLAUDE.md; full detail here."
metadata:
  type: rule
---

# The per-function decomp loop (manual path)

The condensed spine and load-bearing invariants live in CLAUDE.md (`## The per-function
loop + WIP checkpoints`). This file is the full procedural detail — the engine measures,
routes, and gates; the agent writes the C.

The GRINDER is the DEFAULT autonomous workflow and uses its own richer ledgers
(`memory/grind/<func>/`, append-only, driver-managed) rather than this loop; it converts
any existing WIP entry into a seed ledger on first contact. The steps below are the MANUAL
path (the `decomp-orchestrate` skill, one focused agent on `main`).

## The loop

0. **Take the top of the queue** — `queue next` (also surfaced by the SessionStart hook). Work THAT
   function to completion before taking another; don't cherry-pick. (Override only if the user names
   a specific function.) **If the top function has a WIP checkpoint** (banner in the SessionStart
   hook + `wip` block in `queue next`'s output), READ `memory/wip/<func>/meta.json` + `notes.md`
   FIRST — apply `candidate.c` to `src/<file>.c`, confirm the documented floor with `sandbox`,
   and continue from there instead of starting from HEAD. The `rejected_forms` field lists
   constructs the prior agent ruled out; don't re-derive them.
   **If the SessionStart hook surfaces a NEAR-DUPLICATE LEAD** for this function — or you find one
   yourself in `tmp/duplicates_leads.txt` — the RHS is an already-COMPLETED-C analog. Read its `src/`
   body BEFORE writing anything; it's the most efficient starting template. Tool:
   `tools/find_duplicates.py`. Regenerate after big completion batches.
1. **`verify-oracle --rebuild`** once at session start. This makes `build/` the clean canonical
   reference the sandbox scores against. (Skip if `build/` is already clean.)
2. **`canonical <func>`** — route. ASM-region / ASM-STRUCTURAL ⇒ stop the pure-C effort
   (authorized inline asm only; never grind it or inject `$N` asm). `C` ⇒ continue.
3. **`sandbox <func> --disable all`** — the honest cheat-free pure-C distance (`0` = already
   matchable in pure C; just write/keep that C).
4. **Edit `src/<file>.c`** toward the target in pure C, then re-run step 3 — the score is your
   gradient. `diagnose <func>` explains a stuck gap (matchable / control-flow / canonical / plateau).
   Exploring >3 candidate forms? Sweep them in ONE call: `python3 tools/sweep_variants.py --func
   <f> --file <stem> --variants tmp/<f>_variants/` (scores each form via the sandbox, restores src;
   low scores are PROPOSALS — vet against the cheat catalog).
   **Stuck on a recognised rule pattern** (e.g. `shared-end-label`, `register-asm-pins`,
   `loop-rotation-two-shift`)? `python3 tools/permuter_annotate.py --func <f> --hint <rule-slug>`
   writes a PERM_*-annotated candidate to `tmp/permuter_candidates/<f>.c` — feed it to permuter to
   validate the maneuver instead of hand-iterating. `--list-hints` for the catalog. Rule:
   `.claude/rules/permuter-directives.md`. **Closing forms from permuter are PROPOSALS — vet against
   [[no-new-park-categories]] before committing; layer-2 cheat-reviewer still mandatory.**
   **Still stuck after the local levers?** `python3 tools/decomp_me_scrape.py search --asm-file
   asm/funcs/<func>.s` queries the downloaded decomp.me corpus (the BB2 toolchain class:
   gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5) for scratches whose target asm overlaps yours. Coarse
   pre-filter; manual inspect the top hits for an analogous C shape.
5. **Score 0 ⇒ finish.** `retire <func>` deletes the function's now-unneeded regfix/asmfix rules,
   rebuilds, and SHA1-gates (auto-rollback on mismatch). A pure-C function with no rules to remove
   just needs `verify-oracle --rebuild` to confirm the byte+link match. **Note (masked-0 caveat):**
   the sandbox distance is masked (register names normalised out), so a `0` can hide a real register
   diff — `retire`/`verify-oracle` (full SHA1) is the only proof. If `retire` rolls back, the gap is
   genuine reg-alloc work; keep editing.
   Then `queue done <func>` records completion (it re-verifies ZERO rules + SHA1 == oracle), AND
   delete `memory/wip/<func>/` if one existed (the checkpoint's purpose is served on close-out). If
   the item is genuinely not pure-C-closable (canonical-asm needing user auth, or a documented
   plateau), `queue park <func> --reason "…"` instead so the queue advances.
5b. **Score lowered but not 0 ⇒ checkpoint.** If you measurably lowered the floor below HEAD's but
   couldn't close, do NOT modify `src/` (oracle stays green). Save the progress as a WIP entry under
   `memory/wip/<func>/`: candidate.c + meta.json (append to `sessions[]`, update
   `scores.candidate_floor`, record any rejected forms) + notes.md. **Invoke `cheat-reviewer` on the
   candidate FIRST** — record the verdict in `meta.json.reviewer`. Commit under `wip: <func>`.
   Next session resumes from your checkpoint instead of from HEAD. The next agent reads `meta.json`,
   applies `candidate.c`, confirms the documented floor, iterates from there.
6. **Register findings.** Before committing, ask: did this match reveal a *reusable* codegen
   pattern or a non-obvious gotcha that the next agent would benefit from? If yes, record it where
   future agents will actually see it:
   - **reusable pattern** ⇒ add/update a doc in `.claude/rules/<slug>.md` with `paths:
     [".claude/rules/<slug>.md"]` (ON-DEMAND — technique rules do NOT get a broad `src/*.c`
     glob; that cost ~200k tokens/session pre-2026-06-11) **AND add a one-line symptom entry
     to `.claude/rules/codegen-technique-index.md`** (the index is what auto-loads). The
     metrics layer fingerprints the `slug`. Link related rules with `[[other-slug]]`. Only
     enforcement-critical POLICY rules keep a broad glob.
   - **function-specific fact** ⇒ a `memory/` entry (per the memory rules in CLAUDE.md).
   - **routine / no-op match** ⇒ skip; don't manufacture a finding.
7. **Commit** (`cheat-cleanup:` / `Match` / `engine:` prefix per docs/COMMIT_CONVENTIONS.md).

**Reference gotcha:** the sandbox scores your edited, cheat-stripped `.o` against
`build/src/<file>.o`, which must stay the *pristine* canonical build (= the target bytes). During
the edit loop use only `sandbox` — it builds into `tmp/` and never touches `build/`. **Enforced
2026-06-12:** `verify-oracle --rebuild` REFUSES (exit 3) while build-input files have uncommitted
edits — that refusal means you're misusing it as an iteration tool; `--allow-dirty` overrides for
legitimate cases (e.g. mid-revert restoration). The final SHA1 gate is always honest.

## WIP checkpoints (`memory/wip/<func>/`) — manual path

The GRINDER uses its own richer ledgers (`memory/grind/<func>/`, append-only, driver-managed)
and converts any existing WIP entry into a seed ledger on first contact. MANUAL multi-session
work still checkpoints here: best candidate + measured floor + hypotheses + cheat-reviewer
verdict in `memory/wip/<func>/` (candidate.c + meta.json + notes.md + rejected/) — tracked in
git, OUTSIDE the build pipeline, surfaced by the SessionStart hook and `queue next`'s `wip`
block. Full schema + usage: `memory/wip/README.md`. Load-bearing rules:
- **Resume from the checkpoint, not HEAD** — apply candidate.c, confirm the documented floor
  with `sandbox`, don't re-derive `rejected_forms`.
- **Cheat discipline:** invoke `cheat-reviewer` on the candidate BEFORE saving; FAIL ⇒ save
  under `rejected/<slug>.c` (named violated rule), not as candidate.c.
- **Compaction contract (ENFORCED by `wip_compaction_guard.py`, 2026-06-12):** WIP files are
  CURRENT-STATE docs — notes.md ≤120 lines (ONE TL;DR, rewritten in place), meta.json
  sessions[] ≤3 (fold older into `prior_sessions_summary`, one line each). History lives in git.
- On COMPLETED-C, **delete `memory/wip/<func>/`**; generalizable lessons → `.claude/rules/`.

## Related
- [[completion-standard]] — the three function states this loop drives toward
- [[review-discipline-before-commit]] — the mandatory layer-2 cheat-reviewer gate
- [[no-new-park-categories]] — vet permuter/auto-search closing forms against this
- [[metrics-system]] — the `slug` fingerprinting referenced in step 6
