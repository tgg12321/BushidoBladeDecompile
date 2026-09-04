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
                            "result": "floor 14 -> 12", "verdict": "KILLED",
                            "kill_scope": "instance", "measured_on": "test chassis"}],
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
                                     "result": "it seems hard", "verdict": "KILLED",
                                     "kill_scope": "instance",
                                     "measured_on": "test chassis"}])
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
        # R2 order (owner ruling asm-until-matched, 2026-08-19): synthesis at s6
        # (index 5), before forensics/rederive. Solver rung added 2026-08-24
        # (escalation-not-parked rollout): s7 (index 6), between synthesis and
        # forensics.
        self.assertEqual(G.assign_modality(0), "recon")
        self.assertEqual(G.assign_modality(1), "structural")
        self.assertEqual(G.assign_modality(2), "structural")
        self.assertEqual(G.assign_modality(3), "permuter")
        self.assertEqual(G.assign_modality(5), "synthesis")
        self.assertEqual(G.assign_modality(6), "solver")
        self.assertEqual(G.assign_modality(7), "forensics")
        self.assertEqual(G.assign_modality(9), "rederive")
        self.assertEqual(G.assign_modality(11), "structural")  # ladder repeats
        self.assertEqual(G.assign_modality(15), "synthesis")

    def test_permuter_hard_cap(self):
        # R3: two permuter sessions ever, regardless of yield — a 3rd is never
        # mandated (measured 0 drops / 64 sessions).
        st = {"floor_history": [
            {"floor": 20, "modality": "structural"},
            {"floor": 15, "modality": "permuter"},   # yielded (15 < 20)
            {"floor": 12, "modality": "permuter"},   # yielded again
        ]}
        # session_count 3 -> LADDER index 3 = permuter slot; cap must walk past
        self.assertNotEqual(G.assign_modality(3, st), "permuter")
        # one prior YIELDING permuter does not trip the cap (zero-yield gate
        # also must not fire: 15 < 20 is a drop)
        st2 = {"floor_history": [
            {"floor": 20, "modality": "structural"},
            {"floor": 15, "modality": "permuter"},
        ]}
        self.assertEqual(G.assign_modality(3, st2), "permuter")

    def test_add_judge_constraint(self):
        G.add_judge_constraint(self.root, "func_X", "do-while(0) RA-weighting rejected")
        st = G.load_state(self.root, "func_X")
        self.assertIn("do-while(0) RA-weighting rejected", st["judge_constraints"])


class TestExhaustionEscalation(unittest.TestCase):
    MODS = ["structural", "permuter", "forensics", "rederive"]

    def hist(self, floors, mods=None, audited=True):
        mods = mods or self.MODS
        st = {"floor_history": [
            {"session": i + 1, "floor": f, "modality": mods[i % len(mods)]}
            for i, f in enumerate(floors)]}
        if audited:  # object-model gate (2026-09-03) already satisfied
            st["object_model_audited"] = 1
        return st

    def test_flat_positive_floor_escalates(self):
        st = self.hist([3] * G.ESCALATION_FLAT_SESSIONS)
        self.assertEqual(G.assign_modality(8, st), "escalation")

    # Object-model gate (2026-09-03, func_80033550 post-mortem): exhaustion
    # routes through ONE object-model audit session before escalation.
    def test_exhausted_unaudited_routes_to_object_model(self):
        st = self.hist([3] * G.ESCALATION_FLAT_SESSIONS, audited=False)
        self.assertEqual(G.assign_modality(8, st), "object-model")

    def test_object_model_not_forced_while_floor_moves(self):
        st = self.hist([5, 5, 5, 5, 5, 5, 5, 4], audited=False)
        self.assertNotIn(G.assign_modality(8, st), ("object-model", "escalation"))

    def test_object_model_outcome_requires_evidence_and_stamps_state(self):
        tmp = tempfile.TemporaryDirectory()
        try:
            root = tmp.name
            G.init_ledger(root, "func_X", "text1a_c")
            base = {"result": "progress", "floor": 3, "hypotheses": [], "frontier": []}
            ok, why = G.validate_outcome(dict(base, evidence=["no audit here"]),
                                         "object-model", root, "func_X")
            self.assertFalse(ok)
            self.assertIn("OBJECT MODEL", why)
            o = dict(base, evidence=["OBJECT MODEL: D_1 MATCHES; D_2 MISMATCH measured score 4"])
            ok, why = G.validate_outcome(o, "object-model", root, "func_X")
            self.assertTrue(ok, why)
            G.apply_outcome(root, "func_X", o, "object-model")
            st = G.load_state(root, "func_X")
            self.assertEqual(st["object_model_audited"], 1)
            # a stamped ledger at exhaustion now escalates instead of re-auditing
            st.update(self.hist([3] * G.ESCALATION_FLAT_SESSIONS, audited=False))
            st["object_model_audited"] = 1
            self.assertEqual(G.assign_modality(8, st), "escalation")
        finally:
            tmp.cleanup()

    def test_recon_without_signals_needs_no_object_model(self):
        # temp root has no asm/ -> data_model_signals degrades to no flags
        tmp = tempfile.TemporaryDirectory()
        try:
            G.init_ledger(tmp.name, "func_X", "text1a_c")
            o = {"result": "progress", "floor": 3, "frontier": [
                {"hypothesis": "h", "mechanism": "m", "next_probe": "p"}],
                "evidence": ["baseline"], "hypotheses": []}
            ok, why = G.validate_outcome(o, "recon", tmp.name, "func_X")
            self.assertTrue(ok, why)
        finally:
            tmp.cleanup()

    def test_flat_zero_floor_escalates(self):
        # 2026-08-11 `main` regression: sandbox floor 0 but a scorer-invisible
        # 2-byte residual blocked completion — flat 0 must escalate, not loop.
        st = self.hist([0] * G.ESCALATION_FLAT_SESSIONS)
        self.assertEqual(G.assign_modality(8, st), "escalation")

    def test_moving_floor_keeps_grinding(self):
        st = self.hist([5, 5, 5, 5, 5, 5, 5, 4])
        self.assertNotEqual(G.assign_modality(8, st), "escalation")

    def test_short_history_keeps_grinding(self):
        st = self.hist([0] * (G.ESCALATION_FLAT_SESSIONS - 1))
        self.assertNotEqual(G.assign_modality(7, st), "escalation")

    def test_few_modalities_keeps_grinding(self):
        st = self.hist([0] * G.ESCALATION_FLAT_SESSIONS,
                       mods=["structural", "permuter"])
        self.assertNotEqual(G.assign_modality(8, st), "escalation")

    def test_non_int_floor_never_escalates(self):
        st = self.hist(["loop body solved"] + [0] * (G.ESCALATION_FLAT_SESSIONS - 1))
        self.assertNotEqual(G.assign_modality(8, st), "escalation")


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


