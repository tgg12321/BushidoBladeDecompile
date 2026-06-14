#!/usr/bin/env python3
"""tools/test_board.py — hand-rolled check/eq tests for tools/board.py.

NO network: board_sync's gh client + mutation helpers are stubbed, and every
test asserts that NO list/items query is ever issued by a move command. Run:

    python tools/test_board.py        # -> "... 0 failed"

Style: 2-arg check(desc, cond) / eq(desc, got, want), mirroring the project's
other hand-rolled suites. LF line endings.
"""
from __future__ import annotations

import io
import json
import sys
import tempfile
from contextlib import redirect_stdout
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

import board  # noqa: E402
import board_sync  # noqa: E402

_FAILS = []
_RUN = 0


def check(desc, cond):
    global _RUN
    _RUN += 1
    if not cond:
        _FAILS.append(desc)
        print(f"  FAIL: {desc}")
    else:
        print(f"  ok:   {desc}")


def eq(desc, got, want):
    check(f"{desc} (got={got!r} want={want!r})", got == want)


# ---------------------------------------------------------------------------
# helpers: point board.py at temp files + capture a command's stdout
# ---------------------------------------------------------------------------

class Tmp:
    """A temp workspace with synthetic queue.json / index / cards / ledger,
    wired into board.py's module globals for the duration of a `with`."""

    def __init__(self):
        self.dir = Path(tempfile.mkdtemp(prefix="board_test_"))
        self._saved = {}

    def __enter__(self):
        self.queue = self.dir / "queue.json"
        self.index = self.dir / "board_index.json"
        self.cards = self.dir / "cards"
        self.ledger = self.dir / "ledger.json"
        self.cards.mkdir(parents=True, exist_ok=True)
        for name, val in (("QUEUE", self.queue), ("INDEX_PATH", self.index),
                          ("CARDS_DIR", self.cards), ("LEDGER_PATH", self.ledger)):
            self._saved[name] = getattr(board, name)
            setattr(board, name, val)
        # reset the in-process project/field cache between tests
        board._PROJECT_CACHE["project_id"] = None
        board._PROJECT_CACHE["field_map"] = None
        return self

    def __exit__(self, *exc):
        for name, val in self._saved.items():
            setattr(board, name, val)

    def write_queue(self, items):
        self.queue.write_text(json.dumps({"items": items}), encoding="utf-8")

    def write_index(self, index):
        self.index.write_text(json.dumps(index), encoding="utf-8")

    def write_ledger(self, ledger):
        self.ledger.write_text(json.dumps(ledger), encoding="utf-8")

    def write_card(self, func, body):
        (self.cards / f"{func}.md").write_text(body, encoding="utf-8")

    def read_index(self):
        return json.loads(self.index.read_text(encoding="utf-8"))


def run(*argv):
    """Run board.main(argv) capturing stdout -> (rc, stdout_text)."""
    buf = io.StringIO()
    rc = 0
    with redirect_stdout(buf):
        try:
            rc = board.main(list(argv))
        except SystemExit as e:
            rc = e.code
    return rc, buf.getvalue()


class GhSpy:
    """Stub board_sync's gh layer. Records every mutation; FAILS the test if a
    list/items query is ever issued (the no-full-list invariant). Patches
    find_project / ensure_fields / _set_field / _mutate / gh_graphql."""

    def __init__(self, project_id="PROJ", visible=None):
        self.project_id = project_id
        self.visible = visible or {}
        self.set_field_calls = []   # (item_id, field, value)
        self.mutate_calls = []      # (query, variables)
        self.graphql_calls = []     # (query, variables)
        self.list_queries = 0
        self._saved = {}

    def __enter__(self):
        spy = self

        def find_project(title, login):
            return spy.project_id

        def ensure_fields(project_id):
            # Status option ids only needed by the real _set_field; stubbed here.
            return {"Status": {"id": "F_STATUS", "options": {
                "In-Progress": "o1", "Blocked": "o2", "Done": "o3", "Backlog": "o4"}}}

        def _set_field(project_id, field_map, item_id, field, value, delay):
            spy.set_field_calls.append((item_id, field, value))

        def _mutate(query, variables=None, delay=0.2, max_tries=5):
            spy.mutate_calls.append((query, variables))
            return {}

        def gh_graphql(query, variables=None):
            spy.graphql_calls.append((query, variables))
            # The only legitimate gh_graphql consumer here is index --rebuild's
            # visible read; serve a single page from spy.visible.
            if "items(" in query:
                spy.list_queries += 1
                nodes = [{"id": v["item_id"],
                          "content": {"__typename": "DraftIssue", "id": v["draft_id"], "title": t}}
                         for t, v in spy.visible.items()]
                return {"node": {"items": {"nodes": nodes,
                                           "pageInfo": {"hasNextPage": False, "endCursor": None}}}}
            return {}

        for name, fn in (("find_project", find_project), ("ensure_fields", ensure_fields),
                         ("_set_field", _set_field), ("_mutate", _mutate),
                         ("gh_graphql", gh_graphql)):
            self._saved[name] = getattr(board_sync, name)
            setattr(board_sync, name, fn)
        return self

    def __exit__(self, *exc):
        for name, fn in self._saved.items():
            setattr(board_sync, name, fn)


