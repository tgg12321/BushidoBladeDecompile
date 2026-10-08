#!/usr/bin/env python3
"""PreToolUse hook on Bash|PowerShell. Targeted BLOCK of the shell-nesting
footguns that repeatedly waste agent turns on this project.

The decomp toolchain only runs under WSL, so engine/build commands are really
`wsl bash -c 'cd <root> && source .venv && python3 -m engine.cli ...'`. Authoring
that through the Bash tool nests THREE shells (Git Bash -> wsl -> bash); every
`$`, quote and backslash is parsed three times. Inline awk/sed/heredocs and
hand-escaped quotes break in ways that look like tool bugs but are quoting.

This hook blocks ONLY the clear footguns and points to the safe path. Simple
one-command `wsl bash -c 'git status'`, pipes, and POSIX one-liners are allowed.

Blocks (exit 2, reason on stderr):
  1. Nested single-quote escaping  '"'"'  or  '\''   (always a footgun)
  2. Hand-rolled engine invocation `python3 -m engine.cli ...` (use tools/wteng.ps1 main)
  3. Inside a `wsl bash -c '...'`: a heredoc (<<), a shell function def `() {`,
     or inline awk/sed touching `$` -> write a .py/.sh/.ps1 file in tmp/ and run it.

Exit codes:
  0 — allow
  2 — block
"""
from __future__ import annotations

import json
import re
import sys

NESTED_QUOTE_RE = re.compile(r"""'"'"'|'\\''""")
ENGINE_DIRECT_RE = re.compile(r"\bpython3?\s+-m\s+engine\.cli\b")
WSL_NEST_RE = re.compile(r"\bwsl(?:\.exe)?\s+bash\s+-l?c\b")
HEREDOC_RE = re.compile(r"<<-?\s*[\"']?\w")
FUNCDEF_RE = re.compile(r"\b[A-Za-z_]\w*\s*\(\)\s*\{")
AWK_SED_DOLLAR_RE = re.compile(r"\b(?:awk|sed)\b[^|;&]*\\?\$")
WSL_VAR_RE = re.compile(r"\$\{?[A-Za-z_]")
WSL_WRITE_RE = re.compile(r"\b(?:ln|cp|mv|rm|tee|install)\s|[^>&2]>{1,2}\s*[^&\s]")
ORACLE_CC1_WRITE_RE = re.compile(
    r"\b(?:ln|cp|mv|rm|install|Copy-Item|Move-Item|Remove-Item|New-Item)\b[^\n;|&]*"
    r"gcc-2\.7\.2[/\\]build[/\\]cc1(?![.\w])", re.IGNORECASE)
# Rule 4: a heredoc-fed `python3 - <<TAG` run by the WINDOWS-side shell (Git Bash's
# `python3` is the Windows Store Python) writes text-mode files with CRLF. Four
# recurrences (2026-08-06, 09-01, 09-04, 09-22) — see memory
# workflow/windows-python-crlf-write-text. Only heredoc python NOT routed via wsl.
PY_HEREDOC_RE = re.compile(r"\bpython3?(?:\.exe)?\s+-\s*<<")
TEXT_WRITE_RE = re.compile(
    r"\.write_text\(|\bopen\([^)\n]*,\s*['\"][wa]t?\+?['\"]"
)

WORKTREE_REMOVE_RE = re.compile(r"\bgit\b[^\n;&|]*\bworktree\s+remove\b")
WORKTREE_RMRF_RE = re.compile(
    r"(?:\brm\s+-[a-zA-Z]*r[a-zA-Z]*f?|\bRemove-Item\b[^\n;|]*-Recurse)[^\n;|]*bb2-worktrees",
    re.IGNORECASE)

SAFE = ("\nThe safe path:\n"
        "  - engine/build commands  ->  the PowerShell tool with  tools/wteng.ps1 main\n"
        "        & tools/wteng.ps1 main queue next\n"
        "        & tools/wteng.ps1 main sandbox func_X --disable all\n"
        "  - anything beyond ONE simple command (awk/sed/heredoc/multi-statement)\n"
        "        ->  Write a .py/.sh/.ps1 file to tmp/ and run THAT (zero nested quoting)\n"
        "  - a multi-line commit message  ->  Write tmp/msg.txt, then  git commit -F tmp/msg.txt\n"
        "See the 'PowerShell-first scripting' section of CLAUDE.md / AGENTS.md.")


_HEREDOC_OPEN_RE = re.compile(r"<<-?\s*[\"']?(\w+)[\"']?")


def _strip_heredoc_bodies(cmd: str) -> str:
    """Drop the BODY of every heredoc (the lines up to its terminator), keeping
    the `<<TAG` opener. A heredoc body is data: a Python patch that merely
    CONTAINS the text `wsl bash -c` (e.g. one editing tools/wsl.sh) is not a
    wsl invocation (false block, 2026-09-22)."""
    lines = cmd.split("\n")
    out, i = [], 0
    while i < len(lines):
        out.append(lines[i])
        tags = _HEREDOC_OPEN_RE.findall(lines[i])
        i += 1
        for tag in tags:
            while i < len(lines) and lines[i].strip() != tag:
                i += 1
            if i < len(lines):
                out.append(lines[i])   # the terminator line
                i += 1
    return "\n".join(out)


_WSL_BODY_RE = re.compile(r"""(?:^|[;&|(\s])wsl(?:\.exe)?\s+bash\s+-l?c\s+(?:'([^']*)'|"((?:[^"\\]|\\.)*)")""")


