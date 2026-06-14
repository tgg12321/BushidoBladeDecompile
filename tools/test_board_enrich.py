#!/usr/bin/env python3
"""Unit tests for tools/board_enrich.py — no network.

Stubs board_sync.gh_graphql / the board-read + write functions; nothing here
touches gh or the live board.

Run: python tools/test_board_enrich.py   (exit 0 = pass)
"""
from __future__ import annotations

import contextlib
import io
import sys
import tempfile
from pathlib import Path

_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import board_sync  # noqa: E402
import board_enrich  # noqa: E402

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


# ---------------------------------------------------------------------------
# reconcile_bodies — the pure core
# ---------------------------------------------------------------------------

def _d(body, done):
    return {"body": body, "done": done}


def test_reconcile_update_when_body_differs():
    desired = {"func_a": _d("NEW body", False)}
    by_title = {"func_a": {"item_id": "I0", "draft_id": "D0", "body": "OLD body",
                           "is_archived": False, "status": "Backlog"}}
    actions = board_enrich.reconcile_bodies(desired, by_title)
    eq("one action", len(actions), 1)
    eq("op update", actions[0]["op"], "update")
    eq("uses draft id", actions[0]["draft_id"], "D0")
    eq("carries new body", actions[0]["body"], "NEW body")


def test_reconcile_create_when_absent():
    desired = {"func_done": _d("done body", True)}
    actions = board_enrich.reconcile_bodies(desired, {})
    eq("one action", len(actions), 1)
    eq("op create", actions[0]["op"], "create")
    eq("create done flag", actions[0]["done"], True)
    eq("carries body", actions[0]["body"], "done body")


def test_reconcile_skip_when_identical():
    # active func, body matches -> no action (active never finalized here).
    desired = {"func_a": _d("same", False)}
    by_title = {"func_a": {"item_id": "I0", "draft_id": "D0", "body": "same",
                           "is_archived": False, "status": "Backlog"}}
    actions = board_enrich.reconcile_bodies(desired, by_title)
    eq("no action when identical active", actions, [])


def test_reconcile_done_ledger_finalized_skipped():
    # done func recorded in the ledger as finalized -> SKIP (exists, invisible).
    desired = {"func_done": _d("same", True)}
    ledger = {"func_done": {"draft_id": "D0", "item_id": "I0", "finalized": True}}
    actions = board_enrich.reconcile_bodies(desired, {}, ledger)
    eq("finalized ledger entry skipped (no re-create)", actions, [])


def test_reconcile_done_ledger_pending_finalizes_by_stored_id():
    # in ledger, finalized==False (create landed, finalize didn't) -> finalize
    # using the STORED item_id; NO create, NO content-write.
    desired = {"func_done": _d("same", True)}
    ledger = {"func_done": {"draft_id": "D0", "item_id": "I0", "finalized": False}}
    actions = board_enrich.reconcile_bodies(desired, {}, ledger)
    eq("only a finalize (no create)", [a["op"] for a in actions], ["finalize"])
    eq("finalize uses STORED item id", actions[0]["item_id"], "I0")


def test_reconcile_done_not_in_ledger_on_board_finalizes():
    # NOT in ledger but visible on the board (pre-ledger stray) -> finalize via
    # the snapshot item_id (no create). Body matches -> no update.
    desired = {"func_done": _d("same", True)}
    by_title = {"func_done": {"item_id": "I0", "draft_id": "D0", "body": "same",
                              "is_archived": False, "status": "Backlog"}}
    actions = board_enrich.reconcile_bodies(desired, by_title, {})
    eq("visible stray finalized (no create)", [a["op"] for a in actions], ["finalize"])
    eq("finalize uses snapshot item id", actions[0]["item_id"], "I0")


def test_reconcile_done_not_in_ledger_off_board_creates():
    # NOT in ledger AND NOT on board -> create.
    desired = {"func_done": _d("body", True)}
    actions = board_enrich.reconcile_bodies(desired, {}, {})
    eq("create when truly absent", [a["op"] for a in actions], ["create"])
    eq("create done flag", actions[0]["done"], True)


def test_reconcile_done_stray_update_and_finalize_together():
    # visible stray, body differs -> both an update and a finalize.
    desired = {"func_done": _d("new", True)}
    by_title = {"func_done": {"item_id": "I0", "draft_id": "D0", "body": "old",
                              "is_archived": False, "status": "Backlog"}}
    actions = board_enrich.reconcile_bodies(desired, by_title, {})
    eq("update + finalize", sorted(a["op"] for a in actions), ["finalize", "update"])


