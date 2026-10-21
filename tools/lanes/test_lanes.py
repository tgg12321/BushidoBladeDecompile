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

        def busy(ctx, lane, ld, commits, base, log):
            tip = real(ctx, lane, ld, commits, base, log)
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

        def busy(ctx, lane, ld, commits, base, log):
            (self.wt("a") / "src" / "a.c").write_text("int edited;\n", newline="\n")
            return real(ctx, lane, ld, commits, base, log)

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

        def racing(ctx, lane, wt, commits, base, log):
            tip = real(ctx, lane, wt, commits, base, log)
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
