---
name: cheat-audit
description: Ad-hoc invocation of the adversarial LLM cheat auditor (Opus 4.7) on a specific commit, commit range, function snapshot, file, diff, or the staged/working tree. The same auditor that gates `git commit` via the PreToolUse hook, but in advisory mode — verdict + violations come back, nothing is blocked. Invoke whenever the user asks to "audit commit X", "audit the last N commits", "audit function Y for cheats", "check if [thing] has cheats", "run the auditor on [something]", "review [diff/file/commit/function] for asm shortcuts", "make sure I didn't slip in a cheat", or similar phrasings. ONE TARGET PER INVOCATION. Each audit costs ~$0.20–0.30 (Opus 4.7); routine targets that match the allowlist (docs / tooling / harness config) cost $0.
---

# /cheat-audit — ad-hoc adversarial cheat review

You were invoked to run the project's adversarial LLM auditor (`tools/hooks/llm_audit.sh`) on a user-specified target — independent of the git-commit hook. The auditor is Opus 4.7, freshly spawned, given the diff plus the canonical-asm authorization list, and asked: "does this cheat?"

This is **advisory** — no commits are blocked, no markers are set, no state is changed. You hand the user the verdict and reasoning.

---

## §1. Pick the right mode

The user's phrasing determines which `tools/audit_cli.sh` subcommand you invoke:

| User says | Mode | Command |
|---|---|---|
| "audit commit abc123" / "audit the last commit" / "did HEAD slip in a cheat?" | **commit** | `bash tools/audit_cli.sh commit <SHA-or-ref>` |
| "audit the last 5 commits" / "audit main..feature" / "review commits since X" | **range** | `bash tools/audit_cli.sh range <REF1>..<REF2>` |
| "audit function foo_bar" / "is foo_bar's current state clean?" / "check foo_bar for cheats" | **func** | `bash tools/audit_cli.sh func <funcname>` |
| "audit src/text1b.c" / "check this file" | **file** | `bash tools/audit_cli.sh file <path>` |
| "audit what I'm about to commit" / "audit my staged changes" / "dry-run the commit hook" | **staged** | `bash tools/audit_cli.sh staged` |
| "audit my working tree" / "audit everything I haven't committed" | **working** | `bash tools/audit_cli.sh working` |
| "audit this diff file" / "audit /tmp/foo.patch" | **diff** | `bash tools/audit_cli.sh diff <path>` |

If the user's intent isn't clear, ASK which target — don't guess. "audit commit X" can mean "the commit X" or "all commits since X" depending on context.

For ranges, prefer the user's explicit syntax (`HEAD~5..HEAD`, `main..HEAD`). If they say "the last 5 commits", that's `HEAD~5..HEAD`. If they say "audit since I branched", that's `main..HEAD` (or whatever their main branch is).

---

## §2. Invocation rules

1. **One target per invocation.** If the user wants 10 commits audited individually (not as a range), re-invoke this skill 10 times. The auditor is fresh each call by design (no shared context = no contamination).
2. **Do NOT run the auditor in a loop** without user permission. Each call costs real money. A user who says "audit the last 5 commits" wants `range HEAD~5..HEAD` (one call, $0.20-0.30), not five `commit` calls ($1.00-1.50).
3. **Pass user-provided refs verbatim.** Don't try to "interpret" `HEAD~3` into a SHA; the wrapper handles git syntax.
4. **No `--dangerously-skip-permissions`, no `--bare`, no flag mucking.** The wrapper sets up the auditor's environment correctly. Just call it.
5. **Don't pre-emptively filter the diff yourself.** The auditor handles allowlist/pre-filter logic. If you point it at a docs-only commit it'll skip with $0 cost — that's the design.

---

## §3. Reporting the verdict

The wrapper prints the auditor's structured output to stdout. Your job is to surface it to the user in their preferred form.

**On APPROVE:** state it simply, mention cost.
> "Auditor APPROVED. Cost: $0.17. No violations found. (Reasoning: …)"

**On REJECT:** show each violation with file/type/issue. Be neutral — you are the messenger, not the auditor. Do NOT argue with the verdict, do NOT defend the commit, do NOT tell the user "I disagree with the auditor here." If the user disagrees, that's their judgment call.

**On SKIP_PREFILTER / SKIP_ALLOWLIST:** state that the target was filtered as routine non-decomp work (docs / tooling / harness config) and no LLM call was made.

**On ERROR:** the verdict couldn't be reached (network down, parse failure, auth issue). State the error, suggest the user check connectivity / `claude` CLI / API quota.

---

## §4. When NOT to use this skill

- **As part of a normal commit flow.** The hook already runs the auditor on every `git commit` — invoking it here would double-pay. The only time `audit_cli.sh staged` is useful is as a *dry-run* before committing, when the user wants to know if their staged work will pass the hook.
- **For whole-repo sweeps.** Auditing every commit on `main` would cost dozens of dollars. If the user wants this, REFUSE and suggest a sampling strategy (audit recent N commits, audit commits touching a specific file, audit by author, etc.) — then check with them.
- **For sub-function granularity.** The auditor reasons at the file/function level. "Audit this specific 5-line block" doesn't fit — wrap it in a synthetic diff with `audit_cli.sh diff <path>` or audit the containing function with `audit_cli.sh func <name>`.
- **For things the auditor doesn't understand.** It's tuned for the BB2 cheat taxonomy (inline asm, regfix injection, asmfix bridges, register pins, give-up wrappers). It's not a general code-review tool — don't pitch it as one.

---

## §5. What the auditor catches (so you can set user expectations)

Tuned for the project's specific cheat taxonomy:
- `inline_asm` — `__asm__` blocks in src/ outside the `inline_asm_canonical.txt` allowlist
- `register_pin` — `register T x asm("regname")` used to force codegen rather than express ABI
- `regfix_cheat` — `subst`/`splice`/`insert_after` rules rewriting maspsx output
- `bridge` — `asmfix.txt` `replace_with_asmfile` lines without authorization
- `suspicious_comment` — "// cheat", "// hack to match", etc.
- `give_up_wrapper` — tiny C body wrapping a large `__asm__` doing the real work

Will NOT flag (correctly): GTE op macros from `include/gte.h`, PsyQ library calls, empty barrier asm, pure C, comment edits, doc updates, removal of inline asm.

If the user wants the auditor to look for something NOT in this taxonomy (e.g., "audit for memory leaks", "audit for naming consistency"), it's the wrong tool — tell them. Don't try to repurpose it.

---

## §6. Cost transparency

State the cost back to the user after each audit. They authorized Opus 4.7 explicitly knowing the cost, but they should still see what each call cost so they can decide whether further audits are worth it. The wrapper prints `Cost: $X.XXXX` in its output — just include that in your verdict summary.

If the user asks you to audit something large/expensive (e.g., a 100-commit range), give them a cost estimate FIRST (`That's ~100 commits at ~$0.25 each = ~$25. Confirm before I run it?`) and wait for confirmation. Do not assume.
