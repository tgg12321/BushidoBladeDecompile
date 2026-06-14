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


def test_wip_exists():
    with tempfile.TemporaryDirectory() as td:
        wip = Path(td)
        (wip / "func_has").mkdir()
        eq("wip present", board_sync.wip_exists("func_has", wip), True)
        eq("wip absent", board_sync.wip_exists("func_none", wip), False)


def test_build_desired_status_mapping():
    items = [
        {"func": "func_a", "file": "x", "distance": 9, "verdict": "C", "rules": 3, "status": "active"},
        {"func": "func_b", "file": "y", "distance": 4, "verdict": "ASM-STRUCTURAL", "rules": 1, "status": "authorize"},
        {"func": "func_c", "file": "z", "distance": -1, "verdict": "ASM-SUSPECT", "rules": 2, "status": "parked"},
        {"func": "func_w", "file": "w", "distance": 1, "verdict": "C", "rules": 0, "status": "active"},
    ]
    with tempfile.TemporaryDirectory() as td:
        (Path(td) / "func_w").mkdir()
        desired = board_sync.build_desired_from_queue(items, Path(td))
    eq("active -> Backlog", desired["func_a"]["fields"]["Status"], "Backlog")
    eq("authorize -> Needs-Decision", desired["func_b"]["fields"]["Status"], "Needs-Decision")
    eq("parked -> Blocked", desired["func_c"]["fields"]["Status"], "Blocked")
    eq("verdict carried", desired["func_a"]["fields"]["Verdict"], "C")
    eq("distance carried", desired["func_a"]["fields"]["Distance"], 9)
    eq("rules carried", desired["func_a"]["fields"]["Rules"], 3)
    eq("file carried", desired["func_a"]["fields"]["File"], "x")
    eq("wip default no", desired["func_a"]["fields"]["WIP"], "no")
    eq("wip yes integration", desired["func_w"]["fields"]["WIP"], "yes")
    eq("active not archived", desired["func_a"]["archived"], False)
    eq("authorize not archived", desired["func_b"]["archived"], False)
    eq("parked not archived", desired["func_c"]["archived"], False)


def _board_from_desired(desired):
    """Helper: synthesize a 'current' board snapshot that already matches desired
    (used to prove idempotency: reconcile against it must yield no actions)."""
    current = []
    for i, (func, want) in enumerate(desired.items()):
        fields = dict(want["fields"])
        # numbers come back from the API as floats:
        for k in ("Distance", "Rules"):
            if k in fields:
                fields[k] = float(fields[k])
        current.append({"item_id": f"IID_{i}", "title": func,
                        "is_archived": want["archived"], "fields": fields})
    return current

def test_reconcile_add_new():
    desired = {"func_a": {"fields": {"Status": "Backlog", "Distance": 9}, "archived": False}}
    actions = board_sync.reconcile(desired, [])
    eq("one add action", len(actions), 1)
    eq("op is add", actions[0]["op"], "add")
    eq("add carries func", actions[0]["func"], "func_a")

def test_reconcile_idempotent():
    desired = {
        "func_a": {"fields": {"Status": "Backlog", "Verdict": "C", "WIP": "no",
                              "File": "x", "Distance": 9, "Rules": 3}, "archived": False},
    }
    current = _board_from_desired(desired)
    actions = board_sync.reconcile(desired, current)
    eq("no actions when in sync", actions, [])

def test_reconcile_update_changed_field():
    desired = {"func_a": {"fields": {"Status": "Blocked", "Distance": 9}, "archived": False}}
    current = [{"item_id": "IID_0", "title": "func_a", "is_archived": False,
               "fields": {"Status": "Backlog", "Distance": 9.0}}]
    actions = board_sync.reconcile(desired, current)
    eq("one set action", len(actions), 1)
    eq("op is set", actions[0]["op"], "set")
    eq("sets Status", actions[0]["field"], "Status")
    eq("to Blocked", actions[0]["value"], "Blocked")

def test_reconcile_completed_off_queue_archives():
    # func_old is on the board (Backlog) but no longer in the queue -> completed.
    desired = {}
    current = [{"item_id": "IID_0", "title": "func_old", "is_archived": False,
               "fields": {"Status": "Backlog"}}]
    actions = board_sync.reconcile(desired, current)
    ops = {(a["op"], a.get("field"), a.get("value")) for a in actions}
    check("sets Status=Done", ("set", "Status", "Done") in ops)
    check("archives it", any(a["op"] == "archive" for a in actions))

def test_reconcile_never_deletes():
    desired = {"func_a": {"fields": {"Status": "Backlog"}, "archived": False}}
    current = [{"item_id": "IID_0", "title": "func_stale", "is_archived": False,
               "fields": {"Status": "In-Progress"}}]  # In-Progress = not auto-touched in P1
    actions = board_sync.reconcile(desired, current)
    check("never emits delete", all(a["op"] != "delete" for a in actions))


def main():
    test_load_queue()
    test_load_queue_missing()
    test_wip_exists()
    test_build_desired_status_mapping()
    test_reconcile_add_new()
    test_reconcile_idempotent()
    test_reconcile_update_changed_field()
    test_reconcile_completed_off_queue_archives()
    test_reconcile_never_deletes()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0

if __name__ == "__main__":
    sys.exit(main())