class TestBannedConstructTripwire(unittest.TestCase):
    """Regression cover for the 2026-08-12 false-positive discard.

    A long prose ban made mostly of domain vocabulary ('relocated', 'from',
    'inner', 'loop', 'body') matched any vet discussing the same subject — and
    discarded a session whose candidate measured sandbox 0, because its vet
    honestly said the banned construct was NOT present."""

    BAN = ("`j += 1;` relocated from after the inner-loop body to between "
           "`idx = i + j;` and `val = 1;`, chosen from an 8-variant hand-authored "
           "sweep (tmp/grind/func_800645B0/s1/sweep.py) specifically because it "
           "flips which instruction (`addu $s0,$s3,$a0` vs `li $v1,1`) cc1's "
           "first-pass scheduler and reorg.c's back-edge delay-slot steal pick first")

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1b")
        self.assertTrue(G.add_banned_construct(self.root, "func_X", self.BAN))

    def tearDown(self):
        self.tmp.cleanup()

    def vet(self, text):
        with open(G.self_vet_path(self.root, "func_X"), "w", encoding="utf-8") as f:
            f.write(text)
        return G.check_banned_constructs(self.root, "func_X")

    def test_disclaiming_the_ban_does_not_trip(self):
        ok, why = self.vet(
            "# SELF-VET\n\n"
            "CONSTRUCTS: (1) `val` reused for the constant 1 and the OR result;\n"
            "(2) canonical for-loop spelling in both loops.\n\n"
            "PRECEDENT: the session-4 layer-1 FAIL was on the relocated `j += 1;`\n"
            "statement, which is BANNED and is NOT present here: both loops are\n"
            "written in the canonical for form.\n")
        self.assertTrue(ok, why)

    def test_forensic_discussion_without_negation_does_not_trip(self):
        ok, why = self.vet(
            "CONSTRUCTS: (1) `val` reused for the constant 1.\n\n"
            "WHY THE PRIOR AXIS IS CLOSED: session 4 placed `j += 1;` between\n"
            "`idx = i + j;` and `val = 1;`, flipping addu $s0,$s3,$a0 against\n"
            "li $v1,1 in cc1's first pass; reorg.c then took the other insn.\n")
        self.assertTrue(ok, why)

    def test_genuine_redeclaration_still_trips(self):
        ok, why = self.vet(
            "CONSTRUCTS: (1) `j += 1;` relocated from after the inner-loop body to\n"
            "sit between `idx = i + j;` and `val = 1;`, which flips which instruction\n"
            "cc1's first-pass scheduler picks (addu $s0,$s3,$a0 vs li $v1,1), chosen\n"
            "from the s1 sweep.py variants because reorg.c's back-edge delay-slot\n"
            "steal then takes it.\n")
        self.assertFalse(ok)
        self.assertIn("BANNED", why)

    def test_disclaimer_cannot_launder_a_redeclaration(self):
        """Appending "this is NOT the banned construct" to an actual
        re-declaration must not buy a pass: the strip drops only the sentence
        asserting absence, never the affirmative declaration around it."""
        ok, why = self.vet(
            "CONSTRUCTS: (1) `j += 1;` relocated from after the inner-loop body to\n"
            "sit between `idx = i + j;` and `val = 1;`, which flips which instruction\n"
            "cc1's first-pass scheduler picks (addu $s0,$s3,$a0 vs li $v1,1), chosen\n"
            "from the s1 sweep.py variants because reorg.c's back-edge delay-slot\n"
            "steal then takes it.\n"
            "This is NOT the banned construct, because I spelled it differently.\n")
        self.assertFalse(ok, "a disclaimer sentence must not launder a re-declaration")
        self.assertIn("BANNED", why)

    def test_short_paraphrase_is_left_to_the_judge(self):
        """Documents a real limit rather than pretending it away: against a long
        prose ban, a brief paraphrase reproduces too few of its words to trip.
        The tripwire only short-circuits near-verbatim respelling loops; the
        default-FAIL Judge is what actually adjudicates the construct."""
        ok, _ = self.vet(
            "CONSTRUCTS: (1) `j += 1;` moved next to `idx`, flipping addu vs li.\n")
        self.assertTrue(ok)

    def test_mixed_case_heading_still_bounds_the_section(self):
        """The 19:55 recurrence on 2026-08-12: the first fix ended the section
        at the next ALL-CAPS heading and fell back to the WHOLE FILE when it
        found none, so a vet with differently-cased headings tripped again."""
        ok, why = self.vet(
            "CONSTRUCTS: (1) `val` reused for the constant 1.\n"
            "Six-test checklist:\n"
            "T3 the session-4 form put `j += 1;` relocated from after the inner-loop\n"
            "body between `idx = i + j;` and `val = 1;`, chosen from an 8-variant\n"
            "hand-authored sweep because it flips which instruction cc1's first-pass\n"
            "scheduler and reorg.c's back-edge delay-slot steal pick first.\n")
        self.assertTrue(ok, why)

    def test_blank_line_bounds_the_section(self):
        ok, why = self.vet(
            "CONSTRUCTS: (1) `val` reused for the constant 1.\n"
            "\n"
            "The prior form had `j += 1;` relocated from after the inner-loop body to\n"
            "between `idx = i + j;` and `val = 1;`, chosen from an 8-variant\n"
            "hand-authored sweep, flipping which instruction cc1's first-pass\n"
            "scheduler and reorg.c's back-edge delay-slot steal pick first.\n")
        self.assertTrue(ok, why)

    def test_vet_without_constructs_line_does_not_trip(self):
        """No declaration to check. Such a vet is format-invalid on its own —
        the tripwire must not stand in for the format gate by scanning prose."""
        ok, _ = self.vet(
            "The prior form had `j += 1;` relocated from after the inner-loop body\n"
            "to between `idx = i + j;` and `val = 1;` from the 8-variant sweep.\n")
        self.assertTrue(ok)
        # ...and the format gate is what actually rejects it, so nothing escapes.
        valid, why = G.validate_self_vet(self.root, "func_X")
        self.assertFalse(valid, "a vet with no CONSTRUCTS: line must fail the format gate")
        self.assertIn("CONSTRUCTS", why)

    def test_no_ban_banked_means_no_check(self):
        G.init_ledger(self.root, "func_Y", "text1b")
        with open(G.self_vet_path(self.root, "func_Y"), "w", encoding="utf-8") as f:
            f.write("CONSTRUCTS: anything at all\n")
        ok, _ = G.check_banned_constructs(self.root, "func_Y")
        self.assertTrue(ok)

    def test_paperwork_ban_is_still_refused_at_banking(self):
        """The func_800401CC guard: a 'ban' describing vet paperwork rather than
        a C construct would deadlock the function, so it must not be banked."""
        G.init_ledger(self.root, "func_Z", "text1b")
        banked = G.add_banned_construct(
            self.root, "func_Z",
            "Annotation-conformance claim ('One FAKE construct') in the "
            "sanctioned-family-claims line of the self-vet paperwork")
        self.assertFalse(banked)


