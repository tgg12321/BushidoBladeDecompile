# Tooling Incidents Ledger

Append-only record of tooling / shell / environment failures caught by the
tooling-error guard (`tools/hooks/tooling_error_guard.py`) and how they were
resolved. Each entry is one resolution.

This ledger is institutional memory: before reaching for a workaround, scan it
for the failure class you're seeing -- the permanent fix may already be
documented. See `CLAUDE.md` (Hooks) and the `debugging-discipline` memory rule.

How the guard works (formerly pre-slim-2026-10-01:docs/TOOLING_ERROR_GUARD.md): `tools/hooks/tooling_error_guard.py`
(PostToolUse) matches command output against `tools/hooks/tooling_error_signatures.json` and
checks written build files for CRLF; a block-tier hit writes `.bb2_tooling_incident.json` and
`tools/hooks/tooling_incident_stop_guard.sh` (Stop/SubagentStop) refuses to end the turn until
the incident is resolved with `python3 tools/resolve_tooling_incident.py` — `--fixed --guard
<changed file> --root-cause "..." --verify "..."` (the guard file must show a recent change),
`--false-positive "<why>"` (then tighten the signature), or `--defer "<why>"` (logged
known-unfixed). Adding a signature is itself a valid permanent fix. Tests:
`tools/hooks/test_tooling_error_guard.py`, `tools/hooks/test_tooling_incident_e2e.sh`.
Rotate with `python3 tools/rotate_grind_logs.py`.

> Entries before 2026-09-01 (other than DEFERRED) rotated out; full history resolves at git tag pre-slim-2026-10-01.

---

## 2026-05-26 02:16:59 -- DEFERRED (crlf/crlf-build-file)  [known-unfixed]
- **Triggering command:** `Edit C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\src\config.c`
- **Why unfixable now:** CRLF written by the Windows Edit tool into src/config.c. Already normalized to LF via sed (0 CR bytes confirmed) and verified to compile cleanly (sandbox game_SetPlayerCount --disable all => score 0, build_insns 20). The permanent guard already exists and fired correctly here: .gitattributes line 12 (*.c text eol=lf) + the tooling_error_guard.py hook. The residual root cause is the harness Edit tool converting LF->CRLF on in-place writes, which is outside this repo. No new repo-level guard can improve on what already caught this, and the active task constraints forbid editing files other than the target function, so no .gitattributes/hook change is made this turn.

## 2026-07-11 11:22:15 -- DEFERRED (crlf/crlf-build-file)  [known-unfixed]
- **Triggering command:** `Edit C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\regfix.txt`
- **Why unfixable now:** Known signature crlf-via-edit-write already registered; guard caught the CRLF write, normalize_lf.py restored LF (grep -c $'\r' regfix.txt = 0), oracle re-verified build_matches=true. No new permanent fix is available at the Claude Code layer — the incident is the same recurring Windows-side Edit-tool behavior the existing signature already covers.

## 2026-09-07 22:10:19 — FALSE POSITIVE (crlf/crlf-shell-token)
- **Triggering command:** `cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && sed 's|wave_applied.json|wave2_applied.json|; s|--- census regen.*||' tmp/naming-wave-2026-09/post_apply.sh > tmp/naming-wave-2026-09/post_apply2.sh && python3 - <<'EOF'
p='tmp/naming-wave-2026-09/post_apply2.sh'; s=open(p).read()
import re
# drop the census/integrity/dry-run steps (already done by apply2) and the residual grep names; keep m2c fix + CRLF check
s=re.sub(r'echo "--- census regen".*?echo "--- residual grep"\n.*?\n', '', s, `
- **Why not a real failure:** bash syntax error came from an unbalanced quote left by a re.sub edit of tmp/naming-wave-2026-09/post_apply2.sh (line 15 'echo "'), not from CRLF; the guard matched the literal $'\r' token inside the script text that the command itself printed with cat. No file had CR bytes (verified below with a python scan).
- **Action:** tighten signature `crlf-shell-token` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.

