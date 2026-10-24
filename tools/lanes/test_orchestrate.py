#!/usr/bin/env python3
"""Tests for the pure decision logic of tools/lanes/orchestrate.py (no Claude sessions started).

    python tools/lanes/test_orchestrate.py
"""
import io
import sys
import time
from contextlib import redirect_stdout
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

    def test_researching_row_workable_once_its_codex_run_finished(self):
        import json
        import tempfile
        saved = o.CODEX_RUNS
        with tempfile.TemporaryDirectory() as d:
            o.CODEX_RUNS = Path(d)
            try:
                def run(rid, **meta):
                    (Path(d) / rid).mkdir()
                    (Path(d) / rid / "meta.json").write_text(json.dumps(meta), encoding="utf-8")
                run("20261024-070301-research-w78", status="ok", finished="2026-10-24T07:13")
                run("20261024-080000-research-w79", status="running")
                run("20261024-081000-research-w80", status="timeout", finished="2026-10-24T08:40")
                (Path(d) / "20261024-082000-research-w81").mkdir()
                (Path(d) / "20261024-082000-research-w81" / "meta.json").write_text(
                    '{"status": "runn', encoding="utf-8")
                row = lambda note: {"id": "x", "stage": "researching", "note": note}
                self.assertTrue(o.naming_row_workable(
                    row("run ~/.claude/codex-bridge/runs/20261024-070301-research-w78; prompt")))
                self.assertFalse(o.naming_row_workable(row("run 20261024-080000-research-w79")))
                self.assertFalse(o.naming_row_workable(
                    row("20261024-070301-research-w78 then 20261024-080000-research-w79")))
                self.assertTrue(o.naming_row_workable(row("run 20261024-070301-research-w78.")))
                self.assertTrue(o.naming_row_workable(row("run 20261024-081000-research-w80")))
                self.assertFalse(o.naming_row_workable(row("run 20261024-082000-research-w81")))
                self.assertFalse(o.naming_row_workable(row("Codex busy, prompt ready")))
                self.assertFalse(o.naming_row_workable(row("run 20261024-090000-research-gone")))
                self.assertFalse(o.naming_row_workable(
                    {"id": "x", "stage": "landed", "note": "20261024-070301-research-w78"}))
                b = {"naming": [row("20261024-070301-research-w78")], "foundation": []}
                self.assertEqual(o.candidates(b, reg(), st(), NOW, "naming"), ["naming-a"])
            finally:
                o.CODEX_RUNS = saved

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

    def test_idle_without_ever_opening_is_finished_after_grace(self):
        a = {"state": "done", "status": "idle", "pid": 1}
        g = o.NO_OPEN_GRACE_SECS
        self.assertEqual(o.session_phase(slot("codex", idle_since=NOW - g + 5), a, reg(), NOW),
                         "running")
        self.assertEqual(o.session_phase(slot("codex", idle_since=NOW - g), a, reg(), NOW),
                         "finished")
        # someone else owning the lane does not make it this session's
        r = reg(pins={"owner": "OTHER", "heartbeat": NOW})
        self.assertEqual(o.session_phase(slot("pins", idle_since=NOW - g), a, r, NOW), "finished")
        # owning its lane while idle keeps the stalled path
        r = reg(pins={"owner": "S1", "heartbeat": NOW})
        self.assertEqual(o.session_phase(slot("pins", idle_since=NOW - g), a, r, NOW), "running")

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

    def test_never_opened_idle_counts_as_no_progress(self):
        b = bl()
        s = st({"a1": slot("pins", progress_at_start=o.progress_key(b, "pins"))})
        a = {"a1": {"state": "done", "status": "idle", "pid": 1}}
        self.assertEqual(o.update_slots(s, a, reg(), b, NOW), [])  # idle_since set now
        later = NOW + o.NO_OPEN_GRACE_SECS
        self.assertEqual(o.update_slots(s, a, reg(), b, later), [("no-progress", "a1")])
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
                                                 "agents", "sh", "read_json", "PAUSED",
                                                 "CODEX_FILE", "ATTENTION_DIR", "CODEX_BRIEF",
                                                 "transcript_path", "bridge_status", "owner_away")}
        o.STATE, o.MAX_FILE = self.d / "state.json", self.d / "max"
        o.PAUSED, o.CODEX_FILE = self.d / "paused.json", self.d / "codex.json"
        o.ATTENTION_DIR, o.CODEX_BRIEF = self.d / "att", self.d / "brief.md"
        o.CODEX_BRIEF.write_text("brief", encoding="utf-8")
        o.transcript_path = lambda session, cwd, root=None: None
        self.bridge = {"clear": True, "usage": {"primary": {"usedPercent": 10},
                                                "secondary": {"usedPercent": 10}}}
        o.bridge_status = lambda: self.bridge
        o.owner_away = lambda: False
        self.git_log = []
        self.reg = {"lanes": {}}
        self.listing = {}
        self.started, self.stopped = [], []
        o.backlogs = lambda: bl(naming_stages=(), foundation=(("pins", "idea"), ("data", "idea")))
        real_read = self.saved["read_json"]
        o.read_json = lambda p, d: self.reg if p == o.REGISTRY else real_read(p, d)
        o.agents = lambda: self.listing

        self.fail_codex_start, self.codex_attempts = False, 0

        def fake_sh(cmd, timeout=0, cwd=None, merge=True):
            if self.fail_codex_start and "--bg" in cmd and cmd[-1] == o.CODEX_PROMPT:
                self.codex_attempts += 1
                return 1, "boom"
            if "--bg" in cmd:
                sid = f"{len(self.started) + 1:08x}"
                self.started.append(cmd[-1])
                self.listing[sid] = {"state": "working", "pid": 1, "sessionId": "S" + sid,
                                     "name": cmd[cmd.index("-n") + 1]}
                return 0, f"backgrounded · {sid} · lane"
            if cmd[0] == "git":
                if "rev-parse" in cmd:
                    return 0, "abc123\n"
                # self.git_log: (subject, body) pairs, in `%ct%x1f%s%x1f%b%x1e` records
                return 0, "".join(f"{int(time.time())}\x1f{s}\x1f{b}\x1e\n" for s, b in self.git_log)
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
        att = self.d / "att"
        if att.is_dir():
            for f in att.iterdir():
                f.unlink()
            att.rmdir()
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

    def test_no_progress_raises_attention(self):
        o.write_max(1)
        o.tick(self.args)
        sid = next(iter(o.load_state()["sessions"]))
        self.reg["lanes"]["pins"] = {"owner": None, "closed": time.time()}
        self.listing[sid]["state"] = "done"
        o.tick(self.args)
        att = o.load_attention()
        self.assertEqual([(i["lane"], i["sid"]) for i in att], [("pins", sid)])
        self.assertTrue(Path(att[0]["path"]).is_file())
        o.main(["ack", "all"])
        self.assertEqual(o.load_attention(), [])

    def test_ack_never_touches_runner_state(self):
        o.update_attention(lambda items: items + [{"lane": "x", "sid": "1"}])
        o.write_state({"sessions": {"a1": slot("pins")}, "no_progress": [], "codex_history": []})
        before = o.STATE.read_bytes()
        o.main(["ack", "x"])
        o.main(["pause", "data"])
        o.main(["codex", "on"])
        self.assertEqual(o.STATE.read_bytes(), before)
        o.write_state(o.load_state())  # a runner write cannot bring the item back
        self.assertEqual(o.load_attention(), [])

    def test_corrupt_state_skips_then_restores(self):
        o.write_max(1)
        o.tick(self.args)
        good = o.load_state()
        o.STATE.write_text("{not json", encoding="utf-8")
        for _ in range(o.CORRUPT_TICKS - 1):
            self.assertTrue(o.tick(self.args))
            self.assertEqual(o.STATE.read_text(encoding="utf-8"), "{not json")  # never "{}"
        self.assertEqual(len(self.started), 1)
        self.assertTrue(o.tick(self.args))  # third in a row: set aside, last good restored
        self.assertEqual(o.load_state()["sessions"].keys(), good["sessions"].keys())
        self.assertTrue(any(f.name.startswith("state.json.corrupt-") for f in self.d.iterdir()))
        with self.assertRaises(SystemExit):
            o.STATE.write_text("[]", encoding="utf-8")
            o.main(["status"])

    def test_event_error_does_not_kill_the_tick(self):
        o.write_max(1)
        o.tick(self.args)
        sid = next(iter(o.load_state()["sessions"]))
        self.reg["lanes"]["pins"] = {"owner": None, "closed": time.time()}
        self.listing[sid]["state"] = "done"
        saved = o.raise_attention

        def boom(*a, **k):
            raise UnicodeEncodeError("charmap", "→", 0, 1, "nope")
        o.raise_attention = boom
        try:
            self.assertTrue(o.tick(self.args))
        finally:
            o.raise_attention = saved
        st = o.load_state()
        self.assertNotIn(sid, st["sessions"])            # the finished slot is gone for good
        self.assertEqual(self.started[-1], "/foundation-session data")

    def test_when_away_waits_plain_pause_exits(self):
        o.write_max(1)
        o.main(["pause", "pins", "--when-away"])
        o.main(["pause", "data"])
        self.assertTrue(o.tick(self.args))               # pins waits for the owner to be away
        self.assertEqual(self.started, [])
        self.assertIn("pins", o.load_state()["waiting"])
        o.owner_away = lambda: True
        self.assertTrue(o.tick(self.args))
        self.assertEqual(self.started, ["/foundation-session pins"])
        o.owner_away = lambda: False
        o.main(["pause", "pins"])                        # now a plain pause, like data
        o.STATE.unlink()
        for a in self.listing.values():                  # its session has exited (no pid)
            a.pop("pid", None)
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.assertFalse(o.tick(self.args))
        self.assertIn("remaining work is paused", buf.getvalue())

    def test_codex_quota_block_alone_keeps_running(self):
        o.backlogs = lambda: bl(naming_stages=(), foundation=())
        o.main(["codex", "on"])
        o.write_max(1)
        self.bridge["usage"]["secondary"]["usedPercent"] = 85
        self.assertTrue(o.tick(self.args))
        self.assertIn("weekly", o.load_state()["waiting"])
        o.main(["codex", "off"])
        self.assertFalse(o.tick(self.args))

    def test_unavailable_bridge_does_not_keep_it_alive(self):
        o.backlogs = lambda: bl(naming_stages=(), foundation=())
        o.main(["codex", "on"])
        o.write_max(1)
        o.bridge_status = lambda: None
        self.assertFalse(o.tick(self.args))
        st = o.load_state()
        self.assertIn("unavailable", st["codex_blocked"])
        self.assertNotIn("waiting", st)

    def test_bridge_blocked_on_its_own_usage_window_waits(self):
        o.backlogs = lambda: bl(naming_stages=(), foundation=())
        o.main(["codex", "on"])
        o.write_max(1)
        self.bridge.update(clear=False, blockers=["5-hour window at 77% (block >= 75%), resets x"])
        self.assertTrue(o.tick(self.args))
        self.assertIn("BLOCKED", o.load_state()["waiting"])

    def test_lost_state_write_adopts_instead_of_starting_twice(self):
        o.write_max(2)
        real = o.write_state
        o.write_state = lambda st: None   # the start is never recorded
        try:
            o.tick(self.args)
        finally:
            o.write_state = real
        self.assertEqual(self.started, ["/foundation-session pins", "/foundation-session data"])
        self.assertEqual(o.load_state()["sessions"], {})
        o.tick(self.args)                 # both adopted from the listing, neither started again
        self.assertEqual(len(self.started), 2)
        st = o.load_state()
        self.assertEqual(sorted(s["lane"] for s in st["sessions"].values()), ["data", "pins"])
        self.assertTrue(all(s.get("adopted") for s in st["sessions"].values()))

    def test_untracked_duplicate_lane_counts_as_busy(self):
        o.write_max(2)
        self.listing["dddddddd"] = {"name": "lane-pins", "state": "working", "pid": 9,
                                    "sessionId": "SD"}
        o.tick(self.args)
        self.assertNotIn("/foundation-session pins", self.started)
        self.assertEqual(self.started, ["/foundation-session data"])

    def test_codex_start_failure_backs_off(self):
        o.main(["codex", "on"])
        o.write_max(1)
        self.fail_codex_start = True
        o.tick(self.args)
        o.tick(self.args)
        self.assertEqual(self.codex_attempts, 1)
        st = o.load_state()
        st["codex_start_failed"] = time.time() - o.CODEX_RECHECK_SECS
        o.write_state(st)
        o.tick(self.args)
        self.assertEqual(self.codex_attempts, 2)

    def test_paused_lane_not_started_and_persists(self):
        o.main(["pause", "pins"])
        o.write_max(1)
        o.tick(self.args)
        self.assertEqual(self.started, ["/foundation-session data"])
        self.assertIn("pins", o.load_paused())
        o.main(["resume", "pins"])
        self.assertEqual(o.load_paused(), {})

    def _finish_codex(self):
        cx = next(s for s, v in o.load_state()["sessions"].items() if v.get("kind") == "codex")
        self.reg["lanes"]["codex"] = {"owner": None, "closed": time.time()}
        self.listing[cx]["state"] = "done"
        o.tick(self.args)

    def test_codex_lane_outside_n_and_two_strikes(self):
        o.main(["codex", "on"])
        o.write_max(1)
        o.tick(self.args)
        self.assertEqual(self.started, ["/foundation-session pins", o.CODEX_PROMPT])
        self._finish_codex()   # strike 1: restarted
        self._finish_codex()   # strike 2: not restarted
        st = o.load_state()
        self.assertEqual(len(st["codex_history"]), 2)
        self.assertFalse(any(v.get("kind") == "codex" for v in st["sessions"].values()))
        self.assertIn("landed nothing", st["codex_blocked"])
        self.assertEqual(sum(1 for i in o.load_attention() if i["lane"] == "codex"), 2)
        self.assertEqual(self.started.count(o.CODEX_PROMPT), 2)
        o.main(["codex", "on"])  # re-arms without an off first
        o.tick(self.args)
        self.assertEqual(self.started.count(o.CODEX_PROMPT), 3)

    def test_codex_quota_drain_and_landing(self):
        o.main(["codex", "on"])
        o.write_max(0)  # drain: nothing starts, codex included
        self.assertFalse(o.tick(self.args))
        o.write_max(1)
        self.bridge["usage"]["primary"]["usedPercent"] = 95
        o.tick(self.args)
        self.assertNotIn(o.CODEX_PROMPT, self.started)
        self.bridge["usage"]["primary"]["usedPercent"] = 10
        o.tick(self.args)  # rechecks only every CODEX_RECHECK_SECS
        self.assertNotIn(o.CODEX_PROMPT, self.started)
        st = o.load_state()
        st["codex_checked"] = 0
        o.write_state(st)
        o.tick(self.args)
        self.assertIn(o.CODEX_PROMPT, self.started)
        self.git_log = [("4D0A8.c: func t1 label", "x\n\nCodex run: 20261022-1 (thread"),
                        ("cheat-cleanup: claude fake lane", "body")]
        self._finish_codex()
        st = o.load_state()
        self.assertEqual(st["codex_history"][-1]["landed"], ["4D0A8.c: func t1 label"])
        self.assertEqual(self.started.count(o.CODEX_PROMPT), 2)
        self.assertEqual(o.load_attention(), [])

    def test_adopts_running_lane_codex(self):
        self.listing["c0dec0de"] = {"name": "lane-codex", "state": "working", "pid": 5,
                                    "sessionId": "SC", "startedAt": 1000}
        o.write_max(1)
        o.tick(self.args)
        st = o.load_state()
        self.assertEqual(st["sessions"]["c0dec0de"]["kind"], "codex")
        self.assertNotIn(o.CODEX_PROMPT, self.started)


    def test_finished_idle_session_not_adopted_nor_busy(self):
        # an idle `lane-codex` / `lane-pins` long done: process alive, lane not owned
        self.listing["41a20439"] = {"name": "lane-codex", "state": "done", "status": "idle",
                                    "pid": 5, "sessionId": "SC", "startedAt": 1000}
        self.listing["dddddddd"] = {"name": "lane-pins", "state": "done", "status": "idle",
                                    "pid": 9, "sessionId": "SD", "startedAt": 1000}
        o.write_max(1)
        o.tick(self.args)
        st = o.load_state()
        self.assertNotIn("41a20439", st["sessions"])
        self.assertNotIn("dddddddd", st["sessions"])
        self.assertEqual(self.started, ["/foundation-session pins"])

    def test_idle_session_owning_its_lane_is_adopted(self):
        self.listing["dddddddd"] = {"name": "lane-pins", "state": "done", "status": "idle",
                                    "pid": 9, "sessionId": "SD", "startedAt": 1000}
        self.reg["lanes"]["pins"] = {"owner": "SD", "heartbeat": time.time()}
        o.write_max(1)
        o.tick(self.args)
        self.assertTrue(o.load_state()["sessions"]["dddddddd"]["adopted"])
        self.assertNotIn("/foundation-session pins", self.started)

    def test_codex_idle_without_opening_finishes_after_grace(self):
        o.main(["codex", "on"])
        o.write_max(1)
        o.tick(self.args)
        cx = next(s for s, v in o.load_state()["sessions"].items() if v.get("kind") == "codex")
        self.listing[cx].update(state="done", status="idle")
        o.tick(self.args)  # idle, never opened: within the grace period it is still running
        self.assertIn(cx, o.load_state()["sessions"])
        st = o.load_state()
        st["sessions"][cx]["idle_since"] = time.time() - o.NO_OPEN_GRACE_SECS
        o.write_state(st)
        o.tick(self.args)
        st = o.load_state()
        self.assertEqual(st["codex_history"][-1]["sid"], cx)
        self.assertEqual(st["codex_history"][-1]["event"], "finished")
        self.assertIn(cx, self.stopped)


