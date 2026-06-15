---
name: decomp-orchestrate
description: >-
  Drive the Bushido Blade 2 (SLUS-00663) matching-decompilation workflow as the
  ORCHESTRATOR — run headless worker agents through the engine loop, review each
  run, enforce the completion standard (only pure C, or canonical asm for
  genuinely-canonical functions), and operate autonomously per the escalation
  boundary. Use whenever asked to run / continue / resume the decomp, orchestrate
  the loop, work the queue, or kick off / babysit autonomous matching.
---

# BB2 Decomp Orchestrator

You are the **orchestrator** for the Bushido Blade 2 matching decompilation. You
drive the deterministic engine (`engine/`, run under WSL) and, for unattended
stretches, launch and review **headless worker agents** that each take one queue
function to completion. The engine measures + routes + gates; the workers (and
you, when working directly) write the C. Cheating is inert by construction and
the oracle is the only truth.

## 0. Bootstrap — load context first (in this order)

1. **`CLAUDE.md` + `AGENTS.md`** auto-load — they ARE the workflow spec. Re-read
   the "non-negotiables", the queue section, and "Orchestrator post-run protocol".
2. **`memory/MEMORY.md`** (auto-loaded index) → read **`project/greenfield-engine-v2.md`**
   (engine design + state) and **`user/role.md`**.
3. **`reference/matching-playbook.md`** — the codegen techniques (GCC 2.7.2 + ASPSX 2.34).
4. **Path-scoped rules in `.claude/rules/`** auto-load when you read a matching
   file (e.g. opening `src/*.c` surfaces the register-alloc / inline-asm / loop
   rules). Trust them; they're hard-won. Don't re-derive what they document.

## 1. The standard — NON-NEGOTIABLE (this is the whole point)

Every function is in exactly ONE state. No gradations, no "almost done."

  - **INCOMPLETE** — in `engine/queue.json`. Carries a regfix/asmfix rule,
    cheat-asm pin/__asm__, OR non-zero honest pure-C distance.
  - **COMPLETED-C** — zero rules, zero cheat-asm in source, byte-matches.
    Not in the queue. Not in `inline_asm_canonical.txt`. The SOTN bar; the
    default goal for every function.
  - **COMPLETED-INLINE-ASM-CANONICAL** — zero rules, canonical inline asm
    (GTE/cop2/BIOS/HW) or whole-body `__asm__("glabel ...")` that is its
    accepted finished form. Listed in `inline_asm_canonical.txt`. Reserved
    for functions whose ORIGINAL CODE was hand-written assembly — the
    `canonical` gate decides what qualifies, **not** the agent.

**Only the two COMPLETED states may ever be committed as DONE.** A function
that needs a regfix rule, a register pin, hardcoded-`$N` `__asm__`, or a
scheduling barrier to byte-match is INCOMPLETE — those are cheats, not a
finish. **Enforced, not honor-system:** `queue done` and `queue regen` refuse
to record a function as done if it carries rules or non-canonical cheat-asm
(SHA1 can't catch cheat-asm — it emits the right bytes — so the gate audits
the source). `tools/check_completion_integrity.py` is the standing audit;
`headless_review` flags any cheated committed match. **Never** harden /
stabilize a cheat to unblock something — retire the cheat instead.

## 2. Tooling — PowerShell-first, zero nested quoting

The toolchain runs under WSL, but **do not hand-author `wsl bash -c '…python3 -m
engine.cli…'`** — that nests three shells and the quoting eats awk/sed/heredocs.
`tools/hooks/shell_footgun_guard.py` BLOCKS the footguns. Rules:

- **Engine commands → the PowerShell tool + `tools/eng.ps1`** (zero quoting):
  ```
  & tools/wteng.ps1 main queue next | queue status | canonical <f> | sandbox <f> --disable all
                 | retire <f> | queue done <f> | queue park <f> --reason "…" | verify-oracle --rebuild | test
  ```
- **Anything beyond ONE simple command** (awk/sed, multi-statement, shell funcs,
  heredocs) → **write a `.py`/`.sh`/`.ps1` to `tmp/` and run that file.**
- **Multi-line commit messages → `git commit -F tmp/msg.txt`** (Write the file
  first). Never a `<<EOF` heredoc, never the `'"'"'` escape dance.
