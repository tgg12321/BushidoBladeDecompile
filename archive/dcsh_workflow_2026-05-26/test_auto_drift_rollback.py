#!/usr/bin/env python3
"""Test the auto_drift_repair rollback safety net via the
_signal_count predicate.

The rollback fires when:
  - asmfix_applied OR label_applied is True
  - rc != 0 OR _signal_count(after) >= _signal_count(before)

When the predicate fires, the snapshotted asmfix.txt / regfix.txt are
restored. This guards against the misdiagnosis case (the motivating
example: fix-label-drift renaming `.L1076` -> `.L1080` when the actual
root cause was maspsx silently stripping a `.set\\tnoreorder` directive
from a new file-scope `__asm__()` block, leaving subsequent functions
in the wrong reorder mode).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from auto_drift_repair import _signal_count


def empty_signals():
    return {
        "asmfix_funcs": set(),
        "dup_labels": set(),
        "regfix_funcs": set(),
        "ld_undef_labels": set(),
        "sha1_mismatch": set(),
    }


def rollback_predicate(pre, post, asmfix_applied, label_applied, rc):
    """Mirror the auto_drift_repair.main logic for testability."""
    repair_modified_files = asmfix_applied or label_applied
    repair_did_not_help = rc != 0 or _signal_count(post) >= _signal_count(pre)
    return repair_modified_files and repair_did_not_help


# ---- test 1: false positive scenario (the func_800526A0 incident) ---------
# Pre: SHA1 mismatch only (the maspsx-stripping case)
# Post: SHA1 mismatch still (the wrong-label rewrite didn't help)
# Repair: label-drift wrote a regfix rule
pre = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
post = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
assert rollback_predicate(pre, post, asmfix_applied=False, label_applied=True, rc=0), (
    "rollback should fire: SHA1 mismatch -> SHA1 mismatch with label rule rewritten"
)
print("PASS: false-positive label rewrite triggers rollback")


# ---- test 2: legitimate cascade fix (don't roll back) ----------------------
# Pre: dup_labels + asmfix + SHA1 mismatch
# Post: all clean
# Repair: asmfix-drift fixed everything
pre = {
    **empty_signals(),
    "dup_labels": {".L1", ".L2"},
    "asmfix_funcs": {"func_a"},
    "sha1_mismatch": {"build/bb2.exe"},
}
post = empty_signals()
assert not rollback_predicate(pre, post, asmfix_applied=True, label_applied=False, rc=0), (
    "rollback should NOT fire: real cascade was fully fixed"
)
print("PASS: legitimate cascade fix keeps the repair")


# ---- test 3: partial progress (don't roll back) ---------------------------
# Pre: 3 dup_labels + 2 asmfix + SHA1 mismatch (6 signals)
# Post: 1 SHA1 mismatch (the cascade fix landed, residual mismatch is elsewhere)
# Repair: strictly fewer signals -> keep
pre = {
    **empty_signals(),
    "dup_labels": {".L1", ".L2", ".L3"},
    "asmfix_funcs": {"func_a", "func_b"},
    "sha1_mismatch": {"build/bb2.exe"},
}
post = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
assert not rollback_predicate(pre, post, asmfix_applied=True, label_applied=False, rc=0), (
    "rollback should NOT fire: real progress made (6 signals -> 1)"
)
print("PASS: partial-progress cascade fix is kept")


# ---- test 4: repair made things strictly worse (roll back) -----------------
# Pre: SHA1 mismatch only
# Post: SHA1 mismatch + new linker errors (the repair introduced an undef)
pre = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
post = {
    **empty_signals(),
    "sha1_mismatch": {"build/bb2.exe"},
    "ld_undef_labels": {".L999"},
}
assert rollback_predicate(pre, post, asmfix_applied=False, label_applied=True, rc=1), (
    "rollback should fire: repair introduced new linker errors"
)
print("PASS: repair-made-things-worse triggers rollback")


# ---- test 5: no repair tools ran (don't roll back, nothing to roll back) --
# Pre: SHA1 mismatch only
# Post: same
# Repair: neither tool ran (the sha1_only-no-repair-applied early branch)
pre = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
post = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
assert not rollback_predicate(pre, post, asmfix_applied=False, label_applied=False, rc=0), (
    "rollback condition should be False when no repair was applied"
)
print("PASS: no-op (no repair applied) doesn't trigger rollback")


# ---- test 6: build rc != 0 after repair (roll back) ------------------------
# Pre: SHA1 mismatch
# Post: SHA1 mismatch + build crashed
# Repair: label-drift broke something
pre = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
post = {**empty_signals(), "sha1_mismatch": {"build/bb2.exe"}}
assert rollback_predicate(pre, post, asmfix_applied=False, label_applied=True, rc=2), (
    "rollback should fire when rc != 0 even if signal counts are equal"
)
print("PASS: post-repair build failure triggers rollback")


print()
print("All rollback predicate tests passed.")
