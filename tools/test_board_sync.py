#!/usr/bin/env python3
"""Unit tests for tools/board_sync.py.

Run: python tools/test_board_sync.py   (exit 0 = pass)
"""
from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
from pathlib import Path

_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import board_sync  # noqa: E402

_passed = _failed = 0

def check(desc, cond):
    global _passed, _failed
    if cond:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}", file=sys.stderr)

def eq(desc, got, want):
    check(f"{desc} (got {got!r}, want {want!r})", got == want)


def test_load_queue():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "queue.json"
        p.write_text(json.dumps({"items": [
            {"func": "func_1", "file": "a", "distance": 9, "verdict": "C", "rules": 3, "status": "active"},
        ]}), encoding="utf-8")
        items = board_sync.load_queue(p)
        eq("loads one item", len(items), 1)
        eq("item func", items[0]["func"], "func_1")

def test_load_queue_missing():
    items = board_sync.load_queue(Path("does/not/exist.json"))
    eq("missing queue -> empty list", items, [])


def main():
    test_load_queue()
    test_load_queue_missing()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0

if __name__ == "__main__":
    sys.exit(main())
