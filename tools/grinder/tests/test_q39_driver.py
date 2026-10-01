"""Owner ruling Q39 — the grinder driver's layer-2 record wiring (grind.ps1).

Run: python3 tools/grinder/tests/test_q39_driver.py   (or -m unittest)

The driver is PowerShell with no harness that can run a FINAL CALL, so these
pin the wiring STRUCTURALLY, on the script's text, in execution order: the
hash and full definition are taken before the Judge sees the body; a Judge
PASS is recorded against that hash before `queue done`, and a refused record
is its own outcome (never a mislabelled queue-done refusal); a Judge FAIL is
recorded too, after the checkout (so the revert cannot undo it); and the
record survives the ledger deletion after a Match.
"""
import os
import re
import unittest

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
GRIND = os.path.join(ROOT, "tools", "grinder", "grind.ps1")


def _code(text):
    """grind.ps1 without `#` comment lines, so prose cannot satisfy a check."""
    return "\n".join(l for l in text.splitlines() if not l.lstrip().startswith("#"))


class Q39DriverWiring(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with open(GRIND, encoding="utf-8") as fh:
            cls.src = _code(fh.read())
        s = cls.src
        # the FINAL CALL stage of Invoke-CandidatePath (its one `$diff =`)
        cls.final = s[s.index("$diff = (git -C $Root diff -- \"src/$stem.c\""):]
        cls.pass_i = cls.final.index("if ($v.verdict -eq 'PASS') {")
        cls.else_i = cls.final.index("} else {", cls.final.index("MERGED"))

    def _at(self, needle, start=0, region=None):
        region = self.final if region is None else region
        i = region.find(needle, start)
        self.assertGreaterEqual(i, 0, f"missing: {needle}")
        return i

    PASS_REC = ("$l2 = Invoke-Eng @('layer2', 'record', $func, '--verdict', 'PASS', "
                "'--expect-hash', $l2Hash,")
    FAIL_REC = ("$l2f = Invoke-Eng @('layer2', 'record', $func, '--verdict', 'FAIL', "
                "'--expect-hash', $l2Hash,")

    def test_hash_and_definition_taken_before_the_judge(self):
        judge = self._at("$v = Invoke-Judge $func $task")
        self.assertLess(self._at("@('layer2', 'hash', $func, '--file', $stem)"), judge)
        self.assertLess(self._at("@('layer2', 'show', $func, '--file', $stem)"), judge)
        task = self.final[self._at("$task = @\""):judge]
        self.assertIn("$l2Def", task, "the FINAL CALL brief must carry the full definition")

    def test_l2hash_assigned_exactly_once_before_the_judge(self):
        judge = self._at("$v = Invoke-Judge $func $task")
        assigns = [m.start() for m in re.finditer(r"\$l2Hash\s*=(?!=)", self.final)]
        self.assertEqual(len(assigns), 1, "the recorded hash must be the one the Judge saw")
        self.assertLess(assigns[0], judge)

    def test_no_single_body_is_banked_before_the_judge(self):
        judge = self._at("$v = Invoke-Judge $func $task")
        guard = self._at("if (-not $l2Hash) {")
        self.assertLess(guard, judge)
        block = self.final[guard:judge]
        self.assertIn("@('layer2', 'check', $func, '--file', $stem)", block)
        self.assertRegex(block, r"(?s)Bank-CandidateRefusal \$func 'layer2-no-single-body'.*\breturn\b")

    def test_pass_recorded_against_the_judged_hash_before_queue_done(self):
        p = self.final[self.pass_i:self.else_i]
        rec = p.find(self.PASS_REC)
        self.assertGreaterEqual(rec, 0, "PASS record must be sent with --expect-hash $l2Hash")
        qd = p.find("@('queue', 'done', $func)")
        self.assertLess(rec, qd)
        # a refused record returns before queue done, under its own ground
        fail = p[rec:qd]
        self.assertIn("'layer2-record-refused'", fail)
        self.assertRegex(fail, r"(?s)-notmatch '\"ok\"\\s\*:\\s\*true'.*Bank-CandidateRefusal.*return")

    def test_judge_fail_recorded_after_the_checkout(self):
        e = self.final[self.else_i:]
        checkout = e.find("git -C $Root checkout -- .")
        rec = e.find(self.FAIL_REC)
        self.assertGreaterEqual(checkout, 0)
        self.assertGreaterEqual(rec, 0, "FAIL record must be sent with --expect-hash $l2Hash")
        self.assertGreater(rec, checkout, "a FAIL record before the checkout would be reverted")
        self.assertLess(rec, e.find("Add-Decision $func 'final call' 'FAIL'"))

    def test_record_survives_the_ledger_deletion(self):
        # the ledger is closed by grindlib close-ledger (archives layer2.jsonl +
        # layer2_verdicts/ to _completed/<func>/ before deleting; behaviour is
        # pinned in test_grindlib.LedgerClose), after queue done and the Match
        # commit, and never by a bare Remove-Item that would drop the record
        p = self.final[self.pass_i:self.else_i]
        close = self._at("python tools/grinder/grindlib.py close-ledger . $func $bucket", region=p)
        self.assertLess(self._at("@('queue', 'done', $func)", region=p), close)
        self.assertNotIn('Remove-Item -Recurse -Force (Join-Path $Root "memory\\grind\\$func")', p)


if __name__ == "__main__":
    unittest.main()
