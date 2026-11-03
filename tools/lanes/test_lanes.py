#!/usr/bin/env python3
"""Regression tests for tools/lanes/lanes.py on throwaway repos (never the real tree).

  python tools/lanes/test_lanes.py
"""
from __future__ import annotations

import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from contextlib import ExitStack, contextmanager, redirect_stderr, redirect_stdout
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lanes  # noqa: E402

CFG = {
    "worktree_root": "../wts",
    "dir_prefix": "ln-",
    "branch_prefix": "lanes/",
    "remote": "origin",
    "stale_hours": 6,
    "lock_wait_minutes": 0,
    "oracle_sha1": "",
    "oracle_exe": "build/bb2.exe",
    "runner": "local",
    "bootstrap": [],
    "shared_paths": ["shared.txt", "census.csv", "backlog.tsv"],
    "rowmerge": ["backlog.tsv"],
    "regenerate": [{"path": "census.csv", "cmd": "python gen_census.py", "subject": "docs: census refresh"}],
    "gates": {"naming": ["python gate.py"], "foundation": ["python gate.py"], "docs": []},
    "docs_only_paths": ["docs/"],
    "inert_paths": ["docs/", "memory/", ".claude/", "tools/trace/", "tools/lanes/"],
    "quick_gates": ["python -c \"print('quick-gate')\""],
}

GEN = "import pathlib\nn=sum(1 for _ in pathlib.Path('.').glob('src/*.c'))\n" \
      "pathlib.Path('census.csv').write_text(f'files,{n}\\n', newline='\\n')\n"
# fails on a committed FAIL marker anywhere outside docs/, or a .c including a missing header
GATE = ("import pathlib,sys\n"
        "bad = any(p.name == 'FAIL' and p.parts[0] != 'docs' for p in pathlib.Path('.').rglob('FAIL'))\n"
        "for c in pathlib.Path('src').glob('*.c'):\n"
        "    if 'h.h' in c.read_text() and not pathlib.Path('src/h.h').exists():\n"
        "        bad = True\n"
        "sys.exit(1 if bad else 0)\n")


def _rm_readonly(func, path, _exc):
    os.chmod(path, 0o700)  # git objects are read-only on Windows
    func(path)


def sh(*args, cwd):
    r = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True)
    if r.returncode:
        raise AssertionError(f"git {' '.join(args)}: {r.stderr}")
    return r.stdout.strip()


class RegistryRetryTest(unittest.TestCase):
    """Registry retry contracts without disk contention or real backoff delays."""

    def context(self):
        from unittest.mock import Mock
        ctx = object.__new__(lanes.Ctx)
        ctx.reg_path = Mock()
        ctx.mutex = Mock()
        ctx._acquire_mutex = Mock()
        return ctx

    def test_update_retries_only_replace_with_mutex_held(self):
        from unittest.mock import Mock, patch
        ctx = self.context()
        ctx.load = Mock(return_value={"lanes": {}})
        change = Mock(side_effect=lambda reg: reg.update(saved=True) or "result")
        real_path = ctx.reg_path
        attempts = []

        def replace(src, dst):
            ctx._acquire_mutex.assert_called_once_with()
            ctx.mutex.unlink.assert_not_called()
            self.assertIs(src, real_path.with_suffix.return_value)
            self.assertIs(dst, real_path)
            attempts.append(1)
            if len(attempts) <= 3:
                raise PermissionError("reader holds registry")

        with patch.object(lanes.os, "replace", side_effect=replace), \
                patch.object(lanes.time, "sleep") as sleep:
            self.assertEqual(ctx.update(change), "result")
        self.assertEqual(len(attempts), 4)
        self.assertEqual(sleep.call_count, 3)
        ctx.load.assert_called_once_with()
        change.assert_called_once()
        real_path.with_suffix.return_value.write_text.assert_called_once()
        self.assertTrue(json.loads(real_path.with_suffix.return_value.write_text.call_args.args[0])["saved"])
        ctx.mutex.unlink.assert_called_once_with()

    def test_update_replace_budget_raises_and_releases_mutex(self):
        from unittest.mock import Mock, patch
        ctx = self.context()
        ctx.load = Mock(return_value={"lanes": {}})
        change = Mock()
        elapsed = [0.0]

        def sleep(delay):
            self.assertGreater(delay, 0)
            self.assertLessEqual(delay, 0.05)
            ctx.mutex.unlink.assert_not_called()
            elapsed[0] += delay

        with patch.object(lanes.os, "replace", side_effect=PermissionError("busy")) as replace, \
                patch.object(lanes.time, "monotonic", side_effect=lambda: elapsed[0]), \
                patch.object(lanes.time, "sleep", side_effect=sleep):
            with self.assertRaisesRegex(PermissionError, "busy"):
                ctx.update(change)
        self.assertAlmostEqual(elapsed[0], 5)
        self.assertGreater(replace.call_count, 1)
        ctx.load.assert_called_once_with()
        change.assert_called_once()
        ctx.reg_path.with_suffix.return_value.write_text.assert_called_once()
        ctx.mutex.unlink.assert_called_once_with()

    def test_update_does_not_retry_other_replace_errors(self):
        from unittest.mock import Mock, patch
        ctx = self.context()
        ctx.load = Mock(return_value={"lanes": {}})
        with patch.object(lanes.os, "replace", side_effect=OSError("disk full")) as replace, \
                patch.object(lanes.time, "sleep") as sleep:
            with self.assertRaisesRegex(OSError, "disk full"):
                ctx.update(Mock())
        replace.assert_called_once()
        sleep.assert_not_called()
        ctx.mutex.unlink.assert_called_once_with()

    def test_load_retries_permission_error_and_half_read(self):
        from unittest.mock import patch
        ctx = self.context()
        ctx.reg_path.read_text.side_effect = [PermissionError("busy"), '{"lanes":',
                                             '{"lanes": {"a": {}}, "saved": true}']
        with patch.object(lanes.time, "sleep") as sleep:
            self.assertEqual(ctx.load(), {"lanes": {"a": {}}, "saved": True})
        self.assertEqual(ctx.reg_path.read_text.call_count, 3)
        self.assertEqual(sleep.call_count, 2)

    def test_load_exhaustion_preserves_error(self):
        from unittest.mock import patch
        for error in (PermissionError("busy"), json.JSONDecodeError("half-read", "{", 1)):
            with self.subTest(error=type(error).__name__):
                ctx = self.context()
                ctx.reg_path.read_text.side_effect = error
                with patch.object(lanes.time, "sleep") as sleep:
                    with self.assertRaises(type(error)) as raised:
                        ctx.load()
                self.assertIs(raised.exception, error)
                self.assertEqual(ctx.reg_path.read_text.call_count, 5)
                self.assertEqual(sleep.call_count, 4)

    def test_load_missing_registry_still_returns_empty_lanes(self):
        from unittest.mock import patch
        ctx = self.context()
        ctx.reg_path.read_text.side_effect = FileNotFoundError()
        with patch.object(lanes.time, "sleep") as sleep:
            self.assertEqual(ctx.load(), {"lanes": {}})
        sleep.assert_not_called()


class HeartbeatRetryTest(unittest.TestCase):
    def test_heartbeat_survives_value_error_and_warns_once_per_streak(self):
        from types import SimpleNamespace
        from unittest.mock import Mock, patch
        ctx = SimpleNamespace(session="test")
        queue = lanes.GateQueue(ctx, "head")
        stored = dict(queue.ticket, beat=0)
        reg = {"gate_queue": [stored]}
        attempts, refreshed, errors = [], [], []

        def update(fn):
            attempts.append(1)
            if len(attempts) in (1, 2, 4, 5):
                raise ValueError("half-read registry")
            result = fn(reg)
            refreshed.append(stored["beat"])
            return result

        ctx.update = update
        queue._stop = Mock()
        queue._stop.wait.side_effect = [False] * 6 + [True]

        def heartbeat():
            try:
                queue._heartbeat()
            except Exception as error:
                errors.append(error)

        log = io.StringIO()
        with patch.object(lanes.time, "time", side_effect=[100, 200]), redirect_stderr(log):
            thread = threading.Thread(target=heartbeat)
            thread.start()
            thread.join(timeout=2)
        self.assertFalse(thread.is_alive())
        self.assertEqual(errors, [])
        self.assertEqual(len(attempts), 6)
        self.assertEqual(refreshed, [100, 200])
        self.assertEqual(queue.ticket["beat"], 200)
        self.assertEqual(log.getvalue().count("gate heartbeat failed for head"), 2)


