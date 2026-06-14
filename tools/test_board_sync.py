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

def test_val_eq_edges():
    eq("int vs float equal", board_sync._val_eq("Distance", 9, 9.0), True)
    eq("number unset -> not equal", board_sync._val_eq("Distance", 9, None), False)
    eq("both-None numbers equal", board_sync._val_eq("Rules", None, None), True)
    eq("string equal", board_sync._val_eq("Status", "Done", "Done"), True)
    eq("string differ", board_sync._val_eq("Status", "Done", "Backlog"), False)

def test_reconcile_recovers_partial_archive():
    desired = {}
    current = [{"item_id": "IID_0", "title": "func_old", "is_archived": True,
                "fields": {"Status": "Backlog"}}]
    actions = board_sync.reconcile(desired, current)
    ops = [(a["op"], a.get("field"), a.get("value")) for a in actions]
    check("sets Status=Done", ("set", "Status", "Done") in ops)
    check("does not re-archive", all(a["op"] != "archive" for a in actions))


class FakeGh:
    """Records (query, variables) calls and returns scripted responses."""
    def __init__(self, responses):
        self.responses = list(responses)
        self.calls = []
    def __call__(self, query, variables=None):
        self.calls.append({"query": query, "variables": variables or {}})
        return self.responses.pop(0)

def _with_stub(fake, fn):
    saved = board_sync.gh_graphql
    try:
        board_sync.gh_graphql = fake
        return fn()
    finally:
        board_sync.gh_graphql = saved

def test_ensure_project_reuses_existing():
    fake = FakeGh([
        {"user": {"projectsV2": {"nodes": [{"id": "PVT_existing", "number": 3, "title": "BB2 Decomp"}],
                                 "pageInfo": {"hasNextPage": False, "endCursor": None}}}},
    ])
    pid = _with_stub(fake, lambda: board_sync.ensure_project("BB2 Decomp", "tgg12321"))
    eq("reuses existing project id", pid, "PVT_existing")
    eq("only one call (no create)", len(fake.calls), 1)

def test_ensure_project_creates_when_absent():
    fake = FakeGh([
        {"user": {"projectsV2": {"nodes": [], "pageInfo": {"hasNextPage": False, "endCursor": None}}}},
        {"viewer": {"id": "U_me"}},
        {"createProjectV2": {"projectV2": {"id": "PVT_new", "number": 9, "title": "BB2 Decomp"}}},
    ])
    pid = _with_stub(fake, lambda: board_sync.ensure_project("BB2 Decomp", "tgg12321"))
    eq("creates and returns new id", pid, "PVT_new")
    eq("three calls (list, viewer, create)", len(fake.calls), 3)

def test_ensure_project_reuse_second_page():
    fake = FakeGh([
        {"user": {"projectsV2": {"nodes": [{"id": "PVT_a", "number": 1, "title": "Other"}],
                                 "pageInfo": {"hasNextPage": True, "endCursor": "C1"}}}},
        {"user": {"projectsV2": {"nodes": [{"id": "PVT_b", "number": 2, "title": "BB2 Decomp"}],
                                 "pageInfo": {"hasNextPage": False, "endCursor": None}}}},
    ])
    pid = _with_stub(fake, lambda: board_sync.ensure_project("BB2 Decomp", "tgg12321"))
    eq("found on page 2", pid, "PVT_b")
    eq("two list calls, no create", len(fake.calls), 2)
    eq("second call passed cursor", fake.calls[1]["variables"].get("cursor"), "C1")


class FakeProc:
    def __init__(self, returncode=0, stdout="", stderr=""):
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr

def _stub_subprocess(fn, run_impl):
    saved = board_sync.subprocess.run
    try:
        board_sync.subprocess.run = run_impl
        return fn()
    finally:
        board_sync.subprocess.run = saved

def test_gh_graphql_sends_body_and_returns_data():
    captured = {}
    def fake_run(argv, input=None, capture_output=False, text=False):
        captured["argv"] = argv
        captured["body"] = json.loads(input)
        return FakeProc(0, json.dumps({"data": {"ok": 1}}), "")
    out = _stub_subprocess(
        lambda: board_sync.gh_graphql("QUERY", variables={"a": "x", "n": 5, "opts": [{"name": "X"}]}),
        fake_run)
    eq("returns data", out, {"ok": 1})
    argv = captured["argv"]
    check("gh api graphql prefix", argv[:3] == ["gh", "api", "graphql"])
    check("has next-global-id header", "X-Github-Next-Global-ID: 1" in argv)
    check("uses --input stdin", "--input" in argv and "-" in argv)
    eq("body query", captured["body"]["query"], "QUERY")
    eq("body string var", captured["body"]["variables"]["a"], "x")
    eq("body number var", captured["body"]["variables"]["n"], 5)
    eq("body list var preserved", captured["body"]["variables"]["opts"], [{"name": "X"}])

def test_gh_graphql_raises_on_error():
    def fake_run(argv, input=None, capture_output=False, text=False):
        return FakeProc(1, json.dumps({"errors": [{"message": "boom"}]}), "boom")
    try:
        _stub_subprocess(lambda: board_sync.gh_graphql("Q"), fake_run)
        check("should have raised", False)
    except board_sync.GhError:
        check("raises GhError on gh error", True)

