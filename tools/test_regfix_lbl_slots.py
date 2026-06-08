#!/usr/bin/env python3
"""Unit tests for the {lbl#N} function-local label slot substitution added to
tools/regfix.py (2026-06-01). The substitution makes splice / subst /
insert / insert_after rule contents robust to TU-wide `.L<N>` renumbering.

Run: python3 tools/test_regfix_lbl_slots.py
"""
from __future__ import annotations

import io
import contextlib
import sys
from pathlib import Path

# Import regfix.py (not a package, so use the script path directly)
_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import regfix  # noqa: E402


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
# _extract_local_label_defs: scans (text, idx) tuples for `.L<N>:` defs
# ---------------------------------------------------------------------------

def test_extract():
    # Empty input
    eq("extract: empty", regfix._extract_local_label_defs([]), [])

    # Single label, no indent
    eq("extract: single, no indent",
       regfix._extract_local_label_defs([(".L100:\n", None)]),
       [".L100"])

    # Tab-indented (cc1's actual output)
    eq("extract: tab-indented",
       regfix._extract_local_label_defs([("\t.L42:\n", None)]),
       [".L42"])

    # Document order preserved
    lines = [
        ("\tsubu\t$sp,$sp,64\n", 0),
        ("\tlw\t$2,0($a0)\n", 1),
        (".L152:\n", None),
        ("\tbeqz\t$2,.L154\n", 2),
        (".L154:\n", None),
        ("\tjr\t$ra\n", 3),
        (".L160:\n", None),  # at end, no following instr (still extracted)
    ]
    eq("extract: doc order",
       regfix._extract_local_label_defs(lines),
       [".L152", ".L154", ".L160"])

    # Lines with trailing CRLF
    eq("extract: CRLF tolerated",
       regfix._extract_local_label_defs([("\t.L42:\r\n", None)]),
       [".L42"])

    # Not a label (no colon)
    eq("extract: instruction not flagged",
       regfix._extract_local_label_defs([("\tj\t.L42\n", 0)]),
       [])

    # Branch target on same line — only DEFS count, not references
    eq("extract: branch ref not flagged",
       regfix._extract_local_label_defs([
           ("\tbeq\t$2,$0,.L42\n", 0),
           (".L42:\n", None),
       ]),
       [".L42"])


# ---------------------------------------------------------------------------
# _substitute_label_slots: replaces {lbl#N} with the Nth label
# ---------------------------------------------------------------------------

def test_substitute_basic():
    labels = [".L100", ".L200", ".L300", ".L400"]

    eq("substitute: text without placeholder unchanged",
       regfix._substitute_label_slots("plain text no slots", labels),
       "plain text no slots")

    eq("substitute: single {lbl#1}",
       regfix._substitute_label_slots("j\t{lbl#1}", labels),
       "j\t.L100")

    eq("substitute: {lbl#3}",
       regfix._substitute_label_slots("beq\t$2,$0,{lbl#3}", labels),
       "beq\t$2,$0,.L300")

    eq("substitute: multiple in one string",
       regfix._substitute_label_slots("bltz {lbl#1}; j {lbl#4}; j {lbl#1}", labels),
       "bltz .L100; j .L400; j .L100")


def test_substitute_drift():
    """The key property: same {lbl#N} resolves to DIFFERENT labels when cc1's
    output drifts. This is the drift-robustness guarantee."""

    # Original cc1 output: 7 labels at .L152..L174
    original = [".L152", ".L154", ".L156", ".L158", ".L159", ".L160", ".L174"]

    # After drift: same 7 basic blocks, all label numbers shifted +5
    drifted = [".L157", ".L159", ".L161", ".L163", ".L164", ".L165", ".L179"]

    # The splice rule references slots 1, 2, 5, 7 — should resolve to the
    # appropriate label IN EACH cc1-output state.
    template = "bltz $5,{lbl#1}; beq $0,$0,{lbl#2}; j {lbl#5}; j {lbl#7}"

    eq("drift: original output",
       regfix._substitute_label_slots(template, original),
       "bltz $5,.L152; beq $0,$0,.L154; j .L159; j .L174")

    eq("drift: shifted output -- SAME rule, NEW label resolution",
       regfix._substitute_label_slots(template, drifted),
       "bltz $5,.L157; beq $0,$0,.L159; j .L164; j .L179")


def test_substitute_out_of_range():
    labels = [".L100", ".L200"]

    # Slot 3 out of range — should emit unresolved-marker + warning
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        got = regfix._substitute_label_slots("j {lbl#3}", labels, fname="testfn")
    check("out-of-range: emits unresolved-marker",
          got == "j .L_UNRESOLVED_lbl_3")
    check("out-of-range: warns on stderr",
          "regfix: WARNING" in buf.getvalue()
          and "{lbl#3}" in buf.getvalue()
          and "testfn" in buf.getvalue())


# ---------------------------------------------------------------------------
# Integration: process_function with a synthesized splice rule
# ---------------------------------------------------------------------------