def _q(file="text1b", dist=9, rules=0, status="active"):
    """A queue-item factory; func name auto-numbered by call order."""
    _q.n += 1
    return {"func": f"func_{_q.n:08X}", "file": file, "distance": dist,
            "verdict": "C", "rules": rules, "status": status}


_q.n = 0


# ===========================================================================
# tests
# ===========================================================================

def test_next_picks_first_active():
    print("test_next_picks_first_active")
    _q.n = 0
    a, b, c = _q(), _q(), _q()
    with Tmp() as t:
        t.write_queue([a, b, c])
        t.write_index({})
        rc, out = run("next", "--json")
        eq("rc", rc, 0)
        data = json.loads(out)
        eq("picks first active", data["func"], a["func"])
        eq("json has url", data["url"], board.BOARD_URL)
        check("json hint mentions card", "card" in data["hint"])


def test_next_skips_claimed():
    print("test_next_skips_claimed")
    _q.n = 0
    a, b, c = _q(), _q(), _q()
    with Tmp() as t:
        t.write_queue([a, b, c])
        t.write_index({a["func"]: {"item_id": "I_a", "claimed": {"by": "x", "at": 1}}})
        rc, out = run("next", "--json")
        eq("rc", rc, 0)
        data = json.loads(out)
        eq("skips claimed -> second item", data["func"], b["func"])


def test_next_skips_parked():
    print("test_next_skips_parked")
    _q.n = 0
    parked = _q(status="parked")
    active = _q(status="active")
    with Tmp() as t:
        t.write_queue([parked, active])
        t.write_index({})
        rc, out = run("next", "--json")
        data = json.loads(out)
        eq("skips parked -> first active", data["func"], active["func"])


def test_next_claim_success_flags_held():
    print("test_next_claim_success_flags_held")
    _q.n = 0
    a = _q()
    with Tmp() as t:
        import os
        os.environ["CLAUDE_SESSION_ID"] = "me"
        t.write_queue([a])
        t.write_index({a["func"]: {"item_id": "I_a"}})  # unclaimed
        with GhSpy() as spy:
            rc, out = run("next", "--claim", "--json")
        eq("rc", rc, 0)
        data = json.loads(out)
        eq("picks the func", data["func"], a["func"])
        check("claimed flag true (we hold it)", data["claimed"] is True)
        eq("claimed_by is us", data["claimed_by"], "me")
        eq("one Status mutation issued", len(spy.set_field_calls), 1)


def test_next_claim_loses_race_to_other_owner():
    print("test_next_claim_loses_race_to_other_owner")
    _q.n = 0
    a = _q()
    with Tmp() as t:
        import os
        os.environ["CLAUDE_SESSION_ID"] = "me"
        t.write_queue([a])
        # Selection sees the func FREE; then in the race window another owner
        # claims it. Simulate by making load_index() return "free" the first time
        # (selection) and "claimed by other" thereafter (the _do_claim re-read).
        free = {a["func"]: {"item_id": "I_a"}}
        taken = {a["func"]: {"item_id": "I_a", "claimed": {"by": "other", "at": 1}}}
        calls = {"n": 0}
        real_load = board.load_index

        def racey_load(path=None):
            calls["n"] += 1
            return dict(free) if calls["n"] == 1 else dict(taken)

        board.load_index = racey_load
        try:
            with GhSpy() as spy:
                rc, out = run("next", "--claim", "--json")
        finally:
            board.load_index = real_load
        eq("rc", rc, 0)
        data = json.loads(out)
        eq("still reports the func", data["func"], a["func"])
        check("claim did NOT succeed", data["claimed"] is False)
        eq("reports the other owner", data["claimed_by"], "other")
        check("message names the other owner", "other" in (data.get("claim_message") or ""))
        eq("NO Status mutation when claim lost", len(spy.set_field_calls), 0)