class Pause(unittest.TestCase):
    def test_pause_and_when_away(self):
        p = {"pins": {"when_away": False}, "data": {"when_away": True}}
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "foundation", p, False), [])
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "foundation", p, True), ["data"])

    def test_naming_group_and_single_lane_pause(self):
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "naming", {"naming": {}}), [])
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "naming", {"naming-a": {}}),
                         ["naming-b", "naming-c"])


class Io(unittest.TestCase):
    def test_non_ascii_log_on_cp1252_stream(self):
        raw = io.BytesIO()
        stream = io.TextIOWrapper(raw, encoding="cp1252")
        saved = sys.stdout, sys.stderr
        sys.stdout = sys.stderr = stream
        try:
            with self.assertRaises(UnicodeEncodeError):
                o.log("ATTENTION x: → 日本")
            o.utf8_stdio()
            o.log("ATTENTION x: → 日本")
            stream.flush()
        finally:
            sys.stdout, sys.stderr = saved
        self.assertIn("→ 日本".encode("utf-8"), raw.getvalue())

    def test_write_json_unique_temp_and_retry(self):
        import tempfile
        d = Path(tempfile.mkdtemp(prefix="orch-io-"))
        target = d / "x.json"
        real, calls = o.os.replace, []

        def flaky(src, dst):
            calls.append(src)
            if len(calls) < 3:
                raise PermissionError("in use")
            real(src, dst)
        o.os.replace = flaky
        try:
            o.write_json(target, {"a": 1})
        finally:
            o.os.replace = real
        self.assertEqual(len(calls), 3)
        self.assertNotEqual(Path(calls[0]).name, "x.tmp")   # unique name, not a shared .tmp
        self.assertEqual(o.read_json(target, None), {"a": 1})
        self.assertEqual([f.name for f in d.iterdir()], ["x.json"])
        target.unlink()
        d.rmdir()


