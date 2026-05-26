#!/usr/bin/env python3
"""Test the auto_drift_repair maspsx-stripping scanner.

The scanner reads src/*.c and flags file-scope `__asm__()` blocks
that use TAB-form `.set noreorder` / `.set noat` / `.set reorder` /
`.set at` directives without a matching SPACE-form duplicate. Maspsx
silently strips the TAB form (see tools/maspsx/maspsx/__init__.py:894),
so a block missing the SPACE form leaves `as` in the wrong mode for
either the asm body itself or subsequent functions.
"""
import os
import sys
import tempfile
from pathlib import Path

HERE = Path(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, str(HERE))


def test_scanner(tmp_dir: Path, body: str) -> list[tuple[str, int, str]]:
    """Set up a fake ROOT with src/ + given .c body, run the scanner."""
    src_dir = tmp_dir / "src"
    src_dir.mkdir(parents=True, exist_ok=True)
    (src_dir / "test.c").write_text(body, encoding="utf-8", newline="\n")

    import auto_drift_repair
    saved_root = auto_drift_repair.ROOT
    auto_drift_repair.ROOT = tmp_dir
    try:
        return auto_drift_repair._scan_unbalanced_set_directives()
    finally:
        auto_drift_repair.ROOT = saved_root


# ---- test 1: balanced TAB + SPACE directives -> no issues ------------------
balanced = '''
__asm__(
    ".set\\tnoat\\n"
    ".set\\tnoreorder\\n"
    ".set noat\\n"
    ".set noreorder\\n"
    "glabel func_TEST\\n"
    "    nop\\n"
    "    jr   $ra\\n"
    "    nop\\n"
    ".set\\treorder\\n"
    ".set\\tat\\n"
    ".set reorder\\n"
    ".set at\\n"
);
'''

with tempfile.TemporaryDirectory() as td:
    issues = test_scanner(Path(td), balanced)
    assert issues == [], f"balanced should produce no issues, got: {issues}"
print("PASS: balanced TAB + SPACE directives produce no warnings")


# ---- test 2: TAB-only (my func_800526A0 first-attempt mistake) -------------
tab_only = '''
__asm__(
    ".set\\tnoat\\n"
    ".set\\tnoreorder\\n"
    "glabel func_TEST\\n"
    "    nop\\n"
    "    jr   $ra\\n"
    "    nop\\n"
    ".set\\treorder\\n"
    ".set\\tat\\n"
);
'''

with tempfile.TemporaryDirectory() as td:
    issues = test_scanner(Path(td), tab_only)
    found = {directive for _, _, directive in issues}
    assert found == {"noat", "noreorder", "reorder", "at"}, (
        f"TAB-only should flag all 4 directives, got: {found}"
    )
print("PASS: TAB-only block flags all 4 missing SPACE duplicates")


# ---- test 3: non-glabel block (just `__asm__("nop")` style) is ignored -----
non_glabel = '''
void func(void) {
    __asm__("nop\\n");
}
'''
with tempfile.TemporaryDirectory() as td:
    issues = test_scanner(Path(td), non_glabel)
    assert issues == [], f"non-glabel should be ignored, got: {issues}"
print("PASS: function-body `__asm__` blocks are not flagged")


# ---- test 4: partial coverage (TAB has all 4, SPACE has only some) ---------
partial = '''
__asm__(
    ".set\\tnoat\\n"
    ".set\\tnoreorder\\n"
    ".set noreorder\\n"
    "glabel func_TEST\\n"
    "    nop\\n"
    "    jr   $ra\\n"
    "    nop\\n"
    ".set\\treorder\\n"
    ".set\\tat\\n"
    ".set reorder\\n"
);
'''
with tempfile.TemporaryDirectory() as td:
    issues = test_scanner(Path(td), partial)
    found = {directive for _, _, directive in issues}
    # noat has TAB form but no SPACE; at has TAB form but no SPACE
    # noreorder and reorder are balanced
    assert found == {"noat", "at"}, (
        f"partial coverage should flag noat and at only, got: {found}"
    )
print("PASS: partial coverage flags only the missing directives")


# ---- test 5: empty src/ -> no issues, no crash -----------------------------
with tempfile.TemporaryDirectory() as td:
    src_dir = Path(td) / "src"
    src_dir.mkdir(parents=True, exist_ok=True)
    import auto_drift_repair
    saved_root = auto_drift_repair.ROOT
    auto_drift_repair.ROOT = Path(td)
    try:
        issues = auto_drift_repair._scan_unbalanced_set_directives()
    finally:
        auto_drift_repair.ROOT = saved_root
    assert issues == [], f"empty src/ should produce no issues, got: {issues}"
print("PASS: empty src/ handled without crash")


print()
print("All tests passed.")
