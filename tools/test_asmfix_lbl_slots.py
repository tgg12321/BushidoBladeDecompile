#!/usr/bin/env python3
"""Unit tests for the {lbl#N} function-local label slot substitution added to
tools/asmfix.py (2026-06-08). Mirrors tools/test_regfix_lbl_slots.py — the
asmfix mechanism is the exact same shape as regfix's, applied to asmfix's
own rule-replacement strings (rename dst, replace_first/all replacement,
insert_before/after text).

Run: python3 tools/test_asmfix_lbl_slots.py
"""
from __future__ import annotations

import io
import contextlib
import sys
from pathlib import Path

# Import asmfix.py (not a package, so use the script path directly)
_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import asmfix  # noqa: E402


_passed = _failed = 0


def check(desc: str, cond: bool) -> None:
    global _passed, _failed
    if cond:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}", file=sys.stderr)


def eq(desc: str, got, want) -> None:
    check(f"{desc} (got {got!r}, want {want!r})", got == want)


# ---------------------------------------------------------------------------
# _extract_local_label_defs: scans a function-block text for `.L<N>:` defs
# ---------------------------------------------------------------------------

def test_extract():
    # Empty input
    eq("extract: empty", asmfix._extract_local_label_defs(""), [])

    # Single label, no indent
    eq("extract: single, no indent",
       asmfix._extract_local_label_defs(".L100:\n"),
       [".L100"])

    # Tab-indented (cc1's actual output) and untagged variants
    eq("extract: tab-indented",
       asmfix._extract_local_label_defs("\t.L42:\n"),
       [".L42"])

    # Document order preserved
    block = (
        "myfunc:\n"
        "\tsubu\t$sp,$sp,64\n"
        "\tlw\t$2,0($a0)\n"
        ".L152:\n"
        "\tbeqz\t$2,.L154\n"
        ".L154:\n"
        "\tjr\t$ra\n"
        ".L160:\n"
        "\t.end\tmyfunc\n"
    )
    eq("extract: doc order from full block",
       asmfix._extract_local_label_defs(block),
       [".L152", ".L154", ".L160"])

    # Branch refs not flagged — only definitions (`.L<N>:` on its own line).
    # A `.L<N>` appearing as a branch target (e.g. `j\t.L42`) is NOT a def.
    eq("extract: branch ref not flagged",
       asmfix._extract_local_label_defs("\tj\t.L42\n.L42:\n"),
       [".L42"])


# ---------------------------------------------------------------------------
# _substitute_label_slots: replaces {lbl#N} with the Nth label
# ---------------------------------------------------------------------------

def test_substitute_basic():
    labels = [".L100", ".L200", ".L300", ".L400"]

    eq("substitute: text without placeholder unchanged",
       asmfix._substitute_label_slots("plain text no slots", labels),
       "plain text no slots")

    eq("substitute: single {lbl#1}",
       asmfix._substitute_label_slots("j\t{lbl#1}", labels),
       "j\t.L100")

    eq("substitute: {lbl#3}",
       asmfix._substitute_label_slots(".L8003567C", labels),
       ".L8003567C")  # no placeholder = unchanged (regression guard)

    eq("substitute: multiple in one string",
       asmfix._substitute_label_slots("bltz {lbl#1}; j {lbl#4}; j {lbl#1}", labels),
       "bltz .L100; j .L400; j .L100")


def test_substitute_drift():
    """Same {lbl#N} resolves to DIFFERENT labels when cc1's output drifts —
    the drift-robustness property the mechanism exists to provide."""
    original = [".L152", ".L154", ".L156", ".L158", ".L159", ".L160", ".L174"]
    drifted = [".L157", ".L159", ".L161", ".L163", ".L164", ".L165", ".L179"]
    template = "bltz $5,{lbl#1}; j {lbl#5}; j {lbl#7}"

    eq("drift: original",
       asmfix._substitute_label_slots(template, original),
       "bltz $5,.L152; j .L159; j .L174")

    eq("drift: shifted -- SAME rule, NEW resolution",
       asmfix._substitute_label_slots(template, drifted),
       "bltz $5,.L157; j .L164; j .L179")


