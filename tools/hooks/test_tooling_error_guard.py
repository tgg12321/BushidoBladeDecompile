#!/usr/bin/env python3
"""Unit tests for tooling_error_guard.py classification.

Run:  python3 tools/hooks/test_tooling_error_guard.py
Exit 0 = all pass, 1 = failure(s). No external deps.

These tests are the safety net for a hook that gates the Stop event: the
POSITIVE cases prove real failures are caught; the NEGATIVE cases prove normal
build/decomp output never trips the guard (a false positive that blocks
legitimate work is the worst outcome).
"""
from __future__ import annotations

import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import tooling_error_guard as g  # noqa: E402

SIGS = g.load_signatures()

_passed = 0
_failed = 0


def check(desc: str, cond: bool) -> None:
    global _passed, _failed
    if cond:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}")


def expect_block(desc: str, tool: str, text: str, klass: str | None = None) -> None:
    sig = g.classify_text(tool, text, SIGS)
    ok = sig is not None and sig.get("tier") == "block"
    if ok and klass is not None:
        ok = sig.get("class") == klass
    check(f"{desc} -> expected block"
          + (f" (class={klass})" if klass else "")
          + f"; got {sig.get('id') if sig else None}/{sig.get('class') if sig else None}", ok)


def expect_none(desc: str, tool: str, text: str) -> None:
    sig = g.classify_text(tool, text, SIGS)
    check(f"{desc} -> expected NO match; got {sig.get('id') if sig else None}", sig is None)


def main() -> int:
    # --- library sanity ---
    check("signatures.json loads + has signatures", bool(SIGS.get("signatures")))
    check("build_file config present", bool(SIGS.get("build_file", {}).get("suffixes")))

    # --- CRLF (shell) ---
    expect_block("crlf $'\\r' command-not-found", "Bash",
                 "bash: line 1: $'\\r': command not found", "crlf")
    expect_block("crlf bad interpreter", "Bash",
                 "/bin/bash^M: bad interpreter: No such file or directory", "crlf")

    # --- WSL unavailable ---
    expect_block("wsl not recognized (PowerShell)", "PowerShell",
                 "wsl : The term 'wsl' is not recognized as the name of a cmdlet", "wsl")
    expect_block("WslRegisterDistribution failed", "Bash",
                 "WslRegisterDistribution failed with error: 0x8007019e", "wsl")
    expect_block("no installed distributions", "Bash",
                 "Windows Subsystem for Linux has no installed distributions.", "wsl")

    # --- worktree / missing linked dep ---
    expect_block("Toolchain: MISSING", "Bash",
                 "Toolchain: MISSING 1 required dep(s):\n  tools/gcc-2.7.2/build/cc1", "worktree-symlink")
    expect_block("cc1 not found", "Bash",
                 "/bin/sh: cc1: not found", "worktree-symlink")
    expect_block("mipsel toolchain not found", "Bash",
                 "mipsel-linux-gnu-gcc: command not found", "worktree-symlink")
    expect_block("venv activate missing", "Bash",
                 "bash: .venv/bin/activate: No such file or directory", "worktree-symlink")
    expect_block("m2c module missing", "Bash",
                 "ModuleNotFoundError: No module named 'm2c'", "worktree-symlink")

    # --- broken symlink loop ---
    expect_block("symlink loop", "Bash",
                 "ls: cannot access 'disc': Too many levels of symbolic links", "worktree-symlink")

    # --- core tool not found ---
    expect_block("make not found", "Bash", "bash: make: command not found", "environment")
    expect_block("python3 not found", "Bash", "/usr/bin/env: python3: command not found", "environment")

    # --- NEGATIVE: normal output must NOT match ---
    expect_none("clean build success", "Bash", "OK: bb2 matches!\nbuild/bb2.exe: OK")
    expect_none("verify mismatch (decomp signal, not tooling)", "Bash",
                "dc.sh verify func_80012345 -> func_80012345: MISMATCH (3 insns differ)")
    expect_none("grep empty result", "Bash", "")
    expect_none("make entering directory", "Bash",
                "make[1]: Entering directory '/mnt/c/.../Bushido Blade 2 Decompile'")
    expect_none("make no rule (not a missing-tool)", "Bash",
                "make: *** No rule to make target 'foo'.  Stop.")
    expect_none("unrelated command-not-found (non-core tool)", "Bash",
                "bash: foobarbaz: command not found")
    expect_none("the words 'not found' in prose", "Bash",
                "Function func_80012345 not found in WORK_QUEUE.md")
    expect_none("permuter score output", "Bash",
                "[permuter] iteration 4000, best score = 0 (found match!)")

    # --- NEGATIVE: self-inspection (displayed source/config) must NOT match ---
    # grep/cat/head of a script or the signatures JSON that *contains* a failure
    # string is not a tooling failure. (Regression: 2026-05-25 grep of dc.sh.)
    expect_none("grep -n of dc.sh source (echo + >&2 + line-no)", "Bash",
                '379:            echo "ERROR: tools/worktree_bootstrap.sh missing '
                '— checkout is incomplete" >&2')
    expect_none("grep of signatures JSON array element", "Bash",
                '        "worktree_bootstrap.sh missing",')
    # grep -rn prefixes "path:lineno:" before the content, defeating the
    # leading-digit / leading-quote markers. (Regression: 2026-05-26 grep -rn
    # of tools/hooks/ surfaced the signature string in its own test+JSON.)
    expect_none("grep -rn of test source (path:line: prefix)", "Bash",
                'tools/hooks/test_tooling_error_guard.py:113:                \''
                '        "worktree_bootstrap.sh missing",\')')
    expect_none("grep -rn of signatures JSON (path:line: prefix)", "Bash",
                'tools/hooks/tooling_error_signatures.json:91:        '
                '"worktree_bootstrap.sh missing",')
    expect_none("cat of signatures JSON summary key", "Bash",
                '      "summary": "A gitignored build dependency (cc1 / maspsx) is missing.",')
    expect_none("python source printing a tool-error string", "Bash",
                '    print("cc1: not found", file=sys.stderr)')
    # ...but the SAME failure as real (bare) output still blocks:
    expect_block("real bootstrap-missing failure (bare stderr)", "Bash",
                 "ERROR: tools/worktree_bootstrap.sh missing — checkout is incomplete",
                 "worktree-symlink")

    # --- build-file detection ---
    check("src/foo.c is build-critical", g.is_build_critical("src/foo.c", SIGS))
    check("regfix.txt is build-critical", g.is_build_critical("regfix.txt", SIGS))
    check("sdata_funcs.txt is build-critical (glob)", g.is_build_critical("sdata_funcs.txt", SIGS))
    check("tools/x.sh is build-critical", g.is_build_critical("tools/x.sh", SIGS))
    check("docs/STATUS.md is NOT build-critical", not g.is_build_critical("docs/STATUS.md", SIGS))
    check("notes.txt is NOT build-critical", not g.is_build_critical("notes.txt", SIGS))

    # --- CRLF file detection ---
    with tempfile.TemporaryDirectory() as td:
        crlf = Path(td) / "a.c"
        crlf.write_bytes(b"int main(){\r\n  return 0;\r\n}\r\n")
        lf = Path(td) / "b.c"
        lf.write_bytes(b"int main(){\n  return 0;\n}\n")
        check("file_has_crlf detects CRLF", g.file_has_crlf(str(crlf)))
        check("file_has_crlf clean on LF", not g.file_has_crlf(str(lf)))

    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0


if __name__ == "__main__":
    sys.exit(main())