class Live(unittest.TestCase):
    def test_live_lanes(self):
        ag = {"a": {"name": "lane-pins", "pid": 1, "state": "working"},
              "b": {"name": "lane-data", "state": "done"},            # exited: no pid
              "c": {"name": "Decompile next function", "pid": 2},
              "d": {"name": "lane-", "pid": 3},
              "e": {"name": "lane-naming-a", "pid": 4, "state": "done"},  # idle, still alive
              "f": {"name": "lane-naming-b", "pid": 6, "status": "idle", "sessionId": "SF"},
              "g": {"name": "lane-codex", "pid": 7, "state": "done", "status": "idle",
                    "sessionId": "SG"}}  # finished hours ago; codex lane free
        r = reg(**{"naming-b": {"owner": "SF", "heartbeat": NOW},
                   "codex": {"owner": None, "closed": NOW - 9999}})
        # idle sessions count only while they own their lane (waiting on a land)
        self.assertEqual(o.live_lanes(ag, r), {"pins": "a", "naming-b": "f"})
        self.assertEqual(o.live_lanes(ag, reg()), {"pins": "a"})

    def test_candidates_treat_live_as_busy(self):
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "foundation", live={"pins"}),
                         ["data"])
        # a live naming session also uses up a workable naming row
        self.assertEqual(o.candidates(bl(), reg(), st(), NOW, "naming", live={"naming-a"}),
                         ["naming-b"])


