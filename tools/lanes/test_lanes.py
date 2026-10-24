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
import unittest
from contextlib import redirect_stderr, redirect_stdout
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
    "inert_paths": ["docs/", "memory/", ".claude/"],
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

    def test_claims(self):
        self.L("s1", "open", "a"); self.L("s2", "open", "b")
        self.assertEqual(self.L("s1", "claim", "a", "path:src/*.c", "topic:row:r1"), 0, self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:src/a.c"), 1)
        self.assertEqual(self.L("s2", "claim", "b", "topic:row:r1"), 1)
        self.assertEqual(self.L("s2", "claim", "b", "topic:row:r2"), 0, self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:shared.txt"), 1)
        self.assertIn("shared path", self.last)
        self.assertEqual(self.L("s2", "claim", "b", "path:shared.txt!"), 0, self.last)

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

    def _inputs_regen(self, cmd="python gen_events.py", initial="stale\n"):
        self._main_commit("docs/runtime/events.tsv", initial)
        self._main_commit("gen_events.py", "from pathlib import Path\n"
                          "Path('docs/runtime/events.tsv').write_text('fresh\\n')\n")
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
