# Tooling Incidents Ledger

Append-only record of tooling / shell / environment failures caught by the
tooling-error guard (`tools/hooks/tooling_error_guard.py`) and how they were
resolved. Each entry is one resolution.

This ledger is institutional memory: before reaching for a workaround, scan it
for the failure class you're seeing -- the permanent fix may already be
documented. See `CLAUDE.md` (Hooks) and the `debugging-discipline` memory rule.

---

## 2026-05-25 23:26:05 — RESOLVED (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && echo "=== dc.sh size + top ===" && wc -l tools/dc.sh && head -20 tools/dc.sh && echo "=== worktree/bootstrap/start verb dispatch + functions ===" && grep -nE "new-worktree|bootstrap\)|^cmd_|^do_|self.re.exec|EnterWorktree|worktree" tools/dc.sh | head -40`
- **Root cause:** guard matched displayed source (a grep/cat of a script's own echo ERROR statement) as if it were live failure output
- **Permanent guard:** `tools/hooks/tooling_error_guard.py` (uncommitted change)
- **Verified by:** added _real_output() line-filter dropping displayed-source lines (grep -n prefixes, echo/printf/redirects, python print/stderr, JSON elements/keys); 36/36 guard tests pass incl 5 new self-inspection regressions
- **Occurrences this incident:** 1
