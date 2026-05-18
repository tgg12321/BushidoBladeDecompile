# Commit conventions

Subject-line prefixes and body structure used across this project. **These are conventions, not enforced rules** — but the project's tooling (especially `dc.sh lessons` / `tools/commit_lessons.py` and `tools/hooks/llm_audit.sh`) parses commit subjects and bodies for structured information, so consistency improves searchability and audit reliability.

For the enforcement layer, see `tools/hooks/llm_audit.sh` (adversarial cheat-audit on commits) and `tools/hooks/active_func_guard.sh` (active-function commit blocker).

## Subject line format

```
<prefix>[: <target>] [— or --] <one-line summary>
```

Examples:
```
Match func_80052788 (GTE gpf/gpl LERP wrapper, pure C with hardcoded $reg asm)
cheat-cleanup: func_800826CC -- retire 2 lost_codegen via volatile aliasing + switch
auth: func_8004BB68 (text1b.c recursive subdivision, custom callee-save ABI)
naming: medium-tier cleanup -- 2 more HELPER aliases for UNCLEAR stubs
tools: session_summary.py — handoff-ready session progress report
docs: AGENTS.md cross-tool standard + slim CLAUDE.md (Claude-specific)
```

Guidelines:
- Target subject ≤ 72 chars where possible (git convention, but not enforced)
- No trailing period
- Function names use full `func_XXXXXXXX` form (not shortened) — enables exact match in `dc.sh lessons func_X`
- Use `--` (double-hyphen) or `—` (em-dash) consistently as the prefix → summary separator. Both work; em-dash is preferred for readability.

## Prefix catalog (what each one means)

These prefixes are the ones `dc.sh lessons --type <prefix>` matches on. Keep them recognizable:

### Function-level decomp work

| Prefix | Meaning |
|---|---|
| `Match func_XXXXXXXX` | New pure-C match of a function. Body should describe the technique. |
| `cheat-cleanup: <name>` | Retiring a cheat (lost_codegen, wildcard subst, splice, etc.). Body documents what was retired and how. |
| `cleanup: <name>` | Stripping stale regfix rules from already-bridged functions (lighter than full cheat retirement). |
| `auth: <func>` / `inline_asm_canonical: <func>` | Canonical-asm authorization (function declared as hand-coded asm with documented evidence). Body MUST include evidence tags per [evidence-driven-authorization rule](../memory/rules/evidence-driven-authorization.md). |
| `text1b.c: <action>`, `code6cac.c: <action>`, etc. | File-level work (often function matches that benefit from the file context being in the subject). |

### Project infrastructure

| Prefix | Meaning |
|---|---|
| `naming: <topic>` | Symbol rename / alias batch / placeholder refinement. |
| `tools: <name>.py` | Adding or updating a tool. |
| `hooks: <description>` | Hook script additions or updates. |
| `skills: <change>` | Changes to `.claude/skills/*/SKILL.md`. |
| `docs: <change>` | Documentation changes (docs/, README, CLAUDE.md, AGENTS.md). |
| `dc.sh: <subcommand>` | Changes to `tools/dc.sh` or a specific subcommand. |
| `.claude/rules: <change>` | Path-scoped rule additions / migrations. |

### Lifecycle / coordination

| Prefix | Meaning |
|---|---|
| `Merge <branch>: <summary>` | Merge commit from a worktree branch (e.g., naming-evidence). Body summarizes what was merged. |
| `Revert "<original-subject>"` | Standard git revert. Body explains WHY (auditor REJECT, regression caught, etc.). |
| `queue: <change>` / `refresh-queue: <change>` | `WORK_QUEUE.md` regeneration. |
| `audit: <topic>` | Changes to `tools/audit_*.py` or audit policy. |
| `trace: <topic>` | Investigation notes that don't change source — typically commits to `docs/naming/*` or a research file. |
| `research: <topic>` | Exploratory work that did or didn't pan out. Document the outcome. |
| `misnomers: pass N` | Kengo-name reliability audits (see [kengo-name-unreliable rule](../.claude/rules/kengo-name-unreliable.md)). |

If you need a new prefix, just add it. The catalog is descriptive (what's been used), not prescriptive (what's allowed).

## Body conventions

### When a body is required

- **Always** for `cheat-cleanup:`, `cleanup:`, `auth:`, `inline_asm_canonical:`, `Match func_*` — these need explanation for future auditability.
- **Strongly preferred** for any commit that changes more than ~10 lines of source.
- **Optional** for trivial commits: queue refreshes, single-line docs fixes, typo corrections, mechanical migrations.

### Body structure for substantive commits

Subject line + blank line + body. Use a brief context paragraph, then structured sections as appropriate:

#### For `cheat-cleanup:` and `Match` commits

