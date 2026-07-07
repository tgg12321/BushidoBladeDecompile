import json, os, sys, tempfile, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", ".."))
from tools.grinder import grindlib as G


class TestLedgerInit(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name

    def tearDown(self):
        self.tmp.cleanup()

    def test_init_creates_ledger(self):
        d = G.init_ledger(self.root, "func_X", "text1a_c")
        self.assertTrue(os.path.isfile(os.path.join(d, "state.json")))
        self.assertTrue(os.path.isfile(os.path.join(d, "evidence.md")))
        self.assertTrue(os.path.isfile(os.path.join(d, "hypotheses.md")))
        self.assertTrue(os.path.isdir(os.path.join(d, "rejected")))
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st["func"], "func_X")
        self.assertEqual(st["session_count"], 0)
        self.assertEqual(st["origin"], "queue")

    def test_init_is_idempotent(self):
        G.init_ledger(self.root, "func_X", "text1a_c")
        G.append_evidence(self.root, "func_X", "a fact")
        G.init_ledger(self.root, "func_X", "text1a_c")  # must NOT wipe
        ev = open(os.path.join(self.root, "memory", "grind", "func_X", "evidence.md"),
                  encoding="utf-8").read()
        self.assertIn("a fact", ev)

    def test_load_missing_returns_none(self):
        self.assertIsNone(G.load_state(self.root, "nope"))


class TestValidateOutcome(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name

    def tearDown(self):
        self.tmp.cleanup()

    def good(self, **kw):
        base = {
            "result": "progress", "floor": 12, "headline": "killed h1",
            "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                            "result": "floor 14 -> 12", "verdict": "KILLED"}],
            "evidence": ["fact"], "frontier": [{"hypothesis": "h", "mechanism": "m",
                                                "next_probe": "n"}],
            "artifacts": [], "ruling_question": "",
        }
        base.update(kw)
        return base

    def test_valid_progress(self):
        ok, why = G.validate_outcome(self.good(), "structural", self.root)
        self.assertTrue(ok, why)

    def test_blocked_is_not_a_result(self):
        ok, why = G.validate_outcome(self.good(result="blocked"), "structural", self.root)
        self.assertFalse(ok)

    def test_progress_needs_measured_hypothesis(self):
        bad = self.good(hypotheses=[{"statement": "s", "mechanism": "m", "probe": "p",
                                     "result": "it seems hard", "verdict": "KILLED"}])
        ok, why = G.validate_outcome(bad, "structural", self.root)
        self.assertFalse(ok)  # no digits in result => no measurement

    def test_progress_without_hypotheses_invalid(self):
        ok, why = G.validate_outcome(self.good(hypotheses=[]), "structural", self.root)
        self.assertFalse(ok)

    def test_recon_needs_frontier_and_evidence(self):
        rec = self.good(hypotheses=[])
        ok, why = G.validate_outcome(rec, "recon", self.root)
        self.assertTrue(ok, why)
        ok, why = G.validate_outcome(self.good(hypotheses=[], frontier=[]), "recon", self.root)
        self.assertFalse(ok)

    def test_permuter_needs_existing_artifact(self):
        ok, why = G.validate_outcome(self.good(), "permuter", self.root)
        self.assertFalse(ok)  # artifacts empty
        ap = os.path.join(self.root, "perm.log")
        open(ap, "w").write("score 120\n")
        ok, why = G.validate_outcome(self.good(artifacts=["perm.log"]), "permuter", self.root)
        self.assertTrue(ok, why)

    def test_frontier_cap(self):
        f = [{"hypothesis": str(i), "mechanism": "m", "next_probe": "n"} for i in range(4)]
        ok, why = G.validate_outcome(self.good(frontier=f), "structural", self.root)
        self.assertFalse(ok)

    def test_ruling_request_needs_question(self):
        r = self.good(result="ruling-request", ruling_question="")
        ok, why = G.validate_outcome(r, "structural", self.root)
        self.assertFalse(ok)
        r["ruling_question"] = "Is X a sanctioned SOTN form?"
        ok, why = G.validate_outcome(r, "structural", self.root)
        self.assertTrue(ok, why)


class TestApplyAndLadder(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def test_apply_updates_state_and_files(self):
        o = {"result": "progress", "floor": 20, "headline": "h",
             "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                             "result": "22 -> 20", "verdict": "CONFIRMED"}],
             "evidence": ["fact1"], "frontier": [{"hypothesis": "x", "mechanism": "m",
                                                  "next_probe": "n"}],
             "artifacts": [], "ruling_question": ""}
        G.apply_outcome(self.root, "func_X", o, "structural")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st["session_count"], 1)
        self.assertEqual(st["floor_history"][-1]["floor"], 20)
        self.assertEqual(st["floor_history"][-1]["modality"], "structural")
        self.assertEqual(len(st["frontier"]), 1)
        led = os.path.join(self.root, "memory", "grind", "func_X")
        self.assertIn("fact1", open(os.path.join(led, "evidence.md"), encoding="utf-8").read())
        self.assertIn("22 -> 20", open(os.path.join(led, "hypotheses.md"), encoding="utf-8").read())

    def test_ladder(self):
        self.assertEqual(G.assign_modality(0), "recon")
        self.assertEqual(G.assign_modality(1), "structural")
        self.assertEqual(G.assign_modality(2), "structural")
        self.assertEqual(G.assign_modality(3), "permuter")
        self.assertEqual(G.assign_modality(5), "forensics")
        self.assertEqual(G.assign_modality(7), "rederive")
        self.assertEqual(G.assign_modality(9), "synthesis")
        self.assertEqual(G.assign_modality(10), "structural")  # ladder repeats
        self.assertEqual(G.assign_modality(18), "synthesis")

    def test_add_judge_constraint(self):
        G.add_judge_constraint(self.root, "func_X", "do-while(0) RA-weighting rejected")
        st = G.load_state(self.root, "func_X")
        self.assertIn("do-while(0) RA-weighting rejected", st["judge_constraints"])


if __name__ == "__main__":
    unittest.main()