class RegeneratePrefixTest(unittest.TestCase):
    def test_regenerate_matches_input_prefix_case_insensitively(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        entry = {"path": "docs/runtime/output.tsv", "inputs": ["Docs/Runtime/"],
                 "cmd": "regenerate-runtime"}
        cases = [("dOCS/rUNTIME/capture.jsonl", True), ("docs/runtime-other/x", False),
                 ("DOCS/RUNTIME/OUTPUT.TSV", True)]
        for path, expected in cases:
            with self.subTest(path=path):
                ctx = SimpleNamespace(cfg={"regenerate": [entry]}, cand_paths=[path])
                with patch.object(lanes, "run_shell", return_value=0) as run, \
                        patch.object(lanes, "git"), patch.object(lanes, "git_ok", return_value=True), \
                        patch.object(lanes, "matches_input_prefix",
                                     wraps=lanes.matches_input_prefix) as matches:
                    self.assertEqual(lanes.regenerate(ctx, "a", Path("candidate"),
                                                      io.StringIO(), docs_only=True), [])
                self.assertEqual(run.call_count, int(expected))
                if expected:
                    matches.assert_called_once_with(path, entry["inputs"])


class WslRunnerEnvTest(unittest.TestCase):
    """The WSL runner forwards the capture-store variables into run.sh (wsl bash drops them)."""

    def run_wsl(self, env):
        from types import SimpleNamespace
        from unittest.mock import patch
        with tempfile.TemporaryDirectory() as d:
            wt = Path(d)
            done = SimpleNamespace(returncode=0, stdout="", stderr="")
            with patch.dict(os.environ, env, clear=False), \
                    patch.object(lanes.subprocess, "run", return_value=done) as run, \
                    redirect_stdout(io.StringIO()):
                for name in lanes.WSL_FORWARD_ENV:
                    if name not in env:
                        os.environ.pop(name, None)
                rc = lanes.run_shell(SimpleNamespace(cfg={"runner": "wsl"}), wt, "true",
                                     io.StringIO())
            self.assertEqual(rc, 0)
            self.assertEqual(run.call_args.args[0][:2], ["wsl", "bash"])
            return (wt / "tmp" / "lanes" / "run.sh").read_bytes().decode("utf-8")

    def test_store_variables_are_exported_quoted_before_the_command(self):
        text = self.run_wsl({"BB2_CAPTURE_STORE": r"C:\Users\Some One\bb2 cap's",
                             "BB2_CAPTURE_STORE_WRITE": "0"})
        lines = text.split("\n")
        self.assertEqual(lines[0], "set -e")
        self.assertEqual(lines[1], "export BB2_CAPTURE_STORE='C:\\Users\\Some One\\bb2 cap'\"'\"'s'")
        self.assertEqual(lines[2], "export BB2_CAPTURE_STORE_WRITE=0")
        self.assertTrue(lines[3].startswith("cd '"))
        self.assertEqual(lines[-2], "true")
        self.assertNotIn("\r", text)
        import shlex
        self.assertEqual(shlex.split(lines[1]),
                         ["export", r"BB2_CAPTURE_STORE=C:\Users\Some One\bb2 cap's"])

    def test_unset_or_empty_variables_are_not_exported(self):
        self.assertNotIn("export", self.run_wsl({}))
        self.assertNotIn("export", self.run_wsl({"BB2_CAPTURE_STORE": ""}))


from unittest.mock import patch  # noqa: E402


class GateMirrorTest(unittest.TestCase):
    """lanes-gate-mirror: gates in a WSL clone, the Windows repo only read (bar one temp ref)."""
    GATES = ["python3 -m engine.cli verify-oracle --rebuild", "python3 -m engine.cli test"]

    def ctx(self, d, **cfg):
        from types import SimpleNamespace
        base = {"runner": "wsl", "oracle_exe": "build/bb2.exe",
                "gate_mirror": {"path": "bb2-gate", "copy": ["tools/gcc-2.7.2", "../etc", "disc"]}}
        base.update(cfg)
        return SimpleNamespace(cfg=base, main=Path(d) / "main", state_dir=Path(d) / "state")

    def test_mirror_only_for_the_wsl_runner_and_not_when_disabled(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertIsNotNone(lanes.mirror_cfg(self.ctx(d)))
            self.assertIsNone(lanes.mirror_cfg(self.ctx(d, runner="local")))
            self.assertIsNone(lanes.mirror_cfg(self.ctx(d, gate_mirror=None)))
            with patch.dict(os.environ, {"BB2_LANES_NO_MIRROR": "1"}):
                self.assertIsNone(lanes.mirror_cfg(self.ctx(d)))

    def test_scripts_stay_under_home_and_delete_only_a_marked_clone(self):
        with tempfile.TemporaryDirectory() as d:
            c = self.ctx(d)
            s = lanes.mirror_create_script(c, c.cfg["gate_mirror"], reset=True)
            self.assertIn('M="$HOME/bb2-gate"', s)
            self.assertIn("flock 9", s)
            self.assertIn(f'[ -f "$M/{lanes.MIRROR_OWNED}" ] ||', s)
            self.assertLess(s.index(lanes.MIRROR_OWNED), s.index('rm -rf -- "$M"'))
            # OWNED right after the clone, DONE last: a half-made clone can be reset
            self.assertLess(s.index("clone -q"), s.index(f'date > "$M/{lanes.MIRROR_OWNED}"'))
            self.assertLess(s.index("pip install"), s.index(f'date > "$M/{lanes.MIRROR_DONE}"'))
            self.assertIn("set-url --push origin no-push", s)
            self.assertIn("tools/gcc-2.7.2", s)
            self.assertNotIn("../etc", s)  # a copy entry leaving the repo is ignored
            # without --reset only an incomplete clone of ours is removed
            self.assertIn(f'[ -f "$M/{lanes.MIRROR_DONE}" ] || {{',
                          lanes.mirror_create_script(c, c.cfg["gate_mirror"], False))
            for bad in ("", "..", "a/b", "$(x)", "~"):
                with self.assertRaises(lanes.Refuse):
                    lanes.mirror_create_script(c, {"path": bad}, reset=True)
            g = lanes.mirror_gate_script(c, c.cfg["gate_mirror"], "refs/lanes-gate/x", "a" * 40,
                                         self.GATES, "build/bb2.exe", "x")
            self.assertIn('O="$M.out/x"', g)
            self.assertIn("cp build/bb2.map", g)
            with self.assertRaises(lanes.Refuse):
                lanes.mirror_gate_script(c, c.cfg["gate_mirror"], "r", "a" * 40, [], "b", "../x")
            self.assertIn('[ "$(git rev-parse HEAD)" = "' + "a" * 40 + '" ]', g)
            self.assertIn("git checkout -q -f --detach " + "a" * 40, g)
            self.assertLess(g.index("( python3 -m engine.cli verify-oracle --rebuild )"),
                            g.index("( python3 -m engine.cli test )"))
            self.assertNotIn("\r", g)

    def run_mirror(self, out, rc=0):
        from types import SimpleNamespace
        with tempfile.TemporaryDirectory() as d:
            c = self.ctx(d)
            calls = []

            def fake_git(*a, cwd, check=True):
                calls.append(a)
                return "c" * 40 if a[0] == "rev-parse" else ""
            log = io.StringIO()
            with patch.object(lanes, "git", fake_git), \
                    patch.object(lanes, "copy_mirror_artifacts", lambda out, ld: None), \
                    patch.object(lanes, "run_wsl_script",
                                 return_value=SimpleNamespace(returncode=rc, stdout=out, stderr="")), \
                    redirect_stdout(io.StringIO()):
                try:
                    return lanes.run_gates_mirror(c, "x", Path(d), self.GATES, log), c, calls, log
                except lanes.Refuse as e:
                    return e, c, calls, log

    def test_pass_records_the_gated_sha_and_drops_the_temp_ref(self):
        out = ("@@gate 0 start\n@@gate 0 rc=0 secs=40\n@@gate 1 start\n@@gate 1 rc=0 secs=90\n"
               "@@sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa\n")
        got, c, calls, log = self.run_mirror(out)
        self.assertEqual(got, "62efab4f73f992798c43e8c730aa43baa10bb4fa")
        self.assertEqual(c.mirror_gated, "c" * 40)
        self.assertIn(("update-ref", "refs/lanes-gate/x", "c" * 40), calls)
        self.assertEqual(calls[-1], ("update-ref", "-d", "refs/lanes-gate/x"))
        self.assertIn(f"[gate {self.GATES[1]}: 90s]", log.getvalue())

    def test_artifacts_are_copied_back_into_the_landing_build(self):
        with tempfile.TemporaryDirectory() as d:
            staged = Path(d) / "out"
            (staged / "build").mkdir(parents=True)
            (staged / "build" / "bb2.map").write_text("fresh", encoding="utf-8")
            ld = Path(d) / "ld"
            (ld / "build").mkdir(parents=True)
            (ld / "build" / "bb2.map").write_text("stale", encoding="utf-8")
            lanes.copy_mirror_artifacts(f"x\n@@out {staged}\n", ld)
            self.assertEqual((ld / "build" / "bb2.map").read_text(encoding="utf-8"), "fresh")
            with self.assertRaises(lanes.Refuse):
                lanes.copy_mirror_artifacts("no out line", ld)

    def test_tip_check_accepts_an_amended_regenerate_and_nothing_else(self):
        from types import SimpleNamespace
        with tempfile.TemporaryDirectory() as d:
            r = Path(d)
            g = lambda *a: subprocess.run(["git", *a], cwd=r, check=True, capture_output=True,
                                          text=True).stdout.strip()
            g("init", "-q")
            g("config", "user.email", "t@t"); g("config", "user.name", "t")
            (r / "a.c").write_text("1", encoding="utf-8")
            g("add", "-A"); g("commit", "-qm", "base")
            (r / "a.c").write_text("2", encoding="utf-8")
            g("commit", "-qam", "lane")
            gated = g("rev-parse", "HEAD")
            (r / "census.csv").write_text("x", encoding="utf-8")  # regenerate amends
            g("add", "-A"); g("commit", "-q", "--amend", "--no-edit")
            c = SimpleNamespace(cfg={"regenerate": [{"path": "census.csv"}]})
            self.assertEqual(lanes.mirror_tip_extra(c, gated, "HEAD", r), [])
            (r / "a.c").write_text("3", encoding="utf-8")  # anything else is refused
            g("commit", "-qam", "late")
            self.assertEqual(lanes.mirror_tip_extra(c, gated, "HEAD", r), ["a.c"])

    def test_failed_gate_missing_gate_or_mirror_error_refuse_4(self):
        e, c, calls, _ = self.run_mirror("@@gate 0 rc=0 secs=1\n@@gate 1 rc=2 secs=5\n", rc=100)
        self.assertIsInstance(e, lanes.Refuse)
        self.assertEqual(e.code, 4)
        self.assertIn(self.GATES[1], str(e))
        self.assertFalse(getattr(c, "mirror_gated", None))
        self.assertEqual(calls[-1], ("update-ref", "-d", "refs/lanes-gate/x"))
        e, *_ = self.run_mirror("lanes-mirror: HEAD is not ...\n", rc=92)
        self.assertEqual(e.code, 4)
        self.assertIn("gate-mirror --reset", str(e))
        e, *_ = self.run_mirror("@@gate 0 rc=0 secs=1\n@@sha1 missing\n", rc=0)  # one gate lost
        self.assertEqual(e.code, 4)


class MirrorPostTest(unittest.TestCase):
    """lanes-regen-mirror: regenerate and the input / tip checks run in the gate mirror; the
    landing worktree only applies the regenerated files and must reach the mirror's trees."""
    REGEN = "python -c \"open('census.csv','w',newline='').write(open('a.c').read()*2)\""

    def repo(self, d):
        r = Path(d) / "origin"
        r.mkdir()
        g = lambda *a, cwd=r: subprocess.run(["git", *a], cwd=cwd, check=True,
                                             capture_output=True, text=True).stdout.strip()
        g("init", "-q")
        g("config", "core.autocrlf", "false")
        g("config", "user.email", "t@t"); g("config", "user.name", "t")
        (r / "a.c").write_text("1", encoding="utf-8")
        (r / "census.csv").write_text("11", encoding="utf-8")
        g("add", "-A"); g("commit", "-qm", "base")
        base = g("rev-parse", "HEAD")
        return r, g, base

    def clone(self, d, r, g, name):
        c = Path(d) / name
        g("clone", "-q", str(r), str(c), cwd=Path(d))
        g("config", "user.email", "t@t", cwd=c); g("config", "user.name", "t", cwd=c)
        g("config", "core.autocrlf", "false", cwd=c)
        return c

    def mirror(self, m, plan, out):
        old = os.getcwd()
        os.chdir(m)
        try:
            with redirect_stdout(io.StringIO()):
                rc = sys.modules["mirror_post"].main(["x", str(plan), str(out)])
        finally:
            os.chdir(old)
        return rc, json.loads((out / "post.json").read_text(encoding="utf-8"))

    def setUp(self):
        import importlib
        importlib.import_module("mirror_post")

    def land(self, d, edit, checks=None, tip_checks=None):
        """Lane commit `edit(repo)` on base, run mirror_post in a 'mirror' clone and
        apply_mirror_post in an 'ld' clone; returns (rc, result, ld trees or Refuse, ...)."""
        from types import SimpleNamespace
        r, g, base = self.repo(d)
        edit(r)
        g("add", "-A"); g("commit", "-qm", "lane")
        gated = g("rev-parse", "HEAD")
        m, ld = self.clone(d, r, g, "mirror"), self.clone(d, r, g, "ld")
        plan = Path(d) / "plan.json"
        plan.write_text(json.dumps({
            "base": base, "gated": gated, "cand_paths": ["a.c"],
            "regenerate": [{"path": "census.csv", "cmd": self.REGEN}],
            "input_checks": checks or [], "tip_checks": tip_checks or []}), encoding="utf-8")
        out = Path(d) / "post"
        rc, res = self.mirror(m, plan, out)
        res["patch"] = (out / "regen.patch").read_bytes() if res.get("changed") else b""
        ctx = SimpleNamespace(cfg={"regenerate": [{"path": "census.csv", "subject": "s"}]},
                              state_dir=Path(d) / "state", mirror_post=res,
                              replayed={"lanesha": gated})
        try:
            with redirect_stdout(io.StringIO()):
                lanes.apply_mirror_post(ctx, "x", ld, base, io.StringIO())
            got = g("rev-parse", "HEAD^{tree}", cwd=ld)
        except lanes.Refuse as e:
            got = e
        return rc, res, got, g, m, ld, base

    def test_amend_lands_the_mirror_tree(self):
        with tempfile.TemporaryDirectory() as d:
            rc, res, got, g, m, ld, base = self.land(
                d, lambda r: (r / "a.c").write_text("2", encoding="utf-8"))
            self.assertEqual((rc, res["how"], res["changed"]), (0, "amended", ["census.csv"]))
            self.assertEqual(got, g("rev-parse", "HEAD^{tree}", cwd=m))
            self.assertEqual(res["trees"], [got])
            self.assertEqual((ld / "census.csv").read_text(encoding="utf-8"), "22")
            self.assertEqual(g("status", "--porcelain", cwd=ld), "")
            self.assertEqual(g("log", "-1", "--format=%s", cwd=ld), "lane")  # folded, not added

    def test_drop_when_regeneration_undoes_the_only_change(self):
        with tempfile.TemporaryDirectory() as d:
            rc, res, got, g, m, ld, base = self.land(
                d, lambda r: (r / "census.csv").write_text("stale", encoding="utf-8"))
            self.assertEqual((rc, res["how"], res["trees"]), (0, "dropped", []))
            self.assertEqual(g("rev-parse", "HEAD", cwd=ld), base)

    def test_checks_run_on_the_folded_commits_and_failures_name_them(self):
        ok = [{"cmd": "python -c \"import sys; sys.exit(0)\"", "inputs": ["census"]}]
        bad = [{"cmd": "python -c \"import sys; sys.exit(3)\"", "inputs": ["CENSUS"]}]
        edit = lambda r: (r / "a.c").write_text("2", encoding="utf-8")
        with tempfile.TemporaryDirectory() as d:
            rc, res, got, *_ = self.land(d, edit, checks=ok, tip_checks=ok)
            self.assertEqual(rc, 0)
        with tempfile.TemporaryDirectory() as d:  # census.csv only changed by the regenerate
            rc, res, *_ = self.land(d, edit, checks=bad)
            self.assertEqual((rc, res["failed"]["kind"]), (101, "input"))
            self.assertIn("--rev ", res["failed"]["cmd"])
            self.assertIn("input check failed",
                          lanes.mirror_post_refusal(res["failed"], "l.log"))
        with tempfile.TemporaryDirectory() as d:
            rc, res, *_ = self.land(d, edit, tip_checks=bad)
            self.assertEqual((rc, res["failed"]["kind"]), (101, "tip"))

    def test_a_command_shared_by_two_outputs_runs_once(self):
        with tempfile.TemporaryDirectory() as d:
            r, g, base = self.repo(d)
            cmd = "python -c \"open('runs.txt','a').write('x')\""
            plan = Path(d) / "plan.json"
            plan.write_text(json.dumps({"base": base, "gated": base, "regenerate": [
                {"path": "a.c", "cmd": cmd}, {"path": "census.csv", "cmd": cmd}]}),
                encoding="utf-8")
            rc, res = self.mirror(r, plan, Path(d) / "post")
            self.assertEqual((rc, (r / "runs.txt").read_text(encoding="utf-8")), (0, "x"))

    def test_a_tree_the_mirror_did_not_reach_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            from types import SimpleNamespace
            r, g, base = self.repo(d)
            (r / "a.c").write_text("2", encoding="utf-8")
            g("commit", "-qam", "lane")
            ctx = SimpleNamespace(cfg={"regenerate": []}, state_dir=Path(d) / "s",
                                  replayed={}, mirror_post={"changed": [], "trees": ["0" * 40]})
            with self.assertRaisesRegex(lanes.Refuse, "differ from the mirror"):
                lanes.apply_mirror_post(ctx, "x", r, base, io.StringIO())

    def test_gate_script_runs_post_after_gates_and_stages_no_stale_build(self):
        with tempfile.TemporaryDirectory() as d:
            c = GateMirrorTest().ctx(d)
            s = lanes.mirror_gate_script(c, c.cfg["gate_mirror"], "r", "a" * 40,
                                         ["python3 -m engine.cli test"], "build/bb2.exe", "x",
                                         post=("/p/post.py", "/p/plan.json"))
            self.assertLess(s.index("( python3 -m engine.cli test )"), s.index("/p/post.py"))
            self.assertIn('P="$M.post/x"', s)
            self.assertIn("exit 101", s)
            self.assertIn("BB2_CAPTURE_STORE=", s)
            self.assertIn("cp build/bb2.map", s)
            s = lanes.mirror_gate_script(c, c.cfg["gate_mirror"], "r", "a" * 40, [],
                                         "build/bb2.exe", "x", post=("/p/post.py", "/p/plan"))
            self.assertNotIn("cp build/", s)  # no gate ran: the mirror's build/ is stale

    def test_run_gates_mirror_maps_a_post_refusal(self):
        from types import SimpleNamespace
        with tempfile.TemporaryDirectory() as d:
            c = GateMirrorTest().ctx(d)
            pd = Path(d) / "post"
            pd.mkdir()
            (pd / "post.json").write_text(json.dumps(
                {"changed": [], "trees": [], "failed": {"kind": "regenerate", "cmd": "ev"}}),
                encoding="utf-8")
            out = f"@@post regenerate 0 rc=1 secs=3\n@@post-out {pd}\n"
            with patch.object(lanes, "git", lambda *a, cwd, check=True: "c" * 40), \
                    patch.object(lanes, "run_wsl_script",
                                 return_value=SimpleNamespace(returncode=101, stdout=out,
                                                              stderr="")), \
                    redirect_stdout(io.StringIO()):
                with self.assertRaisesRegex(lanes.Refuse, "regenerate failed: ev"):
                    lanes.run_gates_mirror(c, "x", Path(d), [], io.StringIO(),
                                           post={"base": "b"})
            plan = json.loads((c.state_dir / "gate-mirror" / "post-x.json").read_text("utf-8"))
            self.assertEqual(plan["gated"], "c" * 40)
            self.assertEqual((c.state_dir / "gate-mirror" / "post-x.py").read_bytes(),
                             lanes.MIRROR_POST.read_bytes())

    def test_docs_land_uses_the_mirror_only_with_post_work(self):
        from types import SimpleNamespace
        for regen, want in (([{"path": "ev.tsv", "cmd": "ev", "inputs": ["docs/rt/"]}], 1),
                            ([], 0)):
            with self.subTest(regen=bool(regen)):
                ctx = SimpleNamespace(
                    cfg=dict(CFG, runner="wsl", gate_mirror={"path": "bb2-gate"},
                             regenerate=regen, input_checks=[], tip_checks=[]),
                    load=lambda: {"lanes": {"a": {"kind": "foundation"}}})

                def git(*args, **kwargs):
                    return "docs/rt/cap.json" if args[0] == "diff" else ("" if args[0] in (
                        "status", "rev-list") else "tip")

                def gates_mirror(c, lane, ld, gates, log, post=None):
                    self.assertEqual(gates, [])
                    self.assertEqual([g["cmd"] for g in post["regenerate"]], ["ev"])
                    c.mirror_gated = "tip"
                with patch.object(lanes, "run"), patch.object(lanes, "git", side_effect=git), \
                        patch.object(lanes, "replay_commit", return_value="picked"), \
                        patch.object(lanes, "check_untracked"), \
                        patch.object(lanes, "run_gates_mirror", side_effect=gates_mirror) as gm, \
                        patch.object(lanes, "apply_mirror_post") as ap, \
                        patch.object(lanes, "mirror_tip_extra", return_value=[]), \
                        patch.object(lanes, "regenerate", return_value=[]) as rg, \
                        patch.object(lanes, "input_checks") as ic, \
                        patch.object(lanes, "tip_checks") as tc, \
                        redirect_stdout(io.StringIO()):
                    lanes.build_candidate(ctx, "a", Path("ld"), ["c1"], "base", io.StringIO())
                self.assertEqual((gm.call_count, ap.call_count), (want, want))
                self.assertEqual((rg.call_count, ic.call_count, tc.call_count),
                                 (1 - want,) * 3)


class InputCheckTest(unittest.TestCase):
    """Dispatch tests independent of shell transport and lane fixture setup."""

    def context(self):
        from types import SimpleNamespace
        return SimpleNamespace(cfg={"input_checks": [{"cmd": "python3 store.py check-commit",
                                "inputs": ["docs/naming/runtime/"]}]})

    def test_every_matching_replayed_tree_is_checked_in_order(self):
        from unittest.mock import patch
        calls = []
        def git(*args, **kwargs):
            calls.append(args)
            if args[0] == "rev-list":
                return "a" * 40 + "\n" + "b" * 40 + "\n" + "c" * 40
            return "docs/naming/runtime/capture.manifest.json" if args[-1] != "b" * 40 else "src/a.c"
        with patch.object(lanes, "git", side_effect=git), \
                patch.object(lanes, "run_shell", return_value=0) as run:
            lanes.input_checks(self.context(), Path("candidate"), "base", io.StringIO())
        self.assertEqual(calls[0], ("rev-list", "--reverse", "base..HEAD"))
        self.assertEqual([call.args[2] for call in run.call_args_list], [
            "python3 store.py check-commit --rev " + "a" * 40,
            "python3 store.py check-commit --rev " + "c" * 40])

    def test_refusal_stops_before_later_commits(self):
        from unittest.mock import patch
        with patch.object(lanes, "git", side_effect=["a\nb", "docs/naming/runtime/capture.jsonl"]), \
                patch.object(lanes, "run_shell", return_value=1) as run:
            log = io.StringIO()
            log.name = "capture-check.log"
            with self.assertRaisesRegex(lanes.Refuse, "input check failed: .* --rev a"):
                lanes.input_checks(self.context(), Path("candidate"), "base", log)
        self.assertEqual(run.call_count, 1)

    def test_nul_paths_with_unicode_special_characters_and_case_variants(self):
        from unittest.mock import patch
        names = "docs/naming/runtime/é/shot\nname.png\0Docs/Naming/Runtime/other.png\0"
        with patch.object(lanes, "git", side_effect=["a", names]) as git, \
                patch.object(lanes, "run_shell", return_value=0) as run:
            lanes.input_checks(self.context(), Path("candidate"), "base", io.StringIO())
        self.assertIn("-z", git.call_args_list[1].args)
        self.assertEqual(run.call_count, 1)
        # A case-variant spelling alone also triggers the input gate.
        with patch.object(lanes, "git", side_effect=["b", "Docs/naming/runtime/é.png\0"]), \
                patch.object(lanes, "run_shell", return_value=0) as run:
            lanes.input_checks(self.context(), Path("candidate"), "base", io.StringIO())
        self.assertEqual(run.call_count, 1)

    def test_no_checks_does_not_walk_history(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        with patch.object(lanes, "git") as git:
            lanes.input_checks(SimpleNamespace(cfg={}), Path("candidate"), "base", io.StringIO())
        git.assert_not_called()


class TipCheckTest(unittest.TestCase):
    def candidate(self, paths, commits=("first", "second"), quick=False, fail=False,
                  dirty=False, inputs=None):
        """Exercise build_candidate dispatch even where Git fixture creation is denied."""
        from types import SimpleNamespace
        from unittest.mock import patch
        ctx = SimpleNamespace(cfg=dict(CFG, regenerate=[], tip_checks=[{
            "cmd": "check-tip", "inputs": inputs if inputs is not None else ["docs/runtime/"]}]),
            load=lambda: {"lanes": {"a": {"kind": "foundation"}}})
        state = {"fresh": False, "folded": False, "input_checked": False}
        calls = []
        log = io.StringIO()
        log.name = "tip.log"

        def git(*args, **kwargs):
            if args[0] == "diff":
                return "\n".join(paths)
            if args[0] == "status":
                return " M tracked.txt" if dirty and state["input_checked"] else ""
            return "folded-tip" if state["folded"] else "tip"

        def regen(*args, **kwargs):
            state.update(fresh=True, folded=True)
            return ["docs/evidence/fresh.tsv"]

        def check_inputs(*args):
            state["input_checked"] = True

        def shell(_ctx, _ld, cmd, _log):
            if cmd == "check-tip":
                self.assertTrue(state["fresh"], "tip check ran before regeneration")
                self.assertTrue(state["folded"], "tip check ran before the fold")
                self.assertTrue(state["input_checked"], "tip check ran before input_checks")
                calls.append(cmd)
                return 1 if fail else 0
            return 0

        with patch.object(lanes, "run"), patch.object(lanes, "git", side_effect=git), \
                patch.object(lanes, "replay_commit", return_value="picked") as replay, \
                patch.object(lanes, "check_untracked"), \
                patch.object(lanes, "regenerate", side_effect=regen), \
                patch.object(lanes, "input_checks", side_effect=check_inputs), \
                patch.object(lanes, "run_shell", side_effect=shell), redirect_stdout(io.StringIO()):
            tip = lanes.build_candidate(ctx, "a", Path("candidate"), commits, "base", log,
                                        quick_paths=paths if quick else None)
        return ctx, tip, calls, replay.call_count

    def test_candidate_tip_runs_once_after_fold_and_input_checks_on_docs_land(self):
        ctx, tip, calls, replays = self.candidate(["docs/runtime/capture.txt"])
        self.assertEqual(ctx.gated, "docs")
        self.assertEqual(tip, "folded-tip")
        self.assertEqual(replays, 2)
        self.assertEqual(calls, ["check-tip"])

    def test_candidate_tip_runs_on_quick_land(self):
        ctx, _, calls, _ = self.candidate(["src/a.c"], quick=True, inputs=["src/"])
        self.assertEqual(ctx.gated, "quick")
        self.assertEqual(calls, ["check-tip"])

    def test_candidate_tip_skips_unrelated_paths(self):
        for paths in (["docs/note.txt"], ["src/a.c"]):
            with self.subTest(paths=paths):
                self.assertEqual(self.candidate(paths)[2], [])

    def test_candidate_tip_uses_regenerated_paths(self):
        self.assertEqual(self.candidate(["src/a.c"], inputs=["docs/evidence/"])[2],
                         ["check-tip"])

    def test_candidate_tip_failure_names_command_and_log(self):
        with self.assertRaisesRegex(lanes.Refuse, r"tip check failed: check-tip  \(log tip.log\)") as e:
            self.candidate(["docs/runtime/capture.txt"], fail=True)
        self.assertEqual(e.exception.code, 4)

    def test_candidate_tip_preserves_tracked_change_guard(self):
        with self.assertRaisesRegex(lanes.Refuse, "gates left tracked changes") as e:
            self.candidate(["docs/runtime/capture.txt"], dirty=True)
        self.assertEqual(e.exception.code, 4)

    def test_notes_configuration_covers_regeneration_inputs_and_evidence(self):
        cfg = json.loads(Path(__file__).with_name("lanes.json").read_text(encoding="utf-8"))
        inputs = {p for g in cfg["regenerate"] if "notes.py build" in g["cmd"]
                  for p in g["inputs"]} | {"docs/naming/runtime/evidence/"}
        checks = [c for c in cfg["tip_checks"]
                  if c["cmd"] == "python3 tools/trace/scan/notes.py check"]
        self.assertEqual(len(checks), 1)
        self.assertEqual(set(checks[0]["inputs"]), inputs)

    def test_case_insensitive_scoping_includes_regenerated_outputs(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        ctx = SimpleNamespace(cand_paths={"src/a.c"}, cfg={"tip_checks": [
            {"cmd": "check", "inputs": ["docs/evidence/"]}]})
        with patch.object(lanes, "run_shell", return_value=0) as run:
            lanes.tip_checks(ctx, Path("candidate"), io.StringIO(), ["Docs/Evidence/a.tsv"])
        self.assertEqual(run.call_count, 1)


class GateQueueTest(unittest.TestCase):
    """Queue coverage also runs where the sandbox denies Git fixture creation."""
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="gate-queue-")
        self.addCleanup(self.tmp.cleanup)
        self.ctx = object.__new__(lanes.Ctx)
        self.ctx.session = "test"
        self.ctx.state_dir = Path(self.tmp.name)
        self.ctx.main = self.ctx.state_dir
        self.ctx.reg_path = self.ctx.state_dir / "registry.json"
        self.ctx.mutex = self.ctx.state_dir / ".registry.mutex"
        self.ctx.cfg = CFG
        self.ctx.wt_root = self.ctx.state_dir / "wts"

    @contextmanager
    def land_fixture(self, lock=None):
        """Exercise cmd_land with the temp registry, without Git or gate subprocesses."""
        from types import SimpleNamespace
        from unittest.mock import patch
        self.ctx.replayed = []
        with ExitStack() as stack:
            for name, value in {
                "require_owner": None, "recover_lane": None, "git": "commit",
                "unlanded": ["commit"], "paths_of": ["src/a.c"], "check_claims": None,
                "read_lock": lock, "ensure_land_tree": self.ctx.state_dir,
                "cheap_recheck_paths": None, "build_candidate": "commit",
                "mark_landed": None, "advance_lane": None, "run": None,
            }.items():
                stack.enter_context(patch.object(lanes, name, return_value=value))
            yield SimpleNamespace(lane="a", no_push=True, no_wait=True)

    def test_reenqueue_preserves_unique_ticket_and_peer_positions(self):
        first = lanes.GateQueue(self.ctx, "first")
        mine = lanes.GateQueue(self.ctx, "mine")
        last = lanes.GateQueue(self.ctx, "last")
        stale = lanes.GateQueue(self.ctx, "stale")
        self.addCleanup(mine.close)
        original = [dict(q.ticket) for q in (first, mine, last)]
        stale.ticket["beat"] -= lanes.HEARTBEAT_STALE_SECS + 1
        self.ctx.update(lambda reg: reg.update(gate_queue=original + [stale.ticket]))
        with redirect_stdout(io.StringIO()):
            self.assertEqual(mine._poll(), 2)  # stale peer forces _enqueue_and_prune again
        self.assertEqual(self.ctx.load()["gate_queue"], original)
        self.assertEqual(self.ctx.update(mine._enqueue_and_prune), 2)
        self.assertEqual(self.ctx.load()["gate_queue"], original)

    def test_land_closes_queue_last_even_when_cleanup_raises(self):
        from unittest.mock import Mock, patch
        for failure in (None, "abort", "unlink", "log.close", "all"):
            with self.subTest(failure=failure), self.land_fixture() as args:
                events = []
                error = RuntimeError("cleanup failed")
                real_close = lanes.GateQueue.close
                pidfile = lanes.land_pidfile(self.ctx, "a")
                real_unlink = Path.unlink

                def cleanup(name):
                    self.assertEqual(len(self.ctx.load()["gate_queue"]), 1)
                    events.append(name)
                    if name == failure or failure == "all":
                        raise error if name == "abort" or failure != "all" else ValueError(name)

                def unlink(path, *a, **kw):
                    if path == pidfile:
                        cleanup("unlink")
                    return real_unlink(path, *a, **kw)

                def close(queue):
                    events.append("queue.close")
                    real_close(queue)

                log = Mock()
                log.close.side_effect = lambda: cleanup("log.close")
                with patch.object(lanes, "open", return_value=log), \
                        patch.object(lanes, "run", side_effect=lambda *a, **kw: cleanup("abort")), \
                        patch.object(Path, "unlink", unlink), \
                        patch.object(lanes.GateQueue, "close", close), redirect_stdout(io.StringIO()):
                    if failure:
                        with self.assertRaises(RuntimeError) as caught:
                            lanes.cmd_land(self.ctx, args)
                        self.assertIs(caught.exception, error)
                    else:
                        self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                self.assertEqual(events, ["abort", "unlink", "log.close", "queue.close"])
                self.assertEqual(self.ctx.load()["gate_queue"], [])
                if failure not in ("unlink", "all"):
                    self.assertFalse(pidfile.exists())

    def test_land_body_exception_propagates_and_removes_ticket(self):
        from unittest.mock import patch
        error = RuntimeError("gates failed")
        for abort_error in (None, ValueError("abort failed")):
            with self.subTest(abort_error=abort_error), self.land_fixture() as args, \
                    patch.object(lanes, "build_candidate", side_effect=error), \
                    patch.object(lanes, "run", side_effect=abort_error), \
                    redirect_stdout(io.StringIO()):
                with self.assertRaises(RuntimeError) as caught:
                    lanes.cmd_land(self.ctx, args)
            self.assertIs(caught.exception, error)
            self.assertEqual(self.ctx.load()["gate_queue"], [])
            self.assertFalse(lanes.land_pidfile(self.ctx, "a").exists())

    def test_successful_land_in_callers_except_raises_cleanup_error(self):
        from unittest.mock import patch
        error = ValueError("abort failed")
        with self.land_fixture() as args, \
                patch.object(lanes, "run", side_effect=error), redirect_stdout(io.StringIO()):
            try:
                raise RuntimeError("caller's exception")
            except RuntimeError:
                with self.assertRaises(ValueError) as caught:
                    lanes.cmd_land(self.ctx, args)
        self.assertIs(caught.exception, error)
        self.assertEqual(self.ctx.load()["gate_queue"], [])

    def test_cleanup_interrupt_outranks_failed_body_and_ordinary_cleanup_error(self):
        from unittest.mock import Mock, patch
        for interrupt_type in (KeyboardInterrupt, SystemExit):
            with self.subTest(interrupt=interrupt_type), self.land_fixture() as args:
                error = interrupt_type("cleanup interrupted")
                log = Mock()
                log.close.side_effect = error
                real_close = lanes.GateQueue.close
                with patch.object(lanes, "build_candidate", side_effect=RuntimeError("gates")), \
                        patch.object(lanes, "run", side_effect=ValueError("abort failed")), \
                        patch.object(lanes, "open", return_value=log), \
                        patch.object(lanes.GateQueue, "close", autospec=True,
                                     side_effect=real_close) as close, \
                        redirect_stdout(io.StringIO()):
                    with self.assertRaises(interrupt_type) as caught:
                        lanes.cmd_land(self.ctx, args)
                self.assertIs(caught.exception, error)
                log.close.assert_called_once()
                close.assert_called_once()
                self.assertFalse(lanes.land_pidfile(self.ctx, "a").exists())
                self.assertEqual(self.ctx.load()["gate_queue"], [])

    def test_malformed_own_lock_timestamp_queues_as_stale(self):
        from unittest.mock import patch
        for fields in ({"ts": "invalid"}, {"ts": float("nan")}, {}, {"ts": None}):
            with self.subTest(fields=fields), \
                    self.land_fixture({"session": self.ctx.session, **fields}) as args, \
                    patch.object(lanes.time, "time", return_value=1), \
                    patch.object(lanes.GateQueue, "wait") as wait, \
                    patch.object(lanes, "write_lock") as write, redirect_stdout(io.StringIO()):
                self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                wait.assert_called_once_with(True)
                write.assert_not_called()

    def test_malformed_lock_timestamp_lands_through_acquire(self):
        # own lock: queued, then re-adopted by acquire_lock without a crash after the gates
        from unittest.mock import patch
        for fields in ({"ts": "invalid"}, {"ts": float("nan")}, {}, {"ts": None}):
            with self.subTest(fields=fields), \
                    self.fast_forward_fixture({"session": self.ctx.session,
                                               **fields}) as (args, state), \
                    patch.object(lanes.time, "sleep"), \
                    patch.object(lanes.GateQueue, "wait"):
                self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                self.assertEqual(state["main"], "tip")
            self.assertIsNone(lanes.read_lock(self.ctx))

    def test_acquire_lock_steals_peer_lock_with_malformed_timestamp(self):
        from unittest.mock import patch
        lockpath = self.ctx.main / lanes.LOCK_REL
        lockpath.parent.mkdir(parents=True, exist_ok=True)
        for fields in ({"ts": "invalid"}, {"ts": float("nan")}, {}, {"ts": None}):
            with self.subTest(fields=fields):
                lockpath.write_text(json.dumps({"session": "peer", **fields}), encoding="utf-8")
                with patch.object(lanes.time, "sleep"), \
                        patch.object(lanes, "git", return_value="abc1234"):
                    lanes.acquire_lock(self.ctx, "test")
                self.assertEqual(lanes.read_lock(self.ctx)["session"], self.ctx.session)
                lanes.release_lock(self.ctx)

    def test_lock_age_treats_malformed_timestamp_as_stale(self):
        import math
        from unittest.mock import patch
        now = 1700000000
        cases = [({"ts": now - 10}, 10), ({"ts": now - 0.5}, 0.5),
                 ({"ts": str(now - 10)}, 10), ({"ts": None}, math.inf),
                 ({}, math.inf), ({"ts": "abc"}, math.inf),
                 ({"ts": True}, math.inf), ({"ts": False}, math.inf),
                 ({"ts": float("nan")}, math.inf), ({"ts": float("inf")}, math.inf),
                 ({"ts": now + 60}, -60), ({"ts": now + 300}, -300),
                 ({"ts": now + 301}, math.inf), ({"ts": now + 3600}, math.inf),
                 ({"ts": 1.7e12}, math.inf), ({"ts": 1e300}, math.inf)]
        with patch.object(lanes.time, "time", return_value=now):
            for fields, expected in cases:
                with self.subTest(fields=fields):
                    self.assertEqual(lanes.lock_age(fields), expected)

    def test_read_lock_non_dict_is_stale_not_absent(self):
        from unittest.mock import patch
        for raw in ('[]', '"x"', '3', 'null'):
            with self.subTest(raw=raw), patch.object(Path, "read_text", return_value=raw):
                self.assertEqual(lanes.read_lock(self.ctx), {})
        with patch.object(Path, "read_text", return_value='{'):
            self.assertIsNone(lanes.read_lock(self.ctx))
        for error in (FileNotFoundError(), PermissionError()):
            with self.subTest(error=type(error).__name__), \
                    patch.object(Path, "read_text", side_effect=error):
                self.assertIsNone(lanes.read_lock(self.ctx))

    def test_acquire_lock_steals_non_dict_after_recheck(self):
        from unittest.mock import patch
        for first in ({}, {"session": "peer", "ts": 0}):
            with self.subTest(first=first), \
                    patch.object(lanes, "_create_lock", side_effect=[False, True]), \
                    patch.object(lanes, "read_lock", side_effect=[first, {}]) as read, \
                    patch.object(lanes.time, "sleep") as sleep, \
                    patch.object(Path, "unlink") as unlink:
                lanes.acquire_lock(self.ctx, "test")
                self.assertEqual(read.call_count, 2)
                sleep.assert_called_once_with(1)
                unlink.assert_called_once_with()

    def test_status_marks_old_and_non_dict_locks_stale(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        now = 1700000000
        age = lanes.LOCK_STALE_SECS + 1
        for lock in ({"session": "peer", "ts": now - age}, {}):
            out = io.StringIO()
            with self.subTest(lock=lock), \
                    patch.object(lanes, "read_lock", return_value=lock), \
                    patch.object(lanes, "git", return_value="abc1234"), \
                    patch.object(lanes.time, "time", return_value=now), redirect_stdout(out):
                self.assertEqual(lanes.cmd_status(self.ctx, SimpleNamespace(json=False)), 0)
            self.assertIn("HELD", out.getvalue())
            self.assertIn("STALE", out.getvalue())
            if lock:
                self.assertIn(f"({age}s)", out.getvalue())

    def test_preheld_lock_refreshed_before_gates_with_holder_label(self):
        from unittest.mock import patch
        now = 100000

        def gates(path):
            lock = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual(lock["ts"], now)
            self.assertEqual(lock["label"], "outer holder")
            self.assertEqual(lock["session"], self.ctx.session)

        with self.fast_forward_fixture(
                {"session": self.ctx.session, "ts": now - lanes.LOCK_STALE_SECS + 1,
                 "label": "outer holder"}, gates) as (args, state), \
                patch.object(lanes.time, "time", return_value=now):
            self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
            self.assertEqual(state["main"], "tip")

    def test_completed_fast_forward_and_push_warns_on_cleanup_error(self):
        from unittest.mock import Mock, patch
        for failure in ("abort", "log.close", "queue.close"):
            with self.subTest(failure=failure), self.fast_forward_fixture(
                    {"session": self.ctx.session, "ts": time.time()}) as (args, state):
                error = ValueError("cleanup failed")
                log = Mock()
                if failure == "log.close":
                    log.close.side_effect = error
                real_close = lanes.GateQueue.close

                def close(queue):
                    real_close(queue)
                    if failure == "queue.close":
                        raise error

                err = io.StringIO()
                with patch.dict(self.ctx.cfg, {"remote": "origin"}), \
                        patch.object(lanes, "sync_main_with_remote"), \
                        patch.object(lanes, "push_main", return_value="pushed") as push, \
                        patch.object(lanes, "run",
                                     side_effect=error if failure == "abort" else None), \
                        patch.object(lanes, "open", return_value=log), \
                        patch.object(lanes.GateQueue, "close", close), redirect_stderr(err):
                    args.no_push = False
                    self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                self.assertEqual(state["main"], "tip")
                push.assert_called_once_with(self.ctx, "origin")
                self.assertEqual(err.getvalue(),
                                 "lanes: warning: cleanup failed after main fast-forwarded: "
                                 "cleanup failed\n")
                log.close.assert_called_once()
                self.assertFalse(lanes.land_pidfile(self.ctx, "a").exists())
                self.assertEqual(self.ctx.load().get("gate_queue", []), [])

    def test_land_lock_holder_bypasses_queue_but_other_holder_queues(self):
        from unittest.mock import patch
        peer = lanes.GateQueue(self.ctx, "peer").ticket
        peer["session"] = "peer-session"
        now = time.time()
        for session, ts, held in ((self.ctx.session, now, True),
                                  ("other-session", now, False),
                                  (self.ctx.session, now - lanes.LOCK_STALE_SECS, False),
                                  (self.ctx.session, now - lanes.LOCK_STALE_SECS - 1, False)):
            with self.subTest(session=session, ts=ts), \
                    self.land_fixture({"session": session, "ts": ts}) as args, \
                    patch.object(lanes, "write_lock"), \
                    patch.object(lanes.time, "time", return_value=now):
                self.ctx.update(lambda reg: reg.update(gate_queue=[peer]))
                out = io.StringIO()
                real_wait = lanes.GateQueue.wait
                real_poll = lanes.GateQueue._poll
                real_enqueue = lanes.GateQueue._enqueue_and_prune

                def poll(queue):
                    position = real_poll(queue)
                    self.assertEqual(position, 2)
                    self.assertEqual([t["lane"] for t in self.ctx.load()["gate_queue"]],
                                     ["peer", "a"])
                    return position

                with patch.object(lanes.GateQueue, "wait", autospec=True,
                                  side_effect=real_wait) as wait, \
                        patch.object(lanes.GateQueue, "_poll", autospec=True,
                                     side_effect=poll) as poll_mock, \
                        patch.object(lanes.GateQueue, "_enqueue_and_prune", autospec=True,
                                     side_effect=real_enqueue) as enqueue, redirect_stdout(out):
                    if held:
                        self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                        wait.assert_not_called()
                        poll_mock.assert_not_called()
                        enqueue.assert_not_called()
                        self.assertEqual(out.getvalue().count(
                            "this session holds the main lock; not queueing for the full gates"), 1)
                    else:
                        with self.assertRaises(lanes.Refuse) as caught:
                            lanes.cmd_land(self.ctx, args)
                        self.assertEqual(caught.exception.code, 6)
                        wait.assert_called_once()
                        poll_mock.assert_called_once()
                        enqueue.assert_called_once()
                self.assertEqual(self.ctx.load()["gate_queue"], [peer])

    @contextmanager
    def fast_forward_fixture(self, lock, after_gates=None):
        """Drive the critical section with a real temp lock and a small Git state model."""
        from unittest.mock import patch
        read_lock = lanes.read_lock
        lockpath = self.ctx.main / lanes.LOCK_REL
        lockpath.parent.mkdir(parents=True, exist_ok=True)
        lockpath.write_text(json.dumps(lock), encoding="utf-8")
        state = {"main": "base"}

        def git(*args, **kw):
            if args == ("rev-parse", "main"):
                return state["main"]
            if args[:1] == ("merge",):
                state["main"] = args[-1]
                return ""
            return "commit"

        def gates(*args, **kw):
            if after_gates:
                after_gates(lockpath)
            return "tip"

        with self.land_fixture() as args, \
                patch.object(lanes, "git", side_effect=git), \
                patch.object(lanes, "read_lock", side_effect=read_lock), \
                patch.object(lanes, "check_main_idle"), \
                patch.object(lanes, "build_candidate", side_effect=gates), \
                redirect_stdout(io.StringIO()):
            yield args, state

    def test_holder_fast_forward_refreshes_lock_without_acquiring_or_releasing(self):
        from unittest.mock import patch
        now = 100000

        def long_gates(path):
            path.write_text(json.dumps({"session": self.ctx.session, "ts": 1,
                                        "label": "codex-item"}), encoding="utf-8")

        with self.fast_forward_fixture({"session": self.ctx.session, "ts": now - 1,
                                        "label": "codex-item"},
                                       long_gates) as (args, state), \
                patch.object(lanes.time, "time", return_value=now), \
                patch.object(lanes, "acquire_lock") as acquire, \
                patch.object(lanes, "release_lock") as release, \
                patch.object(lanes.GateQueue, "wait") as wait:
            self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
            self.assertEqual(state["main"], "tip")
            acquire.assert_not_called()
            release.assert_not_called()
            wait.assert_not_called()
        lock = lanes.read_lock(self.ctx)
        self.assertEqual(lock["session"], self.ctx.session)
        self.assertEqual(lock["ts"], now)
        self.assertEqual(lock["label"], "codex-item")  # the holder's label survives the refresh

    def test_stale_own_lock_queues_acquires_and_releases_on_fast_forward(self):
        from unittest.mock import patch
        lock = {"session": self.ctx.session, "ts": time.time() - lanes.LOCK_STALE_SECS - 1}
        with self.fast_forward_fixture(lock) as (args, state), \
                patch.object(lanes.time, "sleep"), \
                patch.object(lanes.GateQueue, "wait", autospec=True,
                             side_effect=lanes.GateQueue.wait) as wait, \
                patch.object(lanes, "acquire_lock", wraps=lanes.acquire_lock) as acquire, \
                patch.object(lanes, "release_lock", wraps=lanes.release_lock) as release:
            self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
            self.assertEqual(state["main"], "tip")
            wait.assert_called_once()
            acquire.assert_called_once()
            release.assert_called_once()
        self.assertIsNone(lanes.read_lock(self.ctx))
        self.assertEqual(self.ctx.load()["gate_queue"], [])

    def test_lost_lock_before_fast_forward_refuses_and_keeps_main_at_base(self):
        from unittest.mock import patch
        for initial_session in (self.ctx.session, "peer"):
            for lost in (None, {"session": "peer", "ts": time.time()}):
                with self.subTest(initial_session=initial_session, lost=lost):
                    def lose_lock(path):
                        if lost is None:
                            path.unlink()
                        else:
                            path.write_text(json.dumps(lost), encoding="utf-8")

                    with self.fast_forward_fixture(
                            {"session": initial_session, "ts": time.time()},
                            lose_lock) as (args, state), \
                            patch.dict(self.ctx.cfg, {"remote": "origin"}), \
                            patch.object(lanes, "acquire_lock"), \
                            patch.object(lanes, "sync_main_with_remote") as sync, \
                            patch.object(lanes, "push_main") as push, \
                            patch.object(lanes, "mark_landed") as mark:
                        args.no_push = False
                        with self.assertRaisesRegex(lanes.Refuse, "lost the main lock mid-land"):
                            lanes.cmd_land(self.ctx, args)
                        self.assertEqual(state["main"], "base")
                        sync.assert_not_called()  # refused before sync can move main
                        push.assert_not_called()
                        mark.assert_not_called()
                    self.assertEqual(self.ctx.load().get("gate_queue", []), [])

    def test_lock_lost_during_sync_refuses_before_fast_forward(self):
        from unittest.mock import patch
        for initial_session in (self.ctx.session, "peer"):
            for lost in (None, {"session": "peer", "ts": time.time()}):
                with self.subTest(initial_session=initial_session, lost=lost):
                    lockpath = self.ctx.main / lanes.LOCK_REL

                    def sync(ctx, remote):
                        # the lock is ours at the top check; it goes during the sync
                        if lost is None:
                            lockpath.unlink()
                        else:
                            lockpath.write_text(json.dumps(lost), encoding="utf-8")

                    def acquire(ctx, label):
                        lockpath.write_text(json.dumps({"session": ctx.session,
                                                        "ts": time.time()}), encoding="utf-8")

                    with self.fast_forward_fixture(
                            {"session": initial_session, "ts": time.time()}) as (args, state), \
                            patch.dict(self.ctx.cfg, {"remote": "origin"}), \
                            patch.object(lanes, "acquire_lock", side_effect=acquire), \
                            patch.object(lanes, "sync_main_with_remote",
                                         side_effect=sync) as sync_mock, \
                            patch.object(lanes, "push_main") as push, \
                            patch.object(lanes, "mark_landed") as mark:
                        args.no_push = False
                        with self.assertRaisesRegex(lanes.Refuse, "lost the main lock mid-land"):
                            lanes.cmd_land(self.ctx, args)
                        sync_mock.assert_called_once()  # passed the top check
                        self.assertEqual(state["main"], "base")
                        push.assert_not_called()
                        mark.assert_not_called()
                    self.assertEqual(self.ctx.load().get("gate_queue", []), [])

    def test_fifo_head_check_and_cleanup(self):
        from unittest.mock import patch
        first = lanes.GateQueue(self.ctx, "first")
        second = lanes.GateQueue(self.ctx, "second")
        self.addCleanup(first.close)
        self.addCleanup(second.close)
        first.wait()
        with self.assertRaises(lanes.Refuse) as error:
            second.wait(no_wait=True)
        self.assertEqual(error.exception.code, 6)
        self.assertEqual([t["lane"] for t in self.ctx.load()["gate_queue"]], ["first", "second"])
        sleeps = []
        def advance(_):
            sleeps.append(1)
            first.close()
        with patch.object(lanes.time, "sleep", side_effect=advance):
            second.wait()
        self.assertEqual(sleeps, [1])
        self.assertEqual(self.ctx.load()["gate_queue"], [second.ticket])
        second.close()
        self.assertEqual(self.ctx.load()["gate_queue"], [])

    def test_docs_only_bypass_and_inert_lands_queue(self):
        # lanes.json's own lists (the fixture CFG has docs/ only)
        self.ctx.cfg = dict(CFG, docs_only_paths=["docs/", "memory/"],
                            inert_paths=["docs/", "memory/", ".claude/", "tools/trace/",
                                         "tools/lanes/"])
        for paths in (["docs/note.md"], ["memory/note.md"], ["docs/a.md", "memory/b.md"]):
            self.assertFalse(lanes.queues_full_gates(self.ctx, paths))
            self.assertTrue(lanes.is_docs_only(self.ctx, paths))
        # inert_paths outside docs_only_paths still get the full gates, so they queue (a
        # trace-a land: a scenario, its capture notes and the backlog row)
        for paths in (["tools/lanes/lanes.py"], [".claude/skills/x.md"],
                      ["tools/trace/scenarios/x.py", "docs/naming/runtime/x/notes.md",
                       "docs/foundation/backlog.tsv"],
                      ["docs/note.md", "src/a.c"]):
            self.assertTrue(lanes.queues_full_gates(self.ctx, paths))
            self.assertFalse(lanes.is_docs_only(self.ctx, paths))

    def test_wait_keeps_its_place_through_transient_registry_errors(self):
        from unittest.mock import patch
        first = lanes.GateQueue(self.ctx, "first")
        second = lanes.GateQueue(self.ctx, "second")
        self.addCleanup(first.close)
        self.addCleanup(second.close)
        first.wait()
        with self.assertRaises(lanes.Refuse):
            second.wait(no_wait=True)
        mine = dict(second.ticket)
        real_load = self.ctx.load
        failures = [PermissionError("replace raced a reader"), ValueError("half-read"),
                    lanes.Refuse("registry mutex busy")]

        def flaky_load():
            if failures:
                raise failures.pop(0)
            return real_load()

        def advance(_):
            if not failures:
                first.close()

        with patch.object(self.ctx, "load", side_effect=flaky_load), \
                patch.object(lanes.time, "sleep", side_effect=advance), redirect_stdout(io.StringIO()):
            second.wait()
        queue = self.ctx.load()["gate_queue"]
        self.assertEqual([t["id"] for t in queue], [mine["id"]])
        self.assertEqual(queue[0]["enqueued"], mine["enqueued"])

    def test_waiting_poll_writes_nothing_when_nothing_changed(self):
        from unittest.mock import patch
        first = lanes.GateQueue(self.ctx, "first")
        second = lanes.GateQueue(self.ctx, "second")
        self.addCleanup(first.close)
        self.addCleanup(second.close)
        first.wait()
        with self.assertRaises(lanes.Refuse):
            second.wait(no_wait=True)
        writes, polls = [], []
        real_update = self.ctx.update

        def counting(fn):
            writes.append(fn)
            return real_update(fn)

        def advance(seconds):
            polls.append(seconds)
            if len(polls) == 3:
                self.assertEqual(writes, [])  # three polls at position 2, no rewrite
                first.close()

        with patch.object(self.ctx, "update", side_effect=counting), \
                patch.object(lanes.time, "sleep", side_effect=advance), redirect_stdout(io.StringIO()):
            second.wait()
        self.assertEqual(polls, [lanes.GATE_POLL_SECS] * 3)

    def test_close_never_raises(self):
        from contextlib import redirect_stderr
        from unittest.mock import patch
        queue = lanes.GateQueue(self.ctx, "a")
        self.addCleanup(queue.close)
        queue.wait()
        err = io.StringIO()
        with patch.object(self.ctx, "update", side_effect=PermissionError("held open")), \
                patch.object(lanes.time, "sleep"), redirect_stderr(err):
            queue.close()
        self.assertIn("could not dequeue gate ticket for a", err.getvalue())
        self.assertTrue(queue.enqueued)

    def test_status_prints_queue_positions_and_enqueue_time(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        ticket = lanes.GateQueue(self.ctx, "waiting").ticket
        self.ctx.update(lambda reg: reg.update(gate_queue=[ticket]))
        out = io.StringIO()
        with patch.object(lanes, "read_lock", return_value=None), \
                patch.object(lanes, "git", return_value="abc"), redirect_stdout(out):
            self.assertEqual(lanes.cmd_status(self.ctx, SimpleNamespace(json=False)), 0)
        self.assertIn("1. waiting waiting since", out.getvalue())
        self.assertIn("beat age", out.getvalue())
        out = io.StringIO()
        with patch.object(lanes, "read_lock", return_value=None), redirect_stdout(out):
            lanes.cmd_status(self.ctx, SimpleNamespace(json=True))
        self.assertEqual(json.loads(out.getvalue())["gate_queue"], [ticket])

    def test_dead_removed_but_old_live_preserved(self):
        dead = lanes.GateQueue(self.ctx, "dead").ticket
        dead["pid"] = 99999999
        live = lanes.GateQueue(self.ctx, "live").ticket
        live["enqueued"] -= lanes.LOCK_STALE_SECS + 1
        self.ctx.update(lambda reg: reg.update(gate_queue=[dead, live]))
        waiter = lanes.GateQueue(self.ctx, "waiting")
        self.addCleanup(waiter.close)
        out = io.StringIO()
        with redirect_stdout(out), self.assertRaises(lanes.Refuse):
            waiter.wait(no_wait=True)
        self.assertIn("dropped stale gate ticket for dead", out.getvalue())
        self.assertEqual(self.ctx.load()["gate_queue"], [live, waiter.ticket])

    def test_foreign_fresh_kept_expired_and_reused_pid_dropped(self):
        from unittest.mock import patch
        for namespace in ("foreign-runtime", lanes.pid_namespace()):
            with self.subTest(namespace=namespace):
                owner = lanes.GateQueue(self.ctx, "owner").ticket
                owner["namespace"] = namespace
                self.ctx.update(lambda reg: reg.update(gate_queue=[owner]))
                waiter = lanes.GateQueue(self.ctx, "waiter")
                try:
                    with patch.object(lanes, "gate_pid_alive", return_value=True):
                        with self.assertRaises(lanes.Refuse):
                            waiter.wait(no_wait=True)
                    self.assertEqual(self.ctx.load()["gate_queue"][0], owner)
                finally:
                    waiter.close()
                owner["beat"] = time.time() - lanes.HEARTBEAT_STALE_SECS - 5
                self.ctx.update(lambda reg: reg.update(gate_queue=[owner]))
                out = io.StringIO()
                try:
                    with patch.object(lanes, "gate_pid_alive", return_value=True), redirect_stdout(out):
                        waiter.wait(no_wait=True)
                    self.assertEqual(self.ctx.load()["gate_queue"], [waiter.ticket])
                    self.assertIn("owner session=test (beat age", out.getvalue())
                finally:
                    waiter.close()

    def test_heartbeat_refreshes_during_gate_and_stops_on_exit(self):
        from types import SimpleNamespace
        from unittest.mock import patch, DEFAULT
        real_queue = lanes.GateQueue
        created = []

        def create(*args):
            queue = real_queue(*args)
            created.append(queue)
            return queue

        for error in (None, RuntimeError("gate failed"), KeyboardInterrupt()):
            with self.subTest(error=error):
                self.ctx.update(lambda reg: reg.update(lanes={"a": {"owner": "test"}}))
                self.ctx.replayed = {}
                threads = []

                def gate(*args, **kwargs):
                    thread = created[-1]._thread
                    threads.append(thread)
                    self.assertTrue(thread.is_alive())
                    initial = self.ctx.load()["gate_queue"][0]["beat"]
                    deadline = time.monotonic() + 2
                    while time.monotonic() < deadline:
                        if self.ctx.load()["gate_queue"][0]["beat"] > initial:
                            break
                        threading.Event().wait(0.01)
                    self.assertGreater(self.ctx.load()["gate_queue"][0]["beat"], initial)
                    if error is not None:
                        raise error
                    return "base"

                with patch.object(lanes, "HEARTBEAT_INTERVAL_SECS", 0.01), \
                        patch.object(lanes, "GateQueue", side_effect=create), \
                        patch.multiple(lanes, recover_lane=DEFAULT, ensure_land_tree=DEFAULT,
                                       read_lock=DEFAULT, git=DEFAULT, unlanded=DEFAULT,
                                       paths_of=DEFAULT, check_claims=DEFAULT, build_candidate=DEFAULT,
                                       mark_landed=DEFAULT, advance_lane=DEFAULT, run=DEFAULT) as mocks:
                    mocks["git"].return_value = "base"
                    mocks["read_lock"].return_value = None
                    mocks["unlanded"].return_value = ["commit"]
                    mocks["paths_of"].return_value = ["src/a.c"]
                    mocks["ensure_land_tree"].return_value = self.ctx.state_dir
                    mocks["build_candidate"].side_effect = gate
                    args = SimpleNamespace(lane="a", no_push=True, no_wait=False)
                    if error is None:
                        self.assertEqual(lanes.cmd_land(self.ctx, args), 0)
                    else:
                        with self.assertRaises(type(error)):
                            lanes.cmd_land(self.ctx, args)
                self.assertEqual(len(threads), 1)
                self.assertFalse(threads[0].is_alive())
                self.assertEqual(self.ctx.load()["gate_queue"], [])
                self.assertTrue(created[-1]._stop.is_set())

    def test_heartbeat_refreshes_while_waiting(self):
        from unittest.mock import patch
        first = lanes.GateQueue(self.ctx, "first")
        second = lanes.GateQueue(self.ctx, "second")
        self.addCleanup(first.close)
        self.addCleanup(second.close)
        first.wait()
        def waiting(_):
            initial = second.ticket["beat"]
            deadline = time.monotonic() + 2
            while second.ticket["beat"] <= initial and time.monotonic() < deadline:
                threading.Event().wait(0.01)
            self.assertGreater(second.ticket["beat"], initial)
            first.close()

        with patch.object(lanes, "HEARTBEAT_INTERVAL_SECS", 0.01), \
                patch.object(lanes.time, "sleep", side_effect=waiting):
            second.wait()

    def test_wait_progress_throttled_and_interrupt_cleanup(self):
        from unittest.mock import patch
        first = lanes.GateQueue(self.ctx, "first")
        second = lanes.GateQueue(self.ctx, "second")
        self.addCleanup(first.close)
        first.wait()
        out = io.StringIO()
        with patch.object(lanes.time, "sleep", side_effect=[None, None, KeyboardInterrupt()]), \
                patch.object(lanes.time, "monotonic", side_effect=[0, 59, 60]), redirect_stdout(out):
            try:
                with self.assertRaises(KeyboardInterrupt):
                    second.wait()
            finally:
                second.close()
        self.assertEqual(out.getvalue().count("waiting for full-gate queue"), 2)
        self.assertEqual(self.ctx.load()["gate_queue"], [first.ticket])


class LaneTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="lanes-test-"))
        origin = self.tmp / "origin.git"
        sh("init", "-q", "--bare", "-b", "main", str(origin), cwd=self.tmp)
        self.main = self.tmp / "main"
        sh("clone", "-q", str(origin), str(self.main), cwd=self.tmp)
        m = self.main
        sh("config", "user.email", "t@example.com", cwd=m)
        sh("config", "user.name", "t", cwd=m)
        sh("checkout", "-q", "-b", "main", cwd=m)
        (m / "tools" / "lanes").mkdir(parents=True)
        (m / "tools" / "lanes" / "lanes.json").write_text(json.dumps(CFG), newline="\n")
        (m / "src").mkdir()
        (m / "src" / "a.c").write_text("int a;\n", newline="\n")
        (m / "shared.txt").write_text("".join(f"line{i}\n" for i in range(20)), newline="\n")
        (m / "backlog.tsv").write_text("id\tstage\nr1\tidea\nr2\tidea\nr3\tidea\n", newline="\n")
        (m / "gen_census.py").write_text(GEN, newline="\n")
        (m / "gate.py").write_text(GATE, newline="\n")
        (m / ".gitignore").write_text("tmp/\nFAIL_GATE\n", newline="\n")
        subprocess.run([sys.executable, "gen_census.py"], cwd=m, check=True)
        sh("add", "-A", cwd=m)
        sh("commit", "-q", "-m", "init", cwd=m)
        sh("push", "-q", "-u", "origin", "main", cwd=m)
        os.environ["BB2_LANES_MAIN"] = str(m)

    def tearDown(self):
        os.environ.pop("BB2_LANES_MAIN", None)
        subprocess.run(["git", "worktree", "prune"], cwd=self.main, capture_output=True)
        shutil.rmtree(self.tmp, onerror=_rm_readonly)  # the test's own temp repo

    # helpers
    def L(self, session, *argv):
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            rc = lanes.main(["--session", session, *argv])
        self.last = out.getvalue() + err.getvalue()
        return rc

    def wt(self, lane):
        return self.tmp / "wts" / f"ln-{lane}"

    def commit(self, lane, path, text, msg="work"):
        d = self.wt(lane)
        p = d / path
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, newline="\n")
        sh("add", "-A", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "-m", msg, cwd=d)

    def edit_line(self, lane, path, idx, new):
        d = self.wt(lane)
        lines = (d / path).read_text().splitlines()
        lines[idx] = new
        self.commit(lane, path, "\n".join(lines) + "\n")

    def main_file(self, path):
        return (self.main / path).read_text()

    # tests
    def test_full_lands_fifo_and_docs_bypass(self):
        from concurrent.futures import ThreadPoolExecutor
        from types import SimpleNamespace
        from unittest.mock import patch
        for lane in ("a", "b", "d"):
            self.assertEqual(self.L(lane, "open", lane), 0, self.last)
        self.commit("a", "src/b.c", "int b;\n")
        self.commit("b", "src/c.c", "int c;\n")
        self.commit("d", "docs/note.md", "note\n")
        entered, release, second = threading.Event(), threading.Event(), threading.Event()
        real = lanes.build_candidate
        bases = {}

        def gate(ctx, lane, *args, **kw):
            bases[lane] = args[2]
            if lane == "a" and not entered.is_set():
                entered.set()
                if not release.wait(15):
                    raise AssertionError("first gate was not released")
            if lane == "b":
                second.set()
            return real(ctx, lane, *args, **kw)

        def land(lane):
            return lanes.cmd_land(lanes.Ctx(self.main, lane),
                                  SimpleNamespace(lane=lane, no_push=False, no_wait=False))

        with patch.object(lanes, "build_candidate", side_effect=gate), ThreadPoolExecutor(2) as pool:
            first = pool.submit(land, "a")
            try:
                self.assertTrue(entered.wait(10))
                later = pool.submit(land, "b")
                ctx = lanes.Ctx(self.main, "observer")
                deadline = time.monotonic() + 10
                while len(ctx.load().get("gate_queue", [])) < 2 and time.monotonic() < deadline:
                    time.sleep(0.05)
                self.assertEqual([t["lane"] for t in ctx.load()["gate_queue"]], ["a", "b"])
                self.assertFalse(second.is_set())
                # --no-wait makes the docs-queue mutation fail immediately instead of hanging.
                self.assertEqual(self.L("d", "land", "d", "--no-wait"), 0, self.last)
                self.assertTrue((self.main / "docs/note.md").exists())
                self.assertEqual([t["lane"] for t in ctx.load()["gate_queue"]], ["a", "b"])
            finally:
                release.set()
            self.assertEqual(first.result(timeout=30), 0)
            self.assertEqual(later.result(timeout=30), 0)
        self.assertTrue(second.is_set())
        self.assertTrue(sh("show", f"{bases['b']}:src/b.c", cwd=self.main))
        self.assertEqual(ctx.load()["gate_queue"], [])

    def test_queue_stale_live_no_wait_and_status(self):
        from unittest.mock import patch
        ctx = lanes.Ctx(self.main, "s")
        dead, live = lanes.GateQueue(ctx, "dead"), lanes.GateQueue(ctx, "live")
        dead.ticket["pid"] = 99999999
        live.ticket["enqueued"] = time.time() - lanes.LOCK_STALE_SECS - 1
        ctx.update(lambda reg: reg.update(gate_queue=[dead.ticket, live.ticket]))
        waiter = lanes.GateQueue(ctx, "waiting")
        out = io.StringIO()
        try:
            with redirect_stdout(out):
                with self.assertRaises(lanes.Refuse) as refusal:
                    waiter.wait(no_wait=True)
            self.assertEqual(refusal.exception.code, 6)
            self.assertIn("dropped stale gate ticket for dead", out.getvalue())
            self.assertEqual([t["lane"] for t in ctx.load()["gate_queue"]], ["live", "waiting"])
            self.assertEqual(self.L("s", "status"), 0)
            self.assertIn("1. live waiting since", self.last)
            self.assertIn("2. waiting waiting since", self.last)
            self.assertEqual(self.L("s", "status", "--json"), 0)
            self.assertEqual(len(json.loads(self.last)["gate_queue"]), 2)
        finally:
            waiter.close()
        # Unknown PID namespaces are not mistaken for locally dead processes.
        live.ticket["namespace"] = "foreign"
        ctx.update(lambda reg: reg.update(gate_queue=[live.ticket]))
        with patch.object(lanes, "gate_pid_alive", return_value=False), self.assertRaises(lanes.Refuse):
            try:
                waiter.wait(no_wait=True)
            finally:
                waiter.close()
        self.assertEqual(ctx.load()["gate_queue"], [live.ticket])

    def test_queue_dequeues_on_gate_refusal_exception_and_interrupt(self):
        from unittest.mock import patch
        self.assertEqual(self.L("s", "open", "a"), 0)
        self.commit("a", "src/b.c", "int b;\n")
        ctx = lanes.Ctx(self.main, "s")
        for error in (lanes.Refuse("failed", 4), RuntimeError("broken"), KeyboardInterrupt()):
            def fail(*args, **kw):
                self.assertEqual(ctx.load()["gate_queue"][0]["lane"], "a")
                raise error
            with patch.object(lanes, "build_candidate", side_effect=fail):
                if isinstance(error, lanes.Refuse):
                    self.assertEqual(self.L("s", "land", "a"), 4)
                else:
                    with self.assertRaises(type(error)):
                        self.L("s", "land", "a")
            self.assertEqual(ctx.load()["gate_queue"], [])

    def test_open_ownership(self):
        self.assertEqual(self.L("s1", "open", "naming-a"), 0, self.last)
        self.assertTrue(self.wt("naming-a").exists())
        self.assertEqual(self.L("s2", "open", "naming-a"), 1)
        self.assertIn("owned by live session", self.last)
        self.assertEqual(self.L("s1", "open", "pins"), 1)
        self.assertIn("one session, one lane", self.last)
        self.assertEqual(self.L("s2", "claim", "naming-a", "topic:x"), 1)
        self.assertEqual(self.L("s1", "close", "naming-a"), 0, self.last)
        self.assertEqual(self.L("s2", "open", "naming-a"), 0, self.last)

    def test_open_heal_is_reserved_for_heal_sessions(self):
        self.assertEqual(self.L("s1", "open", "heal"), 1)
        self.assertIn("reserved for the orchestrator's self-heal", self.last)
        self.assertEqual(self.L("heal-1-2", "open", "heal"), 0, self.last)

    def test_claims(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.assertEqual(self.L("s1", "claim", "a", "path:src/*.c", "topic:row:r1"), 0, self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:src/a.c"), 1)
        self.assertEqual(self.L("s2", "claim", "b", "topic:row:r1"), 1)
        self.assertEqual(self.L("s2", "claim", "b", "topic:row:r2"), 0, self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:shared.txt"), 1)
        self.assertIn("shared path", self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:shared.txt!"), 0, self.last)

    def test_topic_overlap_rules(self):
        ov = lanes.topics_overlap
        self.assertTrue(ov("row:r1", "row:r1"))
        self.assertFalse(ov("tuedit:main/4D0A8", "tuedit:main/4D0A8"))  # shared
        self.assertTrue(ov("tuedit:main/4D0A8", "tu:main/4D0A8"))
        self.assertTrue(ov("tu:main/4D0A8", "tuedit:main/4D0A8"))
        self.assertFalse(ov("tuedit:main/4D0A8", "tu:main/9F9C"))
        self.assertTrue(ov("sym:f:x", "sym:f"))      # renaming f vs a local of f
        self.assertTrue(ov("sym:T", "sym:T.u.m"))    # renaming T vs its member
        self.assertFalse(ov("sym:f:x", "sym:f:y"))   # two locals of one function
        self.assertFalse(ov("sym:f:x", "sym:g:x"))
        self.assertFalse(ov("sym:tu:main/a", "tu:main/a"))
        self.assertEqual(lanes.wave_sym_claims("# c\nD_1=g_a, f:var_2=count\nT.u.unk_4=size\n"),
                         {"topic:sym:D_1", "topic:sym:g_a", "topic:sym:f:var_2",
                          "topic:sym:f:count", "topic:sym:T.u.unk_4", "topic:sym:T.u.size"})
        with self.assertRaises(lanes.Refuse):
            lanes.wave_sym_claims("D_1 g_a\n")

    def test_wave_claims(self):
        m = self.main
        for d, pairs, dry in (("w1", "D_1=g_one\nf:var_2=count\n", "-- file edits (1) --\nsrc/a.c\n"),
                              ("w2", "D_3=g_three\n", "-- file edits (1) --\nsrc/a.c\n"),
                              ("w3", "D_1=g_uno\n", "-- file edits (1) --\nsrc/a.c\n"),
                              ("w4", "f=renamed_f\n", "-- file edits (1) --\nsrc/a.c\n")):
            (m / "pending" / d).mkdir(parents=True)
            (m / "pending" / d / "pairs.txt").write_text(pairs, newline="\n")
            (m / "pending" / d / "dry.txt").write_text(dry.replace("src/a.c", "src/main/a.c"),
                                                       newline="\n")
        (m / "pending" / "mv").mkdir()
        (m / "pending" / "mv" / "dry.txt").write_text("-- file edits (1) --\nsrc/main/a.c\n",
                                                     newline="\n")
        self.assertEqual(lanes.wave_claims(m, "pending/w2"),
                         {"topic:sym:D_3", "topic:sym:g_three", "topic:tuedit:main/a"})
        self.assertEqual(lanes.wave_claims(m, "pending/mv"), {"topic:tu:main/a"})
        for s, lane in (("s1", "a"), ("s2", "b"), ("s3", "c")):
            self.assertEqual(self.L(s, "open", lane), 0, self.last)
        self.assertEqual(self.L("s1", "claim", "a", "topic:row:w1", "--wave", "pending/w1"), 0,
                         self.last)
        # another wave in the same TU on other symbols: allowed (tuedit is shared)
        self.assertEqual(self.L("s2", "claim", "b", "--wave", "pending/w2"), 0, self.last)
        before = lanes.Ctx(m, "s3").load()["lanes"]["c"]["claims"]
        # the same old symbol, a rename of the scope of a's local, a whole-TU (move) claim
        for wave in ("pending/w3", "pending/w4", "pending/mv"):
            self.assertEqual(self.L("s3", "claim", "c", "topic:row:x", "--wave", wave), 1, wave)
            self.assertIn("overlaps lane", self.last)
        # all or nothing: the refused calls added no claim (topic:row:x included)
        self.assertEqual(lanes.Ctx(m, "s3").load()["lanes"]["c"]["claims"], before)
        self.assertEqual(self.L("s3", "claim", "c", "--wave", "pending/missing"), 1)
        self.assertEqual(self.L("s3", "claim", "c"), 1)

    def _wave_commit(self, lane, wave, path, text):
        d = self.wt(lane)
        (d / "docs/naming/phase3/pending" / wave).mkdir(parents=True, exist_ok=True)
        (d / "docs/naming/phase3/pending" / wave / "pairs.txt").write_text(f"old_{wave}=new_{wave}\n",
                                                                           newline="\n")
        self.commit(lane, path, text, f"naming: {wave} -- one rename")

    def test_replayed_wave_is_keychecked(self):
        from unittest.mock import patch
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self._wave_commit("a", "wa", "shared.txt",
                          "".join(f"line{i}{'A' if i == 2 else ''}\n" for i in range(20)))
        self._wave_commit("b", "wb", "shared.txt",
                          "".join(f"line{i}{'B' if i == 15 else ''}\n" for i in range(20)))
        calls = []

        def kc(ctx, ld, rev, pairs, log):
            calls.append((sh("log", "-1", "--format=%s", rev, cwd=ld), pairs.strip()))
            return "ok"
        with patch.object(lanes, "_keycheck", side_effect=kc):
            self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
            self.assertEqual(calls, [])  # nothing moved under it: its own keycheck holds
            self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertEqual(calls, [("naming: wb -- one rename", "old_wb=new_wb")])
        self.assertIn("line2A\n", self.main_file("shared.txt"))  # both waves, one file
        self.assertIn("line15B\n", self.main_file("shared.txt"))

    def test_replayed_wave_failing_keycheck_bounces(self):
        from unittest.mock import patch
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self._wave_commit("a", "wa", "src/a.c", "int a;\nint x;\n")
        self._wave_commit("b", "wb", "shared.txt", "changed\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        b_sha = sh("rev-parse", "lanes/b", cwd=self.main)
        tip = sh("rev-parse", "main", cwd=self.main)
        # a FAIL verdict on the replayed commit -> bounce, main untouched
        with patch.object(lanes, "_keycheck", return_value="fail"):
            self.assertEqual(self.L("s2", "land", "b"), 3, self.last)
        self.assertIn("no longer explained by its pairs", self.last)
        self.assertIn(f"drop b {b_sha[:9]}", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), tip)
        # no verdict (crash, missing toolchain) is never read as a pass -> gate failure
        with patch.object(lanes, "_keycheck", return_value="error"):
            self.assertEqual(self.L("s2", "land", "b"), 4, self.last)
        self.assertIn("gave no verdict", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), tip)
        with patch.object(lanes, "_keycheck", return_value="ok"):
            self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertEqual(self.main_file("shared.txt"), "changed\n")

    def test_keycheck_verdict_parsing(self):
        from types import SimpleNamespace
        from unittest.mock import patch
        ctx = SimpleNamespace(cfg={"runner": "local"})
        ld = self.tmp / "ld"
        (ld / "tmp" / "lanes").mkdir(parents=True)
        for rc, out, want in ((0, "OK: every change is explained by the pairs\n", "ok"),
                              (1, "FAIL x\nFAIL: 2 change(s) the pairs do not explain\n", "fail"),
                              (1, "FAIL scoped dependency census toolchain missing\n", "error"),
                              (1, "FAIL scoped dependency census toolchain missing\n"
                                  "FAIL: 1 change(s) the pairs do not explain\n", "error"),
                              (0, "", "error"), (2, "bad pair\n", "error")):
            def fake(c, wt, cmd, log, rc=rc, out=out):
                (wt / "tmp" / "lanes" / "keycheck-abc.out").write_text(out, newline="\n")
                return rc
            with patch.object(lanes, "run_shell", side_effect=fake):
                self.assertEqual(lanes._keycheck(ctx, ld, "abc", "a=b\n", io.StringIO()), want)
        self.assertEqual(list((ld / "tmp" / "lanes").iterdir()), [])

    def test_land_simple_and_push(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())
        self.assertIn("regenerated census.csv", self.last)
        self.assertEqual(self.main_file("census.csv"), "files,2\n")
        # the census is folded into the landed commit: no separate refresh commit
        self.assertEqual(sh("log", "-1", "--format=%s", "main", cwd=self.main), "add b")
        self.assertEqual(sh("show", "--name-only", "--format=", "main", cwd=self.main).split(),
                         ["census.csv", "src/b.c"])
        lane_sha = sh("rev-parse", "lanes/a@{1}", cwd=self.main)
        reg = lanes.Ctx(self.main, "s1").load()
        self.assertEqual(reg["lanes"]["a"]["landed_map"][lane_sha],
                         sh("rev-parse", "main", cwd=self.main))
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)  # the amended commit counts
        self.assertIn("nothing to land", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), sh("rev-parse", "origin/main", cwd=self.main))
        self.assertEqual(sh("rev-parse", "HEAD", cwd=self.wt("a")), sh("rev-parse", "main", cwd=self.main))
        self.assertIsNone(lanes.read_lock(lanes.Ctx(self.main, "s1")))

    def test_parallel_disjoint_lines_merge(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 2, "A")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        t = self.main_file("shared.txt").splitlines()
        self.assertEqual((t[2], t[15]), ("A", "B"))

    def test_conflict_bounces_untouched(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 5, "B")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        main_before = sh("rev-parse", "main", cwd=self.main)
        lane_before = sh("rev-parse", "lanes/b", cwd=self.main)
        self.assertEqual(self.L("s2", "land", "b"), 3, self.last)
        self.assertIn("shared.txt", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), main_before)
        self.assertEqual(sh("rev-parse", "HEAD", cwd=self.wt("b")), lane_before)
        self.assertEqual(sh("status", "--porcelain", "--untracked-files=no", cwd=self.wt("b")), "")
        self.assertIsNone(lanes.read_lock(lanes.Ctx(self.main, "s2")))
        # the lane recovers by redoing on fresh main
        self.assertEqual(self.L("s2", "fresh", "b", "--discard"), 0, self.last)
        self.edit_line("b", "shared.txt", 6, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)

    def test_bounce_drop_keeps_later_commits(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 5, "B")
        bounced = sh("rev-parse", "HEAD", cwd=self.wt("b"))
        self.commit("b", "src/next.c", "int n;\n", "next unit")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 3, self.last)
        self.assertIn(f"lanes.py drop b {bounced[:9]}", self.last)
        self.assertEqual(self.L("s2", "drop", "b", bounced[:9]), 0, self.last)
        self.assertIn("refs/lanes-backup/b/", self.last)
        self.assertTrue((self.wt("b") / "src" / "next.c").exists())
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertTrue((self.main / "src" / "next.c").exists())
        self.assertEqual(self.main_file("shared.txt").splitlines()[5], "A")
        # --discard lists what it drops and keeps a backup ref
        self.commit("b", "src/gone.c", "int g;\n", "gone unit")
        self.assertEqual(self.L("s2", "fresh", "b", "--discard"), 0, self.last)
        self.assertIn("gone unit", self.last)
        refs = sh("for-each-ref", "--format=%(refname)", "refs/lanes-backup/b/", cwd=self.main).split()
        self.assertEqual(len(refs), 2)
        self.assertTrue(any(lanes.git_ok("cat-file", "-e", f"{r}:src/gone.c", cwd=self.main) for r in refs))
        self.assertEqual(self.L("s2", "drop", "b", "main"), 1)

    def test_failed_bootstrap_is_retried(self):
        boot = self.main / "tmp" / "boot.py"
        boot.parent.mkdir(exist_ok=True)
        flag = self.main / "tmp" / "bootfail"
        boot.write_text(f"import pathlib,sys\nsys.exit(1 if pathlib.Path({str(flag)!r}).exists() else 0)\n",
                        newline="\n")
        cfg = dict(CFG, bootstrap=[sys.executable, str(boot)])
        (self.main / "tools" / "lanes" / "lanes.json").write_text(json.dumps(cfg), newline="\n")
        sh("commit", "-qam", "boot cfg", cwd=self.main)
        sh("push", "-q", "origin", "main", cwd=self.main)
        flag.write_text("x")
        self.assertEqual(self.L("s1", "open", "a"), 1, self.last)
        self.assertIn("bootstrap failed", self.last)
        flag.unlink()
        self.assertEqual(self.L("s1", "open", "a"), 0, self.last)  # retried, not reused half set up
        self.assertTrue(lanes.boot_marker(self.wt("a")).exists())
        self.commit("a", "src/b.c", "int b;\n", "add b")
        flag.write_text("x")
        self.assertEqual(self.L("s1", "land", "a"), 1, self.last)  # landing tree bootstrap fails
        self.assertIn("bootstrap failed", self.last)
        flag.unlink()
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        land = self.tmp / "wts" / "lands" / "a"
        self.assertTrue(lanes.boot_marker(land).exists())

    def test_advance_note_when_edits_block_reset(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        d = self.wt("a")
        lines = (d / "backlog.tsv").read_text().splitlines()
        lines[1] = "r1\tlanded"
        self.commit("a", "backlog.tsv", "\n".join(lines) + "\n", "r1")
        lb = (self.wt("b") / "backlog.tsv").read_text().splitlines()
        lb[3] = "r3\tlanded"
        self.commit("b", "backlog.tsv", "\n".join(lb) + "\n", "r3")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        lines[2] = "r2\tactive"  # uncommitted next-unit edit
        (d / "backlog.tsv").write_text("\n".join(lines) + "\n", newline="\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("next `land` skips", self.last)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-qam", "r2", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.main_file("backlog.tsv"),
                         "id\tstage\nr1\tlanded\nr2\tactive\nr3\tlanded\n")

    def test_left_as_is_landed_commit_not_replayed_over_newer_main(self):
        # sixth review: a landed commit still on the branch must not replay (and bounce, or
        # re-apply) after main changed the same lines
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        d = self.wt("a")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 15, "B15")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        lines = (d / "shared.txt").read_text().splitlines()
        lines[10] = "A10"  # uncommitted next unit, in a file main changed: reset --keep refuses
        (d / "shared.txt").write_text("\n".join(lines) + "\n", newline="\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("next `land` skips", self.last)
        self.assertEqual(self.L("s2", "fresh", "b"), 0, self.last)
        self.edit_line("b", "shared.txt", 5, "B5")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-qam", "next", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("landing 1 commit(s)", self.last)
        t = self.main_file("shared.txt").splitlines()
        self.assertEqual((t[5], t[10], t[15]), ("B5", "A10", "B15"))
        self.assertEqual(self.L("s1", "fresh", "a"), 0, self.last)  # nothing unlanded is left

    def test_main_reset_after_land_unlands_again(self):
        # seventh review: `landed` must not hide commits once main no longer holds them
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self.assertEqual(self.L("s1", "land", "a", "--no-push"), 0, self.last)
        sh("reset", "-q", "--hard", "origin/main", cwd=self.main)  # undo the unpushed land
        self.assertEqual(self.L("s1", "fresh", "a"), 1, self.last)  # work is unlanded again
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())

    def test_reopen_after_left_as_is_land(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        d = self.wt("a")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        (d / "shared.txt").write_text("dirty\n", newline="\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("next `land` skips", self.last)
        sh("checkout", "--", "shared.txt", cwd=d)
        self.assertEqual(self.L("s1", "close", "a"), 0, self.last)
        sh("worktree", "remove", "--force", str(d), cwd=self.main)
        self.assertEqual(self.L("s1", "open", "a"), 0, self.last)
        self.assertEqual(sh("rev-parse", "HEAD", cwd=d), sh("rev-parse", "main", cwd=self.main))

    def _left_as_is_land(self, lane, *flags):
        d = self.wt(lane)
        keep = (d / "shared.txt").read_text()
        (d / "shared.txt").write_text("dirty\n", newline="\n")
        rc = self.L("s1", "land", lane, *flags)
        (d / "shared.txt").write_text(keep, newline="\n")
        self.assertEqual(rc, 0, self.last)
        self.assertIn("left as is", self.last)

    def test_reset_during_land_does_not_hide_work(self):
        # eighth review, blocking 1: main reset while a land's gates run
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self._left_as_is_land("a", "--no-push")
        self.commit("a", "src/c2.c", "int c2;\n", "c2")
        real, fired = lanes.build_candidate, []

        def racing(ctx, lane, ld, commits, base, log, **kw):
            if not fired:
                fired.append(1)
                sh("reset", "-q", "--hard", "origin/main", cwd=self.main)
            return real(ctx, lane, ld, commits, base, log, **kw)
        lanes.build_candidate = racing
        try:
            self.assertEqual(self.L("s1", "land", "a", "--no-push"), 0, self.last)
        finally:
            lanes.build_candidate = real
        self.assertEqual(self.main_file("shared.txt").splitlines()[5], "A")
        self.assertTrue((self.main / "src" / "c2.c").exists())

    def test_reset_before_first_attempt_does_not_hide_work(self):
        # ninth review: main reset between the first unlanded() and attempt 0's base
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self._left_as_is_land("a", "--no-push")
        self.commit("a", "src/c2.c", "int c2;\n", "c2")
        real = lanes.ensure_land_tree

        def resetting(ctx, lane):
            sh("reset", "-q", "--hard", "origin/main", cwd=self.main)
            return real(ctx, lane)
        lanes.ensure_land_tree = resetting
        try:
            self.assertEqual(self.L("s1", "land", "a", "--no-push"), 0, self.last)
        finally:
            lanes.ensure_land_tree = real
        self.assertEqual(self.main_file("shared.txt").splitlines()[5], "A")
        self.assertTrue((self.main / "src" / "c2.c").exists())

    def test_commit_racing_the_branch_move_is_kept(self):
        # tenth review B1: a commit between advance_lane's HEAD read and its reset
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        real, fired = lanes.run, []

        def racing(cmd, *args, **kw):
            if cmd[:4] == ["git", "reset", "-q", "--keep"] and not fired:
                fired.append(1)
                self.commit("a", "src/new.c", "int precious;\n", "precious")
            return real(cmd, *args, **kw)
        lanes.run = racing
        try:
            self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        finally:
            lanes.run = real
        self.assertTrue(fired)
        self.assertIn("was put back on it", self.last)
        self.assertTrue((self.wt("a") / "src" / "new.c").exists())
        self.assertEqual(sh("log", "-1", "--format=%s", "lanes/a", cwd=self.main), "precious")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("landing 1 commit(s)", self.last)

    def test_advance_leaves_detached_or_busy_lane_alone(self):
        # eleventh review: detached HEAD / a rebase in progress are never moved or aborted
        self.L("s1", "open", "a")
        d = self.wt("a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        sh("checkout", "-q", "--detach", cwd=d)
        head = sh("rev-parse", "HEAD", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("HEAD is not on the branch", self.last)
        self.assertEqual(sh("rev-parse", "HEAD", cwd=d), head)
        self.assertNotIn("raced", self.last)
        sh("checkout", "-q", "lanes/a", cwd=d)
        self.commit("a", "src/x1.c", "1\n", "x1")
        self.commit("a", "src/x2.c", "2\n", "x2")
        r = subprocess.run(["git", "-c", "user.email=t@example.com", "-c", "user.name=t", "rebase",
                            "-q", "-x", "exit 1", "HEAD~1"], cwd=d, capture_output=True)
        self.assertNotEqual(r.returncode, 0)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("is in progress", self.last)
        self.assertTrue(Path(d, sh("rev-parse", "--git-path", "rebase-merge", cwd=d)).exists())
        sh("rebase", "--abort", cwd=d)

    def test_close_remove_deletes_landed_branch_only(self):
        stub = self.main / "tools" / "safe_remove_worktree.ps1"  # untracked stand-in
        stub.write_text(f"param($p, [switch]$Force)\ngit -C '{self.main}' worktree remove --force $p\n"
                        "exit $LASTEXITCODE\n", newline="\n")
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s1", "close", "a", "--remove"), 0, self.last)
        self.assertIn("deleted branch lanes/a", self.last)
        self.assertFalse(self.wt("a").exists())
        self.assertFalse((self.tmp / "wts" / "lands" / "a").exists())
        self.assertFalse(lanes.git_ok("rev-parse", "--verify", "-q", "refs/heads/lanes/a", cwd=self.main))
        # unlanded work keeps its branch even under --force
        self.L("s1", "open", "a")
        self.commit("a", "src/c.c", "int c;\n", "add c")
        self.assertEqual(self.L("s1", "close", "a", "--remove"), 1, self.last)
        self.assertEqual(self.L("s1", "open", "a"), 0, self.last)
        self.assertEqual(self.L("s1", "close", "a", "--remove", "--force"), 0, self.last)
        self.assertIn("kept branch lanes/a", self.last)

    def test_tidy_removes_only_closed_lanes_leftovers(self):
        ctx = lanes.Ctx(self.main, "orch")
        # lane a: landed branch and an empty folder left behind after close
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s1", "close", "a"), 0, self.last)
        sh("worktree", "remove", "--force", str(self.wt("a")), cwd=self.main)
        (self.wt("a") / "build" / "x").mkdir(parents=True)  # empty dirs only
        # lane b: closed with unlanded work, folder not empty: kept, reported
        self.L("s2", "open", "b")
        self.commit("b", "src/c.c", "int c;\n", "add c")
        self.assertEqual(self.L("s2", "close", "b", "--force"), 0, self.last)
        sh("worktree", "remove", "--force", str(self.wt("b")), cwd=self.main)
        self.wt("b").mkdir()
        (self.wt("b") / "keep.txt").write_text("x", newline="\n")
        # lane c: still owned: untouched even with a landed branch
        self.L("s3", "open", "c")
        self.assertEqual(lanes.tidy_leftovers(ctx, dry_run=True),
                         [("would", f"folder {self.wt('a')}", ""),
                          ("would", "branch lanes/a", ""),
                          ("look", f"folder {self.wt('b')}", "not a worktree and not empty; inspect it"),
                          ("look", "branch lanes/b", "1 unlanded commit(s); kept")])
        self.assertTrue(self.wt("a").exists())
        got = lanes.tidy_leftovers(ctx)
        self.assertEqual([g[0] for g in got], ["done", "done", "look", "look"])
        self.assertFalse(self.wt("a").exists())
        self.assertFalse(lanes.git_ok("rev-parse", "--verify", "-q", "refs/heads/lanes/a",
                                      cwd=self.main))
        self.assertTrue((self.wt("b") / "keep.txt").exists())
        self.assertTrue(lanes.git_ok("rev-parse", "--verify", "-q", "refs/heads/lanes/b",
                                     cwd=self.main))
        self.assertTrue(self.wt("c").exists())
        self.assertTrue(lanes.git_ok("rev-parse", "--verify", "-q", "refs/heads/lanes/c",
                                     cwd=self.main))

    def test_empty_tree_refuses_a_junctioned_or_linked_folder(self):
        from unittest.mock import patch
        d = self.tmp / "emptydir"
        d.mkdir()
        self.assertTrue(lanes.empty_tree(d))
        with patch.object(lanes.os.path, "isjunction", return_value=True, create=True):
            self.assertFalse(lanes.empty_tree(d))

    def test_tidy_failure_is_reported_not_raised(self):
        ctx = lanes.Ctx(self.main, "orch")
        self.L("s1", "open", "a")
        self.assertEqual(self.L("s1", "close", "a"), 0, self.last)
        sh("worktree", "remove", "--force", str(self.wt("a")), cwd=self.main)
        real = lanes.git

        def locked(*a, **k):
            if a[:2] == ("branch", "-D"):
                raise RuntimeError("Unable to create '.git/packed-refs.lock': File exists")
            return real(*a, **k)
        from unittest.mock import patch
        with patch.object(lanes, "git", side_effect=locked):
            got = lanes.tidy_leftovers(ctx)
        self.assertEqual([(g[0], g[1]) for g in got], [("failed", "branch lanes/a")])
        self.assertEqual([g[0] for g in lanes.tidy_leftovers(ctx)], ["done"])  # retried later

    def test_heal_land_flips_a_blocked_row_end_to_end(self):
        import heal
        stub = self.main / "tools" / "safe_remove_worktree.ps1"  # untracked stand-in
        stub.write_text(f"param($p, [switch]$Force)\ngit -C '{self.main}' worktree remove --force $p\n"
                        "exit $LASTEXITCODE\n", newline="\n")
        p = self.main / heal.BACKLOG
        p.parent.mkdir(parents=True)
        head = "id\tkind\tstage\tdraft\tnote\n"
        for text, msg in ((head + "r\tdata\tidea\t-\tn\n", "add"),
                          (head + "r\tdata\tblocked\t-\tn\n", "block"),
                          (head + "r\tdata\tblocked\t-\tn EVIDENCE cap-1 (trace-a 2026-11-03)\n", "evidence")):
            p.write_text(text, newline="\n")
            sh("add", "-A", cwd=self.main)
            sh("commit", "-q", "-m", msg, cwd=self.main)
        sh("push", "-q", "origin", "main", cwd=self.main)
        self.assertEqual(heal.cmd_land(), 0, (self.main / "tmp" / "lanes-state" / "logs" /
                                              "heal.log").read_text())
        res = json.loads((self.main / "tmp" / "lanes-state" / "heal.json").read_text())
        self.assertEqual((res["step"], res["rows"]), ("landed", ["r"]))
        self.assertIn("r\tdata\tidea\t-\tUnblocked (heal.py)", self.main_file(heal.BACKLOG))
        msg = sh("log", "-1", "--format=%B", "main", cwd=self.main)
        self.assertIn("Lane-Replay: python3 tools/lanes/heal.py apply", msg)
        e = lanes.Ctx(self.main, "x").load()["lanes"]["heal"]
        self.assertIsNone(e["owner"])
        self.assertFalse(self.wt("heal").exists())
        # nothing left to move: the next run lands nothing
        self.assertEqual(heal.cmd_land(), 0)
        res = json.loads((self.main / "tmp" / "lanes-state" / "heal.json").read_text())
        self.assertEqual(res["step"], "nothing")
        self.assertFalse(self.wt("heal").exists())  # the scan alone opened nothing

    def test_heal_land_failure_leaves_the_lane_clean_for_the_next_run(self):
        import heal
        from unittest.mock import patch
        stub = self.main / "tools" / "safe_remove_worktree.ps1"
        stub.write_text(f"param($p, [switch]$Force)\ngit -C '{self.main}' worktree remove --force $p\n"
                        "exit $LASTEXITCODE\n", newline="\n")
        p = self.main / heal.BACKLOG
        p.parent.mkdir(parents=True)
        head = "id\tkind\tstage\tdraft\tnote\n"
        for text, msg in ((head + "r\tdata\tblocked\t-\tn\n", "block"),
                          (head + "r\tdata\tblocked\t-\tn EVIDENCE c (trace-a 2026-11-03)\n", "ev")):
            p.write_text(text, newline="\n")
            sh("add", "-A", cwd=self.main)
            sh("commit", "-q", "-m", msg, cwd=self.main)
        sh("push", "-q", "origin", "main", cwd=self.main)
        real_git = heal.git

        def boom(*a, **k):
            if a[:1] == ("add",):
                raise RuntimeError("index.lock exists")
            return real_git(*a, **k)
        with patch.object(heal, "git", side_effect=boom):
            self.assertEqual(heal.cmd_land(), 3)
        self.assertEqual(sh("status", "--porcelain", "--untracked-files=no", cwd=self.wt("heal")), "")
        self.assertIsNone(lanes.Ctx(self.main, "x").load()["lanes"]["heal"]["owner"])
        self.assertEqual(heal.cmd_land(), 0, (self.main / "tmp" / "lanes-state" / "logs" /
                                              "heal.log").read_text())
        self.assertIn("r\tdata\tidea\t", self.main_file(heal.BACKLOG))

    def test_heal_refuses_a_lane_path_that_is_not_its_own(self):
        import heal
        from unittest.mock import patch
        from types import SimpleNamespace
        fake = SimpleNamespace(returncode=0, stdout="\n")
        with patch.object(heal.subprocess, "run", return_value=fake):
            self.assertIsNone(heal.lane_dir(lambda s: None))
        fake.stdout = str(self.main) + "\n"
        with patch.object(heal.subprocess, "run", return_value=fake):
            self.assertIsNone(heal.lane_dir(lambda s: None))
        fake.stdout = str(self.tmp / "wts" / "ln-notheal") + "\n"  # ends in "heal", wrong lane
        with patch.object(heal.subprocess, "run", return_value=fake):
            self.assertIsNone(heal.lane_dir(lambda s: None))
        fake.stdout = str(self.tmp / "wts" / "ln-heal") + "\n"
        with patch.object(heal.subprocess, "run", return_value=fake):
            self.assertEqual(heal.lane_dir(lambda s: None), self.tmp / "wts" / "ln-heal")

    def test_reap_closes_only_an_unchanged_idle_owner(self):
        ctx = lanes.Ctx(self.main, "orch")
        self.L("s1", "open", "a")
        self.L("s1", "claim", "a", "path:src/**")
        self.assertFalse(lanes.reap_lane(ctx, "a", "someone-else", "gone"))
        ctx.update(lambda reg: reg.setdefault("gate_queue", []).append({"lane": "a"}))
        self.assertFalse(lanes.reap_lane(ctx, "a", "s1", "gone"))  # queued to land
        ctx.update(lambda reg: reg.__setitem__("gate_queue", []))
        self.assertTrue(lanes.reap_lane(ctx, "a", "s1", "gone"))
        e = ctx.load()["lanes"]["a"]
        self.assertIsNone(e["owner"])
        self.assertEqual(e["claims"], [])
        self.assertEqual(e["reaped"]["owner"], "s1")
        self.assertTrue(self.wt("a").exists())  # worktree kept for the next session
        self.assertEqual(self.L("s2", "open", "a"), 0, self.last)

    def test_mark_landed_failure_leaves_main_unmoved(self):
        # tenth review S1: the record is written before main moves
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        before = sh("rev-parse", "main", cwd=self.main)
        real = lanes.mark_landed

        def boom(*a, **k):
            raise lanes.Refuse("registry mutex busy")
        lanes.mark_landed = boom
        try:
            self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        finally:
            lanes.mark_landed = real
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())

    def test_nothing_to_land_still_advances(self):
        # tenth review N2
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self._left_as_is_land("a")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("nothing to land", self.last)
        self.assertEqual(sh("rev-parse", "HEAD", cwd=self.wt("a")), sh("rev-parse", "main", cwd=self.main))

    def test_partial_reset_keeps_earlier_lands_landed(self):
        # eighth review, blocking 2: undoing only the latest land must not re-replay older ones
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.commit("a", "src/b.c", "int b;\n", "A adds b.c")
        self.edit_line("b", "shared.txt", 15, "B")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self._left_as_is_land("a")  # pushed: M1 on origin holds b.c
        self.commit("a", "src/c2.c", "int c2;\n", "c2")
        self._left_as_is_land("a", "--no-push")
        sh("reset", "-q", "--hard", "origin/main", cwd=self.main)
        self.assertEqual(self.L("s2", "fresh", "b"), 0, self.last)
        sh("rm", "-q", "src/b.c", cwd=self.wt("b"))
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-qm", "rm b", cwd=self.wt("b"))
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("landing 1 commit(s)", self.last)  # only c2
        self.assertFalse((self.main / "src" / "b.c").exists())
        self.assertTrue((self.main / "src" / "c2.c").exists())

    def test_rename_out_of_src_still_gated(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/h.h", "x\n", "h")
        self.commit("a", "src/a.c", '#include "h.h"\n', "use h")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        d = self.wt("a")
        (d / "docs").mkdir(exist_ok=True)
        sh("mv", "src/h.h", "docs/h.h", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-qm", "mv", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertNotIn("docs-only", self.last)

    def test_replay_trailer(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        d = self.wt("b")
        (d / "rename.py").write_text(
            "import pathlib\np=pathlib.Path('shared.txt')\n"
            "p.write_text(p.read_text().replace('line5','RENAMED').replace('A\\n','RENAMED\\n'),newline='\\n')\n",
            newline="\n")
        sh("add", "-A", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "-m", "tool", cwd=d)
        subprocess.run([sys.executable, "rename.py"], cwd=d, check=True)
        sh("add", "-A", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "-m",
           "naming: wave\n\nLane-Replay: python rename.py", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertIn("replayed", self.last)
        self.assertEqual(self.main_file("shared.txt").splitlines()[5], "RENAMED")
        self.assertIn("Lane-Replay", sh("log", "-1", "--format=%B", "main", cwd=self.main) + sh("log", "-2", "--format=%B", "main", cwd=self.main))

    def test_rowmerge_backlog(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.commit("a", "backlog.tsv", "id\tstage\nr1\tlanded\nr2\tidea\nr3\tidea\n")
        self.commit("b", "backlog.tsv", "id\tstage\nr1\tidea\nr2\treviewed\nr3\tidea\nr4\tidea\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertEqual(self.main_file("backlog.tsv"),
                         "id\tstage\nr1\tlanded\nr2\treviewed\nr3\tidea\nr4\tidea\n")

    def test_rowmerge_same_row_conflicts(self):
        self.assertIsNone(lanes.rowmerge("h\nr1\ta\n", "h\nr1\tb\n", "h\nr1\tc\n"))
        self.assertEqual(lanes.rowmerge("h\nr1\ta\nr2\tx\n", "h\nr1\tb\nr2\tx\n", "h\nr1\ta\n"), "h\nr1\tb\n")

    def test_regen_conflict_takes_main_and_regenerates(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.commit("a", "census.csv", "stale-a\n")
        self.commit("b", "census.csv", "stale-b\n")
        self.commit("b", "src/c.c", "int c;\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertEqual(self.main_file("census.csv"), "files,2\n")

    def test_gate_failure_keeps_main(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/FAIL", "x\n")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)
        self.assertEqual(sh("rev-parse", "--abbrev-ref", "HEAD", cwd=self.wt("a")), "lanes/a")

    def test_docs_only_skips_gates(self):
        self.L("s1", "open", "a")
        self.commit("a", "docs/FAIL", "x\n")  # would fail the gate if it ran
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("docs-only", self.last)

    def _scoped_gate_cfg(self):
        (self.main / "strict.py").write_text("import sys\nsys.exit(1)\n", newline="\n")
        cfg = dict(CFG, gates={"naming": ["python gate.py", "python strict.py"],
                               "foundation": ["python gate.py", "python strict.py"], "docs": []},
                   gate_scope={"python strict.py": {"light": [".claude/"], "renames": ["src/"]}})
        self._main_commit("tools/lanes/lanes.json", json.dumps(cfg))

    def test_gate_scope_skips_light_and_token_only_renames(self):
        self._scoped_gate_cfg()
        self.L("s1", "open", "a")
        self.commit("a", ".claude/rules/r.md", "r\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("gate skipped: python strict.py", self.last)
        self.assertIn("$ python gate.py", self.last)  # unscoped gates always run
        self.commit("a", "src/a.c", "int alpha;\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("token-only rename", self.last)

    def test_gate_scope_runs_gate_on_real_changes(self):
        self._scoped_gate_cfg()
        self.L("s1", "open", "a")
        for path, text in (("src/a.c", "long a;\n"),        # keyword swap: structure
                           ("src/a.c", "int a; int b;\n"),  # token count
                           ("src/new.c", "int n;\n"),       # added file
                           ("tools/x.py", "x = 1\n")):      # outside light / renames
            with self.subTest(path=path, text=text):
                self.L("s1", "fresh", "a", "--discard")
                self.commit("a", path, text)
                self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
                self.assertIn("gate failed: python strict.py", self.last)

    def test_token_only_rename_needs_a_bijection(self):
        m = self.main
        (m / "src" / "a.c").write_text("int a; int b;\n", newline="\n")
        sh("commit", "-q", "-am", "two", cwd=m)
        base = sh("rev-parse", "HEAD", cwd=m)
        for text, ok in (("int x; int y;\n", True), ("int x; int x;\n", False),
                         ("int a; int a;\n", False), ("int t0; int b;\n", False),
                         ("int  a ;\n  int b;\n", True)):
            with self.subTest(text=text):
                (m / "src" / "a.c").write_text(text, newline="\n")
                sh("commit", "-q", "-am", "edit", cwd=m)
                self.assertEqual(lanes.token_only_rename(m, base, ["src/a.c"])[0], ok)
                sh("reset", "-q", "--hard", base, cwd=m)

    def test_trace_scenarios_are_docs_only(self):
        cfg = json.loads((Path(__file__).parent / "lanes.json").read_text(encoding="utf-8"))
        ctx = object.__new__(lanes.Ctx)
        ctx.cfg = cfg
        self.assertTrue(lanes.is_docs_only(ctx, ["tools/trace/scenarios/x-1.json",
                                                 "docs/naming/runtime/e.tsv"]))
        self.assertTrue(lanes.is_docs_only(ctx, ["tools/trace/scenarios/segments/s.json"]))
        for code in ("tools/trace/scan/diff.py", "tools/trace/scenarios/scen.py",
                     "tools/trace/scenarios/test_scen.py"):
            self.assertFalse(lanes.is_docs_only(ctx, [code]), code)

    def test_token_only_rename_refuses_names_tools_read(self):
        m = self.main
        (m / "tools").mkdir(exist_ok=True)
        (m / "tools" / "t.py").write_text("NAMES = ['a']\n", newline="\n")
        sh("add", "-A", cwd=m)
        sh("commit", "-q", "-m", "tool", cwd=m)
        base = sh("rev-parse", "HEAD", cwd=m)
        (m / "src" / "a.c").write_text("int alpha;\n", newline="\n")
        sh("commit", "-q", "-am", "rename", cwd=m)
        self.assertTrue(lanes.token_only_rename(m, base, ["src/a.c"])[0])
        ok, why = lanes.token_only_rename(m, base, ["src/a.c"], ["engine/", "tools/"])
        self.assertFalse(ok)
        self.assertIn("tools/t.py", why)
        self.assertTrue(lanes.token_only_rename(m, base, ["src/a.c"], ["engine/"])[0])
        self.assertTrue(lanes.token_only_rename(m, base, ["src/a.c"],
                                                ["tools/", ":!tools/t.py"])[0])
        # a new name a reader already uses is refused too
        (m / "tools" / "t.py").write_text("NAMES = ['alpha']\n", newline="\n")
        sh("commit", "-q", "-am", "tool2", cwd=m)
        base2 = sh("rev-parse", "HEAD", cwd=m)
        sh("checkout", "-q", base, "--", "src/a.c", cwd=m)
        sh("commit", "-q", "-m", "back", cwd=m)
        base3 = sh("rev-parse", "HEAD", cwd=m)
        (m / "src" / "a.c").write_text("int alpha;\n", newline="\n")
        sh("commit", "-q", "-am", "rename2", cwd=m)
        self.assertNotEqual(base2, base3)
        self.assertFalse(lanes.token_only_rename(m, base3, ["src/a.c"], ["tools/"])[0])
        # root name lists are a reader: the real config's pathspec reaches them
        (m / "tools" / "t.py").write_text("NAMES = []\n", newline="\n")
        (m / "names.txt").write_text("a\n", newline="\n")
        sh("add", "-A", cwd=m)
        sh("commit", "-q", "-m", "list", cwd=m)
        sh("checkout", "-q", base, "--", "src/a.c", cwd=m)
        sh("commit", "-q", "-m", "back2", cwd=m)
        base4 = sh("rev-parse", "HEAD", cwd=m)
        (m / "src" / "a.c").write_text("int alpha;\n", newline="\n")
        sh("commit", "-q", "-am", "rename3", cwd=m)
        cfg = json.loads((Path(__file__).parent / "lanes.json").read_text(encoding="utf-8"))
        spec = cfg["gate_scope"]["python3 -m engine.cli test"]["unread_by"]
        ok, why = lanes.token_only_rename(m, base4, ["src/a.c"], spec)
        self.assertFalse(ok)
        self.assertIn("names.txt", why)

    def _capture_checks(self):
        source = Path(__file__).resolve().parents[1] / "trace/store.py"
        self._main_commit("tools/trace/store.py", source.read_text(encoding="utf-8"))
        cfg = dict(CFG, input_checks=[{
            "cmd": "python tools/trace/store.py check-commit",
            "inputs": ["docs/naming/runtime/"]}])
        self._main_commit("tools/lanes/lanes.json", json.dumps(cfg))

    def test_input_check_skips_unrelated_docs_and_build_inputs(self):
        self._capture_checks()
        self.L("s1", "open", "a")
        self.commit("a", "docs/note.txt", "note")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertNotIn("store.py check-commit --rev", self.last)
        self.commit("a", "src/b.c", "int b;\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertNotIn("store.py check-commit --rev", self.last)

    def test_capture_check_uses_replayed_commit_tree_on_docs_land(self):
        import hashlib
        self._capture_checks()
        self.L("s1", "open", "a")
        self.commit("a", "docs/naming/runtime/sample/capture.jsonl", "right")
        rec = dict(path="capture.jsonl", sha1=hashlib.sha1(b"right").hexdigest(), size=5)
        self.commit("a", "docs/naming/runtime/sample/capture.manifest.json",
                    json.dumps(dict(outputs=[rec])))
        # Force replay onto a different parent; judge the new SHA, not the lane SHA.
        old = sh("rev-parse", "HEAD", cwd=self.wt("a"))
        self._main_commit("docs/unrelated.txt", "main advanced")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        new = sh("rev-parse", "main", cwd=self.main)
        self.assertNotEqual(old, new)
        self.assertIn("check-commit --rev " + new, self.last)
        self.assertIn("outputs=1 refused=0", self.last)
        self.assertIn("docs-only", self.last)

    def test_capture_check_refuses_invalid_intermediate_commit(self):
        import hashlib
        self._capture_checks()
        self.L("s1", "open", "a")
        self.commit("a", "docs/naming/runtime/sample/capture.jsonl", "wrong")
        rec = dict(path="capture.jsonl", sha1=hashlib.sha1(b"right").hexdigest(), size=5)
        self.commit("a", "docs/naming/runtime/sample/capture.manifest.json",
                    json.dumps(dict(outputs=[rec])))
        # The final tree is valid, but the preceding commit is not publishable.
        self.commit("a", "docs/naming/runtime/sample/capture.jsonl", "right")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertIn("input check failed", self.last)
        self.assertIn("SHA1/size mismatch", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)

    def test_input_check_runs_when_commits_cancel_in_final_diff(self):
        self._main_commit("check_tree.py", "import sys\nprint('TREE REFUSED')\nsys.exit(1)\n")
        cfg = dict(CFG, input_checks=[{"cmd": "python check_tree.py",
                                       "inputs": ["docs/runtime/"]}])
        self._main_commit("tools/lanes/lanes.json", json.dumps(cfg))
        self._main_commit("docs/runtime/capture.txt", "old")
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/capture.txt", "new")
        self.commit("a", "docs/runtime/capture.txt", "old")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertIn("TREE REFUSED", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)

    def _tip_checks(self, inputs, body="print('TIP CHECK RAN')\n"):
        self._main_commit("check_tip.py", body)
        cfg = json.loads(self.main_file("tools/lanes/lanes.json"))
        cfg["tip_checks"] = [{"cmd": "python check_tip.py", "inputs": inputs}]
        self._main_commit("tools/lanes/lanes.json", json.dumps(cfg))

    def test_tip_check_runs_once_after_regenerate_fold(self):
        self._inputs_regen()
        self._tip_checks(["docs/runtime/"],
                         "from pathlib import Path\nimport subprocess\n"
                         "assert Path('docs/runtime/events.tsv').read_text() == 'fresh\\n'\n"
                         "assert subprocess.check_output(['git', 'show', "
                         "'HEAD:docs/runtime/events.tsv'], text=True) == 'fresh\\n'\n"
                         "print('TIP CHECK RAN')\n")
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/first.txt", "first\n")
        self.commit("a", "docs/runtime/second.txt", "second\n")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("docs-only", self.last)
        self.assertEqual(self.last.count("TIP CHECK RAN"), 1)
        self.assertEqual(sh("rev-list", "--count", f"{before}..main", cwd=self.main), "2")

    def test_tip_check_scopes_to_regenerated_output(self):
        self._inputs_regen()
        self._tip_checks(["docs/runtime/events.tsv"])
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/capture.txt", "capture\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.last.count("TIP CHECK RAN"), 1)

    def test_tip_check_skips_unrelated_docs_and_build_inputs(self):
        self._tip_checks(["docs/runtime/"])
        self.L("s1", "open", "a")
        for path, text in [("docs/note.txt", "note\n"), ("src/b.c", "int b;\n")]:
            self.commit("a", path, text)
            self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
            self.assertNotIn("$ python check_tip.py", self.last)
            self.assertNotIn("TIP CHECK RAN", self.last)

    def test_failing_tip_check_refuses_with_main_unmoved(self):
        self._tip_checks(["docs/runtime/"], "import sys\nsys.exit(1)\n")
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/capture.txt", "capture\n")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertRegex(self.last, r"tip check failed: python check_tip.py  \(log .*\.log\)")
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)

    def test_tip_check_tracked_edits_refuse(self):
        self._tip_checks(["docs/runtime/"],
                         "from pathlib import Path\nPath('shared.txt').write_text('dirty')\n")
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/capture.txt", "capture\n")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertIn("gates left tracked changes", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)

    def test_tip_check_runs_on_quick_candidate(self):
        self._tip_checks(["src/"])
        self.L("s1", "open", "a")
        base = sh("rev-parse", "main", cwd=self.main)
        self.commit("a", "src/b.c", "int b;\n")
        commit = sh("rev-parse", "HEAD", cwd=self.wt("a"))
        ctx = lanes.Ctx(self.main, "s1")
        ld = lanes.ensure_land_tree(ctx, "a")
        log = io.StringIO()
        log.name = "quick-tip.log"
        with redirect_stdout(io.StringIO()):
            lanes.build_candidate(ctx, "a", ld, [commit], base, log, quick_paths=["src/b.c"])
        self.assertEqual(ctx.gated, "quick")
        self.assertEqual(log.getvalue().count("TIP CHECK RAN"), 1)

    def _inputs_regen(self, cmd="python gen_events.py", initial="stale\n"):
        self._main_commit("docs/runtime/events.tsv", initial)
        self._main_commit("gen_events.py", "from pathlib import Path\n"
                          "Path('docs/runtime/events.tsv').write_text('fresh\\n', newline='\\n')\n")
        entry = {"path": "docs/runtime/events.tsv", "cmd": cmd,
                 "subject": "docs: events refresh", "inputs": ["docs/runtime/"]}
        cfg = dict(CFG, regenerate=CFG["regenerate"] + [entry])
        self._main_commit("tools/lanes/lanes.json", json.dumps(cfg))

    def test_docs_inputs_regenerate_and_fold(self):
        self._inputs_regen()
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/capture.txt", "capture\n", "capture")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("docs-only", self.last)
        self.assertNotIn("$ python gate.py", self.last)
        self.assertNotIn("quick-gate", self.last)
        self.assertNotIn("$ python gen_census.py", self.last)
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "fresh\n")
        self.assertEqual(sh("log", "--format=%s", f"{before}..main", cwd=self.main), "capture")
        self.assertEqual(sh("show", "--name-only", "--format=", "main", cwd=self.main).split(),
                         ["docs/runtime/capture.txt", "docs/runtime/events.tsv"])
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("nothing to land", self.last)

    def test_docs_unrelated_to_inputs_skips_regeneration(self):
        self._inputs_regen()
        self.L("s1", "open", "a")
        self.commit("a", "docs/note.md", "note\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertNotIn("$ python gen_events.py", self.last)
        self.assertNotIn("$ python gen_census.py", self.last)
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "stale\n")

    def test_non_docs_unrelated_to_inputs_still_runs_census(self):
        self._inputs_regen()
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertNotIn("$ python gen_events.py", self.last)
        self.assertIn("regenerated census.csv", self.last)
        self.assertEqual(self.main_file("census.csv"), "files,2\n")
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "stale\n")

    def test_inputs_output_only_does_not_trigger(self):
        self._inputs_regen()
        self.L("s1", "open", "a")
        self.commit("a", "docs/runtime/events.tsv", "manual\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertNotIn("$ python gen_events.py", self.last)
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "manual\n")

    def test_failing_inputs_command_refuses_only_touching_land(self):
        self._inputs_regen(cmd='python -c "import sys; sys.exit(1)"')
        self.L("s1", "open", "a")
        self.L("s2", "open", "b")
        self.commit("a", "docs/runtime/capture.txt", "bad capture\n")
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertIn("regenerate failed", self.last)
        self.assertRegex(self.last, r"regenerate failed: .*  \(log .*\.log\)")
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)
        self.commit("b", "docs/note.md", "note\n")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.commit("b", "src/b.c", "int b;\n")
        self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
        self.assertIn("regenerated census.csv", self.last)
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "stale\n")

    def test_docs_inputs_regen_only_last_commit_dropped(self):
        self._inputs_regen(initial="fresh\n")
        self.L("s1", "open", "a")
        before = sh("rev-parse", "main", cwd=self.main)
        self.commit("a", "docs/runtime/capture.txt", "capture\n", "capture")
        self.commit("a", "docs/runtime/events.tsv", "manual\n", "manual events")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("held only regenerated files and was dropped", self.last)
        self.assertEqual(sh("log", "--format=%s", f"{before}..main", cwd=self.main), "capture")
        self.assertEqual(self.main_file("docs/runtime/events.tsv"), "fresh\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("nothing to land", self.last)

    def test_freeze_blocks_other_landing(self):
        self.L("s1", "open", "pins"); self.L("s2", "open", "naming-a")
        self.assertEqual(self.L("s1", "claim", "pins", "path:shared.txt!"), 0, self.last)
        self.edit_line("naming-a", "shared.txt", 1, "N")
        self.assertEqual(self.L("s2", "land", "naming-a"), 1, self.last)
        self.assertIn("claimed by lane 'pins'", self.last)
        self.L("s1", "unclaim", "pins")
        self.assertEqual(self.L("s2", "land", "naming-a"), 0, self.last)

    def test_dirty_main_refuses(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        (self.main / "src" / "a.c").write_text("int a2;\n", newline="\n")
        self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        self.assertIn("nobody edits main", self.last)

    def test_lock_held_by_other_refuses(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertTrue(lanes._create_lock(lanes.Ctx(self.main, "other"), "someone"))
        self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        self.assertIn("main lock held", self.last)

    def test_merge_commit_refused(self):
        self.L("s1", "open", "a")
        d = self.wt("a")
        sh("checkout", "-q", "-b", "side", cwd=d)
        self.commit("a", "src/evil.c", "int e;\n")
        sh("checkout", "-q", "lanes/a", cwd=d)
        self.commit("a", "src/c.c", "int c;\n")
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "merge", "-q", "--no-ff", "-m", "m",
           "side", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        self.assertIn("merge commits", self.last)

    def test_push_failure_is_an_error(self):
        hook = self.tmp / "origin.git" / "hooks" / "pre-receive"
        hook.write_text("#!/bin/sh\nexit 1\n", newline="\n")
        os.chmod(hook, 0o755)
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertEqual(self.L("s1", "land", "a"), 5, self.last)
        self.assertIn("PUSH FAILED", self.last)
        self.assertIsNone(lanes.read_lock(lanes.Ctx(self.main, "s1")))

    def test_replay_rejects_stray_paths(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.edit_line("a", "shared.txt", 5, "A")
        d = self.wt("b")
        (d / "rename.py").write_text(
            "import pathlib\n"
            "p = pathlib.Path('shared.txt')\n"
            "t = p.read_text().splitlines()\n"
            "t[5] = 'R'\n"
            "p.write_text('\\n'.join(t) + '\\n', newline='\\n')\n"
            "pathlib.Path('src/stray.c').write_text('x', newline='\\n')\n", newline="\n")
        sh("add", "rename.py", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "-m", "tool", cwd=d)
        lines = (d / "shared.txt").read_text().splitlines()
        lines[5] = "R"
        (d / "shared.txt").write_text("\n".join(lines) + "\n", newline="\n")
        sh("add", "shared.txt", cwd=d)
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "-m",
           "naming: wave\n\nLane-Replay: python rename.py", cwd=d)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "b"), 3, self.last)
        self.assertIn("src/stray.c", self.last)
        self.assertFalse((self.main / "src" / "stray.c").exists())

    def test_gates_see_only_committed_files(self):
        # the lane forgot to commit a header its .c needs: the gate must fail, main must not move
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", '#include "h.h"\n')
        (self.wt("a") / "src" / "h.h").write_text("int h;\n")  # untracked
        before = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 4, self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), before)
        self.assertTrue((self.wt("a") / "src" / "h.h").exists())  # the lane's file is untouched

    def test_work_during_land_is_safe(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        real = lanes.build_candidate

        def busy(ctx, lane, ld, commits, base, log, **kw):
            tip = real(ctx, lane, ld, commits, base, log, **kw)
            self.commit("a", "src/later.c", "int later;\n", "made during the land")
            return tip

        lanes.build_candidate = busy
        try:
            self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        finally:
            lanes.build_candidate = real
        self.assertTrue((self.main / "src" / "b.c").exists())
        self.assertFalse((self.main / "src" / "later.c").exists())  # ungated: not landed
        self.assertNotIn("made during", sh("log", "--format=%s", "origin/main", cwd=self.main))
        # it survives on the lane branch, rebased onto the new main, and lands next time
        self.assertEqual(sh("rev-list", "--count", "main..lanes/a", cwd=self.main), "1")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "later.c").exists())

    def test_uncommitted_edit_during_land_survives(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        real = lanes.build_candidate

        def busy(ctx, lane, ld, commits, base, log, **kw):
            (self.wt("a") / "src" / "a.c").write_text("int edited;\n", newline="\n")
            return real(ctx, lane, ld, commits, base, log, **kw)

        lanes.build_candidate = busy
        try:
            self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        finally:
            lanes.build_candidate = real
        self.assertEqual((self.wt("a") / "src" / "a.c").read_text(), "int edited;\n")

    def test_reland_after_interrupted_push_adds_nothing(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        old_tip = sh("rev-parse", "lanes/a", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        n = sh("rev-list", "--count", "origin/main", cwd=self.main)
        sh("reset", "-q", "--hard", old_tip, cwd=self.wt("a"))  # as if killed before advancing
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        # (the replayed commit may even share the old SHA, then there is nothing to land at all)
        self.assertTrue("already in main" in self.last or "nothing to land" in self.last, self.last)
        self.assertEqual(sh("rev-list", "--count", "origin/main", cwd=self.main), n)
        # force distinct SHAs: amend the old tip's date, then re-land
        sh("-c", "user.email=t@example.com", "-c", "user.name=t", "commit", "-q", "--amend",
           "--no-edit", "--date=2001-01-01T00:00:00", cwd=self.wt("a"))
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("already in main", self.last)
        self.assertEqual(sh("rev-list", "--count", "origin/main", cwd=self.main), n)

    def test_push_retry(self):
        hook = self.tmp / "origin.git" / "hooks" / "pre-receive"
        hook.write_text("#!/bin/sh\nexit 1\n", newline="\n")
        os.chmod(hook, 0o755)
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertEqual(self.L("s1", "land", "a"), 5, self.last)
        hook.unlink()
        self.assertEqual(self.L("s1", "push"), 0, self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main),
                         sh("rev-parse", "origin/main", cwd=self.main))

    def test_concurrent_land_refused(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        ctx = lanes.Ctx(self.main, "s1")
        p = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(30)"])
        try:
            lanes.land_pidfile(ctx, "a").write_text(str(p.pid))
            self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
            self.assertIn("still running", self.last)
            self.assertEqual(self.L("s1", "recover", "a"), 1, self.last)
        finally:
            p.kill()
            p.wait()

    def test_takeover_releases_dead_owners_lock(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertTrue(lanes._create_lock(lanes.Ctx(self.main, "s1"), "lanes land a"))
        # s1 crashed; s2 takes the lane over (it is stale)
        reg_p = self.main / "tmp" / "lanes-state" / "registry.json"
        reg = json.loads(reg_p.read_text())
        reg["lanes"]["a"]["heartbeat"] = 0
        reg_p.write_text(json.dumps(reg))
        self.assertEqual(self.L("s2", "open", "a"), 0, self.last)
        self.assertEqual(self.L("s2", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())

    def test_freeze_arriving_during_lock_wait(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "pins")
        self.edit_line("a", "shared.txt", 1, "N")
        ctx = lanes.Ctx(self.main, "s1")
        real = lanes.acquire_lock

        def slow_acquire(c, label):
            # the other lane freezes the file while we wait for the lock
            self.assertEqual(self.L("s2", "claim", "pins", "path:shared.txt!"), 0)
            real(c, label)

        lanes.acquire_lock = slow_acquire
        try:
            rc = self.L("s1", "land", "a")
        finally:
            lanes.acquire_lock = real
        self.assertEqual(rc, 1, self.last)
        self.assertIn("claimed by lane 'pins'", self.last)
        self.assertIsNone(lanes.read_lock(ctx))

    def test_lock_already_held_is_kept(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        self.assertTrue(lanes._create_lock(lanes.Ctx(self.main, "s1"), "codex reintegration"))
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertEqual(lanes.read_lock(lanes.Ctx(self.main, "s1"))["session"], "s1")

    def test_rowmerge_order_and_safety(self):
        base = "id\tv\na\t1\nb\t1\nc\t1\n"
        # a row the lane inserts mid-file keeps its place
        self.assertEqual(lanes.rowmerge(base, "id\tv\na\t2\nb\t1\nc\t1\n",
                                        "id\tv\na\t1\nb\t1\nx\t9\nc\t1\n"),
                         "id\tv\na\t2\nb\t1\nx\t9\nc\t1\n")
        # a pure move by the lane is not silently dropped
        self.assertIsNone(lanes.rowmerge(base, "id\tv\na\t2\nb\t1\nc\t1\n",
                                         "id\tv\nc\t1\na\t1\nb\t1\n"))
        # a missing side or blank lines go to a human
        self.assertIsNone(lanes.rowmerge(base, "", "id\tv\na\t1\nb\t2\nc\t1\n"))
        self.assertIsNone(lanes.rowmerge(base, "id\tv\na\t2\n\nb\t1\nc\t1\n",
                                         "id\tv\na\t1\nb\t2\nc\t1\n"))

    def test_lock_is_exclusive(self):
        c1, c2 = lanes.Ctx(self.main, "s1"), lanes.Ctx(self.main, "s2")
        self.assertTrue(lanes._create_lock(c1, "x"))
        self.assertFalse(lanes._create_lock(c2, "y"))
        with self.assertRaises(lanes.Refuse):
            lanes.write_lock(c2, "y")  # heartbeat only while ours
        lanes.release_lock(c2)
        self.assertEqual(lanes.read_lock(c1)["session"], "s1")
        lanes.release_lock(c1)
        self.assertIsNone(lanes.read_lock(c1))


    def test_interrupted_land_recovers(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        lane_tip = sh("rev-parse", "lanes/a", cwd=self.main)
        ctx = lanes.Ctx(self.main, "s1")
        # simulate a land killed under the lock: dead pid file, lock held, landing tree dirty
        ld = lanes.ensure_land_tree(ctx, "a")
        (ld / "src" / "a.c").write_text("half-applied\n")
        lanes.land_pidfile(ctx, "a").write_text("999999")
        self.assertTrue(lanes._create_lock(ctx, "lanes land a"))
        self.assertEqual(self.L("s1", "recover", "a"), 0, self.last)
        self.assertIn("recovered", self.last)
        self.assertIsNone(lanes.read_lock(ctx))
        self.assertEqual(sh("rev-parse", "HEAD", cwd=self.wt("a")), lane_tip)
        # and land itself recovers too
        self.assertTrue(lanes._create_lock(ctx, "lanes land a"))
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())
        # a lock left by another session for another purpose is never touched
        self.assertTrue(lanes._create_lock(lanes.Ctx(self.main, "zz"), "codex reintegration"))
        self.assertEqual(self.L("s1", "recover", "a"), 0, self.last)
        self.assertEqual(lanes.read_lock(ctx)["session"], "zz")

    def test_main_moving_during_gates_rebuilds(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.commit("a", "src/b.c", "int b;\n")
        self.commit("b", "src/c.c", "int c;\n")
        real = lanes.build_candidate
        calls = []

        def racing(ctx, lane, wt, commits, base, log, **kw):
            tip = real(ctx, lane, wt, commits, base, log, **kw)
            if not calls:  # the other lane lands while our gates run
                calls.append(1)
                lanes.build_candidate = real
                # Simulate an external full land (e.g. codex_worker) that bypasses the FIFO.
                from unittest.mock import patch
                with patch.object(lanes, "queues_full_gates", return_value=False):
                    self.assertEqual(self.L("s2", "land", "b"), 0, self.last)
                lanes.build_candidate = racing
            return tip

        lanes.build_candidate = racing
        try:
            rc = self.L("s1", "land", "a")
        finally:
            lanes.build_candidate = real
        self.assertEqual(rc, 0, self.last)
        self.assertTrue((self.main / "src" / "b.c").exists() and (self.main / "src" / "c.c").exists())

    # ---- cheap re-check when main moved only in disjoint files
    def test_tooling_inert_paths(self):
        cfg = json.loads((Path(__file__).parent / "lanes.json").read_text(encoding="utf-8"))
        for path in ("tools/trace/scan/events.py", "tools/lanes/lanes.py"):
            with self.subTest(path=path):
                def full(lane, moved):
                    return lanes.needs_full_gates(lane, moved, cfg["inert_paths"],
                                                 cfg["docs_only_paths"])[0]
                self.assertFalse(full([path], ["docs/new.md"]))
                self.assertFalse(full(["src/main/a.c"], [path]))
                self.assertFalse(full(["tools/trace/a.py"], ["tools/lanes/b.py"]))
                for build in ("tools/other.py", "engine/cli.py"):
                    self.assertTrue(full(["src/main/a.c"], [path, build]))
                    self.assertTrue(full([path, build], ["src/main/b.c"]))
                self.assertTrue(full([path], [path]))  # overlap still requires full gates
                self.assertTrue(full([path], ["src/main/b.c"]))  # inert is not docs-only

    def test_needs_full_gates_decision(self):
        docs_only = ("docs/", "memory/")
        merge = ("docs/naming/function-names.csv", "docs/naming/phase3/backlog.tsv")

        def full(lane, moved):
            return lanes.needs_full_gates(lane, moved, lanes.INERT_DEFAULT, docs_only, merge)[0]
        # build-input lane, main moved only in docs/memory/.claude: cheap
        self.assertFalse(full(["src/main/a.c", "include/bb2.h"],
                              ["docs/x.md", "memory/grind/f/l.md", ".claude/rules/r.md"]))
        # build inputs on both sides, even disjoint ones: full
        self.assertTrue(full(["src/main/a.c"], ["src/main/b.c"]))
        self.assertTrue(full(["src/main/a.c"], ["Makefile"]))
        self.assertTrue(full(["named_syms.txt"], ["tools/lanes/lanes.py"]))
        self.assertTrue(full(["engine/x.py"], ["bb2.ld"]))
        self.assertTrue(full(["asm/funcs/f.s"], ["AGENTS.md"]))  # root files count as build inputs
        # overlapping paths: full, inert or not
        self.assertTrue(full(["src/a.c", "docs/n.md"], ["docs/n.md"]))
        self.assertTrue(full(["docs/a.md"], ["DOCS/A.md"]))  # case-insensitive (NTFS)
        # regenerated / row-merged files may change on both sides
        self.assertFalse(full(["src/a.c", "docs/naming/function-names.csv",
                               "docs/naming/phase3/backlog.tsv"],
                              ["docs/naming/function-names.csv", "docs/naming/phase3/backlog.tsv",
                               "docs/naming/phase3/m10/manifest.csv"]))
        # docs-only lane: main's build inputs cannot touch it
        self.assertFalse(full(["docs/a.md"], ["src/a.c", "Makefile"]))
        # an inert lane that is still gated (.claude/ is not docs-only) needs main inert too
        self.assertTrue(full([".claude/rules/r.md"], ["src/a.c"]))
        self.assertFalse(full([".claude/rules/r.md"], ["docs/x.md"]))
        # nothing relevant moved
        self.assertFalse(full(["src/a.c"], []))
        self.assertFalse(full(["src/a.c"], ["docs/naming/function-names.csv"]))
        # backslashes normalise
        self.assertTrue(full(["src\\a.c"], ["src/a.c"]))

    def _race(self, mover):
        """Wrap build_candidate: run mover(n) after its n-th call. Returns (real, the list of
        quick_paths each call received); the caller restores lanes.build_candidate = real."""
        real, seen = lanes.build_candidate, []

        def racing(ctx, lane, ld, commits, base, log, **kw):
            tip = real(ctx, lane, ld, commits, base, log, **kw)
            seen.append(kw.get("quick_paths"))
            mover(len(seen))
            return tip
        lanes.build_candidate = racing
        return real, seen

    def _main_commit(self, path, text):
        p = self.main / path
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, newline="\n")
        sh("add", "-A", cwd=self.main)
        sh("commit", "-q", "-m", f"main {path}", cwd=self.main)
        sh("push", "-q", "origin", "main", cwd=self.main)

    def test_main_moved_in_docs_gets_quick_recheck(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self.commit("b", "docs/note.md", "n\n", "note")
        box = {}

        def other_lands(n):
            if n == 1:  # lane b lands (unwrapped) while lane a's gates run
                mine, lanes.build_candidate = lanes.build_candidate, box["real"]
                rc = self.L("s2", "land", "b")
                lanes.build_candidate = mine
                self.assertEqual(rc, 0, self.last)
        box["real"], seen = self._race(other_lands)
        try:
            rc = self.L("s1", "land", "a")
        finally:
            lanes.build_candidate = box["real"]
        self.assertEqual(rc, 0, self.last)
        self.assertEqual(len(seen), 2)
        self.assertIsNone(seen[0])
        self.assertIn("src/b.c", seen[1])
        self.assertIn("quick re-check", self.last)
        self.assertIn("quick-gate", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), 1)  # the full gates ran once
        self.assertTrue((self.main / "src" / "b.c").exists())
        self.assertTrue((self.main / "docs" / "note.md").exists())
        self.assertEqual(self.main_file("census.csv"), "files,2\n")
        self.assertEqual(sh("log", "-1", "--format=%s", "main", cwd=self.main), "add b")

    def test_build_inputs_on_both_sides_regate_in_full(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        real, seen = self._race(
            lambda n: self._main_commit("src/m.c", "int m;\n") if n == 1 else None)
        try:
            rc = self.L("s1", "land", "a")
        finally:
            lanes.build_candidate = real
        self.assertEqual(rc, 0, self.last)
        self.assertEqual(seen, [None, None])
        self.assertNotIn("quick re-check", self.last)
        self.assertIn("build inputs changed on both sides", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), 2)
        self.assertEqual(self.main_file("census.csv"), "files,3\n")

    def test_quick_recheck_still_fails_closed(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        cfg = dict(CFG, quick_gates=["python -c \"import sys; sys.exit(1)\""])
        (self.main / "tools" / "lanes" / "lanes.json").write_text(json.dumps(cfg), newline="\n")
        sh("commit", "-qam", "cfg", cwd=self.main)
        sh("push", "-q", "origin", "main", cwd=self.main)
        real, seen = self._race(
            lambda n: self._main_commit("docs/d.md", "d\n") if n == 1 else None)
        try:
            rc = self.L("s1", "land", "a")
        finally:
            lanes.build_candidate = real
        self.assertEqual(rc, 4, self.last)
        self.assertEqual(len(seen), 1)  # the second (quick) call raised
        self.assertFalse((self.main / "src" / "b.c").exists())

    def test_retry_budget_full_and_quick(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        real, seen = self._race(lambda n: self._main_commit(f"src/m{n}.c", "int m;\n"))
        try:
            self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        finally:
            lanes.build_candidate = real
        self.assertEqual(len(seen), lanes.FULL_GATE_ATTEMPTS)  # build inputs: full every time
        self.assertIn("main kept moving", self.last)
        self.assertFalse((self.main / "src" / "b.c").exists())
        real, seen = self._race(lambda n: self._main_commit(f"docs/d{n}.md", "d\n"))
        try:
            self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        finally:
            lanes.build_candidate = real
        self.assertEqual(len(seen), lanes.LAND_ATTEMPTS)  # docs only: one full, then quick ones
        self.assertEqual(sum(q is None for q in seen), 1)
        self.assertIn("main kept moving", self.last)

    def _land_racing(self, mover, lane="a", rc=0):
        real, seen = self._race(mover)
        try:
            got = self.L("s1", "land", lane)
        finally:
            lanes.build_candidate = real
        self.assertEqual(got, rc, self.last)
        return seen

    def test_quick_fallback_on_new_path_respects_cap(self):
        # quick replays that are never mechanical: each one falls back to the full gates, and the
        # land stops at FULL_GATE_ATTEMPTS full runs (it never runs a fourth)
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        real_cheap = lanes.cheap_recheck_paths

        def shrunk(*a):
            got = real_cheap(*a)
            return None if got is None else []  # every lane path now counts as new
        lanes.cheap_recheck_paths = shrunk
        try:
            self._land_racing(lambda n: self._main_commit(f"docs/d{n}.md", "d\n"), rc=1)
        finally:
            lanes.cheap_recheck_paths = real_cheap
        self.assertIn("not mechanical (new paths: src/b.c)", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), lanes.FULL_GATE_ATTEMPTS)
        self.assertNotIn("quick-gate", self.last)
        self.assertIn(f"during {lanes.FULL_GATE_ATTEMPTS + 1} land attempts "
                      f"({lanes.FULL_GATE_ATTEMPTS} full gate runs)", self.last)
        self.assertFalse((self.main / "src" / "b.c").exists())

    def test_quick_fallback_on_lane_replay_lands_after_full_gates(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        real_replay = lanes.replay_commit

        def as_replayed(*a):
            real_replay(*a)
            return "replayed"  # as if a Lane-Replay command had re-run on the new base
        lanes.replay_commit = as_replayed
        try:
            self._land_racing(lambda n: self._main_commit("docs/d.md", "d\n") if n == 1 else None)
        finally:
            lanes.replay_commit = real_replay
        self.assertIn("not mechanical (a Lane-Replay re-ran)", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), 2)
        self.assertNotIn("quick-gate", self.last)
        self.assertTrue((self.main / "src" / "b.c").exists())

    def test_main_rewritten_since_gated_base_gets_full_gates(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n", "add b")
        self._main_commit("docs/y.md", "y\n")  # the gated base: main's tip above the lane's base

        def rewrite(n):
            if n == 1:  # that tip is replaced (docs only) and force-pushed
                (self.main / "docs").mkdir(exist_ok=True)
                (self.main / "docs" / "r.md").write_text("r\n", newline="\n")
                sh("add", "-A", cwd=self.main)
                sh("commit", "-q", "--amend", "-m", "y rewritten", cwd=self.main)
                sh("push", "-q", "-f", "origin", "main", cwd=self.main)
        seen = self._land_racing(rewrite)
        self.assertEqual(seen, [None, None])
        self.assertIn("main was rewritten", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), 2)
        self.assertTrue((self.main / "src" / "b.c").exists())

    def test_gated_claude_lane_regates_when_main_changes_build_inputs(self):
        self.L("s1", "open", "a")
        self.commit("a", ".claude/rules/r.md", "rule\n", "rule")  # inert, but not docs-only
        seen = self._land_racing(
            lambda n: self._main_commit("src/m.c", "int m;\n") if n == 1 else None)
        self.assertEqual(seen, [None, None])
        self.assertIn("main changed build inputs under a gated lane", self.last)
        self.assertEqual(self.last.count("$ python gate.py"), 2)
        self.assertEqual(self.main_file("census.csv"), "files,2\n")

    def test_regen_only_commit_is_dropped_and_counts_as_landed(self):
        self.L("s1", "open", "a")
        before = sh("rev-parse", "main", cwd=self.main)
        self.commit("a", "src/a.c", "int a2;\n", "edit a")  # the census does not change
        self.commit("a", "census.csv", "stale\n", "hand-edited census")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("held only regenerated files and was dropped", self.last)
        # the census-only lane commit vanished: regenerating undid it
        self.assertEqual(sh("log", "--format=%s", f"{before}..main", cwd=self.main).splitlines(),
                         ["edit a"])
        self.assertEqual(self.main_file("census.csv"), "files,1\n")
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("nothing to land", self.last)
        # a lane holding nothing but a regenerated-file edit lands as nothing at all
        self.commit("a", "census.csv", "stale\n", "hand-edited census 3")
        head = sh("rev-parse", "main", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("already in main", self.last)
        self.assertEqual(sh("rev-parse", "main", cwd=self.main), head)
        ctx = lanes.Ctx(self.main, "s1")
        self.assertEqual(lanes.unlanded(ctx, "a", "lanes/a"), [])
        self.assertEqual(self.L("s1", "land", "a"), 0, self.last)
        self.assertIn("nothing to land", self.last)

    def test_unpushed_main_commit_blocks_land(self):
        self.L("s1", "open", "a")
        self.commit("a", "src/b.c", "int b;\n")
        (self.main / "rogue.txt").write_text("x\n", newline="\n")
        sh("add", "rogue.txt", cwd=self.main)
        sh("commit", "-q", "-m", "rogue", cwd=self.main)
        self.assertEqual(self.L("s1", "land", "a"), 1, self.last)
        self.assertIn("not on origin/main", self.last)
        self.assertNotIn("rogue", sh("log", "--oneline", "origin/main", cwd=self.main))


if __name__ == "__main__":
    unittest.main(verbosity=1)