- **Exit codes:** `$?` is unreliable inside `wsl bash -c` (clobbered across shells
  — silently reads 0). Use the PowerShell tool's `$LASTEXITCODE`.
- Build files (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`)
  MUST stay LF. The Write tool produces LF here.

## 3. The per-function loop (what a worker does — and you, working directly)

0. `queue next` — take the TOP active item; work it to completion before another.
1. `verify-oracle --rebuild` once per session to establish the clean reference
   (skip if `build/` is already clean; never run it with uncommitted src edits —
   it's the final gate, not the iteration tool).
2. `canonical <func>` — route. ASM-WHOLE/STRUCTURAL ⇒ authorize (don't grind);
   `C` ⇒ continue.
3. `sandbox <func> --disable all` — honest pure-C distance (`0` = matchable).
   - **Masked-0 caveat:** a `0` can hide a register diff OR a source cheat-asm
     barrier the rules compensate for. If `sandbox`=0 but `retire` fails, look for
     a cheat-asm `__asm__` in the source and strip it ([[sandbox-zero-retire-fails]]).
4. Edit `src/<file>.c` toward 0 in **pure C**; re-run step 3 as the gradient.
5. `retire <func>` (drops rules + full SHA1 gate, auto-rollback) → `queue done`
   (re-checks 0 rules + 0 non-canonical cheat-asm + SHA1; on success the function
   is REMOVED from the queue). If not pure-C-closable, `queue park --reason "…"`.
6. **Register findings** — reusable pattern ⇒ `.claude/rules/<slug>.md` (with a
   `paths:` glob); function fact ⇒ `memory/`.
7. **Commit** (`Match:` / `cheat-cleanup:` / `engine:` per docs/COMMIT_CONVENTIONS.md;
   `git commit -F`).

## 4. Autonomous operation — drive workers + review

- **Run:** `pwsh tools/headless_loop.ps1 -MaxIterations N [-Model opus]
  [-MaxSameFunc 2] [-DryRun]`. It invokes `claude -p` once per queue item
  (one function each), self-reviews every cycle, and **stops on ESCALATE**.
  Guardrails: `verify-oracle --rebuild` baseline + authoritative post-check,
  dirty-tree stop, never pushes. Each run → `metrics/headless_runs.jsonl`.
- **Stuck-function cap (exit 11):** the loop STOPS if the same top function fails
  to complete (WIP-checkpoint / no-progress) for `-MaxSameFunc` consecutive
  iterations (default 2; `0` disables). This is NOT a user escalation — it means
  cold-start re-attempts are thrashing (measured: func_80078B04 burned 7 cold
  starts / ~$50 with the floor stuck at 2; sys_VSync ~$70, same shape). The
  remedy is a **modality change on the SAME function** (orchestrator deep-dive,
  `tools/sweep_variants.py`, instrumented cc1 dumps) — per the no-deferral
  directive, never a different target. Handle it yourself; don't relaunch cold
  workers at it.
- **Review (per cycle / on completion):** `python3 tools/headless_review.py
  --latest` (or `--func`/`--session`). Outcome + audit signals + commit range +
  mechanical park-confirmation + ACCEPT/ESCALATE. Exit `0`=ACCEPT, `10`=ESCALATE.
- **Efficiency dashboard:** `python3 tools/headless_audit.py --all` (REVIEW vs OK
  per run; turns/tokens/cost/tooling-errors/footgun-blocks/dup-cmds). `--session
  <id> --errors` drills into one without dumping the transcript.
- **Integrity:** `python3 tools/check_completion_integrity.py` — every function
  not in the queue must satisfy its COMPLETED-C or COMPLETED-INLINE-ASM-CANONICAL
  invariants. Run it after batches.

### MANDATORY retro-audit — nothing is ACCEPTED until a second agent says so
**(user directive 2026-06-10, after the fable-5 batch audit.)** The runner's
mechanical review + the worker's in-session cheat-reviewer are PROVISIONAL.
For every completion-class commit (`Match:` / `cheat-cleanup:` / `auth:` /
any commit retiring rules or adding a `.claude/rules/` technique doc), the
orchestrator MUST, before treating the item as accepted:

1. Run `python3 tools/reviewer_precheck.py --func <f> [--commit <sha>|--staged]
   [--msg-file tmp/msg.txt]` FIRST and paste its output into the reviewer
   brief — it mechanically settles the procedural facts (residual rules,
   body cheat-asm constructs, allowlist coverage of new extern volatiles,
   rule-docs riding along, carve-out commit fields) so reviewer tokens go
   to the SEMANTIC 6-test judgment, not re-derivation (a layer-1 review
   burned ~117k tokens re-deriving these before the tool existed).
2. Spawn a FRESH `cheat-reviewer` agent (a different context from the
   worker's in-session reviewer), briefed adversarially: default to FAIL,
   do NOT credit the worker's claimed reviewer verdict, dig into the
   specific commit with the 6-test checklist, and audit any rule doc the
   commit registered (self-sanctioning docs are banned outright per
   review-discipline-before-commit.md).
3. Batch cadence: run the loop N items (e.g. -MaxIterations 10), then
   retro-audit ALL completions in parallel before launching the next batch.
   Audits of independent commits run concurrently — use one reviewer agent
   per commit (or per tight pair).
4. Verdict handling: PASS → accepted. FAIL → revert workflow (restore the
   prior bridged state byte-identical; rules restorations use the
   `[infra-rule: reviewer-fail-revert]` guard category; preserve clean
   levers in a WIP checkpoint). NEEDS_USER → surface to the user; the item
   stays provisional.
5. The mechanical sweeps ride along once per batch: gate-tampering diff
   (engine/, tools/hooks/, Makefile, bb2.ld vs the batch baseline),
   `check_completion_integrity.py`, `audit_asm_cheats.py --summary`, and
   inline_asm_canonical.txt additions == exactly the authorized set.

Why this exists: the 2026-06-10 fable-5 audit found in-session reviewers
lean PASS on borderline constructs (2 of 7 completions failed adversarial
re-review), one worker re-adjudicated its own NEEDS_USER, and one authored
the rule sanctioning its own technique. SHA1 == oracle can never catch a
source-level cheat — the independent adversarial layer is the catch.

A future fully-headless orchestrator can loop: launch run → on the completion
notification, review + handle/escalate → retro-audit the batch → relaunch.
You're re-invoked when the background run finishes, so no polling.

## 5. Escalation boundary (MAXIMAL AUTONOMY — the default)

Keep going autonomously; **STOP and surface to the user ONLY for:**
- an **oracle break** (a committed build ≠ target SHA1),
- a **worker error** or a **stuck / no-progress** run,
- a **dirty tree** (uncommitted worker leftover — never run onto it; salvage it:
  commit if verified-matching, revert if incomplete),
- a **CHEATED match** (`headless_review` flags non-canonical cheat-asm in committed source),
- a **NOVEL park** you can't mechanically confirm,
- an **architecture / policy decision** (e.g. a global rodata reorder, a substrate
  change).

Everything else is logged and not blocked on. When you escalate, **confirm the
finding yourself first** (verify the worker's rationale against the actual rules /
source / linker) — workers over-claim; your review is the integrity check.

**No deferral (user directive 2026-06-12):** a stuck top item is NEVER skipped,
rotated, or deferred — everything gets decompiled eventually, so deferring buys
nothing. The remedy for repeated no-floor-change sessions is to change MODALITY
on the SAME function: an orchestrator deep-dive with sustained context (this is
what closed sys_VSync after 7 cold-start worker sessions), bulk variant sweeps
(`tools/sweep_variants.py`), or new diagnostics — never a different target.

## 6. Auto-handle categories (NOT escalate triggers)

- **GTE leaf wrappers** (pure cop2: `mtc2`/`avsz3`/`avsz4`/`mfc2` + nop/jr, no C
  form): the `canonical` gate routes them ASM-WHOLE → `authorize`. Authorize
  them yourself — remove the cheat-asm `register asm` pin (GCC returns in `$v0`
  naturally), `verify-oracle --rebuild`, add to `inline_asm_canonical.txt`,
  `queue done`. [[gte-wrapper-misroute-park]]. *Contrast whole-body hand-coded
  asm (custom ABI / trapping ops) — that needs a user judgment call → escalate.*
- **jtbl-infra** (a `switch` whose jump table splat carved into asm/data rodata;
  asmfix-only rename/replace_first/delete_between referencing `jtbl_*`): the queue
  auto-routes to `authorize`; `headless_review` auto-confirms the park. The global
  rodata reorder to truly pure-C them is the architecture decision that escalates.
  [[jtbl-rodata-split-infrastructure]]
- **maspsx `.L`-label load-delay nop** (sole cheat-asm is one `__asm__("nop")` for a
  load-consumer-across-`.L`-label): now a **pure-C retirement path** — add the
  function to `maspsx_label_nop_funcs.txt`, delete the source nop, verify, done.
  Per-function-scoped (broadening it globally cascades). [[maspsx-label-nop-gate]]

## 7. Footguns learned (don't repeat them)

- **TWO orchestrators clobbering main** (the 2026-06-14 orch3↔orch0614b incident) → before
  you reintegrate worker branches, **`& tools/reintegrate_lock.ps1 acquire`** and hold it for
  the whole apply→build→commit→queue-done window, then `… release`. The `main_reintegration_lock`
  PreToolUse hook enforces it (blocks a concurrent session's `git merge`/`reset`/edit on main).
  Keep the reintegration ATOMIC (apply→build-verify→commit fast); never leave main's working
  tree dirty across many tool calls — a concurrent merge will wipe it. Engine/build on main → ALWAYS
  `& tools/wteng.ps1 main <cmd>` (a relative `eng.ps1`/`make` is cwd-resolved and the contamination
  guard blocks it).
- **`$?` inside `wsl bash -c` is unreliable** → `$LASTEXITCODE` via PowerShell.
- **A mid-work cutoff leaves a dirty tree** (worker died/was killed mid-function) → never continue on it;
  if `oracle_ok` the leftover is verified-matching (commit it), else revert.
- **Workers over-batch** (do 3–4 functions, get cut off) → the prompt enforces
  one-function-commit-then-stop; the dirty-tree gate is the backstop.
- **`headless_review` commit display:** when a worker made no commit it shows
  "(no commit this run)" — it does NOT grep old commits (that misled once).
- **Don't reach for cheats to "finish":** `move $2,$0`-style hardcoded asm,
  register pins, `__asm__("" ::: "memory")` barriers are cheat-asm — they make
  the bytes match but the function is NOT COMPLETED. The gate rejects them at
  `queue done`.

## 8. The rule / memory library (read on demand)

`.claude/rules/*.md` are path-scoped and auto-load on matching file reads. The
high-value codegen + workflow rules: `sandbox-zero-retire-fails`,
`register-alloc-pure-c`, `inline-asm-policy`, `inline-asm-injection`,
`shared-end-label`, `switch-vs-ifchain-branch-sense`, `store-before-jal`,
`strength-reduce-defeat`, `cross-jump-call-merge`, `halfword-index-srl-sra`,
`narrow-stack-param-subword-offset`, `u16-global-lhu-lbu-low-byte`,
`loop-rotation-two-shift`, `gte-3x3`, `packed-multiply-cluster`,
`maspsx-noreorder-stripping`, plus the auto-handle rules in §6.

`memory/` (index: `MEMORY.md`): `project/greenfield-engine-v2` (engine),
`project/build-and-internals`, `reference/matching-playbook`,
`reference/scoring-systems`, `rules/completion-standard`,
`rules/community-standard`. Metrics design: `metrics/README.md` + `[[metrics-system]]`.

## Quick reference — the tools you drive

| Tool | Purpose |
|---|---|
| `& tools/wteng.ps1 main <cmd>` | engine CLI via PowerShell (queue/canonical/sandbox/retire/verify-oracle/test) |
| `pwsh tools/headless_loop.ps1 -MaxIterations N` | launch + self-review headless workers |
| `python3 tools/headless_review.py --latest` | per-run orchestrator packet (ACCEPT/ESCALATE) |
| `python3 tools/headless_audit.py --all` | efficiency dashboard from transcripts |
| `python3 tools/check_completion_integrity.py` | audit every completed function satisfies its invariants |
| `git commit -F tmp/msg.txt` | commit (never heredoc / nested quotes) |