def _wsl_bodies(code: str) -> list[str]:
    """The script argument of every `wsl bash -c '...'` / "..." invocation (the text
    the nested bash runs), not prose that merely mentions it."""
    return [a or b for a, b in _WSL_BODY_RE.findall(code)]


def reasons_for(cmd: str) -> list[str]:
    out: list[str] = []
    # The wsl-nesting checks below look at the command, not heredoc data.
    code = _strip_heredoc_bodies(cmd)

    if NESTED_QUOTE_RE.search(cmd):
        out.append(
            "Nested single-quote escaping ('\"'\"' or '\\'') detected — the classic "
            "three-shell-nesting footgun. Don't hand-escape quotes through wsl."
        )

    if ENGINE_DIRECT_RE.search(cmd) and "eng.ps1" not in cmd:
        out.append(
            "Hand-rolled `python3 -m engine.cli ...`. Engine commands go through the "
            "PowerShell wrapper so there is zero quoting: "
            "`& tools/wteng.ps1 main <subcommand> ...`."
        )

    if WSL_NEST_RE.search(code):
        if "$?" in cmd:
            out.append(
                "`$?` inside `wsl bash -c '...'` is UNRELIABLE — the exit code is "
                "clobbered across the nested shells (you read the outer shell's "
                "status, not the inner command's; it silently shows 0). Capture exit "
                "codes via the PowerShell tool's `$LASTEXITCODE` after the `wsl`/`& "
                "tools/wteng.ps1 main` call instead."
            )
        if HEREDOC_RE.search(cmd):
            out.append(
                "Heredoc (<<) inside `wsl bash -c '...'`. Heredocs through nested shells "
                "are fragile. For commit messages use `git commit -F tmp/msg.txt`; "
                "otherwise write a script file to tmp/ and run it."
            )
        if FUNCDEF_RE.search(cmd):
            out.append(
                "Shell function definition `() { ... }` inside `wsl bash -c '...'`. "
                "Put the logic in a .py/.sh file in tmp/ and run that file instead."
            )
        if any(WSL_VAR_RE.search(b) and WSL_WRITE_RE.search(b) for b in _wsl_bodies(code)):
            out.append(
                "`$VAR` expansion plus a file write (ln/cp/mv/rm/tee/>) inside `wsl bash "
                "-c '...'`. A variable that expands to empty in one of the nested shells "
                "turns `cd \"$R/x\" && ...; ln -sf \"$R/...\" ...` into a write in the CURRENT "
                "directory, the main checkout (2026-10-08: main's oracle cc1 replaced by a "
                "dangling symlink). Write a .sh under tmp/ that starts `set -eu` and `cd "
                "<absolute path>`, and run that file."
            )
        if AWK_SED_DOLLAR_RE.search(cmd):
            out.append(
                "Inline awk/sed touching `$` inside `wsl bash -c '...'`. The `$` is parsed "
                "by three shells before awk sees it. Write a .py file to tmp/ and run it "
                "(a Python normalizer is both more robust and easier to read)."
            )

    if (PY_HEREDOC_RE.search(code) and not WSL_NEST_RE.search(code)
            and "wsl.sh" not in code and TEXT_WRITE_RE.search(cmd)
            and "newline=" not in cmd):
        out.append(
            "Windows-side `python3 - <<EOF` heredoc writing a file in TEXT mode "
            "(write_text / open(...,'w')) without newline='\\n'. The Bash tool's "
            "python3 is Windows Python: every line comes back CRLF and breaks .sh/.c/"
            "build files. Pass newline='\\n' (open(p, 'w', newline='\\n')), use "
            "write_bytes, or run the script under WSL (bash tools/wsl.sh)."
        )
    # Rule 5: removing a worktree directly. Worktrees here carry directory
    # JUNCTIONS to main's .venv / tools / build; `git worktree remove --force`,
    # `rm -rf` or `Remove-Item -Recurse` on one follows the junctions and empties
    # MAIN's targets (2026-06-03 and again 2026-10-06: main's .venv wiped).
    if (WORKTREE_REMOVE_RE.search(code) or WORKTREE_RMRF_RE.search(code)) \
            and "safe_remove_worktree" not in code:
        out.append(
            "Direct worktree removal. Worktrees hold junctions to main's .venv / tools / "
            "build, and a recursive delete follows them and wipes MAIN's copies. Use "
            "`pwsh tools/safe_remove_worktree.ps1 <worktree-path>` (detaches junctions "
            "with `cmd /c rmdir`, then runs `git worktree remove`)."
        )
    # Rule 6: the operative oracle compiler is installed ONLY by
    # tools/build_oracle_cc1.sh --install (self-checked); a direct ln/cp/mv/rm on it
    # is how it was lost on 2026-10-08.
    if ORACLE_CC1_WRITE_RE.search(code) and "build_oracle_cc1" not in code:
        out.append(
            "Direct write to the oracle compiler tools/gcc-2.7.2/build/cc1. It is "
            "installed only by `bash tools/build_oracle_cc1.sh --install` "
            "(docs/ORACLE-COMPILER.md); never ln/cp/mv/rm it by hand."
        )
    return out


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0
    if payload.get("tool_name") not in ("Bash", "PowerShell"):
        return 0
    cmd = payload.get("tool_input", {}).get("command", "")
    if not cmd:
        return 0

    reasons = reasons_for(cmd)
    if reasons:
        print("BLOCKED by shell_footgun_guard.py — shell-nesting footgun:", file=sys.stderr)
        for r in reasons:
            print(f"  - {r}", file=sys.stderr)
        print(SAFE, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