```
Subject + blank line

One-paragraph context: what the function does, why this match was tricky,
what HEAD looked like before the change.

Pure-C attempts:
  [1] technique=<name>  score=<diff_count>  outcome=<one-line observation>
  [2] technique=<name>  score=<diff_count>  outcome=<one-line>
  ...

Coercion technique (if §6.1 barriers or specific regfix rules used):
  - Reason for each barrier (single-instruction asm) with citation to the
    cc1 gap it closes
  - Reason for each regfix rule

Regfix changes:
  - Added/removed N rules
  - Net delta vs HEAD

Verification:
  - dc.sh verify --all --force result
  - cascade check on sibling functions

Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>
```

The `Pure-C attempts:` block is the load-bearing part — `tools/hooks/llm_audit.sh` (adversarial cheat-auditor) checks for it on canonical-asm and minimize-asm-when-blocked work. See [evidence-driven-authorization](../memory/rules/evidence-driven-authorization.md) for the exact grammar.

#### For `auth:` / `inline_asm_canonical:` commits

Same as above, plus an evidence-tag block per [evidence-driven-authorization](../memory/rules/evidence-driven-authorization.md):

```
DECISIVE evidence:
  (1) <signal>: <citation to target.s line>
  (2) <signal>: <citation>
  ...
classify_func: permanently_blocked:<reason>
```

Where signals are documented patterns: `custom-$s0-ABI`, `trapping addi`, `unsaved $ra in leaf`, `ghost-callee-save`, `bios-trampoline-jr`, `multu/mflo-pacing`, etc.

#### For `naming:` commits

```
Subject + blank line

What batch this is (cluster name / topic / source of names).

Renames applied:
  <old> -> <new>  (kengo:HIGH per kengo_matches.csv line N)
  <old> -> <new>  (body-derived: <one-line evidence>)
  ...

SHA1 verification: PASS / NO CHANGE / N functions affected
```

Atomic renames per [`tools/rename_funcs.py --apply`](../tools/rename_funcs.py). If the rename cascade requires regfix updates, list them.

#### For `tools:` / `hooks:` / `docs:`

Lighter. One paragraph of context + a what-changed list. Co-author trailer if AI-assisted.

## Required trailers

### Co-author trailer (AI-assisted commits)

```
Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>
```

Add when an agent generated substantive content. Trivial mechanical commits (queue refreshes, file moves) don't need it.

This trailer is **stripped by `tools/commit_lessons.py truncate_body()`** so it doesn't crowd the summary view — but git keeps it for attribution and PR review.

## How CommitAtlas (`dc.sh lessons`) uses these conventions

Direct dependencies:

- `--type <prefix>` does case-insensitive subject-prefix match. Stable prefixes → reliable filter.
- Function-name auto-detect (regex `^func_[0-9A-F]{8}$`) auto-enables pickaxe search. Spell function names fully and use the canonical form.
- `truncate_body()` cuts the Co-Authored-By trailer before display. Don't put body content after the trailer.
- Subject + body text is the primary search corpus. Substantive bodies = better discoverability later.

Loose dependencies (graceful degradation if violated):
- Variance in prefix capitalization works (`cheat-cleanup` / `Cheat-Cleanup` / `CHEAT-CLEANUP` all match)
- Function-name search via pickaxe works even if the message text is wrong
- Free-text search hits any term anywhere in subject + body

## What the audit hook (`llm_audit.sh`) actually enforces

Not commit-message format directly, but content-of-diff structure that interacts with commit messages:

- Canonical-asm authorizations (`inline_asm_canonical.txt` additions) require a documented evidence tag — usually in both the commit message body AND the `inline_asm_canonical.txt` line comment
- New cheat patterns (regfix `subst ".*"`, `splice` over thresholds, `insert "addu $sN, $0, $zero"` patterns) are blocked at commit time
- `minimize-asm-when-blocked` work without a `Pure-C attempts:` block of ≥10 enumerated entries is blocked

See [`memory/rules/evidence-driven-authorization.md`](../memory/rules/evidence-driven-authorization.md), [`memory/rules/minimize-asm-when-blocked.md`](../memory/rules/minimize-asm-when-blocked.md), and `tools/audit_asm_cheats.py` for the formal gates.

## Examples of good commits

| Commit | Why it's a good model |
|---|---|
| `d33ea6b` (voice rewrite) | Full Pure-C attempts log + per-barrier justification + per-rule justification + verification |
| `b5f36c8` (func_80069AE4 cleanup) | Auditor APPROVED; clean technique description |
| `7106004` (canonical-asm tightening) | Skills change with clear before/after reasoning |
| `806003d` (path-scoped rules migration) | Tool/infrastructure change with full rationale and trigger map |

Anti-models (don't emulate):
| Anti-model | Issue |
|---|---|
| `2447d6d` (AddTbpOfst_80047EE8) | Auditor REJECT — claimed pure-C while introducing register pins + inline asm + fabricated stack padding |
| `82138ff` (func_8007C7A0, on worktree, not main) | Auditor REJECT — explicit audit-evasion via subst-reformulation of blocked cheat |

Anti-models are useful as cautionary cases — keep them in history (don't revert just because the auditor rejected, if the work was previously merged).
