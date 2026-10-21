#!/usr/bin/env python3
"""Tests for the pure decision logic of tools/lanes/orchestrate.py (no Claude sessions started).

    python tools/lanes/test_orchestrate.py
"""
import sys
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import orchestrate as o  # noqa: E402

NOW = 1_000_000.0


def bl(naming_stages=("reviewed", "idea"), foundation=(("pins", "idea"), ("pins", "idea"),
                                                       ("bss", "landed"), ("data", "active"))):
    return {"naming": [{"id": f"m{i}", "stage": s, "note": ""} for i, s in enumerate(naming_stages)],
            "foundation": [{"id": f"f{i}", "lane": l, "stage": s, "note": ""}
                           for i, (l, s) in enumerate(foundation)]}


def st(sessions=None, no_progress=()):
    return {"sessions": sessions or {}, "no_progress": list(no_progress)}


def slot(lane, session="S1", started=NOW - 600, **kw):
    return {"lane": lane, "session": session, "started": started, **kw}


def reg(**lanes):
    return {"lanes": lanes}


class Candidates(unittest.TestCase):
    def test_alternates_and_skips_done_lanes(self):
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, None),
                         ["naming-a", "pins", "naming-b", "data"])

    def test_naming_limited_by_workable_rows(self):
        b = bl(naming_stages=("reviewed", "landed", "researching"))
        self.assertEqual(o.candidates(b, reg(), st({"x": slot("naming-a")}), NOW, "naming"), [])
        self.assertEqual(o.candidates(b, reg(), st(), NOW, "naming"), ["naming-a"])

    def test_skips_running_owned_and_no_progress(self):
        r = reg(pins={"owner": "other", "heartbeat": NOW}, **{"naming-a": {"owner": "z", "heartbeat": NOW}})
        self.assertEqual(o.candidates(bl(), r, st(), NOW, None), ["naming-b", "data"])
        self.assertEqual(o.candidates(bl(), reg(), st({"x": slot("data")}, ["naming"]), NOW, None),
                         ["pins"])

    def test_stale_owner_does_not_block(self):
        r = reg(pins={"owner": "old", "heartbeat": NOW - o.STALE_SECS - 1})
        self.assertIn("pins", o.candidates(bl(), r, st(), NOW, "foundation"))


class Phase(unittest.TestCase):
    def test_new_session_not_yet_open_is_running(self):
        self.assertEqual(o.session_phase(slot("pins"), {"state": "working", "pid": 1}, reg(), NOW), "running")

    def test_idle_waiting_on_land_is_running_then_stalled(self):
        r = reg(pins={"owner": "S1"})
        self.assertEqual(o.session_phase(slot("pins", idle_since=NOW - 60), {"state": "done", "pid": 1}, r, NOW),
                         "running")
        self.assertEqual(o.session_phase(slot("pins", idle_since=NOW - o.STALL_SECS),
                                         {"state": "done", "pid": 1}, r, NOW), "stalled")

    def test_closed_and_idle_is_finished(self):
        r = reg(pins={"owner": None, "closed": NOW - 5})
        self.assertEqual(o.session_phase(slot("pins"), {"state": "done", "pid": 1}, r, NOW), "finished")
        # closed but still working on its report: not yet
        self.assertEqual(o.session_phase(slot("pins"), {"state": "working", "pid": 1}, r, NOW), "running")

    def test_old_close_from_a_previous_session_is_not_this_one(self):
        r = reg(pins={"owner": None, "closed": NOW - 7200})
        self.assertEqual(o.session_phase(slot("pins"), {"state": "done", "pid": 1}, r, NOW), "running")

    def test_exited_without_closing_is_lost(self):
        r = reg(pins={"owner": "S1"})
        self.assertEqual(o.session_phase(slot("pins"), None, r, NOW), "lost")
        self.assertEqual(o.session_phase(slot("pins", seen_open=True), {"state": "stopped"},
                                         reg(pins={"owner": None}), NOW), "finished")


class Slots(unittest.TestCase):
    def test_finish_with_progress_and_without(self):
        b = bl()
        s = st({"a1": slot("pins", progress_at_start="old"),
                "b2": slot("naming-a", session="S2", progress_at_start=o.progress_key(b, "naming-a"))})
        r = reg(pins={"owner": None, "closed": NOW}, **{"naming-a": {"owner": None, "closed": NOW}})
        ev = o.update_slots(s, {"a1": {"state": "done", "pid": 1}, "b2": {"state": "done", "pid": 1}}, r, b, NOW)
        self.assertEqual(sorted(ev), [("finished", "a1"), ("no-progress", "b2")])
        self.assertEqual(s["sessions"], {})
        self.assertEqual(s["no_progress"], ["naming"])

    def test_seen_open_then_closed(self):
        s = st({"a1": slot("pins", started=NOW - 99999)})
        o.update_slots(s, {"a1": {"state": "working", "pid": 1}}, reg(pins={"owner": "S1"}), bl(), NOW)
        self.assertTrue(s["sessions"]["a1"]["seen_open"])
        ev = o.update_slots(s, {"a1": {"state": "done", "pid": 1}}, reg(pins={"owner": None}), bl(), NOW)
        self.assertEqual(ev, [("no-progress", "a1")] if s["no_progress"] else [("finished", "a1")])
        self.assertEqual(s["sessions"], {})

    def test_stall_warned_once(self):
        s = st({"a1": slot("pins", idle_since=NOW - o.STALL_SECS)})
        r = reg(pins={"owner": "S1"})
        self.assertEqual(o.update_slots(s, {"a1": {"state": "done", "pid": 1}}, r, bl(), NOW), [("stalled", "a1")])
        self.assertEqual(o.update_slots(s, {"a1": {"state": "done", "pid": 1}}, r, bl(), NOW), [])
        self.assertIn("a1", s["sessions"])