def test_substitute_out_of_range():
    labels = [".L100", ".L200"]
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        got = asmfix._substitute_label_slots("j {lbl#7}", labels, fname="testfn")
    check("out-of-range: emits unresolved-marker",
          got == "j .L_UNRESOLVED_lbl_7")
    msg = buf.getvalue()
    check("out-of-range: warns on stderr",
          "asmfix: WARNING" in msg and "{lbl#7}" in msg and "testfn" in msg)


# ---------------------------------------------------------------------------
# Integration: apply_ops through every rule kind that accepts {lbl#N}
# ---------------------------------------------------------------------------

_FUNC_BLOCK = (
    "myfunc:\n"
    "\tsubu\t$sp,$sp,32\n"
    "\tsw\t$ra,28($sp)\n"
    ".L100:\n"
    "\tbeqz\t$4,.L200\n"
    "\tnop\n"
    ".L200:\n"
    "\tlw\t$2,0($4)\n"
    "\tj\t.L300\n"
    "\tnop\n"
    ".L300:\n"
    "\tlw\t$ra,28($sp)\n"
    "\tjr\t$ra\n"
    "\taddiu\t$sp,$sp,32\n"
    "\t.end\tmyfunc\n"
)


def test_apply_ops_rename_dst():
    """rename's destination accepts {lbl#N}; source pattern is a literal label."""
    # Within _FUNC_BLOCK labels are [.L100, .L200, .L300] — so {lbl#3} = .L300.
    out = asmfix.apply_ops("myfunc", _FUNC_BLOCK, [("rename", ".L100", "{lbl#3}_x")])
    check("rename: {lbl#3} expanded in destination",
          ".L300_x:" in out)
    check("rename: original .L100 label replaced",
          ".L100:" not in out)


def test_apply_ops_replace_first():
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("replace_first", r"j\t\.L300", "j\t{lbl#1}")],
    )
    check("replace_first: {lbl#1} expanded",
          "j\t.L100" in out)


def test_apply_ops_replace_all():
    # Replace all `.L200` refs with {lbl#3} = .L300 (covering both the branch
    # AND the label definition itself, since replace_all is a regex over text).
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("replace_all", r"\.L200", "{lbl#3}")],
    )
    check("replace_all: {lbl#3} expanded everywhere",
          ".L200" not in out and out.count(".L300") >= 2)


def test_apply_ops_insert_after():
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("insert_after", r"\.L100:", "\tj\t{lbl#2}")],
    )
    check("insert_after: {lbl#2} expanded",
          "j\t.L200" in out)


def test_apply_ops_insert_before():
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("insert_before", r"\.L300:", "\tj\t{lbl#1}")],
    )
    check("insert_before: {lbl#1} expanded",
          "j\t.L100" in out)


def test_apply_ops_delete_between_no_sub():
    """delete_between with literal `.L` patterns (no `{lbl#N}`) — verifies the
    rule still works unchanged when no slot refs are present (regression
    guard for the source-side substitution added 2026-06-08 Phase 2b)."""
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("delete_between", r"\.L100:", r"\.L200:")],
    )
    check("delete_between: still operates on plain source patterns",
          ".L100:" not in out and ".L200:" in out)


# ---------------------------------------------------------------------------
# Phase 2b (2026-06-08): source-side {lbl#N} substitution
# ---------------------------------------------------------------------------

def test_apply_ops_rename_src_sub():
    """rename's SOURCE pattern accepts {lbl#N}. Within _FUNC_BLOCK the labels
    are [.L100, .L200, .L300], so `{lbl#1}` resolves to `.L100`."""
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("rename", "{lbl#1}", "jtbl_800108CC")],
    )
    check("rename src: {lbl#1} resolved on source side",
          ".L100:" not in out and "jtbl_800108CC:" in out)