def test_reconcile_done_ledger_overrides_board_visibility():
    # Even if a done func somehow appears in by_title, a finalized ledger entry
    # wins -> skip (the ledger is authoritative for done cards).
    desired = {"func_done": _d("same", True)}
    by_title = {"func_done": {"item_id": "I0", "draft_id": "D0", "body": "same",
                              "is_archived": False, "status": "Backlog"}}
    ledger = {"func_done": {"draft_id": "D0", "item_id": "I0", "finalized": True}}
    actions = board_enrich.reconcile_bodies(desired, by_title, ledger)
    eq("ledger-finalized wins over board presence", actions, [])


def test_reconcile_active_body_match_no_finalize():
    # active func (done=False) with matching body -> NO finalize, NO action.
    desired = {"func_a": _d("same", False)}
    by_title = {"func_a": {"item_id": "I0", "draft_id": "D0", "body": "same",
                           "is_archived": False, "status": "Backlog"}}
    actions = board_enrich.reconcile_bodies(desired, by_title)
    check("active never finalized by this tool", all(a["op"] != "finalize" for a in actions))
    eq("no action", actions, [])


def test_reconcile_never_deletes():
    # board has a card not in desired -> must be left alone (no delete op).
    desired = {"func_a": _d("b", False)}
    by_title = {
        "func_a": {"item_id": "I0", "draft_id": "D0", "body": "b", "is_archived": False, "status": "Backlog"},
        "func_stale": {"item_id": "I1", "draft_id": "D1", "body": "x", "is_archived": True, "status": "Done"},
    }
    actions = board_enrich.reconcile_bodies(desired, by_title)
    eq("only the (none) for func_a, stale untouched", actions, [])
    check("no delete op ever", all(a["op"] != "delete" for a in actions))


def test_reconcile_mixed():
    desired = {
        "func_upd": _d("new", False),     # update
        "func_new": _d("seed", True),     # create
        "func_ok": _d("ok", False),       # skip
    }
    by_title = {
        "func_upd": {"item_id": "I0", "draft_id": "D0", "body": "old", "is_archived": False, "status": "Backlog"},
        "func_ok": {"item_id": "I2", "draft_id": "D2", "body": "ok", "is_archived": False, "status": "Backlog"},
    }
    actions = board_enrich.reconcile_bodies(desired, by_title)
    ops = sorted(a["op"] for a in actions)
    eq("one update + one create", ops, ["create", "update"])


# ---------------------------------------------------------------------------
# body-truncation guard
# ---------------------------------------------------------------------------

def test_truncate_under_limit_untouched():
    body = "x" * 100
    eq("short body untouched", board_enrich.truncate_body(body, limit=200), body)


def test_truncate_over_limit():
    body = "x" * 70000
    out = board_enrich.truncate_body(body, limit=65000)
    check("truncated to <= limit", len(out) <= 65000)
    check("carries truncation note", "truncated by board_enrich" in out)


def test_read_card_body_truncates(tmpcards=None):
    with tempfile.TemporaryDirectory() as td:
        (Path(td) / "func_big.md").write_text("y" * 70000, encoding="utf-8")
        out = board_enrich.read_card_body(td, "func_big")
        check("read+truncated <= MAX_BODY", len(out) <= board_enrich.MAX_BODY)
        eq("absent card -> None", board_enrich.read_card_body(td, "nope"), None)


# ---------------------------------------------------------------------------
# build_desired_bodies — active/done split + missing-card drop
# ---------------------------------------------------------------------------

def test_build_desired_bodies_split():
    with tempfile.TemporaryDirectory() as td:
        for f in ("func_active", "func_done"):
            (Path(td) / f"{f}.md").write_text(f"body of {f}", encoding="utf-8")
        # func_nocard intentionally has no file.
        queue_funcs = {"func_active"}
        inventory = {"func_active": "text1b", "func_done": "display", "func_nocard": "display"}
        desired = board_enrich.build_desired_bodies(queue_funcs, inventory, td)
    eq("active not done", desired["func_active"]["done"], False)
    eq("completed is done", desired["func_done"]["done"], True)
    eq("body carried", desired["func_active"]["body"], "body of func_active")
    check("func_nocard dropped (no card file)", "func_nocard" not in desired)