## 2026-09-22 15:34:01 — RESOLVED (crlf/crlf-shell-token)
- **Triggering command:** `cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; cat > tmp/f2d0/perm/compile.sh <<'EOF'
#!/usr/bin/env bash
INPUT="$(realpath "$1")"
OUTPUT="$(realpath "$3")"
TMPC="$(mktemp /tmp/permbaseXXXXXX.c)"
trap 'rm -f "$TMPC"' EXIT
cp "$INPUT" "$TMPC"
cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 \
  -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -`
- **Root cause:** Windows-side python3 heredoc wrote tmp scratch .sh/.c files via open(...,'w') text mode -> CRLF (4th recurrence of the windows-python-crlf vector)
- **Permanent guard:** `tools/hooks/shell_footgun_guard.py` (committed in dc773f76)
- **Verified by:** guard rule 4 blocks the exact triggering spelling; test_shell_footgun_guard.py 12 allow + 8 block, 0 failures; offending files normalized with sed -i (grep -c CR = 0)
- **Occurrences this incident:** 1

## 2026-09-26 17:31:06 — FALSE POSITIVE (crlf/crlf-shell-token)
- **Triggering command:** `cat "/c/Users/Trenton/.claude/projects/C--Users-Trenton-desktop-Bushido-Blade-2-Decompile/memory/workflow/windows-python-crlf-write-text.md"`
- **Why not a real failure:** cat of a memory note whose prose quotes the literal text grep -c $'\r'; no CRLF file or shell parse failure was involved (the cat succeeded)
- **Action:** tighten signature `crlf-shell-token` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.

## 2026-09-26 18:03:38 — RESOLVED (crlf/crlf-shell-token)
- **Triggering command:** `git diff --stat -- docs/tooling_incidents.md; git diff -- docs/tooling_incidents.md | Select-Object -First 30`
- **Root cause:** crlf-shell-token substring $'\r' matched any output that merely quotes that text (e.g. git diff of docs/tooling_incidents.md); real bash CRLF errors always read "$'\r': command not found" or "bad interpreter"
- **Permanent guard:** `tools/hooks/tooling_error_signatures.json` (uncommitted change)
- **Verified by:** signature narrowed to full error phrases; JSON validates; file LF (0 CR); same false positive fired twice today
- **Occurrences this incident:** 1

## 2026-10-06 05:43:12 — FALSE POSITIVE (worktree-symlink/worktree-dep-missing)
- **Triggering command:** `wsl bash /mnt/c/Users/Trenton/Desktop/bb2-worktrees/rev-w2b4/tmp/r/chk.sh --base rb4_base 2>&1 | Select-Object -Last 8`
- **Why not a real failure:** Throwaway review worktree: check.sh's 'source .venv/bin/activate' fails because the repo .venv is a Windows venv (no bin/activate from WSL; worker 2's worktree is identical); the build ran on system python3 and reported SHA1 OK + snapshot, so no dependency was actually missing.
- **Action:** tighten signature `worktree-dep-missing` in tools/hooks/tooling_error_signatures.json so it no longer fires on this output.

## 2026-10-06 06:05 — FIXED (worktree-symlink/worktree-dep-missing) — corrects the 05:43 "false positive"
- **Root cause:** a reviewer removed its throwaway worktree with a recursive delete that followed the worktree's
  `.venv` junction and emptied the MAIN checkout's `.venv` (as on 2026-06-03). The 05:43 entry misread the
  resulting broken `.venv` as a Windows venv; the dependency really was missing (engine layer2 record failed 11x).
- **Fix:** main `.venv` rebuilt (`python3 -m venv --copies .venv` + `pip install -r requirements.txt`, WSL);
  verify-oracle --rebuild SHA1 62efab4f OK; the 11 failed records retried OK.
- **Permanent guard:** `tools/hooks/shell_footgun_guard.py` rule 5 (04171ba7f) blocks `git worktree remove`
  and recursive deletes under `bb2-worktrees` unless they go through `tools/safe_remove_worktree.ps1`
  (detaches junctions with `cmd /c rmdir` first). Unit-tested on 8 cases.