def test_next_claim_loses_race_human_output():
    print("test_next_claim_loses_race_human_output")
    _q.n = 0
    a = _q()
    with Tmp() as t:
        import os
        os.environ["CLAUDE_SESSION_ID"] = "me"
        t.write_queue([a])
        free = {a["func"]: {"item_id": "I_a"}}
        taken = {a["func"]: {"item_id": "I_a", "claimed": {"by": "other", "at": 1}}}
        calls = {"n": 0}
        real_load = board.load_index

        def racey_load(path=None):
            calls["n"] += 1
            return dict(free) if calls["n"] == 1 else dict(taken)

        board.load_index = racey_load
        try:
            with GhSpy():
                rc, out = run("next", "--claim")
        finally:
            board.load_index = real_load
        eq("rc", rc, 0)
        check("human output flags FAILED claim", "FAILED" in out and "do NOT hold" in out)
        check("human output names the other owner", "other" in out)


def test_next_human_readable():
    print("test_next_human_readable")
    _q.n = 0
    a = _q(rules=3)
    with Tmp() as t:
        t.write_queue([a])
        t.write_index({})
        rc, out = run("next")
        eq("rc", rc, 0)
        check("prints func", a["func"] in out)
        check("prints board url", board.BOARD_URL in out)
        check("prints card hint", "board.py card" in out)


def test_card_local_read():
    print("test_card_local_read")
    with Tmp() as t:
        t.write_card("func_AAAA0001", "# CARD BODY MARKER\n")
        rc, out = run("card", "func_AAAA0001")
        eq("rc", rc, 0)
        check("prints local card body", "CARD BODY MARKER" in out)


def test_card_missing_triggers_refresh():
    print("test_card_missing_triggers_refresh")
    with Tmp() as t:
        called = {"func": None}

        def fake_regen(func):
            called["func"] = func
            body = "# REGENERATED\n"
            (t.cards / f"{func}.md").write_text(body, encoding="utf-8")
            return body

        saved = board._regen_card
        board._regen_card = fake_regen
        try:
            rc, out = run("card", "func_BBBB0002")
        finally:
            board._regen_card = saved
        eq("rc", rc, 0)
        eq("regen called for missing func", called["func"], "func_BBBB0002")
        check("prints regenerated body", "REGENERATED" in out)


def test_index_rebuild_merges_and_preserves():
    print("test_index_rebuild_merges_and_preserves")
    with Tmp() as t:
        # ledger = Done ids
        t.write_ledger({"DoneFunc": {"item_id": "I_done", "draft_id": "D_done", "finalized": True}})
        # existing index carries a claim that must survive rebuild
        t.write_index({"ActiveFunc": {"item_id": "OLD", "claimed": {"by": "me", "at": 5}}})
        visible = {"ActiveFunc": {"item_id": "I_active", "draft_id": "D_active"}}
        with GhSpy(visible=visible) as spy:
            rc, out = run("index", "--rebuild")
        eq("rc", rc, 0)
        idx = t.read_index()
        eq("done id from ledger", idx["DoneFunc"]["item_id"], "I_done")
        check("done finalized", idx["DoneFunc"].get("finalized") is True)
        eq("active id from visible read", idx["ActiveFunc"]["item_id"], "I_active")
        check("claim preserved across rebuild", idx["ActiveFunc"].get("claimed") == {"by": "me", "at": 5})
        check("report mentions counts", "1 active + 1 done" in out)
        eq("exactly one list query issued", spy.list_queries, 1)


def test_index_stats_no_rebuild():
    print("test_index_stats_no_rebuild")
    with Tmp() as t:
        t.write_index({"A": {"item_id": "x", "finalized": True}, "B": {"item_id": "y"}})
        with GhSpy() as spy:
            rc, out = run("index")
        eq("rc", rc, 0)
        check("stats line printed", "1 active + 1 done" in out)
        eq("no API on plain index", len(spy.graphql_calls), 0)


def test_claim_updates_index_and_mutates():
    print("test_claim_updates_index_and_mutates")
    with Tmp() as t:
        t.write_index({"FuncC": {"item_id": "I_c"}})
        import os
        os.environ["CLAUDE_SESSION_ID"] = "sess-42"
        with GhSpy() as spy:
            rc, out = run("claim", "FuncC")
        eq("rc", rc, 0)
        idx = t.read_index()
        eq("claim owner recorded", idx["FuncC"]["claimed"]["by"], "sess-42")
        eq("one Status set", len(spy.set_field_calls), 1)
        eq("status value In-Progress", spy.set_field_calls[0][1:], ("Status", "In-Progress"))
        eq("set on right item", spy.set_field_calls[0][0], "I_c")
        eq("NO list query on claim", spy.list_queries, 0)


