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
  & tools/eng.ps1 queue next | queue status | canonical <f> | sandbox <f> --disable all
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
  [-DryRun]`. It invokes `claude -p` once per queue item
  (one function each), self-reviews every cycle, and **stops on ESCALATE**.
  Guardrails: `verify-oracle --rebuild` baseline + authoritative post-check,
  dirty-tree stop, never pushes. Each run → `metrics/headless_runs.jsonl`.
- **Review (per cycle / on completion):** `python3 tools/headless_review.py
  --latest` (or `--func`/`--session`). Outcome + audit signals + commit range +
  mechanical park-confirmation + ACCEPT/ESCALATE. Exit `0`=ACCEPT, `10`=ESCALATE.
- **Efficiency dashboard:** `python3 tools/headless_audit.py --all` (REVIEW vs OK
  per run; turns/tokens/cost/tooling-errors/footgun-blocks/dup-cmds). `--session
  <id> --errors` drills into one without dumping the transcript.
- **Integrity:** `python3 tools/check_completion_integrity.py` — every function
  not in the queue must satisfy its COMPLETED-C or COMPLETED-INLINE-ASM-CANONICAL
  invariants. Run it after batches.

A future fully-headless orchestrator can loop: launch run → on the completion
notification, review + handle/escalate → relaunch. You're re-invoked when the
background run finishes, so no polling.

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
| `& tools/eng.ps1 <cmd>` | engine CLI via PowerShell (queue/canonical/sandbox/retire/verify-oracle/test) |
| `pwsh tools/headless_loop.ps1 -MaxIterations N` | launch + self-review headless workers |
| `python3 tools/headless_review.py --latest` | per-run orchestrator packet (ACCEPT/ESCALATE) |
| `python3 tools/headless_audit.py --all` | efficiency dashboard from transcripts |
| `python3 tools/check_completion_integrity.py` | audit every completed function satisfies its invariants |
| `git commit -F tmp/msg.txt` | commit (never heredoc / nested quotes) |
