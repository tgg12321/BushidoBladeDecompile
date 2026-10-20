#!/usr/bin/env python3
"""Tests for tools/hooks/lane_guard.py against a fake main + registry.

  python tools/hooks/test_lane_guard.py
"""
from __future__ import annotations

import io
import json
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from contextlib import redirect_stderr
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lane_guard  # noqa: E402


class GuardTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="lane guard "))  # a space, like the real main
        self.main = self.tmp / "main repo"
        (self.main / "tools" / "lanes").mkdir(parents=True)
        (self.main / "tmp" / "lanes-state").mkdir(parents=True)
        (self.main / "src" / "main").mkdir(parents=True)
        (self.main / "src" / "main" / "a.c").write_text("int a;\n")
        (self.main / ".gitignore").write_text("tmp/\n")
        (self.main / "tools" / "lanes" / "lanes.json").write_text(json.dumps(
            {"worktree_root": "../wts", "dir_prefix": "ln-", "stale_hours": 12}))
        for args in (["init", "-q"], ["add", "-A"],
                     ["-c", "user.email=t@e", "-c", "user.name=t", "commit", "-q", "-m", "i"]):
            subprocess.run(["git", *args], cwd=self.main, check=True, capture_output=True)
        now = time.time()
        self.reg = {"lanes": {
            "naming-a": {"owner": "S1", "heartbeat": now, "claims": ["topic:row:m09"]},
            "fake": {"owner": "S2", "heartbeat": now, "claims": ["path:src/main/9F9C.c"]},
            "old": {"owner": "S3", "heartbeat": now - 30 * 3600, "claims": ["path:src/main/43500.c"]},
        }}
        self.save()
        self._orig = lane_guard.MAIN
        lane_guard.MAIN = self.main
        self.wts = (self.tmp / "wts").resolve()
        self.wt = str(self.wts / "ln-naming-a")
        self.m = str(self.main)

    def save(self):
        (self.main / "tmp" / "lanes-state" / "registry.json").write_text(json.dumps(self.reg))

    def tearDown(self):
        lane_guard.MAIN = self._orig
        def rm_readonly(func, path, _exc):
            import os
            os.chmod(path, 0o700)  # git objects are read-only on Windows
            func(path)
        shutil.rmtree(self.tmp, onerror=rm_readonly)  # the test's own temp repo

    def hook(self, sid, tool, post=False, **ti):
        payload = {"session_id": sid, "tool_name": tool, "tool_input": ti, "cwd": self.m}
        sys.stdin = io.StringIO(json.dumps(payload))
        argv, sys.argv = sys.argv, ["lane_guard.py"] + (["--post"] if post else [])
        err = io.StringIO()
        try:
            with redirect_stderr(err):
                rc = lane_guard.main()
        finally:
            sys.stdin, sys.argv = sys.__stdin__, argv
        self.err = err.getvalue()
        return rc

    def sh(self, sid, cmd, tool="Bash"):
        return self.hook(sid, tool, command=cmd)

    # ---- edits
    def test_edits(self):
        lane_file = str(self.wts / "ln-naming-a" / "src/main/a.c")
        self.assertEqual(self.hook("S1", "Edit", file_path=lane_file), 0)
        self.assertEqual(self.hook("S2", "Edit", file_path=lane_file), 2)
        self.assertEqual(self.hook("NEW", "Write", file_path=str(self.wts / "ln-fake" / "x.c")), 2)
        # another live lane's claim binds (case-insensitively); a stale lane's does not
        self.assertEqual(self.hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/9F9C.c")), 2)
        self.assertEqual(self.hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/9f9c.c")), 2)
        self.assertEqual(self.hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/43500.c")), 0)
        # lane owners stay out of main, except tmp/
        self.assertEqual(self.hook("S1", "Edit", file_path=str(self.main / "src/main/a.c")), 2)
        self.assertEqual(self.hook("S1", "Edit", file_path="src/main/a.c"), 2)
        self.assertEqual(self.hook("S1", "Write", file_path=str(self.main / "tmp/msg.txt")), 0)
        # sessions without a lane are unaffected outside lane worktrees
        self.assertEqual(self.hook("OTHER", "Edit", file_path=str(self.main / "src/main/a.c")), 0)
        self.assertEqual(self.hook("OTHER", "Edit", file_path=str(self.wts.parent / "apply" / "a.c")), 0)

    def test_stale_owner_still_guarded(self):
        self.reg["lanes"]["naming-a"]["heartbeat"] = 0
        self.save()
        self.assertEqual(self.hook("S1", "Edit", file_path=str(self.main / "src/main/a.c")), 2)

    # ---- shell: what must be blocked
    def test_owner_blocked_from_main(self):
        m, wt = self.m, self.wt
        for cmd in [
            "git commit -F tmp/m.txt",
            f'git -C "{m}" commit -F tmp/m.txt',
            f'git -C "{m.replace(chr(92), "/")}" add -A',
            f'python tools/lanes/lanes.py status; git -C "{m}" reset --hard',
            f'git -C "{wt}" status; git commit -F tmp/m.txt',
            f'git -C "{wt}" log -1 && git reset --hard HEAD~1',
            f'cd "{wt}" && cd "{m}" && git add -A',
            "cd tmp && git commit -m x",
            "python tools/naming_wave.py --from-census --apply",
            "python3 tools/naming_member_apply.py tmp/pairs.txt",
            "& tools/wteng.ps1 main verify-oracle --rebuild",
            "pwsh -NoProfile -File tools/wteng.ps1 main make",
            "git stash",
            "git branch -D lanes/x",
        ]:
            self.assertEqual(self.sh("S1", cmd), 2, cmd)

    def test_others_blocked_from_lane(self):
        wt = self.wt
        for cmd in [f'git -C "{wt}" commit -m x', f'git -C "{wt.upper()}" commit -m x',
                    f'cd "{wt}" && git add -A', f'cd "/mnt/{wt[0].lower()}/{wt[3:].replace(chr(92), "/")}" && git reset --hard',
                    f'& tools/wteng.ps1 "{wt}" verify-oracle --rebuild',
                    f'cd "{wt}" && python tools/naming_wave.py --apply']:
            self.assertEqual(self.sh("S2", cmd), 2, cmd)

    # ---- shell: what must pass
    def test_owner_documented_workflow_passes(self):
        m, wt = self.m, self.wt
        for cmd in [
            f'git -C "{wt}" commit -F "{m}\\tmp\\msg.txt"',
            f'cd "{wt}" && python tools/naming_wave.py --from-census --only 0x80017200 --apply',
            f'cd "{wt}" && python3 tools/naming_wavefollow.py m09 symbol_addrs.txt',
            f'& tools/wteng.ps1 "{wt}" verify-oracle --rebuild',
            "& tools/wteng.ps1 main queue status",
            "& tools/wteng.ps1 main dossier func_80012345",
            "python tools/lanes/lanes.py land naming-a",
            f'python "{m}\\tools\\lanes\\lanes.py" status',
            "git status --short",
            "git log --oneline -5",
            f'git -C "{m}" diff --stat',
            "Set-Content -Path tmp\\m.txt -Value \"lanes: fix foo\" -Encoding utf8",
            "Out-File -FilePath tmp\\o.txt -Encoding utf8",
            "New-Item -ItemType Directory -Force tmp\\scratch",
            f'Set-Content -Path "{wt}\\notes.txt" -Value "hello world"',
            "sed -n '1,20p' src/main/a.c",
            "cat > tmp/m.txt <<'EOF'\nmake: the subject\nrm this line is data\ngit commit -m inside heredoc\nEOF",
            "@'\ngit reset --hard\nrm src\n'@ | Set-Content tmp\\m.txt",
            "python tmp/probe.py > tmp/out.txt",
        ]:
            self.assertEqual(self.sh("S1", cmd, tool="PowerShell" if "Set-Content" in cmd or "@'" in cmd else "Bash"), 0,
                             f"{cmd}\n{self.err}")

    def test_non_owner_sessions_unaffected(self):
        wt = self.wt
        for cmd in ["git commit -F tmp/m.txt", "& tools/wteng.ps1 main make", "python tools/naming_wave.py --apply",
                    f'git -C "{wt}" log -5 2>$null', f'rg -n "make" "{wt}\\Makefile" 2>&1',
                    'git commit -m "notes about bb2-worktrees/ln-pins"',
                    f'python tools/lanes/lanes.py status; git -C "{wt}" log -3']:
            self.assertEqual(self.sh("OTHER", cmd), 0, f"{cmd}\n{self.err}")

    def test_third_review_probes(self):
        m, wt = self.m, self.wt
        wsl_m = "/mnt/" + m[0].lower() + "/" + m[3:].replace(chr(92), "/")
        for cmd in [f"wsl git -C '{wsl_m}' commit -m x",
                    f'bash -c "cd \'{wsl_m}\' && git commit -m x"',
                    "cmd /c git commit -m x",
                    f'cd "{wt}"; python "{m}/tools/naming_wave.py" --apply']:
            self.assertEqual(self.sh("S1", cmd), 2, cmd)
        for cmd in ['$wt="x"; git -C $wt commit -m x', 'git -C "$WT" add -A',
                    f'python "{wt}/tools/naming_wave.py" --apply',
                    f'python tools/format.py "{wt}/src/main/a.c"']:
            self.assertEqual(self.sh("S1", cmd), 0, f"{cmd}\n{self.err}")

    # ---- post check
    def test_post_reports_commit_untracked_and_redirty(self):
        def run(cmd, act):
            self.assertEqual(self.hook("S1", "Bash", command=cmd, tool_use_id="t1"), 0)
            act()
            return self.hook("S1", "Bash", post=True, command=cmd, tool_use_id="t1")

        def commit_main():
            (self.main / "src" / "main" / "a.c").write_text("int c;\n")
            subprocess.run(["git", "-c", "user.email=t@e", "-c", "user.name=t", "commit", "-qam", "r"],
                           cwd=self.main, check=True)
        self.assertEqual(run("echo", commit_main), 2)
        self.assertIn("HEAD moved", self.err)
        self.assertEqual(run("echo", lambda: (self.main / "src" / "new.c").write_text("x")), 2)
        self.assertIn("src/new.c", self.err)
        time.sleep(0.05)
        self.assertEqual(run("echo", lambda: (self.main / "src" / "new.c").write_text("xyz")), 2)
        self.assertEqual(run("echo", lambda: None), 0)
        self.assertNotIn("restore", self.err)

    def test_post_detects_new_main_changes(self):
        self.assertEqual(self.sh("S1", "cp tmp/x.c src/main/a.c"), 0)  # not parseable as a write
        (self.main / "src" / "main" / "a.c").write_text("int b;\n")     # ...but it wrote main
        self.assertEqual(self.hook("S1", "Bash", post=True, command="cp tmp/x.c src/main/a.c"), 2)
        self.assertIn("src/main/a.c", self.err)
        # pre-existing dirt is not blamed on the next command
        self.assertEqual(self.sh("S1", "ls"), 0)
        self.assertEqual(self.hook("S1", "Bash", post=True, command="ls"), 0)
        # sessions without a lane are never post-checked
        self.assertEqual(self.hook("OTHER", "Bash", post=True, command="ls"), 0)


if __name__ == "__main__":
    unittest.main(verbosity=1)
