#!/usr/bin/env python3
"""Stdlib-only tests for main_reintegration_lock's timestamp handling."""
from __future__ import annotations

import io
import json
import math
import sys
import tempfile
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import main_reintegration_lock as hook  # noqa: E402


class LockAgeTest(unittest.TestCase):
    def test_ages_and_block_messages(self):
        now = 20000
        cases = [
            ({"ts": now - 60}, 60),
            ({"ts": now - hook.STALE_SECS - 60}, hook.STALE_SECS + 60),
            ({"ts": str(now - 60)}, 60),
            ({"ts": str(now - hook.STALE_SECS - 60)}, hook.STALE_SECS + 60),
            ({"ts": now - hook.STALE_SECS}, hook.STALE_SECS),
            ({"ts": now + 60}, -60),
            ({"ts": now + hook.LOCK_CLOCK_SKEW_SECS}, -hook.LOCK_CLOCK_SKEW_SECS),
            ({"ts": now + hook.LOCK_CLOCK_SKEW_SECS + 1}, math.inf),
            ({"ts": 1.7e12}, math.inf),
            ({"ts": 1e300}, math.inf),
            ({}, math.inf),
        ]
        cases += [({"ts": value}, math.inf)
                  for value in ("abc", "nan", "inf", None, True, False,
                                math.nan, math.inf, -math.inf)]
        for lock, expected in cases:
            with self.subTest(lock=lock), mock.patch.object(hook.time, "time", return_value=now):
                self.assertEqual(hook._lock_age(lock), expected)
                err = io.StringIO()
                with redirect_stderr(err):
                    self.assertEqual(hook.block("caller", lock, "mutation"), 2)
                text = err.getvalue()
                self.assertEqual("steal -Reason" in text, expected > hook.STALE_SECS)
                self.assertEqual("ts is unreadable" in text, math.isinf(expected))
                if math.isfinite(expected):
                    self.assertIn(f"held {int(expected / 60)} min", text)

    def test_absent_lock_blocks_without_steal_hint(self):
        err = io.StringIO()
        with redirect_stderr(err):
            self.assertEqual(hook.block("caller", None, "mutation"), 2)
        self.assertIn("Acquire it first", err.getvalue())
        self.assertNotIn("steal -Reason", err.getvalue())

    def test_non_dict_lock_files_block_in_main(self):
        with tempfile.TemporaryDirectory() as root:
            path = Path(root) / hook.LOCK_REL
            path.parent.mkdir(parents=True)
            payload = {"tool_name": "Bash", "tool_input": {"command": "git merge topic"},
                       "session_id": "caller", "cwd": root}
            for value in ([], "x", 3):
                with self.subTest(value=value):
                    path.write_text(json.dumps(value), encoding="utf-8", newline="\n")
                    self.assertEqual(hook.read_lock(root), {})
                    err = io.StringIO()
                    with mock.patch.object(hook, "main_repo_path", return_value=root), \
                            mock.patch.object(hook, "targeted_repo", return_value="main"), \
                            mock.patch.object(hook, "workers_live", return_value=False), \
                            mock.patch.object(hook, "_git", side_effect=AssertionError("no git")), \
                            mock.patch.object(sys, "stdin", io.StringIO(json.dumps(payload))), \
                            redirect_stderr(err):
                        self.assertEqual(hook.main(), 2)
                    self.assertIn("ts is unreadable", err.getvalue())
                    self.assertIn("steal -Reason", err.getvalue())
                    self.assertNotIn("fail-open", err.getvalue())

    def test_unreadable_or_unparseable_lock_file_is_absent(self):
        with tempfile.TemporaryDirectory() as root:
            self.assertIsNone(hook.read_lock(root))
            path = Path(root) / hook.LOCK_REL
            path.parent.mkdir(parents=True)
            path.write_text("{", encoding="utf-8", newline="\n")
            self.assertIsNone(hook.read_lock(root))


if __name__ == "__main__":
    unittest.main()