class TestCitedRuleScopes(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        rules = os.path.join(self.root, ".claude", "rules")
        os.makedirs(rules)
        with open(os.path.join(rules, "do-while-zero-exception.md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write('---\nname: do-while-zero-exception\n'
                    'description: "SANCTIONED for ANY codegen effect"\n---\nbody\n')
        with open(os.path.join(rules, "unrelated-rule.md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write('---\nname: unrelated-rule\ndescription: "never cited"\n---\n')
        G.init_ledger(self.root, "func_X", "stem")

    def tearDown(self):
        self.tmp.cleanup()

    def test_rule_descriptions_reads_frontmatter(self):
        d = G.rule_descriptions(self.root)
        self.assertEqual(d["do-while-zero-exception"], "SANCTIONED for ANY codegen effect")
        self.assertEqual(d["unrelated-rule"], "never cited")

    def test_cited_scopes_from_rejected_header(self):
        rj = os.path.join(self.root, "memory", "grind", "func_X", "rejected", "x.c")
        with open(rj, "w", encoding="utf-8", newline="\n") as f:
            f.write("/* REJECTED: do-while-zero-exception scope is reorg.c ONLY */\n")
        got = G.cited_rule_scopes(self.root, "func_X")
        self.assertEqual(got, [("do-while-zero-exception", "SANCTIONED for ANY codegen effect")])

    def test_cited_scopes_from_state_and_hypotheses(self):
        G.add_judge_constraint(self.root, "func_X", "see unrelated-rule for why")
        G.append_hypothesis(self.root, "func_X",
                            {"statement": "uses do-while-zero-exception", "mechanism": "m",
                             "probe": "p", "result": "15", "verdict": "KILLED"}, session=1)
        slugs = [s for s, _ in G.cited_rule_scopes(self.root, "func_X")]
        self.assertEqual(slugs, ["do-while-zero-exception", "unrelated-rule"])

    def test_brief_carries_current_scope_block(self):
        G.add_judge_constraint(self.root, "func_X", "do-while-zero-exception applies")
        brief = G.build_brief(self.root, "func_X", "structural", "/tmp/o.json")
        self.assertIn("CURRENT SCOPE OF EVERY RULE THIS LEDGER CITES", brief)
        self.assertIn('do-while-zero-exception: "SANCTIONED for ANY codegen effect"', brief)

    def test_brief_omits_block_when_nothing_cited(self):
        brief = G.build_brief(self.root, "func_X", "structural", "/tmp/o.json")
        self.assertNotIn("CURRENT SCOPE OF EVERY RULE", brief)

    def test_cited_scopes_from_candidate_c(self):
        cand = os.path.join(self.root, "memory", "grind", "func_X", "candidate.c")
        with open(cand, "w", encoding="utf-8", newline="\n") as f:
            f.write("/* sanctioned narrow exception per "
                    ".claude/rules/do-while-zero-exception.md */\nvoid f(void) {}\n")
        got = G.cited_rule_scopes(self.root, "func_X")
        self.assertEqual(got, [("do-while-zero-exception", "SANCTIONED for ANY codegen effect")])

    def test_cited_scopes_from_floor_history(self):
        st = G.load_state(self.root, "func_X")
        st["floor_history"].append(
            {"session": 1, "floor": 12, "headline": "blocked by unrelated-rule scope"})
        G.save_state(self.root, "func_X", st)
        slugs = [s for s, _ in G.cited_rule_scopes(self.root, "func_X")]
        self.assertEqual(slugs, ["unrelated-rule"])

    def test_rule_without_description_is_skipped(self):
        p = os.path.join(self.root, ".claude", "rules", "no-desc-rule.md")
        with open(p, "w", encoding="utf-8", newline="\n") as f:
            f.write("---\nname: no-desc-rule\n---\nbody\n")
        self.assertNotIn("no-desc-rule", G.rule_descriptions(self.root))

    def test_rules_dir_absent_gives_empty(self):
        with tempfile.TemporaryDirectory() as bare:
            self.assertEqual(G.rule_descriptions(bare), {})
            G.init_ledger(bare, "func_Y", "stem")
            self.assertEqual(G.cited_rule_scopes(bare, "func_Y"), [])
            brief = G.build_brief(bare, "func_Y", "structural", "/tmp/o.json")
            self.assertIn("GRIND SESSION", brief)
            self.assertNotIn("CURRENT SCOPE OF EVERY RULE", brief)

    def test_unquoted_description_with_dash(self):
        p = os.path.join(self.root, ".claude", "rules", "dashy-rule.md")
        with open(p, "w", encoding="utf-8", newline="\n") as f:
            f.write("---\nname: dashy-rule\n"
                    "description: Owner ruling 2026-08-19 - scope is everything\n---\n")
        self.assertEqual(G.rule_descriptions(self.root)["dashy-rule"],
                         "Owner ruling 2026-08-19 - scope is everything")

    def test_rendered_line_carries_rule_path_and_truncates(self):
        long = "x" * 400
        out = G.render_rule_scopes([("slug-a", long)])
        self.assertIn('  - slug-a: "' + "x" * 300 + '…" (.claude/rules/slug-a.md)', out)
        short = G.render_rule_scopes([("slug-b", "short desc")])
        self.assertIn('  - slug-b: "short desc" (.claude/rules/slug-b.md)', short)


class TestKillHygiene(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "stem")

    def tearDown(self):
        self.tmp.cleanup()

    def killed(self, floor=15, **kw):
        h = {"statement": "tail duplication into 3 arms", "mechanism": "flow.c reg_n_refs",
             "probe": "sandbox", "result": "flat 15", "verdict": "KILLED",
             "kill_scope": "instance", "measured_on": "HEAD chassis, staged read L3 present"}
        h.update(kw)
        return {"result": "progress", "floor": floor, "headline": "h",
                "hypotheses": [h], "evidence": ["e"], "frontier": [], "artifacts": []}

    def test_instance_kill_valid(self):
        ok, why = G.validate_outcome(self.killed(), "structural", self.root)
        self.assertTrue(ok, why)

    def test_killed_requires_kill_scope(self):
        o = self.killed(); del o["hypotheses"][0]["kill_scope"]
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("kill_scope", why)

    def test_killed_requires_measured_on(self):
        o = self.killed(measured_on="")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("measured_on", why)

    def test_class_claim_wording_needs_class_scope(self):
        o = self.killed(statement="tail duplication is unreachable by any natural geometry")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("class-level claim", why)

    def test_ordinary_negative_wording_is_not_a_class_claim(self):
        o = self.killed(statement="A closed-form flip condition exists; the LUID tiebreak "
                                  "cannot be moved by this spelling")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_class_kill_needs_predicate_cite(self):
        o = self.killed(kill_scope="class")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("predicate_cite", why)

    def _make_cited_file(self, rel="tools/gcc-2.7.2/loop.c", lines=800):
        p = os.path.join(self.root, *rel.split("/"))
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join("l%d" % i for i in range(lines)))
        return p

    def test_class_kill_with_cite_valid(self):
        self._make_cited_file()
        o = self.killed(kill_scope="class", predicate_cite="tools/gcc-2.7.2/loop.c:705",
                        statement="no movable can pass with n_times_set != 1 — all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_class_kill_cite_must_resolve(self):
        st = "no movable can pass with n_times_set != 1 — all forms"
        o = self.killed(kill_scope="class", predicate_cite="tools/gcc-2.7.2/nope.c:12",
                        statement=st)
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("does not resolve", why)
        self._make_cited_file(lines=100)
        o = self.killed(kill_scope="class", predicate_cite="tools/gcc-2.7.2/loop.c:705",
                        statement=st)
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("does not resolve", why)

    def test_cite_bare_compiler_filename_resolves(self):
        self._make_cited_file()
        o = self.killed(kill_scope="class", predicate_cite="loop.c:705",
                        statement="no movable can pass with n_times_set != 1 — all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_cite_bare_filename_missing_fails(self):
        o = self.killed(kill_scope="class", predicate_cite="nosuch.c:12",
                        statement="no movable can pass with n_times_set != 1 — all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("does not resolve", why)

    def test_cite_bare_config_mips_filename_resolves(self):
        self._make_cited_file(rel="tools/gcc-2.7.2/config/mips/mips.c", lines=5000)
        o = self.killed(kill_scope="class", predicate_cite="mips.c:4680",
                        statement="no movable can pass with n_times_set != 1 - all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_cite_bare_project_source_resolves(self):
        self._make_cited_file(rel="src/main.c", lines=800)
        o = self.killed(kill_scope="class", predicate_cite="main.c:100",
                        statement="no movable can pass with n_times_set != 1 - all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_cite_parent_escape_rejected(self):
        o = self.killed(kill_scope="class", predicate_cite="../../x.c:1",
                        statement="no movable can pass with n_times_set != 1 - all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("does not resolve", why)

    def test_cite_bare_name_matching_directory_fails(self):
        os.makedirs(os.path.join(self.root, "tools", "gcc-2.7.2", "mips.c"),
                    exist_ok=True)
        o = self.killed(kill_scope="class", predicate_cite="mips.c:1",
                        statement="no movable can pass with n_times_set != 1 - all forms")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok); self.assertIn("does not resolve", why)

    def test_result_field_wording_does_not_trip(self):
        o = self.killed(result="floor flat 15; the arm is unreachable on this chassis")
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_kill_gate_runs_on_owner_gated(self):
        dec = os.path.join(self.root, "docs", "grind")
        os.makedirs(dec, exist_ok=True)
        with open(os.path.join(dec, "decisions.md"), "w", encoding="utf-8",
                  newline="\n") as f:
            f.write("## 2026-09-01 — func_X — OWNER-ESCALATION filed\n")
        o = self.killed()
        o["result"] = "owner-gated"
        o["escalation_ref"] = "docs/grind/decisions.md OWNER-ESCALATION func_X"
        ok, why = G.validate_outcome(o, "escalation", self.root)
        self.assertTrue(ok, why)
        del o["hypotheses"][0]["kill_scope"]
        ok, why = G.validate_outcome(o, "escalation", self.root)
        self.assertFalse(ok); self.assertIn("kill_scope", why)

    def test_brief_no_kill_ledger_when_no_kills(self):
        o = self.killed()
        o["hypotheses"][0]["verdict"] = "CONFIRMED"
        G.apply_outcome(self.root, "func_X", o, "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertNotIn("## KILL LEDGER", brief)

    def test_prose_floor_in_window_no_reaudit_and_no_crash(self):
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        G.apply_outcome(self.root, "func_X", self.killed(floor="prose floor"), "structural")
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertNotIn("KILL RE-AUDIT REQUIRED", brief)
        self.assertIn("## KILL LEDGER", brief)

    def test_kills_capped_at_60(self):
        for _ in range(65):
            G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(len(st["kills"]), 60)
        self.assertEqual(st["kills"][-1]["session"], 65)

    def test_confirmed_needs_no_scope(self):
        o = self.killed(verdict="CONFIRMED"); del o["hypotheses"][0]["kill_scope"]
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertTrue(ok, why)

    def test_apply_records_kills_structurally(self):
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(len(st["kills"]), 1)
        k = st["kills"][0]
        self.assertEqual(k["session"], 1)
        self.assertEqual(k["kill_scope"], "instance")
        self.assertEqual(k["measured_on"], "HEAD chassis, staged read L3 present")
        hyp = open(os.path.join(self.root, "memory", "grind", "func_X", "hypotheses.md"),
                   encoding="utf-8").read()
        self.assertIn("- kill_scope: instance", hyp)
        self.assertIn("- measured_on: HEAD chassis, staged read L3 present", hyp)

    def test_legacy_state_without_kills_key_still_applies(self):
        st = G.load_state(self.root, "func_X"); del st["kills"]; G.save_state(self.root, "func_X", st)
        G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        self.assertEqual(len(G.load_state(self.root, "func_X")["kills"]), 1)

    def test_brief_reaudit_block_on_flat_floor(self):
        for _ in range(3):
            G.apply_outcome(self.root, "func_X", self.killed(), "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertIn("KILL RE-AUDIT REQUIRED", brief)
        self.assertIn("tail duplication into 3 arms", brief)

    def test_brief_no_reaudit_while_floor_moves(self):
        G.apply_outcome(self.root, "func_X", self.killed(floor=20), "structural")
        G.apply_outcome(self.root, "func_X", self.killed(floor=15), "structural")
        brief = G.build_brief(self.root, "func_X", "permuter", "/tmp/o.json")
        self.assertNotIn("KILL RE-AUDIT REQUIRED", brief)
        self.assertIn("## KILL LEDGER", brief)


class TestGrantRescan(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_A", "s")
        G.init_ledger(self.root, "func_B", "s")
        rj = os.path.join(self.root, "memory", "grind", "func_A", "rejected", "inline-both-call-sites.c")
        with open(rj, "w", encoding="utf-8", newline="\n") as f:
            f.write("/* REJECTED s8: compound-address duplication across call arg-lists scores 0 but cheat-by-spelling */\n")
        G.add_banned_construct(self.root, "func_A", "duplicating the compound address expression *(s16**)(arg0+4) at each call site")
        G.add_judge_constraint(self.root, "func_B", "unrelated constraint")

    def tearDown(self):
        self.tmp.cleanup()

    def wheres(self, hits, func):
        return [h["where"] for h in hits[func]]

    def test_scan_finds_only_matching_ledger(self):
        from tools.grinder import grant_rescan as R
        hits = R.scan(self.root, ["compound-address duplication", "compound address expression"])
        self.assertEqual(sorted(hits), ["func_A"])
        w = self.wheres(hits, "func_A")
        self.assertIn("rejected/inline-both-call-sites.c", w)
        self.assertTrue(any(x.startswith("banned_constructs[0]") for x in w))
        for h in hits["func_A"]:
            self.assertIn(h["term"], ("compound-address duplication", "compound address expression"))
            self.assertLessEqual(len(h["text_snippet"]), 80)
        self.assertIn("func_A", R.format_hits(hits))

    def test_apply_injects_constraint_and_supersedes_ban(self):
        from tools.grinder import grant_rescan as R
        hits = R.scan(self.root, ["compound address expression"])
        R.apply(self.root, hits, family="F3 compound-address duplication", ref=".claude/rules/no-new-park-categories.md:377", date="2026-09-01")
        st = G.load_state(self.root, "func_A")
        self.assertEqual(st["banned_constructs"], [])
        self.assertEqual(len(st["superseded_bans"]), 1)
        self.assertIn("F3 compound-address duplication", st["superseded_bans"][0]["superseded_by"])
        self.assertTrue(any("RE-ADJUDICATE" in c for c in st["judge_constraints"]))
        self.assertTrue(all(len(c) <= 400 for c in st["judge_constraints"]))
        self.assertEqual(G.load_state(self.root, "func_B")["banned_constructs"], [])

    def test_constraint_keeps_instruction_when_hits_are_long(self):
        from tools.grinder import grant_rescan as R
        d = os.path.join(self.root, "memory", "grind", "func_A", "rejected")
        for n in range(3):
            fn = ("s%d-a-very-long-banked-form-slug-describing-the-compound-address-"
                  "duplication-attempt-with-plenty-of-words-%d.c" % (n, n))
            with open(os.path.join(d, fn), "w", encoding="utf-8", newline="\n") as f:
                f.write("/* compound address expression duplicated at each call site */\n")
        hits = R.scan(self.root, ["compound address expression"])
        R.apply(self.root, hits,
                family="F3 compound-address duplication across call argument lists",
                ref=".claude/rules/no-new-park-categories.md:377", date="2026-09-01")
        cs = [c for c in G.load_state(self.root, "func_A")["judge_constraints"]
              if "RE-ADJUDICATE" in c]
        self.assertEqual(len(cs), 1)
        c = cs[0]
        self.assertLessEqual(len(c), 400)
        self.assertIn("Restore the matching banked form", c)
        self.assertIn("SCOPE quoted verbatim", c)
        self.assertIn("moved to superseded_bans", c)

    def test_supersede_bans_is_case_insensitive_and_preserves_others(self):
        G.add_banned_construct(self.root, "func_A", "some other construct")
        moved = G.supersede_bans(self.root, "func_A", ["COMPOUND ADDRESS"], "grant X")
        self.assertEqual(len(moved), 1)
        self.assertIn("compound address expression", moved[0]["text"])
        st = G.load_state(self.root, "func_A")
        self.assertEqual(st["banned_constructs"], ["some other construct"])
        self.assertEqual(st["superseded_bans"][0]["superseded_by"], "grant X")

    def test_brief_shows_superseded_bans(self):
        G.supersede_bans(self.root, "func_A", ["compound address"], "grant X (2026-09-01)")
        brief = G.build_brief(self.root, "func_A", "structural", "/tmp/o.json")
        self.assertIn("SUPERSEDED BANS", brief)
        self.assertIn("grant X (2026-09-01)", brief)

    def test_preview_superseded_bans(self):
        pv = G.preview_superseded_bans(self.root, "func_A", ["compound address"])
        self.assertEqual(len(pv), 1)
        self.assertEqual(G.preview_superseded_bans(self.root, "func_B", ["compound address"]), [])

    def test_term_with_apostrophe_does_not_broaden_needle(self):
        from tools.grinder import grant_rescan as R
        G.init_ledger(self.root, "func_C", "s")
        G.add_banned_construct(self.root, "func_C", "reusing the caller's frame slot for the temp")
        G.add_banned_construct(self.root, "func_C", "caller-saved register pinning")
        hits = R.scan(self.root, ["caller's frame"])
        self.assertEqual(sorted(hits), ["func_C"])
        self.assertEqual([h["term"] for h in hits["func_C"]], ["caller's frame"])
        R.apply(self.root, hits, family="F9 frame reuse", ref="docs/x.md:1", date="2026-09-01")
        st = G.load_state(self.root, "func_C")
        # the apostrophe must NOT have split the needle into a broad "caller"
        self.assertEqual(st["banned_constructs"], ["caller-saved register pinning"])
        self.assertEqual(len(st["superseded_bans"]), 1)

    def test_scan_skips_dir_without_state_json(self):
        from tools.grinder import grant_rescan as R
        d = os.path.join(self.root, "memory", "grind", "not_a_ledger")
        os.makedirs(os.path.join(d, "rejected"))
        with open(os.path.join(d, "rejected", "x.c"), "w", encoding="utf-8", newline="\n") as f:
            f.write("compound address expression\n")
        hits = R.scan(self.root, ["compound address expression"])
        self.assertEqual(sorted(hits), ["func_A"])

    def test_bans_only_filters_soft_hits(self):
        from tools.grinder import grant_rescan as R
        with open(os.path.join(self.root, "memory", "grind", "func_B", "evidence.md"),
                  "a", encoding="utf-8", newline="\n") as f:
            f.write("\nthe compound address expression appears only in prose here\n")
        hits = R.scan(self.root, ["compound address expression"])
        self.assertEqual(sorted(hits), ["func_A", "func_B"])
        self.assertEqual(sorted(R.filter_bans_only(hits)), ["func_A"])

    def test_apply_unparks_foreclosed_queue_item(self):
        from tools.grinder import grant_rescan as R
        os.makedirs(os.path.join(self.root, "engine"), exist_ok=True)
        qp = os.path.join(self.root, "engine", "queue.json")
        with open(qp, "w", encoding="utf-8", newline="\n") as f:
            json.dump({"items": [
                {"func": "func_A", "status": "foreclosed", "file": "s", "distance": 3, "verdict": "C"},
                {"func": "func_B", "status": "active", "file": "s", "distance": 4, "verdict": "C"}]}, f)
        hits = R.scan(self.root, ["compound address expression"])
        fails = R.apply(self.root, hits, family="F3", ref="r:1", date="2026-09-01")
        self.assertEqual(fails, 0)
        items = {i["func"]: i for i in json.load(open(qp, encoding="utf-8"))["items"]}
        self.assertEqual(items["func_A"]["status"], "active")
        self.assertEqual(items["func_B"]["status"], "active")
        self.assertNotIn("unpark_reason", items["func_B"])

    def test_run_refuses_apply_over_max_hits(self):
        from tools.grinder import grant_rescan as R
        G.init_ledger(self.root, "func_D", "s")
        G.add_banned_construct(self.root, "func_D", "the compound address expression again")
        rc = R.run(["--term", "compound address expression", "--apply", "--family", "F3",
                    "--ref", "r:1", "--max-hits", "1"], root=self.root)
        self.assertEqual(rc, 2)
        # nothing mutated
        self.assertEqual(len(G.load_state(self.root, "func_A")["banned_constructs"]), 1)
        rc = R.run(["--term", "compound address expression", "--apply", "--family", "F3",
                    "--ref", "r:1", "--max-hits", "1", "--yes"], root=self.root)
        self.assertEqual(rc, 0)
        self.assertEqual(G.load_state(self.root, "func_A")["banned_constructs"], [])

    def test_run_only_restricts_application(self):
        from tools.grinder import grant_rescan as R
        G.init_ledger(self.root, "func_D", "s")
        G.add_banned_construct(self.root, "func_D", "the compound address expression again")
        rc = R.run(["--term", "compound address expression", "--apply", "--family", "F3",
                    "--ref", "r:1", "--only", "func_D"], root=self.root)
        self.assertEqual(rc, 0)
        self.assertEqual(G.load_state(self.root, "func_D")["banned_constructs"], [])
        self.assertEqual(len(G.load_state(self.root, "func_A")["banned_constructs"]), 1)

    def test_run_rejects_bad_date(self):
        from tools.grinder import grant_rescan as R
        rc = R.run(["--term", "compound address expression", "--apply", "--family", "F3",
                    "--ref", "r:1", "--date", "09/01/2026"], root=self.root)
        self.assertEqual(rc, 2)

    def test_run_refuses_apply_while_grinder_lock_live(self):
        from tools.grinder import grant_rescan as R
        os.makedirs(os.path.join(self.root, "tmp", "grind"), exist_ok=True)
        with open(os.path.join(self.root, "tmp", "grind", "grind.lock"),
                  "w", encoding="utf-8", newline="\n") as f:
            f.write(str(os.getpid()))
        rc = R.run(["--term", "compound address expression", "--apply", "--family", "F3",
                    "--ref", "r:1"], root=self.root)
        self.assertEqual(rc, 2)
        self.assertEqual(len(G.load_state(self.root, "func_A")["banned_constructs"]), 1)
        # a dry run is always allowed
        self.assertEqual(R.run(["--term", "compound address expression"], root=self.root), 0)

    def test_dry_run_prints_would_supersede(self):
        from tools.grinder import grant_rescan as R
        import io, contextlib
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            R.run(["--term", "compound address expression"], root=self.root)
        out = buf.getvalue()
        self.assertIn("WOULD SUPERSEDE:", out)
        self.assertIn("compound address expression", out)
        # func_B has no hits at all here, so only func_A is reported
        buf2 = io.StringIO()
        G.append_evidence(self.root, "func_B", "compound address expression in prose only")
        with contextlib.redirect_stdout(buf2):
            R.run(["--term", "compound address expression"], root=self.root)
        self.assertIn("(no bans affected)", buf2.getvalue())


class TestForeclosureMechanics(unittest.TestCase):
    """Owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): an unpark
    resets the exhaustion window; standing-ruling foreclosure is scoped to the
    endgame-lock floor (<= ENDGAME_LOCK_MAX_FLOOR); a spent probe is progress."""
    MODS6 = ["structural", "permuter", "synthesis", "solver", "forensics", "rederive"]

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name

    def tearDown(self):
        self.tmp.cleanup()

    def hist(self, floors, mods=None, base=None):
        mods = mods or self.MODS6
        st = {"session_count": len(floors), "floor_history": [
            {"session": i + 1, "floor": f, "modality": mods[i % len(mods)]}
            for i, f in enumerate(floors)],
            "object_model_audited": 1}  # gate (2026-09-03) satisfied
        if base is not None:
            st["exhaustion_base"] = base
        return st

    # Ruling 1 — the window restarts at exhaustion_base
    def test_unpark_base_hides_prior_flat_history(self):
        st = self.hist([3] * 8, base=8)
        self.assertNotEqual(G.assign_modality(8, st), "escalation")

    def test_fresh_window_after_base_escalates_again(self):
        st = self.hist([3] * 16, base=8)
        self.assertEqual(G.assign_modality(16, st), "escalation")

    def test_partial_window_after_base_keeps_grinding(self):
        st = self.hist([3] * 15, base=8)
        self.assertNotEqual(G.assign_modality(15, st), "escalation")

    def test_sync_unpark_stamps_base_once(self):
        G.init_ledger(self.root, "func_U", "s")
        st = G.load_state(self.root, "func_U")
        st["session_count"] = 9
        st["floor_history"] = [{"session": i + 1, "floor": 2, "modality": self.MODS6[i % 6]}
                               for i in range(9)]
        G.save_state(self.root, "func_U", st)
        os.makedirs(os.path.join(self.root, "engine"), exist_ok=True)
        with open(os.path.join(self.root, "engine", "queue.json"), "w",
                  encoding="utf-8", newline="\n") as f:
            json.dump({"items": [{"func": "func_U", "status": "active", "file": "s",
                                  "distance": 2, "verdict": "C",
                                  "unpark_reason": "owner ruling 2026-09-02: probe X"}]}, f)
        self.assertTrue(G.sync_unpark(self.root, "func_U"))
        st = G.load_state(self.root, "func_U")
        self.assertEqual(st["exhaustion_base"], 9)
        self.assertEqual(st["last_unpark_reason"], "owner ruling 2026-09-02: probe X")
        self.assertNotEqual(G.assign_modality(9, st), "escalation")
        # idempotent: the same reason never re-stamps (a later session must not
        # keep pushing the base forward)
        st["session_count"] = 12
        G.save_state(self.root, "func_U", st)
        self.assertFalse(G.sync_unpark(self.root, "func_U"))
        self.assertEqual(G.load_state(self.root, "func_U")["exhaustion_base"], 9)

    def test_sync_unpark_without_directive_is_noop(self):
        G.init_ledger(self.root, "func_V", "s")
        os.makedirs(os.path.join(self.root, "engine"), exist_ok=True)
        with open(os.path.join(self.root, "engine", "queue.json"), "w",
                  encoding="utf-8", newline="\n") as f:
            json.dump({"items": [{"func": "func_V", "status": "active", "file": "s",
                                  "distance": 2, "verdict": "C"}]}, f)
        self.assertFalse(G.sync_unpark(self.root, "func_V"))
        self.assertNotIn("exhaustion_base", G.load_state(self.root, "func_V"))

    # Ruling 2 — floor > ENDGAME_LOCK_MAX_FLOOR needs a second full ladder cycle
    def test_wide_floor_does_not_escalate_at_eight(self):
        st = self.hist([G.ENDGAME_LOCK_MAX_FLOOR + 1] * 8)
        self.assertNotEqual(G.assign_modality(8, st), "escalation")

    def test_wide_floor_escalates_after_two_cycles(self):
        n = G.ESCALATION_FLAT_SESSIONS_WIDE
        st = self.hist([15] * n)
        self.assertEqual(G.assign_modality(n, st), "escalation")

    def test_wide_floor_needs_six_modalities(self):
        n = G.ESCALATION_FLAT_SESSIONS_WIDE
        st = self.hist([15] * n, mods=["structural", "permuter", "forensics", "rederive"])
        self.assertNotEqual(G.assign_modality(n, st), "escalation")

    def test_endgame_floor_keeps_eight_session_trigger(self):
        st = self.hist([G.ENDGAME_LOCK_MAX_FLOOR] * 8)
        self.assertEqual(G.assign_modality(8, st), "escalation")

    def test_autoescalate_title_follows_floor_scope(self):
        os.makedirs(os.path.join(self.root, "docs", "grind"), exist_ok=True)
        open(os.path.join(self.root, "docs", "grind", "decisions.md"), "w").close()
        G.init_ledger(self.root, "func_W", "s")
        st = G.load_state(self.root, "func_W")
        st["floor_history"] = [{"session": 1, "floor": 15, "modality": "structural"}]
        G.save_state(self.root, "func_W", st)
        ref = G.autoescalate(self.root, "func_W", "s", "LOW", 0, "2026-09-02")
        self.assertIn("LADDER EXHAUSTED (non-endgame residual, floor 15)", ref)
        self.assertNotIn("RESOLVED BY STANDING RULING", ref)
        st["floor_history"] = [{"session": 1, "floor": 2, "modality": "structural"}]
        G.save_state(self.root, "func_W", st)
        ref = G.autoescalate(self.root, "func_W", "s", "LOW", 0, "2026-09-02")
        self.assertIn("RESOLVED BY STANDING RULING (2026-07-27)", ref)

    # Ruling 3 — a foreclosure record is only valid in escalation modality (both titles)
    def test_ladder_exhausted_title_refused_outside_escalation(self):
        os.makedirs(os.path.join(self.root, "docs", "grind"), exist_ok=True)
        with open(os.path.join(self.root, "docs", "grind", "decisions.md"), "w",
                  encoding="utf-8", newline="\n") as f:
            f.write("## x — func_Z — OWNER-ESCALATION — LADDER EXHAUSTED (non-endgame residual, floor 15): FORECLOSED\n")
        o = {"result": "owner-gated", "escalation_ref": "func_Z — LADDER EXHAUSTED (non-endgame residual, floor 15): FORECLOSED"}
        ok, why = G.validate_outcome(o, "structural", self.root)
        self.assertFalse(ok)
        self.assertIn("escalation", why)
        ok, _ = G.validate_outcome(o, "escalation", self.root)
        self.assertTrue(ok)


class TestDeclarationPunScan(unittest.TestCase):
    """Per-use address puns on splat symbols (2026-09-03): both func_80033550
    layer-1 FAILs were these, one session apart."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")
        self.cand = os.path.join(self.root, "memory", "grind", "func_X", "candidate.c")

    def tearDown(self):
        self.tmp.cleanup()

    def _write(self, body):
        with open(self.cand, "w", encoding="utf-8") as fh:
            fh.write(body)

    def test_flags_each_pun_shape(self):
        self._write("void f(s32 *p) {\n"
                    "    *(Word3 *)(((u8 *)(&D_80107850)) + i * 12) = *(Word3 *)p;\n"
                    "    if (*(&D_800A3918 + i) == 0) return;\n"
                    "    (&g_leaf_slot_state)[i] = 1;\n"
                    "}\n")
        hits = G.scan_declaration_puns(self.root, "func_X")
        self.assertEqual(len(hits), 3, hits)
        self.assertIn("[D_80107850]", hits[0])
        self.assertIn("[D_800A3918]", hits[1])
        self.assertIn("[g_leaf_slot_state]", hits[2])

    def test_clean_declaration_merge_is_silent(self):
        self._write("void f(LeafPos *p) {\n"
                    "    D_800A3918[i] = 1;\n"
                    "    D_80107850[i] = *p;\n"
                    "    q = &D_80107850[i];\n"
                    "}\n")
        self.assertEqual(G.scan_declaration_puns(self.root, "func_X"), [])

    def test_missing_candidate_is_silent(self):
        if os.path.exists(self.cand):
            os.remove(self.cand)
        self.assertEqual(G.scan_declaration_puns(self.root, "func_X"), [])


class TestReviewLoopBreaker(unittest.TestCase):
    """2026-09-04: body-keyed review verdicts close the layer-1/Judge loop."""
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1b")

    def tearDown(self):
        self.tmp.cleanup()

    def test_body_hash_ignores_comments_and_whitespace(self):
        a = "/* s14 header */ s32 func_X(s32 a) { return a + 1; }\nvoid f(void) { func_X(3); }"
        b = "s32   func_X (s32 a)\n{ // re-filed with new citations\n  return a+1; }"
        self.assertEqual(G.body_hash(a, "func_X"), G.body_hash(b, "func_X"))
        self.assertNotEqual(G.body_hash(a, "func_X"), G.body_hash(a.replace("+ 1", "+ 2"), "func_X"))

    def test_body_hash_keys_on_the_function_not_the_file(self):
        a = "s32 other(void) { return 0; } s32 func_X(s32 a) { return a; }"
        b = "s32 other(void) { return 99; } s32 func_X(s32 a) { return a; }"
        self.assertEqual(G.body_hash(a, "func_X"), G.body_hash(b, "func_X"))
        # a prototype / call is not the definition
        self.assertEqual(G.extract_function_body(G.normalize_c("s32 func_X(s32); s32 func_X(s32 a){return a;}"), "func_X"),
                         "func_X(s32 a){return a;}")
        self.assertIsNone(G.extract_function_body(G.normalize_c("s32 func_X(s32);"), "func_X"))
        self.assertTrue(G.body_hash("no such function here", "func_X"))  # falls back, never empty

    def test_disposition_timeline(self):
        h = "abcd"
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "fresh")
        G.record_review_verdict(self.root, "func_X", "layer1", "FAIL", h, "smells")
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "layer1-repeat")
        G.record_judge_clearance(self.root, "func_X", h, "decisions.md 2026-09-04 ruling PASS", "ordinary C")
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "judge-cleared")
        # a later layer-1 FAIL does not outrank the Judge
        G.record_review_verdict(self.root, "func_X", "layer1", "FAIL", h, "still smells")
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "judge-cleared")
        G.record_review_verdict(self.root, "func_X", "judge", "FAIL", h, "final call")
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "judge-failed")
        # a NEW ruling PASS re-clears it
        G.record_judge_clearance(self.root, "func_X", h, "later ruling", "ban was stale")
        self.assertEqual(G.review_disposition(self.root, "func_X", h), "judge-cleared")
        # other bodies are untouched
        self.assertEqual(G.review_disposition(self.root, "func_X", "ffff"), "fresh")

    def test_reviewer_notes_are_not_constraints(self):
        G.add_reviewer_note(self.root, "func_X", "LAYER-1 CHEAT-REVIEWER FAIL: x")
        G.add_reviewer_note(self.root, "func_X", "LAYER-1 CHEAT-REVIEWER FAIL: x")  # dedupe
        G.add_judge_constraint(self.root, "func_X", "keep the epilogue as one tree shape")
        G.add_judge_constraint(self.root, "func_X", "keep the epilogue as one tree shape")  # dedupe
        G.add_judge_constraint(self.root, "func_X", "LAYER-1 CHEAT-REVIEWER FAIL: legacy line")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(len(st["reviewer_history"]), 1)
        self.assertEqual(len(st["judge_constraints"]), 2)
        jc, notes = G.split_constraints(st)
        self.assertEqual(jc, ["keep the epilogue as one tree shape"])
        self.assertEqual(len(notes), 2)
        brief = G.build_brief(self.root, "func_X", "structural", "/tmp/o.json")
        self.assertIn("NOT precedent", brief)
        self.assertNotIn("  - LAYER-1 CHEAT-REVIEWER FAIL: legacy line\n\nRejected", brief)

    def test_review_context_block(self):
        G.record_judge_clearance(self.root, "func_X", "abcd", "ref1", "ordinary C, clear bans 3-4")
        G.record_review_verdict(self.root, "func_X", "layer1", "FAIL", "abcd", "laundered")
        blk = G.render_review_context(self.root, "func_X", "abcd")
        self.assertIn("Judge PASS rulings on record", blk)
        self.assertIn("Prior verdicts on THIS EXACT body", blk)
        self.assertIn("not precedent", blk)
        self.assertIn("no prior review verdict", G.render_review_context(self.root, "func_X", "zzzz"))


class TestSiblingLedgers(unittest.TestCase):
    """2026-09-04 post-mortem: CD_datasync sat 41 sessions at floor 7 while its
    sibling CD_sync (foreclosed, off the queue) held the fix at floor 2. The
    brief now surfaces sibling ledgers and a sibling floor drop forces ONE
    rederive session on every ledger that cites it."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "CD_datasync", "system")
        G.init_ledger(self.root, "CD_sync", "system")
        G.init_ledger(self.root, "main", "system")        # prose-word name
        G.init_ledger(self.root, "func_80011111", "text1a")  # unrelated
        os.makedirs(os.path.join(self.root, "docs", "grind"))
        os.makedirs(os.path.join(self.root, "engine"))
        json.dump({"items": [{"func": "CD_sync", "status": "foreclosed"},
                             {"func": "CD_datasync", "status": "active"}]},
                  open(os.path.join(self.root, "engine", "queue.json"), "w"))

    def tearDown(self):
        self.tmp.cleanup()

    def progress(self, func, n_floors, mod="structural"):
        for fl in n_floors:
            G.apply_outcome(self.root, func, {
                "result": "progress", "floor": fl, "headline": f"floor {fl}",
                "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                                "result": f"-> {fl}", "verdict": "KILLED"}],
                "evidence": [], "frontier": []}, mod)

    def journal(self, lines):
        with open(os.path.join(self.root, "docs", "grind", "journal.md"), "a",
                  encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")

    def test_outbound_mention_detects_sibling(self):
        st = G.load_state(self.root, "CD_datasync")
        st["session_count"] = 20
        G.save_state(self.root, "CD_datasync", st)
        G.append_hypothesis(self.root, "CD_datasync",
                            {"statement": "the twin CD_sync shares this window"}, session=16)
        sibs = G.sibling_ledgers(self.root, "CD_datasync")
        self.assertEqual([s["func"] for s in sibs], ["CD_sync"])
        self.assertEqual(sibs[0]["mentioned_at_session"], 16)
        self.assertEqual(sibs[0]["queue_status"], "foreclosed")

    def test_inbound_mention_detects_sibling(self):
        G.append_evidence(self.root, "CD_sync", "CD_datasync carries the same block", session=3)
        sibs = G.sibling_ledgers(self.root, "CD_datasync")
        self.assertEqual([s["func"] for s in sibs], ["CD_sync"])
        self.assertIsNone(sibs[0]["mentioned_at_session"])

    def test_prose_word_names_never_match(self):
        G.append_evidence(self.root, "CD_datasync", "the main lever is on main now")
        self.assertEqual(G.sibling_ledgers(self.root, "CD_datasync"), [])

    def test_old_name_alias_matches(self):
        st = G.load_state(self.root, "CD_sync")
        st["func"] = "cpu_side_move_dir_4"      # ledger created under the old name
        G.save_state(self.root, "CD_sync", st)
        G.append_evidence(self.root, "CD_datasync", "cpu_side_move_dir_4 has the fix")
        sibs = G.sibling_ledgers(self.root, "CD_datasync")
        self.assertEqual([s["func"] for s in sibs], ["CD_sync"])
        self.assertIn("cpu_side_move_dir_4", sibs[0]["names"])

    def test_floor_since_and_journal_dates(self):
        self.progress("CD_sync", [10, 7, 7, 2, 2])
        self.progress("CD_datasync", [9, 7])
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.journal(["- 2026-09-01 10:00 CD_datasync s1 [structural] floor=9: a",
                      "- 2026-09-02 10:00 CD_sync s4 [rederive] floor=2: b",
                      "- 2026-09-03 10:00 CD_datasync s2 [structural] floor=7: c"])
        s = G.sibling_ledgers(self.root, "CD_datasync")[0]
        self.assertEqual(s["floor"], 2)
        self.assertEqual(s["floor_since_session"], 4)
        self.assertEqual(s["floor_since_date"], "2026-09-02 10:00")
        self.assertEqual(s["mentioned_at_date"], "2026-09-01 10:00")
        self.assertTrue(s["unspent"])   # improved AFTER the ledger last read it

    def test_floor_since_dates_trailing_run_not_running_min(self):
        # WIP-imported shape: session-0 seed at the final floor, then higher, then back
        st = G.load_state(self.root, "CD_sync")
        st["floor_history"] = [{"session": 0, "floor": 2}, {"session": 1, "floor": 7},
                               {"session": 2, "floor": 2}, {"session": 3, "floor": 2}]
        st["session_count"] = 3
        G.save_state(self.root, "CD_sync", st)
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        s = G.sibling_ledgers(self.root, "CD_datasync")[0]
        self.assertEqual((s["floor"], s["floor_since_session"]), (2, 2))

    def test_inline_foreign_session_tag_is_not_a_mention_session(self):
        self.progress("CD_datasync", [9, 7])
        G.append_evidence(self.root, "CD_datasync",
                          "the prong CD_sync died on (CD_sync evidence [s68])", session=2)
        s = G.sibling_ledgers(self.root, "CD_datasync")[0]
        self.assertEqual(s["mentioned_at_session"], 2)

    def test_caller_above_our_floor_is_listed_not_mandated(self):
        self.progress("CD_datasync", [2])
        self.progress("func_80011111", [30])
        G.append_evidence(self.root, "func_80011111", "calls CD_datasync here", session=1)
        s = [x for x in G.sibling_ledgers(self.root, "CD_datasync") if x["func"] == "func_80011111"][0]
        self.assertIsNone(s["mentioned_at_session"])
        self.assertFalse(s["unspent"])

    def test_spent_when_mention_postdates_drop(self):
        self.progress("CD_sync", [10, 2])
        self.progress("CD_datasync", [9])
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.journal(["- 2026-09-01 10:00 CD_sync s2 [structural] floor=2: b",
                      "- 2026-09-03 10:00 CD_datasync s1 [structural] floor=9: c"])
        s = G.sibling_ledgers(self.root, "CD_datasync")[0]
        self.assertFalse(s["unspent"])

    def test_unspent_without_dates_when_sibling_is_lower(self):
        self.progress("CD_sync", [2])
        self.progress("CD_datasync", [7])
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        s = G.sibling_ledgers(self.root, "CD_datasync")[0]
        self.assertTrue(s["unspent"])

    def test_brief_carries_sibling_block(self):
        self.progress("CD_sync", [2])
        self.progress("CD_datasync", [7])
        with open(os.path.join(self.root, "memory", "grind", "CD_sync", "candidate.c"), "w") as f:
            f.write("s32 CD_sync(void) { return 0; }\n")
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        b = G.build_brief(self.root, "CD_datasync", "structural", "OUT.json")
        self.assertIn("SIBLING LEDGERS", b)
        self.assertIn("memory/grind/CD_sync/candidate.c", b)
        self.assertIn("UNSPENT", b)

    def test_brief_omits_block_without_siblings(self):
        b = G.build_brief(self.root, "func_80011111", "structural", "OUT.json")
        self.assertNotIn("SIBLING LEDGERS", b)

    def test_floor_drop_notifies_citing_siblings_once(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.progress("CD_datasync", [7])
        self.progress("CD_sync", [10, 10])            # first floor + flat: no notice
        self.assertNotIn("sibling_progress", G.load_state(self.root, "CD_datasync"))
        self.progress("CD_sync", [2])                 # drop: notice
        sp = G.load_state(self.root, "CD_datasync")["sibling_progress"]
        self.assertEqual(len(sp), 1)
        self.assertEqual((sp[0]["from"], sp[0]["floor"], sp[0]["session"]), ("CD_sync", 2, 3))
        self.assertIsNone(sp[0]["consumed"])
        self.progress("CD_sync", [1])                 # second drop replaces, no pile-up
        sp = G.load_state(self.root, "CD_datasync")["sibling_progress"]
        self.assertEqual([e["floor"] for e in sp], [1])
        # the unrelated ledger never hears about it
        self.assertNotIn("sibling_progress", G.load_state(self.root, "func_80011111"))

    def test_sibling_progress_forces_one_rederive_then_consumes(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.progress("CD_datasync", [7])
        self.progress("CD_sync", [9, 2])
        st = G.load_state(self.root, "CD_datasync")
        self.assertEqual(G.assign_modality(st["session_count"], st), "rederive")
        b = G.build_brief(self.root, "CD_datasync", "rederive", "OUT.json")
        self.assertIn("SIBLING PROGRESS SINCE YOUR LAST SESSION", b)
        self.progress("CD_datasync", [7])             # the rederive session ran
        st = G.load_state(self.root, "CD_datasync")
        self.assertEqual(st["sibling_progress"][0]["consumed"], 2)
        self.assertNotEqual(G.assign_modality(st["session_count"], st), "rederive")

    def test_sibling_progress_not_below_own_floor_is_advisory_only(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.progress("CD_datasync", [2])
        self.progress("CD_sync", [30, 12])
        st = G.load_state(self.root, "CD_datasync")
        self.assertNotEqual(G.assign_modality(st["session_count"], st), "rederive")

    def test_override_order_fixup_and_recon_win(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.progress("CD_sync", [9, 2])
        st = G.load_state(self.root, "CD_datasync")   # session_count 0 -> recon first
        self.assertEqual(G.assign_modality(0, st), "recon")
        self.progress("CD_datasync", [7])
        G.set_pending_fixup(self.root, "CD_datasync", "annotation", "fix comment")
        st = G.load_state(self.root, "CD_datasync")
        self.assertEqual(G.assign_modality(st["session_count"], st), "annotation-fix")

    def test_sibling_progress_beats_exhaustion(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        # flat at 3 (<= ENDGAME_LOCK_MAX_FLOOR) across 4 modalities => exhaustion
        mods = ["structural", "permuter", "synthesis", "forensics"] * 3
        for m in mods[:G.ESCALATION_FLAT_SESSIONS]:
            self.progress("CD_datasync", [3], mod=m)
        st = G.load_state(self.root, "CD_datasync")
        st["object_model_audited"] = 1
        self.assertEqual(G.assign_modality(st["session_count"], st), "escalation")
        self.progress("CD_sync", [9, 2])
        st = G.load_state(self.root, "CD_datasync")
        st["object_model_audited"] = 1
        self.assertEqual(G.assign_modality(st["session_count"], st), "rederive")

    def test_apply_survives_broken_sibling_ledger(self):
        G.append_hypothesis(self.root, "CD_datasync", {"statement": "CD_sync twin"}, session=1)
        self.progress("CD_datasync", [7])
        with open(os.path.join(self.root, "memory", "grind", "CD_datasync", "state.json"),
                  "w") as f:
            f.write("{not json")
        self.progress("CD_sync", [9, 2])              # must not raise
        self.assertEqual(G.load_state(self.root, "CD_sync")["floor_history"][-1]["floor"], 2)


if __name__ == "__main__":
    unittest.main()