def test_build_desired_only_active():
    with tempfile.TemporaryDirectory() as td:
        for f in ("func_active", "func_done"):
            (Path(td) / f"{f}.md").write_text("b", encoding="utf-8")
        desired = board_enrich.build_desired_bodies(
            {"func_active"}, {"func_active": "a", "func_done": "d"}, td, only_active=True)
    check("only active included", set(desired) == {"func_active"})


def test_build_desired_only_done():
    with tempfile.TemporaryDirectory() as td:
        for f in ("func_active", "func_done"):
            (Path(td) / f"{f}.md").write_text("b", encoding="utf-8")
        desired = board_enrich.build_desired_bodies(
            {"func_active"}, {"func_active": "a", "func_done": "d"}, td, only_done=True)
    check("only done included", set(desired) == {"func_done"})


# ---------------------------------------------------------------------------
# Done-card ledger — load/save round-trip
# ---------------------------------------------------------------------------

def test_ledger_missing_file_is_empty():
    eq("missing ledger -> {}", board_enrich.load_ledger(Path("does/not/exist.json")), {})


def test_ledger_round_trip():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "ledger.json"
        led = {"func_a": {"draft_id": "D0", "item_id": "I0", "finalized": True},
               "func_b": {"draft_id": "D1", "item_id": "I1", "finalized": False}}
        board_enrich.save_ledger(p, led)
        check("file written", p.exists())
        eq("round-trips", board_enrich.load_ledger(p), led)


def test_ledger_corrupt_file_is_empty():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "ledger.json"
        p.write_text("{ not json", encoding="utf-8")
        eq("corrupt ledger -> {}", board_enrich.load_ledger(p), {})


def test_ledger_save_creates_parent_dir():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "nested" / "deep" / "ledger.json"
        board_enrich.save_ledger(p, {"f": {"finalized": True}})
        check("nested ledger written", p.exists())


# ---------------------------------------------------------------------------
# rate-limit-wait helper
# ---------------------------------------------------------------------------

def test_maybe_wait_below_floor_sleeps():
    slept = {"secs": None}
    # remaining 10 < floor 200; reset 1000s in the future from now=0.
    waited = board_enrich.maybe_wait_for_budget(
        floor=200, slack=5,
        read=lambda: (10, 1000),
        sleep=lambda s: slept.__setitem__("secs", s),
        now=lambda: 0.0)
    with contextlib.redirect_stdout(io.StringIO()):
        pass
    eq("decides to wait", waited, True)
    eq("sleeps until reset + slack", slept["secs"], 1005)


def test_maybe_wait_above_floor_no_sleep():
    slept = {"called": False}
    waited = board_enrich.maybe_wait_for_budget(
        floor=200, slack=5,
        read=lambda: (5000, 9999),
        sleep=lambda s: slept.__setitem__("called", True),
        now=lambda: 0.0)
    eq("no wait when budget healthy", waited, False)
    eq("never slept", slept["called"], False)


def test_maybe_wait_unknown_budget_no_sleep():
    slept = {"called": False}
    waited = board_enrich.maybe_wait_for_budget(
        floor=200,
        read=lambda: (None, None),
        sleep=lambda s: slept.__setitem__("called", True))
    eq("unknown budget -> proceed", waited, False)
    eq("never slept", slept["called"], False)


# ---------------------------------------------------------------------------
# _BudgetPoller (I1) — poll periodically, not every content-write
# ---------------------------------------------------------------------------

def test_budget_poller_polls_periodically():
    reads = {"n": 0}

    def fake_read():
        reads["n"] += 1
        return (5000, 0)

    poller = board_enrich._BudgetPoller(
        poll_every=25, floor=200, wait=lambda **k: False, read=fake_read)
    # Simulate 60 content-writes: maybe_wait before each, tick after each.
    for _ in range(60):
        poller.maybe_wait()
        poller.tick()
    # Polls at write #1 (forced first), then after every 25 -> #26, #51 = 3 polls.
    eq("bounded polls (not once-per-write)", reads["n"], 3)
    eq("poller exposes poll count", poller.polls, 3)
    check("far fewer polls than writes", reads["n"] < 60)


