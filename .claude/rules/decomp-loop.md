---
name: decomp-loop
paths: [".claude/rules/decomp-loop.md"]
description: "Manual-path per-function loop (dossier → queue → canonical → sandbox → edit → verify → done), WIP checkpoints, findings registration, the sandbox-vs-build/ reference gotcha, and the full-picture-first dossier mandate."
metadata:
  type: rule
---

# The per-function decomp loop (manual path)

The Grinder (default autonomous path) uses its own ledgers (`memory/grind/<func>/`) and
seeds them from any WIP entry. This is the MANUAL path (`decomp-manual` skill, one agent on
`main`). Engine commands: `& tools/wteng.ps1 main <cmd>`.

## Full picture first (owner directive 2026-08-24)

Any session, investigation or review that touches a specific function STARTS with
`dossier <func>`: aliases (rulings often live under old names), queue item + directive,
verified src representation, rule/gate/config memberships, ledger digest, record-trail
headings and a cross-surface consistency audit. `dossier --audit-all` is the standing drift
detector: run it before a Grinder launch and after any bulk operation; a warning is work.

Write discipline: anchor current-state claims ("HEAD carries X") to a commit or date;
prefer pointers to commands over counts in prose; a rename updates the borderline alias
table in the same change; a bulk tool updates EVERY coupled surface (queue, src, ledger,
docs) in one commit.

## The loop

0. **Take the queue top** (`queue next`); work it to completion, no cherry-picking (unless
   the owner names a function). If it has a WIP checkpoint or ledger, resume from it:
   apply `candidate.c`, confirm the documented floor with `sandbox`, don't re-derive
   `rejected_forms`. A near-duplicate lead (`tmp/duplicates_leads.txt`,
   `tools/find_duplicates.py`) is a COMPLETED-C analog: read its body first.
1. **`verify-oracle --rebuild`** once at session start so `build/` is the clean reference.
2. **`canonical <func>`**: ASM-region / ASM-STRUCTURAL ⇒ stop pure-C work (authorized
   inline asm only, never `$N` injection). `C` ⇒ continue.
3. **`sandbox <func> --disable all`** (add `--diff` to see WHERE): the honest distance.
4. **Edit `src/<file>.c`** in pure C; re-run step 3 as your gradient. `diagnose <func>`
   classifies a stuck gap. Multiple forms: `python3 tools/sweep_variants.py --func <f>
   --file <stem> --variants tmp/<f>_variants/`. Rule-pattern maneuvers:
   `python3 tools/permuter_annotate.py --func <f> --hint <slug>` ([[permuter-directives]]).
   Corpus search: `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/<f>.s`.
   Every auto-search closing form is a PROPOSAL: vet it against [[no-new-park-categories]].
5. **Score 0 ⇒ finish.** The masked score can hide a register diff; `verify-oracle
   --rebuild` (full SHA1) is the only proof. Then the layer-2 `cheat-reviewer`
   ([[review-discipline-before-commit]]), `layer2 record`, `queue done <func>`, and delete
   `memory/wip/<func>/` if present.
5b. **Lowered but not 0 ⇒ checkpoint.** Do not leave `src/` modified (oracle stays green).
   Save `memory/wip/<func>/` (candidate.c + meta.json + notes.md); run `cheat-reviewer` on
   the candidate FIRST (FAIL ⇒ `rejected/<slug>.c`). Commit as `wip: <func>`.
6. **Register findings** only if reusable: a technique ⇒ `.claude/rules/<slug>.md` with
   `paths: [".claude/rules/<slug>.md"]` (on-demand) plus one line in
   `codegen-technique-index.md`; a function fact ⇒ the ledger. A new technique-family rule
   never ships in the match commit that uses it.
7. **Commit** per docs/COMMIT_CONVENTIONS.md.

**Reference gotcha:** the sandbox scores against `build/src/<file>.o`, which must stay the
pristine canonical build. Iterate only with `sandbox` (builds in `tmp/`).
`verify-oracle --rebuild` refuses (exit 3) while build inputs are dirty; `--allow-dirty`
is for legitimate cases (e.g. mid-revert).

## WIP checkpoints (`memory/wip/<func>/`)

Schema: `memory/wip/README.md`. CURRENT-STATE docs, enforced by `wip_compaction_guard.py`:
notes.md ≤120 lines (one TL;DR, rewritten in place), meta.json `sessions[]` ≤3 (older folded
into `prior_sessions_summary`). History lives in git. Verify file/line claims against current
`main` before recording them.

Related: [[review-discipline-before-commit]] ·
[[no-new-park-categories]] · [[rotation-not-foreclosure]]