class Blocked(unittest.TestCase):
    def test_blocked_rows_count_as_done(self):
        b = bl(foundation=(("pins", "blocked"), ("data", "idea"), ("data", "blocked")))
        self.assertEqual(o.candidates(b, reg(), st(), NOW, "foundation"), ["data"])


class Codex(unittest.TestCase):
    def test_landed_by_body(self):
        commits = [("cheat-cleanup: a", "the Claude fake lane's own work"),
                   ("docs: Codex lane note", ""),
                   ("4D0A8.c: func_80060768 t1 label", "...\nCodex run: 20261022-165732-x (thread"),
                   ("docs: fake-reduce", "for the fake lane, not the Codex lane."),
                   ("Match: f", ""),
                   ("tools: orchestrate.py surfaces stopped lanes' questions, persistent pause, "
                    "built-in Codex lane", "the Codex lane restarts while usage allows"),
                   ("tools: codex_worker records every layer-2 verdict", "see the Codex run: line"),
                   ("Match: g", "x\n  Codex run: indented is not the line")]
        self.assertEqual(o.codex_landed(commits), ["4D0A8.c: func_80060768 t1 label"])

    def test_parse_log(self):
        out = ("100\x1fs1\x1fb1\nmore\n\x1e\n"
               "50\x1fs2: café\x1f\x1e\n"
               "garbage\x1e")
        self.assertEqual(o.parse_log(out), [("s1", "b1\nmore"), ("s2: café", "")])
        self.assertEqual(o.parse_log(out, since_ts=60), [("s1", "b1\nmore")])

    def test_gate(self):
        on = {"enabled": True, "armed": 0}
        self.assertIsNone(o.codex_gate(on, {}, False, []))
        self.assertIn("off", o.codex_gate({}, {}, False, []))
        self.assertIn("paused", o.codex_gate(on, {"codex": {"when_away": True}}, False, []))
        self.assertIsNone(o.codex_gate(on, {"codex": {"when_away": True}}, True, []))
        nothing, some = {"ended": 5, "landed": []}, {"ended": 5, "landed": ["x"]}
        self.assertIsNone(o.codex_gate(on, {}, False, [nothing]))
        self.assertIsNone(o.codex_gate(on, {}, False, [nothing, some]))
        self.assertIsNone(o.codex_gate(on, {}, False, [nothing, some, nothing]))
        self.assertIn("landed nothing", o.codex_gate(on, {}, False, [some, nothing, nothing]))
        # re-armed after those two: allowed again
        self.assertIsNone(o.codex_gate({"enabled": True, "armed": 6}, {}, False, [nothing, nothing]))

    def test_quota(self):
        ok = {"clear": True, "usage": {"primary": {"usedPercent": 89}, "secondary": {"usedPercent": 79}}}
        self.assertIsNone(o.codex_quota_block(ok))
        hi5 = {"clear": True, "usage": {"primary": {"usedPercent": 90}, "secondary": {"usedPercent": 1}}}
        hiw = {"clear": True, "usage": {"primary": {"usedPercent": 1}, "secondary": {"usedPercent": 80}}}
        self.assertIn("5-hour", o.codex_quota_block(hi5))
        self.assertIn("weekly", o.codex_quota_block(hiw))
        self.assertIn("BLOCKED", o.codex_quota_block({"clear": False, "blockers": ["busy"]}))
        self.assertIsNotNone(o.codex_quota_block(None))
        self.assertIn("unknown", o.codex_quota_block({"clear": True, "usage": None}))

    def test_usage_block_vs_unavailable(self):
        u = {"primary": {"usedPercent": 77}, "secondary": {"usedPercent": 10}}
        self.assertTrue(o.codex_usage_block({"clear": True, "usage": {
            "primary": {"usedPercent": 95}, "secondary": {"usedPercent": 1}}}))
        self.assertTrue(o.codex_usage_block({"clear": False, "usage": u,
                                             "blockers": ["5-hour window at 77% (block >= 75%)"]}))
        self.assertFalse(o.codex_usage_block(None))                       # bridge failed
        self.assertFalse(o.codex_usage_block({"clear": False, "usage": None,
                                              "blockers": ["usage unknown"]}))
        self.assertFalse(o.codex_usage_block({"clear": False, "usage": u, "blockers": [
            "5-hour window at 77%", "another Codex headless-task is running (pid 1)"]}))