class Cli(unittest.TestCase):
    def test_number_means_run(self):
        seen = {}
        orig = o.cmd_run
        o.cmd_run = lambda a: seen.update(n=a.n, only=a.only)
        try:
            o.main(["4", "--only", "naming"])
        finally:
            o.cmd_run = orig
        self.assertEqual(seen, {"n": 4, "only": "naming"})


class Robustness(unittest.TestCase):
    def test_exited_row_without_pid_is_ended(self):
        r = reg(pins={"owner": "S1"})
        self.assertEqual(o.session_phase(slot("pins"), {"state": "done"}, r, NOW), "lost")

    def test_missing_from_listing_needs_several_ticks(self):
        s = st({"a1": slot("pins")})
        r = reg(pins={"owner": "S1"})
        for _ in range(o.MISS_TICKS - 1):
            self.assertEqual(o.update_slots(s, {}, r, bl(), NOW), [])
        self.assertEqual(o.update_slots(s, {}, r, bl(), NOW), [("lost", "a1")])

    def test_session_id_filled_from_listing(self):
        s = st({"a1": slot("pins", session="")})
        o.update_slots(s, {"a1": {"state": "working", "pid": 1, "sessionId": "FULL"}},
                       reg(pins={"owner": "FULL"}), bl(), NOW)
        self.assertEqual(s["sessions"]["a1"]["session"], "FULL")
        self.assertTrue(s["sessions"]["a1"]["seen_open"])

    def test_listing_parse_tolerates_noise(self):
        saved = o.sh
        o.sh = lambda cmd, timeout=0, cwd=None, merge=True: (
            0, 'note [x]\n[{"id": "ab", "kind": "background", "pid": 1}]\n(node) Warning: x')
        try:
            self.assertEqual(list(o.agents()), ["ab"])
        finally:
            o.sh = saved


class Tick(unittest.TestCase):
    """tick() end to end with the claude CLI, registry and backlogs faked."""

    def setUp(self):
        import tempfile
        self.d = Path(tempfile.mkdtemp(prefix="orch-test-"))
        self.saved = {k: getattr(o, k) for k in ("STATE", "MAX_FILE", "REGISTRY", "backlogs",
                                                 "agents", "sh", "read_json")}
        o.STATE, o.MAX_FILE = self.d / "state.json", self.d / "max"
        self.reg = {"lanes": {}}
        self.listing = {}
        self.started, self.stopped = [], []
        o.backlogs = lambda: bl(naming_stages=(), foundation=(("pins", "idea"), ("data", "idea")))
        real_read = self.saved["read_json"]
        o.read_json = lambda p, d: self.reg if p == o.REGISTRY else real_read(p, d)
        o.agents = lambda: self.listing

        def fake_sh(cmd, timeout=0, cwd=None, merge=True):
            if "--bg" in cmd:
                sid = f"{len(self.started) + 1:08x}"
                self.started.append(cmd[-1])
                self.listing[sid] = {"state": "working", "pid": 1, "sessionId": "S" + sid}
                return 0, f"backgrounded · {sid} · lane"
            if cmd[1:2] == ["stop"]:
                self.stopped.append(cmd[2])
                self.listing[cmd[2]] = {"state": "done"}
                return 0, "stopped"
            return 0, ""
        o.sh = fake_sh

        class A:
            only = None
            model = None
            permission_mode = "bypassPermissions"
        self.args = A

    def tearDown(self):
        for k, v in self.saved.items():
            setattr(o, k, v)
        for f in self.d.iterdir():
            f.unlink()
        self.d.rmdir()

    def test_fills_then_replaces_finished(self):
        o.write_max(1)
        self.assertTrue(o.tick(self.args))
        self.assertEqual(self.started, ["/foundation-session pins"])
        sid = next(iter(o.load_state()["sessions"]))
        self.reg["lanes"]["pins"] = {"owner": "S" + sid, "heartbeat": time.time()}
        self.assertTrue(o.tick(self.args))  # open and working: nothing new
        self.assertEqual(len(self.started), 1)
        self.reg["lanes"]["pins"] = {"owner": None, "closed": time.time()}
        self.listing[sid]["state"] = "done"
        self.assertTrue(o.tick(self.args))
        self.assertEqual(self.stopped, [sid])
        self.assertEqual(self.started[-1], "/foundation-session data")
        self.assertIn("pins", o.load_state()["no_progress"])

    def test_failed_listing_keeps_slots(self):
        o.write_max(1)
        o.tick(self.args)
        o.agents = lambda: None
        self.assertTrue(o.tick(self.args))
        self.assertEqual(len(o.load_state()["sessions"]), 1)
        self.assertEqual(len(self.started), 1)

    def test_start_failure_stops_run(self):
        o.write_max(2)
        o.sh = lambda cmd, timeout=0, cwd=None, merge=True: (1, "boom")
        self.assertFalse(o.tick(self.args))

    def test_max_zero_drains(self):
        o.write_max(0)
        self.assertFalse(o.tick(self.args))
        self.assertEqual(self.started, [])


if __name__ == "__main__":
    unittest.main(verbosity=1)