def test_apply_ops_replace_first_both_sides_sub():
    """replace_first with {lbl#N} on BOTH source pattern and replacement —
    each side resolves to the function-local label sequence. Python's re
    treats literal `{...}` (when not a valid quantifier) as text, so
    `{lbl#3}` in the source pattern is replaced verbatim BEFORE the regex
    compiles, yielding a regex like `j\\t\\.L300`."""
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("replace_first", r"j\t{lbl#3}", "j\t{lbl#1}")],
    )
    # Source resolves to literal `j\t.L300`; replacement to `j\t.L100`.
    check("replace_first src + dst: both {lbl#N} resolved",
          "j\t.L100" in out and "j\t.L300" not in out)


def test_apply_ops_replace_all_src_sub():
    """replace_all source-side {lbl#N}: replace every `.L200` ref using
    `{lbl#2}` on the source side, substituted before regex compile."""
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("replace_all", r"{lbl#2}", ".LMOVED")],
    )
    # {lbl#2}=.L200 — source resolves to literal `.L200` (regex matches it).
    # Note: `.L200` will match anywhere `.L200` appears (the `.` is regex any,
    # but it also matches the literal dot). Should clear both ref and def.
    check("replace_all src: {lbl#2} (= .L200) resolved on source side",
          ".L200" not in out and out.count(".LMOVED") >= 2)


def test_apply_ops_delete_between_both_sides_sub():
    """delete_between with `{lbl#N}` on both pattern sides — each resolves
    independently."""
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("delete_between", r"{lbl#1}:", r"{lbl#2}:")],
    )
    # {lbl#1}=.L100, {lbl#2}=.L200. So delete from `.L100:` to `.L200:`.
    check("delete_between: both endpoints resolved via {lbl#N}",
          ".L100:" not in out and ".L200:" in out)


def test_apply_ops_insert_before_src_sub():
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("insert_before", r"{lbl#3}:", "\tj\t{lbl#1}")],
    )
    # {lbl#3}=.L300, so insert before `.L300:`
    check("insert_before: src {lbl#3} resolved",
          ".L300:" in out and "j\t.L100" in out)


def test_apply_ops_insert_after_src_sub():
    out = asmfix.apply_ops(
        "myfunc", _FUNC_BLOCK,
        [("insert_after", r"{lbl#1}:", "\tj\t{lbl#2}")],
    )
    check("insert_after: src {lbl#1} resolved",
          ".L100:" in out and "j\t.L200" in out)


def test_apply_ops_src_out_of_range_fails_loud():
    """Out-of-range slot refs on the source side emit the same warning +
    unresolved-marker as replacement-side refs do."""
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        asmfix.apply_ops(
            "myfunc", _FUNC_BLOCK,
            [("replace_first", r"{lbl#99}", "X")],
        )
    msg = buf.getvalue()
    check("out-of-range src: warning on stderr for {lbl#99}",
          "asmfix: WARNING" in msg and "{lbl#99}" in msg)


def test_apply_ops_out_of_range_fails_loud():
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        out = asmfix.apply_ops(
            "myfunc", _FUNC_BLOCK,
            [("insert_after", r"\.L100:", "\tj\t{lbl#99}")],
        )
    check("out-of-range integration: warning on stderr",
          "asmfix: WARNING" in buf.getvalue() and "{lbl#99}" in buf.getvalue())
    check("out-of-range integration: unresolved marker in output",
          ".L_UNRESOLVED_lbl_99" in out)


def main():
    test_extract()
    test_substitute_basic()
    test_substitute_drift()
    test_substitute_out_of_range()
    test_apply_ops_rename_dst()
    test_apply_ops_replace_first()
    test_apply_ops_replace_all()
    test_apply_ops_insert_after()
    test_apply_ops_insert_before()
    test_apply_ops_delete_between_no_sub()
    test_apply_ops_out_of_range_fails_loud()
    # Phase 2b: source-side substitution
    test_apply_ops_rename_src_sub()
    test_apply_ops_replace_first_both_sides_sub()
    test_apply_ops_replace_all_src_sub()
    test_apply_ops_delete_between_both_sides_sub()
    test_apply_ops_insert_before_src_sub()
    test_apply_ops_insert_after_src_sub()
    test_apply_ops_src_out_of_range_fails_loud()

    print(f"\n{_passed} passed, {_failed} failed")
    return 0 if _failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