def test_gh_graphql_auth_failure_exits():
    def fake_run(argv, input=None, capture_output=False, text=False):
        return FakeProc(1, "", "gh: Bad credentials (HTTP 401)")
    try:
        _stub_subprocess(lambda: board_sync.gh_graphql("Q"), fake_run)
        check("should have exited", False)
    except SystemExit:
        check("auth failure -> SystemExit", True)

def test_gh_graphql_missing_binary_exits():
    def fake_run(argv, input=None, capture_output=False, text=False):
        raise FileNotFoundError("gh")
    try:
        _stub_subprocess(lambda: board_sync.gh_graphql("Q"), fake_run)
        check("should have exited", False)
    except SystemExit:
        check("missing gh -> SystemExit", True)


def test_ensure_fields_creates_missing():
    # First call: list existing fields -> only built-in Title (no custom fields).
    # Then one createProjectV2Field response per FIELD_SPEC, in order.
    list_resp = {"node": {"fields": {"nodes": [
        {"id": "F_title", "name": "Title"},
    ], "pageInfo": {"hasNextPage": False, "endCursor": None}}}}
    create_resps = []
    for name, dtype, opts in board_sync.FIELD_SPECS:
        if dtype == "SINGLE_SELECT":
            create_resps.append({"createProjectV2Field": {"projectV2Field": {
                "id": f"F_{name}", "name": name,
                "options": [{"id": f"opt_{name}_{o}", "name": o} for o in opts]}}})
        else:
            create_resps.append({"createProjectV2Field": {"projectV2Field": {
                "id": f"F_{name}", "name": name}}})
    fake = FakeGh([list_resp] + create_resps)
    fmap = _with_stub(fake, lambda: board_sync.ensure_fields("PVT_x"))
    eq("all specced fields present", sorted(fmap), sorted(n for n, _, _ in board_sync.FIELD_SPECS))
    eq("status is single-select with options", fmap["Status"]["options"]["Backlog"], "opt_Status_Backlog")
    eq("number field has no options", fmap["Distance"]["options"], {})

def test_ensure_fields_reuses_existing():
    nodes = [{"id": "F_title", "name": "Title"}]
    for name, dtype, opts in board_sync.FIELD_SPECS:
        if dtype == "SINGLE_SELECT":
            nodes.append({"id": f"F_{name}", "name": name,
                          "options": [{"id": f"opt_{name}_{o}", "name": o} for o in opts]})
        else:
            nodes.append({"id": f"F_{name}", "name": name})
    fake = FakeGh([{"node": {"fields": {"nodes": nodes,
                   "pageInfo": {"hasNextPage": False, "endCursor": None}}}}])
    fmap = _with_stub(fake, lambda: board_sync.ensure_fields("PVT_x"))
    eq("no create calls when all present", len(fake.calls), 1)
    eq("reused status option id", fmap["Status"]["options"]["Done"], "opt_Status_Done")


def test_create_field_single_select_passes_option_list():
    fake = FakeGh([{"createProjectV2Field": {"projectV2Field": {
        "id": "F_S", "name": "Status", "options": [{"id": "o1", "name": "Backlog"}]}}}])
    res = _with_stub(fake, lambda: board_sync._create_field("PVT_x", "Status", "SINGLE_SELECT", ["Backlog"]))
    opts = fake.calls[0]["variables"]["opts"]
    check("opts is a list", isinstance(opts, list))
    eq("opt count", len(opts), 1)
    eq("opt name", opts[0]["name"], "Backlog")
    check("opt has color", "color" in opts[0])
    check("opt has description key", "description" in opts[0])
    eq("returns option map", res["options"]["Backlog"], "o1")


def _page(nodes, has_next, cursor):
    return {"node": {"items": {"nodes": nodes,
            "pageInfo": {"hasNextPage": has_next, "endCursor": cursor}}}}

def _item_node(iid, func, archived, status):
    return {"id": iid, "isArchived": archived,
            "content": {"title": func},
            "fieldValues": {"nodes": [
                {"__typename": "ProjectV2ItemFieldSingleSelectValue", "name": status,
                 "optionId": "x", "field": {"name": "Status"}},
            ]}}

def test_list_items_paginates_and_parses():
    fake = FakeGh([
        _page([_item_node("IID_0", "func_a", False, "Backlog")], True, "CUR1"),
        _page([_item_node("IID_1", "func_b", True, "Done")], False, None),
    ])
    items = _with_stub(fake, lambda: board_sync.list_items("PVT_x"))
    eq("two items across pages", len(items), 2)
    eq("second page cursor passed", fake.calls[1]["variables"].get("cursor"), "CUR1")
    eq("title parsed", items[0]["title"], "func_a")
    eq("archived parsed", items[1]["is_archived"], True)
    eq("single-select value parsed", items[0]["fields"]["Status"], "Backlog")


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
    test_val_eq_edges()
    test_reconcile_recovers_partial_archive()
    test_ensure_project_reuses_existing()
    test_ensure_project_creates_when_absent()
    test_ensure_project_reuse_second_page()
    test_gh_graphql_sends_body_and_returns_data()
    test_gh_graphql_raises_on_error()
    test_gh_graphql_auth_failure_exits()
    test_gh_graphql_missing_binary_exits()
    test_ensure_fields_creates_missing()
    test_ensure_fields_reuses_existing()
    test_create_field_single_select_passes_option_list()
    test_list_items_paginates_and_parses()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0

if __name__ == "__main__":
    sys.exit(main())