def test_budget_poller_sleeps_when_low_on_poll():
    # On a poll boundary, a low budget must trigger the wait.
    waits = {"n": 0}

    def fake_wait(**k):
        # honor the injected read to confirm the cached snapshot is passed through
        rem, _reset = k["read"]()
        if rem < k["floor"]:
            waits["n"] += 1
            return True
        return False

    poller = board_enrich._BudgetPoller(
        poll_every=25, floor=200, wait=fake_wait, read=lambda: (10, 999))
    poller.maybe_wait()   # first call forces a poll; remaining 10 < 200 -> wait
    eq("waited on low budget at poll", waits["n"], 1)


def test_budget_poller_no_wait_between_boundaries():
    # Between poll boundaries maybe_wait must NOT poll or wait.
    reads = {"n": 0}
    waits = {"n": 0}

    poller = board_enrich._BudgetPoller(
        poll_every=25, floor=200,
        wait=lambda **k: waits.__setitem__("n", waits["n"] + 1) or False,
        read=lambda: (reads.__setitem__("n", reads["n"] + 1) or (5000, 0)))
    poller.maybe_wait(); poller.tick()   # write 1 -> polls once
    for _ in range(10):                  # writes 2..11 -> no poll
        poller.maybe_wait(); poller.tick()
    eq("only the first poll so far", reads["n"], 1)
    eq("only the first wait so far", waits["n"], 1)


def test_budget_poller_graceful_on_unparseable():
    # read returns (None, None); poller must not crash and must proceed.
    poller = board_enrich._BudgetPoller(
        poll_every=25, floor=200,
        wait=board_enrich.maybe_wait_for_budget, read=lambda: (None, None))
    poller.maybe_wait()   # must not raise
    check("survives unparseable budget", True)
    eq("remaining recorded as None", poller.remaining, None)


# ---------------------------------------------------------------------------
# list_draft_items — parses + paginates + filters non-drafts/titleless
# ---------------------------------------------------------------------------

class FakeGh:
    def __init__(self, responses):
        self.responses = list(responses)
        self.calls = []

    def __call__(self, query, variables=None):
        self.calls.append({"query": query, "variables": variables or {}})
        return self.responses.pop(0)


def _with_gh(fake, fn):
    saved = board_sync.gh_graphql
    try:
        board_sync.gh_graphql = fake
        return fn()
    finally:
        board_sync.gh_graphql = saved


def _page(nodes, has_next, cursor):
    return {"node": {"items": {"nodes": nodes,
            "pageInfo": {"hasNextPage": has_next, "endCursor": cursor}}}}


def _draft_node(iid, did, title, body, archived, status):
    return {"id": iid, "isArchived": archived,
            "content": {"__typename": "DraftIssue", "id": did, "title": title, "body": body},
            "fieldValues": {"nodes": [
                {"__typename": "ProjectV2ItemFieldSingleSelectValue", "name": status,
                 "field": {"name": "Status"}}]}}


def test_list_draft_items_parses_and_paginates():
    fake = FakeGh([
        _page([_draft_node("I0", "D0", "func_a", "body a", False, "Backlog")], True, "C1"),
        _page([
            _draft_node("I1", "D1", "func_b", "body b", True, "Done"),
            # non-draft (Issue) -> skipped
            {"id": "I2", "isArchived": False, "content": {"__typename": "Issue", "title": "issue_x"},
             "fieldValues": {"nodes": []}},
            # titleless draft -> skipped
            _draft_node("I3", "D3", None, "x", False, None),
        ], False, None),
    ])
    by_title = _with_gh(fake, lambda: board_enrich.list_draft_items("PVT_x"))
    eq("two drafts kept", len(by_title), 2)
    eq("second page cursor passed", fake.calls[1]["variables"].get("cursor"), "C1")
    eq("draft id captured", by_title["func_a"]["draft_id"], "D0")
    eq("body captured", by_title["func_a"]["body"], "body a")
    eq("archived captured", by_title["func_b"]["is_archived"], True)
    eq("status captured", by_title["func_a"]["status"], "Backlog")
    check("non-draft issue skipped", "issue_x" not in by_title)


# ---------------------------------------------------------------------------
# content-write secondary-limit backoff
# ---------------------------------------------------------------------------

