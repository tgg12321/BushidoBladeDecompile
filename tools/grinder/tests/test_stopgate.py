"""Stop-gate tests — tools/grinder/stopgate.py.

Run: python3 tools/grinder/tests/test_stopgate.py   (or -m unittest)

The gate's job is to convert a session the driver WOULD have discarded into an
in-session repair. These pin the three properties that make it safe to leave
armed: it blocks the discard classes that were actually firing, it never taxes
a valid session, and it always exits 0 so a bug here can only fail open.
"""
import json
import os
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GATE = os.path.join(ROOT, "tools", "grinder", "stopgate.py")


class StopGateCase(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.outcome = os.path.join(self.tmp.name, "outcome.json")
        # The scope gate reads `git status` of the root it is handed, so the
        # outcome-gate cases need a CLEAN repo of their own — this project's
        # own tree is routinely dirty and would trip the scope gate first.
        self.repo = os.path.join(self.tmp.name, "repo")
        os.makedirs(self.repo)
        for a in (["init", "-q"], ["config", "user.email", "t@e"],
                  ["config", "user.name", "t"]):
            subprocess.run(["git", "-C", self.repo] + a, capture_output=True)

    def tearDown(self):
        self.tmp.cleanup()

    def gate(self, sid="s", func="func_X", modality="structural", root=None):
        env = dict(os.environ)
        env.pop("GRIND_FUNC", None)
        if func:
            env["GRIND_FUNC"] = func
        env.update({"GRIND_OUTCOME_PATH": self.outcome,
                    "GRIND_MODALITY": modality, "CLAUDE_SESSION_ID": sid})
        p = subprocess.run([sys.executable, GATE, root or self.repo],
                           capture_output=True, text=True, env=env)
        self.assertEqual(p.returncode, 0,
                         "the gate must ALWAYS exit 0 (fail open)")
        return p.stdout.strip()

    def write_outcome(self, o):
        with open(self.outcome, "w", encoding="utf-8", newline="\n") as f:
            json.dump(o, f)

    VALID = {"result": "progress", "floor": 19, "headline": "killed the hoist",
             "hypotheses": [{"statement": "hoisting 0xFF into a local closes the gap",
                             "verdict": "KILLED", "kill_scope": "instance",
                             "result": "floor rose 19 -> 21 (measured)",
                             "measured_on": "candidate.c"}],
             "frontier": []}

    def test_unarmed_outside_a_grind_session(self):
        self.assertEqual(self.gate(func=""), "")

    def test_missing_outcome_blocks(self):
        # The 'no outcome file / unparseable JSON' discard class.
        self.assertIn("have not written your outcome JSON", self.gate())

    def test_unparseable_outcome_blocks(self):
        with open(self.outcome, "w", encoding="utf-8") as f:
            f.write("{not json")
        self.assertIn("not parseable", self.gate())

    def test_invalid_outcome_blocks_and_quotes_the_validator(self):
        # The single largest discard class (34 of 61 in 2026-09-08..16).
        self.write_outcome({"result": "progress", "floor": 19,
                            "headline": "did stuff", "hypotheses": [], "frontier": []})
        out = self.gate()
        self.assertIn("would be REJECTED by the driver", out)
        self.assertIn("CONFIRMED/KILLED", out,
                      "the agent needs the validator's OWN message to act on")

    def test_valid_outcome_does_not_block(self):
        self.write_outcome(self.VALID)
        self.assertEqual(self.gate(), "",
                         "a good session must never be taxed by this gate")

    def test_retry_cap_silences_the_gate(self):
        # An agent that cannot satisfy the validator must not loop forever;
        # after the cap the driver discards exactly as it does today.
        self.write_outcome({"result": "progress", "floor": 1, "headline": "x",
                            "hypotheses": [], "frontier": []})
        for _ in range(3):
            self.assertTrue(self.gate(sid="cap"))
        self.assertEqual(self.gate(sid="cap"), "")

    def test_cap_counter_actually_increments(self):
        # Regression: _bump once computed its new value INSIDE `open(..., "w")`,
        # which truncates first — so it rewrote 1 forever and the cap never fired.
        self.write_outcome({"result": "progress", "floor": 1, "headline": "x",
                            "hypotheses": [], "frontier": []})
        counter = os.path.join(self.repo, "tmp", "grind", "stopgate.cnt.count")
        for expected in ("1", "2", "3"):
            self.gate(sid="cnt")
            with open(counter, encoding="utf-8") as f:
                self.assertEqual(f.read().strip(), expected)

    def test_scope_violation_blocks(self):
        self.write_outcome(self.VALID)
        with open(os.path.join(self.repo, "regfix.txt"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write("x\n")
        out = self.gate()
        self.assertIn("outside your allowed surface", out)
        self.assertIn("regfix.txt", out)

    def test_outcome_gate_takes_precedence_over_scope(self):
        # Both wrong: report the outcome first — it is the one the agent is
        # most likely to be mid-way through fixing.
        self.write_outcome({"result": "progress", "floor": 1, "headline": "x",
                            "hypotheses": [], "frontier": []})
        with open(os.path.join(self.repo, "regfix.txt"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write("x\n")
        self.assertIn("REJECTED by the driver", self.gate())


class ChassisCheckParseCase(unittest.TestCase):
    """The driver parses `sandbox` stdout for the chassis check. NOTHING ever
    checked that the two agreed, and they didn't: grind.ps1 grepped for a
    `"distance"` key sandbox has never emitted (it emits `"score"`), so
    $headFloor was always empty and every brief from 2026-08-18 to 2026-09-16
    read 'measurement unavailable' under a heading saying 'trust THIS number'.

    These pin the contract from both ends so it cannot rot again silently.
    """

    def test_sandbox_result_carries_the_key_the_driver_greps(self):
        sys.path.insert(0, ROOT)
        from engine import sandbox  # noqa: E402
        import inspect
        src = inspect.getsource(sandbox.sandbox_score)
        self.assertIn('"score"', src.replace("'", '"'),
                      "sandbox_score must produce a 'score' key")
        self.assertNotIn('"distance"', src.replace("'", '"'),
                         "sandbox has no 'distance' key -- if that changes, "
                         "grind.ps1's chassis-check regex must change with it")

    def test_driver_regex_matches_real_sandbox_output(self):
        import re
        grind_ps1 = os.path.join(ROOT, "tools", "grinder", "grind.ps1")
        with open(grind_ps1, encoding="utf-8") as f:
            driver = f.read()
        self.assertIn(r'''if ($sb -match '"score"\s*:\s*(\d+)')''', driver,
                      "the chassis check must grep the key sandbox emits")
        # A representative sandbox stdout, banner included.
        out = (r'[wteng] target=main  root=C:\repo' + '\n'
               '{\n  "score": 38,\n  "scorable": true,\n  "target_insns": 200,\n'
               '  "func": "_SsVmInit",\n  "file": "main"\n}\n')
        m = re.search(r'"score"\s*:\s*(\d+)', out)
        self.assertIsNotNone(m, "the regex must match real sandbox stdout")
        self.assertEqual(m.group(1), "38")
        self.assertIsNone(re.search(r'"distance"\s*:\s*(\d+)', out),
                          "the OLD regex must be shown not to match -- this is "
                          "the bug being regressed")


if __name__ == "__main__":
    unittest.main(verbosity=2)
