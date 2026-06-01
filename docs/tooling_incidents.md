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

## 2026-05-26 00:30:08 — RESOLVED (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `cd "C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile" && echo "=== which hook file references dc.sh / worktree_bootstrap / diagnose_batch / cc1psx_* (live invocation vs signature-DB string)? ===" && grep -rn -E 'dc\.sh|worktree_bootstrap|diagnose_batch|cc1psx_diagnostic|cc1psx_wrapper' tools/hooks/ | head -40`
- **Root cause:** recursive-grep path:line: prefix defeated the displayed-source filter; markers only handled plain grep -n leading-digit form
- **Permanent guard:** `tools/hooks/tooling_error_guard.py` (uncommitted change)
- **Verified by:** added ^\S+:\d+[:-] display marker + 2 regression tests; test suite 38/38 pass
- **Occurrences this incident:** 1

## 2026-05-26 02:16:59 -- DEFERRED (crlf/crlf-build-file)  [known-unfixed]
- **Triggering command:** `Edit C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\src\config.c`
- **Why unfixable now:** CRLF written by the Windows Edit tool into src/config.c. Already normalized to LF via sed (0 CR bytes confirmed) and verified to compile cleanly (sandbox game_SetPlayerCount --disable all => score 0, build_insns 20). The permanent guard already exists and fired correctly here: .gitattributes line 12 (*.c text eol=lf) + the tooling_error_guard.py hook. The residual root cause is the harness Edit tool converting LF->CRLF on in-place writes, which is outside this repo. No new repo-level guard can improve on what already caught this, and the active task constraints forbid editing files other than the target function, so no .gitattributes/hook change is made this turn.

## 2026-05-30 13:24:09 — RESOLVED (crlf/crlf-build-file)
- **Triggering command:** `Edit C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\permuter\csmd4_v8\base.c`
- **Root cause:** Edit tool wrote CRLF to permuter/csmd4_v8/base.c (gitignored workspace .c file); existing .gitattributes had *.c eol=lf but git ignore-listed paths aren't normalized by git
- **Permanent guard:** `.gitattributes` (uncommitted change)
- **Verified by:** sed -i 's/\r$//' permuter/csmd4_v8/base.c, then grep -cP '\r$' = 0; .gitattributes now explicitly maps permuter/**/*.c eol=lf so any future git add/staging normalises; the tooling_error_guard post-write check remains the live net
- **Occurrences this incident:** 1

## 2026-05-31 20:34:31 — FALSE POSITIVE (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `which mipsel-linux-gnu-objdump 2>&1 ; mipsel-linux-gnu-objdump --version 2>&1 | head -2`
- **Why not a real failure:** Ran mipsel-linux-gnu-objdump from Windows Git Bash (no WSL prefix); the toolchain only exists inside WSL and the build itself is healthy (display.o just built at 20:32). The actual command needed wsl prefix — the guard fired on an exploratory command, not a build failure.
- **Action:** tighten signature `worktree-dep-missing` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.