def test_content_write_backoff_then_success():
    calls = {"n": 0}

    def flaky(query, variables=None):
        calls["n"] += 1
        if calls["n"] == 1:
            raise board_sync.GhError("You have exceeded a secondary rate limit")
        return {"ok": True}

    saved_gh, saved_sleep = board_sync.gh_graphql, board_enrich.time.sleep
    try:
        board_sync.gh_graphql = flaky
        board_enrich.time.sleep = lambda *a, **k: None
        with contextlib.redirect_stdout(io.StringIO()):
            out = board_enrich._content_write("Q", {}, delay=0)
        eq("retried then succeeded", out, {"ok": True})
        eq("two attempts", calls["n"], 2)
    finally:
        board_sync.gh_graphql = saved_gh
        board_enrich.time.sleep = saved_sleep


def test_content_write_reraises_other_error():
    def boom(query, variables=None):
        raise board_sync.GhError("validation: something else")
    saved_gh = board_sync.gh_graphql
    try:
        board_sync.gh_graphql = boom
        try:
            board_enrich._content_write("Q", {}, delay=0)
            check("should have raised", False)
        except board_sync.GhError:
            check("non-rate-limit re-raised", True)
    finally:
        board_sync.gh_graphql = saved_gh


# ---------------------------------------------------------------------------
# end-to-end dry-run: zero writes, correct counts
# ---------------------------------------------------------------------------

def test_dry_run_no_writes_correct_counts():
    saved = (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
             board_sync.load_inventory, board_enrich.list_draft_items,
             board_enrich.update_body, board_enrich.create_card)

    def must_not_write(*a, **k):
        raise AssertionError("dry-run must not write")

    try:
        board_sync.find_project = lambda title, login: "PVT_x"
        board_sync.ensure_fields = lambda pid: (_ for _ in ()).throw(
            AssertionError("dry-run must not ensure_fields"))
        board_sync.load_queue = lambda p: [{"func": "func_active"}]
        board_sync.load_inventory = lambda m, e: {"func_active": "a", "func_done": "d"}
        board_enrich.update_body = must_not_write
        board_enrich.create_card = must_not_write
        # Board already has func_active with a STALE body (-> 1 update planned),
        # and func_done is absent (-> 1 create planned).
        board_enrich.list_draft_items = lambda pid: {
            "func_active": {"item_id": "I0", "draft_id": "D0", "body": "stale",
                            "is_archived": False, "status": "Backlog"}}
        with tempfile.TemporaryDirectory() as td:
            for f in ("func_active", "func_done"):
                (Path(td) / f"{f}.md").write_text("fresh body", encoding="utf-8")
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=Path(td) / "ledger.json",
                    dry_run=True)
            out = buf.getvalue()
            # Dry-run must NOT write the ledger either.
            check("dry-run leaves ledger absent", not (Path(td) / "ledger.json").exists())
        eq("1 update + 1 create remaining", res["remaining"], 2)
        eq("zero updated", res["updated"], 0)
        eq("zero created", res["created"], 0)
        check("dry run mentions both funcs", "func_active" in out and "func_done" in out)
    finally:
        (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
         board_sync.load_inventory, board_enrich.list_draft_items,
         board_enrich.update_body, board_enrich.create_card) = saved


def test_run_enrich_exits_when_project_absent():
    saved = board_sync.find_project
    try:
        board_sync.find_project = lambda title, login: None
        try:
            with contextlib.redirect_stdout(io.StringIO()):
                board_enrich.run_enrich(dry_run=True)
            check("should have exited", False)
        except SystemExit:
            check("absent project -> SystemExit", True)
    finally:
        board_sync.find_project = saved


# ---------------------------------------------------------------------------
# apply path: update + create-then-Done-archive, with --limit + idempotent skip
# ---------------------------------------------------------------------------