def test_process_function_splice_integration():
    """End-to-end through process_function: synthetic function with 3 labels,
    a splice rule using {lbl#1} and {lbl#2}, verify output."""

    # Synthetic cc1-output for a 5-instruction function with 3 labels
    lines = [
        ("\taddu\t$2,$0,$zero\n", 0),
        ("\tbeqz\t$2,.L100\n", 1),
        ("\taddiu\t$3,$0,5\n", 2),
        (".L100:\n", None),
        ("\tlw\t$4,0($5)\n", 3),
        (".L200:\n", None),
        ("\tjr\t$ra\n", 4),
        (".L300:\n", None),
    ]
    # Splice replacing idx 2 with two new instructions using {lbl#1} and {lbl#2}
    config = {
        '__name__': 'test_func',
        'swaps': [],
        'substs': [],
        'subst_multis': [],
        'inserts': [],
        'insert_afters': [],
        'insert_labels': [],
        'splices': [(2, 2, ["j\t{lbl#1}", "nop", "addiu\t$3,$0,99"])],
        'deletes': [],
        'fill_delays': [],
        'drain_delays': [],
        'reorders': [],
    }

    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        result = regfix.process_function(lines, config)

    # The result is the post-splice line list
    text_out = "".join(text for text, _idx in result)

    check("integration: {lbl#1} resolved in output",
          "j\t.L100" in text_out)
    check("integration: replacement instructions emitted",
          "addiu\t$3,$0,99" in text_out)
    check("integration: original splice-target instruction removed",
          "addiu\t$3,$0,5" not in text_out)


# ---------------------------------------------------------------------------
# Phase 2b (2026-06-08): source-side {lbl#N} substitution for subst /
# subst_multi rules. The source pattern (the regex used to find the maspsx
# line to rewrite) now also accepts `{lbl#N}` placeholders, resolved against
# the same per-function label sequence as the replacement.
# ---------------------------------------------------------------------------

def _make_config(**kwargs):
    cfg = {
        '__name__': kwargs.pop('__name__', 'test_func'),
        'swaps': [],
        'substs': [],
        'subst_multis': [],
        'inserts': [],
        'insert_afters': [],
        'insert_labels': [],
        'splices': [],
        'deletes': [],
        'fill_delays': [],
        'drain_delays': [],
        'reorders': [],
    }
    cfg.update(kwargs)
    return cfg


def test_subst_src_lbl_slot():
    """A `subst` rule with `{lbl#N}` on the SOURCE side resolves to the
    function-local label before the regex matches."""
    lines = [
        ("\tbeqz\t$2,.L100\n", 0),
        ("\tnop\n", 1),
        (".L100:\n", None),
        ("\tjr\t$ra\n", 2),
        (".L200:\n", None),
        ("\tnop\n", 3),
        (".L300:\n", None),
    ]
    # Source pattern uses {lbl#1} (= .L100) — match `beqz ... {lbl#1}` and
    # rewrite it to `beqz ... {lbl#3}` (= .L300).
    cfg = _make_config(substs=[(0, r"{lbl#1}", "{lbl#3}")])
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        result = regfix.process_function(lines, cfg)
    text_out = "".join(t for t, _ in result)
    check("subst src {lbl#1} -> dst {lbl#3}: beqz target now .L300",
          "beqz\t$2,.L300" in text_out and "beqz\t$2,.L100" not in text_out)


def test_subst_multi_src_lbl_slot():
    """A `subst_multi` rule with `{lbl#N}` in the SOURCE regex resolves
    before the match."""
    lines = [
        ("\tblez\t$2,.L100\n", 0),
        ("\tnop\n", 1),
        (".L100:\n", None),
        ("\tjr\t$ra\n", 2),
    ]
    cfg = _make_config(subst_multis=[(0, r"blez\s+\$2,{lbl#1}", ["slt\t$2,$6,$3", "beq\t$2,$0,{lbl#1}"])])
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        result = regfix.process_function(lines, cfg)
    text_out = "".join(t for t, _ in result)
    check("subst_multi src + dst: {lbl#1} resolved on both sides",
          "slt\t$2,$6,$3" in text_out and "beq\t$2,$0,.L100" in text_out)


def test_subst_src_out_of_range_warns():
    """An out-of-range slot ref on the SOURCE side emits the same warning
    + unresolved-marker (`.L_UNRESOLVED_lbl_N`) the replacement side does;
    the resulting regex will not match anything so the subst is silently
    not-applied (subst already has a 'did-not-match' diagnostic)."""
    lines = [
        ("\tnop\n", 0),
        (".L100:\n", None),
    ]
    cfg = _make_config(substs=[(0, r"{lbl#99}", "X")])
    buf = io.StringIO()
    with contextlib.redirect_stderr(buf):
        regfix.process_function(lines, cfg)
    msg = buf.getvalue()
    check("subst src out-of-range: warning on stderr for {lbl#99}",
          "regfix: WARNING" in msg and "{lbl#99}" in msg)


def main():
    test_extract()
    test_substitute_basic()
    test_substitute_drift()
    test_substitute_out_of_range()
    test_process_function_splice_integration()
    # Phase 2b: source-side substitution
    test_subst_src_lbl_slot()
    test_subst_multi_src_lbl_slot()
    test_subst_src_out_of_range_warns()

    print(f"\n{_passed} passed, {_failed} failed")
    return 0 if _failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
