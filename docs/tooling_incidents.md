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

## 2026-05-31 23:58:51 — FALSE POSITIVE (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate 2>/dev/null && python3 -m m2c.main --target mipsel-gcc --function func_8007CBB0 asm/funcs/func_8007CBB0.s 2>&1 | head -150'`
- **Why not a real failure:** m2c is an optional analysis aid for reverse-engineering asm into draft C, not a build dep. The engine pipeline (cc1+maspsx+as+ld) is fully working — verify-oracle --rebuild just confirmed SHA1 == oracle. m2c-not-installed should not block decomp work; the agent can read asm directly. The signature 'worktree-dep-missing' overmatches: ModuleNotFoundError on a non-build python module is not the same class as a missing toolchain binary.
- **Action:** tighten signature `worktree-dep-missing` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.

## 2026-06-01 20:59:33 — FALSE POSITIVE (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `wsl bash -c 'bash /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"/tmp/rtldump_b6c8_v2.sh' 2>&1 | head -40`
- **Why not a real failure:** Script bug: tmp/rtldump_b6c8_v2.sh did 'cd tmp' before invoking 'tools/gcc-2.7.2/build/cc1' with a relative path, so cc1 was looked up under tmp/tools/gcc-2.7.2/build/cc1 instead of repo-root tools/gcc-2.7.2/build/cc1. cc1 IS installed correctly (4.2MB at tools/gcc-2.7.2/build/cc1, May 18; verify-oracle was passing earlier this session). The 'No such file or directory' was a path resolution error from the buggy script, not a missing dep. The signature 'worktree-dep-missing' currently fires on any 'tools/gcc-2.7.2/build/cc1: No such file or directory' substring without checking that the file actually doesn't exist on disk - the guard could be tightened to test -f the path before classifying as a missing dep.
- **Action:** tighten signature `worktree-dep-missing` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.

## 2026-06-04 01:35:41 — RESOLVED (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `cd "C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/.claude/worktrees/wf_3eeeb757-57e-3/" && powershell -Command "& tools/eng.ps1 sandbox func_8007C7A0 --disable all" 2>&1 | tail -30`
- **Root cause:** worktree spawned by workflow orchestrator without running setup_worktree.ps1; missing .venv/gcc/permuter/disc junctions
- **Permanent guard:** `tools/setup_worktree.ps1` (committed in eb24afac)
- **Verified by:** ran setup_worktree.ps1; engine smoke test (verify-oracle) passed
- **Occurrences this incident:** 1