def test_apply_update_and_create_with_limit():
    saved = (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
             board_sync.load_inventory, board_enrich.list_draft_items,
             board_enrich.update_body, board_enrich.create_card,
             board_sync._set_field, board_sync._mutate,
             board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget)
    log = {"updates": [], "creates": [], "status_sets": [], "archives": []}
    try:
        board_sync.find_project = lambda title, login: "PVT_x"
        board_sync.ensure_fields = lambda pid: {"Status": {"id": "F_S", "options": {"Done": "o_done"}}}
        board_sync.load_queue = lambda p: [{"func": "func_active"}]
        board_sync.load_inventory = lambda m, e: {"func_active": "a", "func_done": "d"}
        board_enrich.list_draft_items = lambda pid: {
            "func_active": {"item_id": "I0", "draft_id": "D0", "body": "stale",
                            "is_archived": False, "status": "Backlog"}}
        board_enrich.update_body = lambda did, body, **k: log["updates"].append((did, body))
        board_enrich.create_card = lambda pid, func, body, **k: (
            log["creates"].append((func, body)) or ("NEWITEM", "NEWDRAFT"))
        board_sync._set_field = lambda pid, fm, iid, field, value, delay: log["status_sets"].append((iid, field, value))
        board_sync._mutate = lambda q, variables=None, delay=0: log["archives"].append(variables["i"])
        board_enrich.maybe_wait_for_budget = lambda *a, **k: False
        board_enrich.read_rate_budget = lambda: (5000, 0)
        with tempfile.TemporaryDirectory() as td:
            for f in ("func_active", "func_done"):
                (Path(td) / f"{f}.md").write_text("fresh", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=ledger_p, dry_run=False)
            final_ledger = board_enrich.load_ledger(ledger_p)
        eq("one update done", res["updated"], 1)
        eq("one create done", res["created"], 1)
        eq("update used draft id", log["updates"], [("D0", "fresh")])
        eq("create made func_done", [c[0] for c in log["creates"]], ["func_done"])
        eq("created card set Done", log["status_sets"], [("NEWITEM", "Status", "Done")])
        eq("created card archived", log["archives"], ["NEWITEM"])
        # ledger written through for the created Done card; active func absent.
        eq("ledger records created Done card finalized",
           final_ledger.get("func_done"),
           {"draft_id": "NEWDRAFT", "item_id": "NEWITEM", "finalized": True})
        check("active func never in ledger", "func_active" not in final_ledger)
    finally:
        (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
         board_sync.load_inventory, board_enrich.list_draft_items,
         board_enrich.update_body, board_enrich.create_card,
         board_sync._set_field, board_sync._mutate,
         board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget) = saved


def test_apply_limit_stops_early():
    saved = (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
             board_sync.load_inventory, board_enrich.list_draft_items,
             board_enrich.update_body, board_enrich.create_card,
             board_sync._set_field, board_sync._mutate,
             board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget)
    log = {"updates": [], "creates": []}
    try:
        board_sync.find_project = lambda title, login: "PVT_x"
        board_sync.ensure_fields = lambda pid: {"Status": {"id": "F_S", "options": {"Done": "o_done"}}}
        board_sync.load_queue = lambda p: []
        board_sync.load_inventory = lambda m, e: {"func_d1": "a", "func_d2": "b", "func_d3": "c"}
        board_enrich.list_draft_items = lambda pid: {}
        board_enrich.update_body = lambda did, body, **k: log["updates"].append(did)
        board_enrich.create_card = lambda pid, func, body, **k: (
            log["creates"].append(func) or ("IT_" + func, "DR_" + func))
        board_sync._set_field = lambda *a, **k: None
        board_sync._mutate = lambda *a, **k: None
        board_enrich.maybe_wait_for_budget = lambda *a, **k: False
        board_enrich.read_rate_budget = lambda: (5000, 0)
        with tempfile.TemporaryDirectory() as td:
            for f in ("func_d1", "func_d2", "func_d3"):
                (Path(td) / f"{f}.md").write_text("b", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            with contextlib.redirect_stdout(io.StringIO()):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=ledger_p, dry_run=False, limit=2)
            final_ledger = board_enrich.load_ledger(ledger_p)
        eq("only 2 creates done", len(log["creates"]), 2)
        eq("created count 2", res["created"], 2)
        check("one remaining after limit", res["remaining"] == 1)
        eq("ledger recorded exactly the 2 created cards", len(final_ledger), 2)
    finally:
        (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
         board_sync.load_inventory, board_enrich.list_draft_items,
         board_enrich.update_body, board_enrich.create_card,
         board_sync._set_field, board_sync._mutate,
         board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget) = saved