def test_claim_idempotent_same_owner():
    print("test_claim_idempotent_same_owner")
    with Tmp() as t:
        import os
        os.environ["CLAUDE_SESSION_ID"] = "sess-7"
        t.write_index({"F": {"item_id": "I_f", "claimed": {"by": "sess-7", "at": 1}}})
        with GhSpy() as spy:
            rc, out = run("claim", "F")
        eq("rc", rc, 0)
        check("reports already claimed by you", "already claimed by you" in out)
        eq("no mutation on re-claim by same owner", len(spy.set_field_calls), 0)


def test_claim_blocked_other_owner():
    print("test_claim_blocked_other_owner")
    with Tmp() as t:
        import os
        os.environ["CLAUDE_SESSION_ID"] = "me"
        t.write_index({"F": {"item_id": "I_f", "claimed": {"by": "other", "at": 1}}})
        with GhSpy() as spy:
            rc, out = run("claim", "F")
        check("reports who holds it", "other" in out)
        eq("no mutation when held by other", len(spy.set_field_calls), 0)


def test_done_archives_and_clears():
    print("test_done_archives_and_clears")
    with Tmp() as t:
        t.write_index({"F": {"item_id": "I_f", "claimed": {"by": "me", "at": 1}}})
        with GhSpy() as spy:
            rc, out = run("done", "F")
        eq("rc", rc, 0)
        idx = t.read_index()
        check("claim cleared", "claimed" not in idx["F"])
        check("finalized set", idx["F"].get("finalized") is True)
        eq("Status set to Done", spy.set_field_calls[0][1:], ("Status", "Done"))
        check("archive mutation issued", any(v and v.get("i") == "I_f" for _q2, v in spy.mutate_calls))
        check("reminder about engine completion", "queue done" in out)
        eq("NO list query on done", spy.list_queries, 0)


def test_block_records_reason():
    print("test_block_records_reason")
    with Tmp() as t:
        t.write_index({"F": {"item_id": "I_f", "claimed": {"by": "me", "at": 1}}})
        with GhSpy() as spy:
            rc, out = run("block", "F", "--reason", "needs canonical auth")
        eq("rc", rc, 0)
        idx = t.read_index()
        eq("reason recorded", idx["F"]["reason"], "needs canonical auth")
        check("claim cleared", "claimed" not in idx["F"])
        eq("Status set to Blocked", spy.set_field_calls[0][1:], ("Status", "Blocked"))
        eq("NO list query on block", spy.list_queries, 0)


def test_release_back_to_backlog():
    print("test_release_back_to_backlog")
    with Tmp() as t:
        t.write_index({"F": {"item_id": "I_f", "claimed": {"by": "me", "at": 1}}})
        with GhSpy() as spy:
            rc, out = run("release", "F")
        eq("rc", rc, 0)
        idx = t.read_index()
        check("claim cleared", "claimed" not in idx["F"])
        eq("Status set to Backlog", spy.set_field_calls[0][1:], ("Status", "Backlog"))
        eq("NO list query on release", spy.list_queries, 0)


def test_move_without_item_id_errors():
    print("test_move_without_item_id_errors")
    with Tmp() as t:
        t.write_index({})  # no entry for the func
        with GhSpy() as spy:
            rc, out = run("claim", "Ghost")
        check("exits nonzero", rc not in (0, None))
        eq("no mutation attempted", len(spy.set_field_calls), 0)


def test_status_counts():
    print("test_status_counts")
    _q.n = 0
    items = [_q(status="active"), _q(status="active"),
             _q(status="parked"), _q(status="active", rules=2)]
    with Tmp() as t:
        t.write_queue(items)
        t.write_index({})
        rc, out = run("status", "--json")
        eq("rc", rc, 0)
        data = json.loads(out)
        eq("total", data["total"], 4)
        eq("active count", data["by_status"]["active"], 3)
        eq("parked count", data["by_status"]["parked"], 1)
        eq("top is first active", data["top"], items[0]["func"])


def main():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_") and callable(v)]
    for t in tests:
        t()
    print(f"\n{_RUN} checks, {len(_FAILS)} failed")
    if _FAILS:
        for d in _FAILS:
            print(f"  - {d}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
