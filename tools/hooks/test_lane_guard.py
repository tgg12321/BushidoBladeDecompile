#!/usr/bin/env python3
"""Tests for tools/hooks/lane_guard.py against a fake main + registry.

  python tools/hooks/test_lane_guard.py
"""
from __future__ import annotations

import io
import json
import shutil
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
        self.tmp = Path(tempfile.mkdtemp(prefix="lane-guard-"))
        self.main = self.tmp / "main"
        (self.main / "tools" / "lanes").mkdir(parents=True)
        (self.main / "tmp" / "lanes-state").mkdir(parents=True)
        (self.main / "tools" / "lanes" / "lanes.json").write_text(json.dumps(
            {"worktree_root": "../wts", "dir_prefix": "ln-", "stale_hours": 6}))
        now = time.time()
        reg = {"lanes": {
            "naming-a": {"owner": "S1", "heartbeat": now, "claims": ["topic:row:m09"]},
            "fake": {"owner": "S2", "heartbeat": now, "claims": ["path:src/main/9F9C.c"]},
            "old": {"owner": "S3", "heartbeat": now - 10 * 3600, "claims": ["path:src/main/43500.c"]},
        }}
        (self.main / "tmp" / "lanes-state" / "registry.json").write_text(json.dumps(reg))
        self._orig = lane_guard.MAIN
        lane_guard.MAIN = self.main
        self.wts = (self.tmp / "wts").resolve()

    def tearDown(self):
        lane_guard.MAIN = self._orig
        shutil.rmtree(self.tmp, ignore_errors=True)

    def run_hook(self, sid, tool, **ti):
        payload = {"session_id": sid, "tool_name": tool, "tool_input": ti, "cwd": str(self.main)}
        sys.stdin = io.StringIO(json.dumps(payload))
        err = io.StringIO()
        with redirect_stderr(err):
            rc = lane_guard.main()
        sys.stdin = sys.__stdin__
        return rc

    def test_edit_own_lane_ok(self):
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/a.c")), 0)

    def test_edit_other_lane_blocked(self):
        self.assertEqual(self.run_hook("S2", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/a.c")), 2)
        self.assertEqual(self.run_hook("NEW", "Write", file_path=str(self.wts / "ln-fake" / "x.c")), 2)

    def test_edit_claimed_by_other_blocked(self):
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/9F9C.c")), 2)
        # a stale lane's claims no longer bind
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/43500.c")), 0)

    def test_lane_owner_editing_main_blocked(self):
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.main / "src/main/a.c")), 2)
        self.assertEqual(self.run_hook("S1", "Write", file_path=str(self.main / "tmp/x.py")), 0)
        self.assertEqual(self.run_hook("S1", "Edit", file_path="src/main/a.c"), 2)  # relative = main

    def test_non_lane_session_unaffected(self):
        self.assertEqual(self.run_hook("OTHER", "Edit", file_path=str(self.main / "src/main/a.c")), 0)
        self.assertEqual(self.run_hook("OTHER", "Bash", command="git commit -F tmp/m.txt"), 0)
        self.assertEqual(self.run_hook("OTHER", "Edit", file_path=str(self.wts.parent / "apply" / "a.c")), 0)

    def test_shell(self):
        wt = str(self.wts / "ln-naming-a").replace("\\", "/")
        self.assertEqual(self.run_hook("S1", "Bash", command=f'git -C "{wt}" commit -F tmp/m.txt'), 0)
        self.assertEqual(self.run_hook("S2", "Bash", command=f'git -C "{wt}" commit -F tmp/m.txt'), 2)
        self.assertEqual(self.run_hook("S2", "Bash", command=f'git -C "{wt}" log --oneline -3'), 0)
        self.assertEqual(self.run_hook("S2", "Bash", command=f'cat "{wt}/src/main/a.c"'), 0)
        self.assertEqual(self.run_hook("S1", "Bash", command="git commit -F tmp/m.txt"), 2)
        self.assertEqual(self.run_hook("S1", "Bash", command="git status --short"), 0)
        self.assertEqual(self.run_hook("S2", "PowerShell", command="python tools/lanes/lanes.py land fake"), 0)
        wsl = "/mnt/c/x/wts/ln-naming-a"
        self.assertEqual(self.run_hook("S2", "Bash", command=f"cd '{wsl}' && make"), 2)

    def test_review_probes(self):
        main = str(self.main)
        wt = str(self.wts / "ln-naming-a")
        # quoted main path with spaces must not hide the git subcommand
        self.assertEqual(self.run_hook("S1", "Bash", command=f'git -C "{main}" commit -F tmp/m.txt'), 2)
        self.assertEqual(self.run_hook("S1", "Bash", command=f'git -C "{main.replace(chr(92), "/")}" add -A'), 2)
        # lanes.py exemption only when the command is solely a lanes.py call
        self.assertEqual(self.run_hook("S1", "Bash", command=f'python tools/lanes/lanes.py status; git -C "{main}" reset --hard'), 2)
        self.assertEqual(self.run_hook("S1", "PowerShell", command="python tools/lanes/lanes.py land naming-a"), 0)
        # non-git writes into main by a lane owner
        self.assertEqual(self.run_hook("S1", "Bash", command="python tools/naming_wave.py --from-census --apply"), 2)
        self.assertEqual(self.run_hook("S1", "Bash", command=f'cd "{wt}" && python tools/naming_wave.py --apply'), 0)
        self.assertEqual(self.run_hook("S1", "Bash", command="cp tmp/x.c src/main/x.c"), 2)
        self.assertEqual(self.run_hook("S1", "Bash", command="echo hi > src/main/x.c"), 2)
        self.assertEqual(self.run_hook("S1", "Bash", command="python tmp/probe.py > tmp/out.txt"), 0)
        self.assertEqual(self.run_hook("S1", "Bash", command="mkdir -p tmp/scratch"), 0)
        self.assertEqual(self.run_hook("S1", "Bash", command=f'cd "{wt}" && echo x > notes.txt'), 0)
        self.assertEqual(self.run_hook("S1", "PowerShell", command="& tools/wteng.ps1 main verify-oracle --rebuild"), 2)
        self.assertEqual(self.run_hook("S1", "PowerShell", command=f'& tools/wteng.ps1 "{wt}" verify-oracle --rebuild'), 0)
        # read-only commands by others don't false-block
        self.assertEqual(self.run_hook("OTHER", "PowerShell", command=f'git -C "{wt}" log -5 2>$null'), 0)
        self.assertEqual(self.run_hook("OTHER", "Bash", command=f'rg -n "make" "{wt}\\Makefile" 2>&1'), 0)
        self.assertEqual(self.run_hook("OTHER", "Bash", command=f'echo x > "{wt}/a.txt"'), 2)
        # case variants
        self.assertEqual(self.run_hook("S2", "Bash", command=f'git -C "{wt.upper()}" commit -m x'), 2)
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.wts / "ln-naming-a" / "src/main/9f9c.c")), 2)

    def test_stale_owner_still_guarded(self):
        reg = json.loads((self.main / "tmp" / "lanes-state" / "registry.json").read_text())
        reg["lanes"]["naming-a"]["heartbeat"] = 0
        (self.main / "tmp" / "lanes-state" / "registry.json").write_text(json.dumps(reg))
        self.assertEqual(self.run_hook("S1", "Edit", file_path=str(self.main / "src/main/a.c")), 2)


if __name__ == "__main__":
    unittest.main(verbosity=1)