def _stub_driver():
    """Save+install a set of driver stubs; returns (saved_tuple, log).
    Caller must restore saved_tuple in a finally."""
    saved = (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
             board_sync.load_inventory, board_enrich.list_draft_items,
             board_enrich.update_body, board_enrich.create_card,
             board_sync._set_field, board_sync._mutate,
             board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget)
    log = {"creates": [], "updates": [], "status_sets": [], "archives": []}
    board_sync.find_project = lambda title, login: "PVT_x"
    board_sync.ensure_fields = lambda pid: {"Status": {"id": "F_S", "options": {"Done": "o_done"}}}
    board_enrich.create_card = lambda pid, func, body, **k: (
        log["creates"].append(func) or ("IT_" + func, "DR_" + func))
    board_enrich.update_body = lambda *a, **k: log["updates"].append(a)
    board_sync._set_field = lambda pid, fm, iid, field, value, delay: log["status_sets"].append((iid, value))
    board_sync._mutate = lambda q, variables=None, delay=0: log["archives"].append(variables["i"])
    board_enrich.maybe_wait_for_budget = lambda *a, **k: False
    board_enrich.read_rate_budget = lambda: (5000, 0)
    return saved, log


def _restore_driver(saved):
    (board_sync.find_project, board_sync.ensure_fields, board_sync.load_queue,
     board_sync.load_inventory, board_enrich.list_draft_items,
     board_enrich.update_body, board_enrich.create_card,
     board_sync._set_field, board_sync._mutate,
     board_enrich.maybe_wait_for_budget, board_enrich.read_rate_budget) = saved


def test_apply_visible_stray_finalize_path():
    # NOT in ledger but visible on board, body matches -> finalize ONLY (no
    # create, no content-write) AND append to the ledger finalized=True.
    saved, log = _stub_driver()
    try:
        board_sync.load_queue = lambda p: []
        board_sync.load_inventory = lambda m, e: {"func_done": "d"}
        board_enrich.list_draft_items = lambda pid: {
            "func_done": {"item_id": "I9", "draft_id": "D9", "body": "fresh",
                          "is_archived": False, "status": "Backlog"}}
        with tempfile.TemporaryDirectory() as td:
            (Path(td) / "func_done.md").write_text("fresh", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            with contextlib.redirect_stdout(io.StringIO()):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=ledger_p, dry_run=False)
            final_ledger = board_enrich.load_ledger(ledger_p)
        eq("no create on stray finalize", log["creates"], [])
        eq("no body content-write (body matched)", log["updates"], [])
        eq("finalized once", res["finalized"], 1)
        eq("Status set Done on existing item", log["status_sets"], [("I9", "Done")])
        eq("archived the existing item", log["archives"], ["I9"])
        eq("ledger now records the stray as finalized",
           final_ledger.get("func_done", {}).get("finalized"), True)
    finally:
        _restore_driver(saved)


def test_apply_ledger_pending_finalize_no_create():
    # Ledger has the card as finalized=False (create landed, finalize didn't on a
    # prior run). by_title can't see it (archived/invisible). Re-run must finalize
    # via the STORED item_id — NO duplicate create.
    saved, log = _stub_driver()
    try:
        board_sync.load_queue = lambda p: []
        board_sync.load_inventory = lambda m, e: {"func_done": "d"}
        board_enrich.list_draft_items = lambda pid: {}   # archived -> invisible
        with tempfile.TemporaryDirectory() as td:
            (Path(td) / "func_done.md").write_text("fresh", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            board_enrich.save_ledger(ledger_p, {
                "func_done": {"draft_id": "Dpend", "item_id": "Ipend", "finalized": False}})
            with contextlib.redirect_stdout(io.StringIO()):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=ledger_p, dry_run=False)
            final_ledger = board_enrich.load_ledger(ledger_p)
        eq("NO create (avoids duplicate)", log["creates"], [])
        eq("finalized once", res["finalized"], 1)
        eq("finalize used the STORED item id", log["status_sets"], [("Ipend", "Done")])
        eq("archived the stored item", log["archives"], ["Ipend"])
        eq("ledger flipped to finalized", final_ledger["func_done"]["finalized"], True)
    finally:
        _restore_driver(saved)


