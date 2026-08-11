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

    def test_frontier_items_need_keys(self):
        ok, why = G.validate_outcome(self.good(frontier=[{"foo": "bar"}]), "structural", self.root)
        self.assertFalse(ok)


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


class TestBriefAndWip(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def test_brief_contains_contract(self):
        G.add_judge_constraint(self.root, "func_X", "no dead constant-holders")
        b = G.build_brief(self.root, "func_X", "structural", "OUTCOME.json")
        for token in ("func_X", "structural", "OUTCOME.json",
                      "no dead constant-holders", "candidate.c", "rejected/"):
            self.assertIn(token, b)
        # "blocked" must never be OFFERED as an outcome value. The bare word is
        # legitimate prose ("Bytes proven but blocked ONLY by...", grindlib:518)
        # — the enforcement that result="blocked" is invalid lives in
        # test_blocked_is_not_a_result / validate_outcome.
        self.assertNotIn('"blocked"', b.lower())

    def test_wip_conversion_seeds_ledger(self):
        wd = os.path.join(self.root, "memory", "wip", "func_W")
        os.makedirs(os.path.join(wd, "rejected"))
        json.dump({"scores": {"candidate_floor": 13},
                   "sessions": [{"n": 1}], "rejected_forms": ["formA"]},
                  open(os.path.join(wd, "meta.json"), "w"))
        open(os.path.join(wd, "notes.md"), "w").write("TL;DR: v9 arm-local\n")
        open(os.path.join(wd, "candidate.c"), "w").write("s32 f(void){}\n")
        open(os.path.join(wd, "rejected", "formA.c"), "w").write("x\n")
        d = G.convert_wip(self.root, "func_W", "text1b")
        st = G.load_state(self.root, "func_W")
        self.assertEqual(st["floor_history"][0]["floor"], 13)
        self.assertTrue(os.path.isfile(os.path.join(d, "candidate.c")))
        self.assertTrue(os.path.isfile(os.path.join(d, "rejected", "formA.c")))
        ev = open(os.path.join(d, "evidence.md"), encoding="utf-8").read()
        self.assertIn("v9 arm-local", ev)

    def test_brief_renders_populated_ledger(self):
        o = {"result": "progress", "floor": 9, "headline": "hl",
             "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                             "result": "10 -> 9", "verdict": "KILLED"}],
             "evidence": ["e"], "frontier": [{"hypothesis": "hy", "mechanism": "me",
                                              "next_probe": "np"}],
             "artifacts": [], "ruling_question": ""}
        G.apply_outcome(self.root, "func_X", o, "structural")
        b = G.build_brief(self.root, "func_X", "permuter", "OUT.json")
        self.assertIn("floor=9", b)
        self.assertIn("hy", b)
        self.assertIn("np", b)


GOOD_VET = """# SELF-VET — func_X
CONSTRUCTS: goto end + shared return, s32 tmp intermediate
## T1 semantic purpose: both carry a real value on every path
## T2 human-programmer: yes, this is the natural shape of the spec
## T3 GCC-internals justification: none needed; program logic explains it
## T4 permuter/search provenance: derived by hand, permuter only confirmed
## T5 family check: no forbidden family; one sanctioned family claimed below
## T6 naming-announces-intent: names are semantic (tmp holds the pitch delta)
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: mixed exit forms
  SCOPE: "deliberately mix goto endK with inline return to defeat find_cross_jump"
  PRECEDENT: src/main/psxsdk/libsnd/vs_vh.c:412
ANNOTATION-CONFORMANCE: n/a — no FAKE construct in this diff
"""


class TestSelfVet(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def write_vet(self, text):
        with open(G.self_vet_path(self.root, "func_X"), "w", encoding="utf-8") as f:
            f.write(text)

    def test_missing_self_vet_fails(self):
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("self_vet.md", why)

    def test_good_self_vet_passes(self):
        self.write_vet(GOOD_VET)
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertTrue(ok, why)

    def test_missing_a_test_answer_fails(self):
        self.write_vet(GOOD_VET.replace(
            "## T4 permuter/search provenance: derived by hand, permuter only confirmed\n", ""))
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("T4", why)

    def test_family_claim_without_scope_quote_fails(self):
        self.write_vet(GOOD_VET.replace(
            '  SCOPE: "deliberately mix goto endK with inline return to defeat find_cross_jump"\n', ""))
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("SCOPE", why)

    def test_family_claim_with_uncited_precedent_fails(self):
        self.write_vet(GOOD_VET.replace("src/main/psxsdk/libsnd/vs_vh.c:412",
                                        "SOTN does the same thing in spirit"))
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("PRECEDENT", why)

    def test_commit_hash_counts_as_a_citation(self):
        self.write_vet(GOOD_VET.replace("src/main/psxsdk/libsnd/vs_vh.c:412", "a1b2c3d4e5"))
        ok, why = G.validate_self_vet(self.root, "func_X")
        self.assertTrue(ok, why)

    def test_candidate_ready_requires_self_vet_when_func_known(self):
        o = {"result": "candidate-ready", "floor": 0, "headline": "matched",
             "hypotheses": [], "evidence": [], "frontier": [], "artifacts": []}
        ok, _ = G.validate_outcome(o, "structural", self.root, "func_X")
        self.assertFalse(ok)
        self.write_vet(GOOD_VET)
        ok, why = G.validate_outcome(o, "structural", self.root, "func_X")
        self.assertTrue(ok, why)
        # back-compat: no func => the gate cannot run and must not fire
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)


class TestBannedConstructs(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")
        with open(G.self_vet_path(self.root, "func_X"), "w", encoding="utf-8") as f:
            f.write(GOOD_VET)

    def tearDown(self):
        self.tmp.cleanup()

    def test_clean_vet_passes(self):
        G.add_banned_construct(self.root, "func_X", "unused local array frame coercion")
        ok, why = G.check_banned_constructs(self.root, "func_X")
        self.assertTrue(ok, why)

    def test_redeclared_banned_construct_is_rejected(self):
        G.add_banned_construct(self.root, "func_X", "shared return label with goto end accumulator")
        ok, why = G.check_banned_constructs(self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("BANNED", why)

    def test_candidate_ready_rejected_on_banned_construct(self):
        G.add_banned_construct(self.root, "func_X", "shared return label with goto end accumulator")
        o = {"result": "candidate-ready", "floor": 0, "headline": "matched",
             "hypotheses": [], "evidence": [], "frontier": [], "artifacts": []}
        ok, why = G.validate_outcome(o, "structural", self.root, "func_X")
        self.assertFalse(ok)
        self.assertIn("BANNED", why)

    def test_ban_is_deduplicated(self):
        G.add_banned_construct(self.root, "func_X", "dead conditional store")
        G.add_banned_construct(self.root, "func_X", "dead conditional store")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st["banned_constructs"], ["dead conditional store"])

    def test_paperwork_prose_ban_is_refused(self):
        # The func_800401CC deadlock (2026-08-11): a layer-1 evidence[].construct
        # field carried prose about the vet's PAPERWORK; banking it made every
        # format-valid vet auto-discard (the mandatory ANNOTATION-CONFORMANCE:
        # line alone supplied the 2 matching terms). Such entries must be refused.
        banked = G.add_banned_construct(
            self.root, "func_X", "Annotation-conformance claim ('One FAKE construct')")
        self.assertFalse(banked)
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st.get("banned_constructs") or [], [])
        # and a genuine construct ban is still accepted
        self.assertTrue(G.add_banned_construct(self.root, "func_X", "dead conditional store"))

    def test_template_collision_examples(self):
        # Bans built from checklist-heading vocabulary would trip every vet.
        for prose in ("sanctioned family claims mismatch",
                      "GCC-internals justification invoked for the construct",
                      "T4 permuter provenance concealed"):
            self.assertFalse(G.add_banned_construct(self.root, "func_X", prose), prose)
        # Real construct descriptions survive.
        for real in ("unused local array frame coercion",
                     "register asm pin on the loop counter",
                     "volatile cast on game-state global D_80101E70"):
            self.assertTrue(G.add_banned_construct(self.root, "func_X", real), real)


class TestModalityRouting(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def bump_sessions(self, n):
        st = G.load_state(self.root, "func_X")
        st["session_count"] = n
        G.save_state(self.root, "func_X", st)

    def test_advance_modality_changes_the_rung(self):
        self.bump_sessions(3)   # ladder index 2 => 'permuter'
        st = G.load_state(self.root, "func_X")
        before = G.assign_modality(3, st)
        after = G.advance_modality(self.root, "func_X")
        self.assertNotEqual(before, after)
        st = G.load_state(self.root, "func_X")
        self.assertEqual(G.assign_modality(3, st), after)

    def test_ladder_skip_persists_across_sessions(self):
        self.bump_sessions(1)
        G.advance_modality(self.root, "func_X")
        st = G.load_state(self.root, "func_X")
        self.assertGreaterEqual(int(st["ladder_skip"]), 1)

    def test_pending_fixup_short_circuits_to_annotation_fix(self):
        self.bump_sessions(4)
        G.set_pending_fixup(self.root, "func_X", "annotation", "FAKE comment lacks a mechanism")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(G.assign_modality(4, st), "annotation-fix")
        G.clear_pending_fixup(self.root, "func_X")
        st = G.load_state(self.root, "func_X")
        self.assertNotEqual(G.assign_modality(4, st), "annotation-fix")

    def test_annotation_fix_brief_is_tiny_scope(self):
        G.set_pending_fixup(self.root, "func_X", "annotation", "FAKE comment lacks a mechanism")
        b = G.build_brief(self.root, "func_X", "annotation-fix", "OUT.json")
        self.assertIn("NO NEW CONSTRUCTS", b)
        self.assertIn("FAKE comment lacks a mechanism", b)

    def test_annotation_fix_progress_needs_only_evidence(self):
        o = {"result": "progress", "floor": 4, "headline": "annotation could not be written",
             "hypotheses": [], "evidence": ["no exhaustion exists to cite"],
             "frontier": [], "artifacts": []}
        ok, why = G.validate_outcome(o, "annotation-fix", self.root, "func_X")
        self.assertTrue(ok, why)
        o["evidence"] = []
        ok, _ = G.validate_outcome(o, "annotation-fix", self.root, "func_X")
        self.assertFalse(ok)

    def test_banned_constructs_are_loud_in_the_brief(self):
        G.add_banned_construct(self.root, "func_X", "dead conditional store")
        b = G.build_brief(self.root, "func_X", "structural", "OUT.json")
        self.assertIn("BANNED CONSTRUCTS", b)
        self.assertIn("dead conditional store", b)

    def test_brief_carries_the_self_vet_requirement(self):
        b = G.build_brief(self.root, "func_X", "structural", "OUT.json")
        self.assertIn("self_vet.md", b)


if __name__ == "__main__":
    unittest.main()