class Transcript(unittest.TestCase):
    CWD = "C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile"

    def setUp(self):
        import tempfile
        self.root = Path(tempfile.mkdtemp(prefix="orch-tr-"))
        self.dir = self.root / "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile"
        self.dir.mkdir()

    def tearDown(self):
        for f in self.dir.iterdir():
            f.unlink()
        self.dir.rmdir()
        self.root.rmdir()

    def test_slug(self):
        self.assertEqual(o.project_slug(self.CWD), self.dir.name)

    def test_last_message(self):
        import json
        recs = [{"type": "user", "message": {"role": "user", "content": "go"}},
                {"type": "assistant", "message": {"content": [{"type": "text", "text": "Q: which?"}]}},
                {"type": "assistant", "isSidechain": True,
                 "message": {"content": [{"type": "text", "text": "subagent"}]}},
                {"type": "assistant", "message": {"content": [{"type": "tool_use", "name": "x"}]}},
                {"type": "custom-title", "customTitle": "t"}]
        p = self.dir / "abc-1.jsonl"
        p.write_text("\n".join(json.dumps(r) for r in recs) + "\n{garbled", encoding="utf-8")
        found = o.transcript_path("abc-1", self.CWD, self.root)
        self.assertEqual(found, p)
        self.assertEqual(o.last_assistant_text(found), "Q: which?")
        self.assertEqual(o.last_assistant_text(found, tail_bytes=20), "Q: which?")  # falls back to full read

    def test_missing_and_garbled(self):
        self.assertIsNone(o.transcript_path("nope", self.CWD, self.root))
        self.assertIsNone(o.transcript_path("../x", self.CWD, self.root))
        self.assertEqual(o.last_assistant_text(None), "")
        p = self.dir / "g.jsonl"
        p.write_bytes(b"\xff\xfe not json\n[1,2]\n")
        self.assertEqual(o.last_assistant_text(p), "")


if __name__ == "__main__":
    unittest.main(verbosity=1)