def test_apply_resume_with_ledger_zero_creates():
    # THE fatal-bug guard: a ledger already marking the Done cards finalized must
    # cause a re-run to make ZERO creates (archived cards are invisible to the
    # board read, so without the ledger they'd be re-created as duplicates).
    saved, log = _stub_driver()
    try:
        board_sync.load_queue = lambda p: []
        board_sync.load_inventory = lambda m, e: {"func_d1": "a", "func_d2": "b"}
        board_enrich.list_draft_items = lambda pid: {}   # archived -> invisible
        with tempfile.TemporaryDirectory() as td:
            for f in ("func_d1", "func_d2"):
                (Path(td) / f"{f}.md").write_text("b", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            board_enrich.save_ledger(ledger_p, {
                "func_d1": {"draft_id": "D1", "item_id": "I1", "finalized": True},
                "func_d2": {"draft_id": "D2", "item_id": "I2", "finalized": True}})
            with contextlib.redirect_stdout(io.StringIO()):
                res = board_enrich.run_enrich(
                    queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                    cards_dir=Path(td), ledger_path=ledger_p, dry_run=False)
        eq("zero creates on resume", log["creates"], [])
        eq("zero finalizes (already done)", res["finalized"], 0)
        eq("both skipped via ledger", res["skipped"], 2)
    finally:
        _restore_driver(saved)


def test_apply_crash_window_leaves_pending_ledger_entry():
    # Crash AFTER addDraft + ledger(finalized=False) but BEFORE finalize lands.
    # The ledger must already carry the card (finalized=False) so a resume
    # finalizes instead of re-creating a duplicate.
    saved, log = _stub_driver()
    saved_fin = board_enrich._finalize_done
    try:
        board_sync.load_queue = lambda p: []
        board_sync.load_inventory = lambda m, e: {"func_done": "d"}
        board_enrich.list_draft_items = lambda pid: {}

        def boom(*a, **k):
            raise RuntimeError("simulated crash during finalize")
        board_enrich._finalize_done = boom
        with tempfile.TemporaryDirectory() as td:
            (Path(td) / "func_done.md").write_text("fresh", encoding="utf-8")
            ledger_p = Path(td) / "ledger.json"
            crashed = False
            try:
                with contextlib.redirect_stdout(io.StringIO()):
                    board_enrich.run_enrich(
                        queue_path=Path("q"), map_path=Path("m"), elf_path=Path("e"),
                        cards_dir=Path(td), ledger_path=ledger_p, dry_run=False)
            except RuntimeError:
                crashed = True
            mid_ledger = board_enrich.load_ledger(ledger_p)
        check("crash propagated", crashed)
        eq("card was created once", log["creates"], ["func_done"])
        check("ledger recorded the card despite the crash", "func_done" in mid_ledger)
        eq("recorded as NOT yet finalized (resume will finalize)",
           mid_ledger["func_done"]["finalized"], False)
        eq("ledger stored the created item id",
           mid_ledger["func_done"]["item_id"], "IT_func_done")
    finally:
        board_enrich._finalize_done = saved_fin
        _restore_driver(saved)


def main():
    test_reconcile_update_when_body_differs()
    test_reconcile_create_when_absent()
    test_reconcile_skip_when_identical()
    test_reconcile_done_ledger_finalized_skipped()
    test_reconcile_done_ledger_pending_finalizes_by_stored_id()
    test_reconcile_done_not_in_ledger_on_board_finalizes()
    test_reconcile_done_not_in_ledger_off_board_creates()
    test_reconcile_done_stray_update_and_finalize_together()
    test_reconcile_done_ledger_overrides_board_visibility()
    test_reconcile_active_body_match_no_finalize()
    test_reconcile_never_deletes()
    test_reconcile_mixed()
    test_truncate_under_limit_untouched()
    test_truncate_over_limit()
    test_read_card_body_truncates()
    test_build_desired_bodies_split()
    test_build_desired_only_active()
    test_build_desired_only_done()
    test_ledger_missing_file_is_empty()
    test_ledger_round_trip()
    test_ledger_corrupt_file_is_empty()
    test_ledger_save_creates_parent_dir()
    test_maybe_wait_below_floor_sleeps()
    test_maybe_wait_above_floor_no_sleep()
    test_maybe_wait_unknown_budget_no_sleep()
    test_budget_poller_polls_periodically()
    test_budget_poller_sleeps_when_low_on_poll()
    test_budget_poller_no_wait_between_boundaries()
    test_budget_poller_graceful_on_unparseable()
    test_list_draft_items_parses_and_paginates()
    test_content_write_backoff_then_success()
    test_content_write_reraises_other_error()
    test_dry_run_no_writes_correct_counts()
    test_run_enrich_exits_when_project_absent()
    test_apply_update_and_create_with_limit()
    test_apply_visible_stray_finalize_path()
    test_apply_ledger_pending_finalize_no_create()
    test_apply_resume_with_ledger_zero_creates()
    test_apply_crash_window_leaves_pending_ledger_entry()
    test_apply_limit_stops_early()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0


if __name__ == "__main__":
    sys.exit(main())
