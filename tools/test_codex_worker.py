#!/usr/bin/env python3
"""Regression tests for tools/codex_worker.py's layer-2 recording and land gate, on a throwaway repo
(never the real tree; the engine call is mocked, no Codex run, nothing pushed).

  python tools/test_codex_worker.py
"""
from __future__ import annotations

import datetime as dt
import hashlib
import io
import json
import os
import shutil
import stat
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from types import SimpleNamespace
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import codex_worker as cw  # noqa: E402

REPO_ROOT = Path(__file__).resolve().parent.parent
REAL_L2MOD = cw.layer2_module

REAL_RECORD = cw.engine_layer2_record   # Repo tests mock it

FUNC = "comb_ExchangePad"
H1, H2 = "799ab313f506448f", "0123456789abcdef"


def _rm_readonly(fn, path, _exc):
    os.chmod(path, stat.S_IWRITE)
    fn(path)


def rec(verdict, h=H1, reviewer="r1"):
    return json.dumps({"func": FUNC, "addr": "0x80028708", "verdict": verdict, "body_hash": h,
                       "reviewer": reviewer}) + "\n"


class LedgerProblems(unittest.TestCase):
    def test_reviewable_latest_pass_ok(self):
        self.assertEqual(cw.ledger_problems({FUNC: (b"", rec("PASS").encode())}, True), [])

    def test_reviewable_fail_after_pass_refused(self):
        new = (rec("PASS") + rec("FAIL", reviewer="r2")).encode()
        p = cw.ledger_problems({FUNC: (b"", new)}, True, {FUNC})
        self.assertEqual(len(p), 1)
        self.assertIn("latest layer-2 record is FAIL", p[0])

    def test_unlanded_fail_on_unchanged_function_says_land_it_first(self):
        p = cw.ledger_problems({FUNC: (b"", rec("FAIL").encode())}, True, set())
        self.assertTrue(p and "land that FAIL first" in p[0], p)

    def test_reviewable_fail_then_new_pass_ok(self):
        old = (rec("PASS") + rec("FAIL")).encode()
        self.assertEqual(cw.ledger_problems({FUNC: (old, old + rec("PASS", H2).encode())}, True), [])

    def test_reviewable_needs_user_refused(self):
        self.assertTrue(cw.ledger_problems({FUNC: (b"", rec("NEEDS_USER").encode())}, True, {FUNC}))

    def test_ledger_only_fail_ok(self):
        old = rec("PASS").encode()
        self.assertEqual(cw.ledger_problems({FUNC: (old, old + rec("FAIL").encode())}, False), [])

    def test_ledger_only_pass_refused(self):
        p = cw.ledger_problems({FUNC: (b"", rec("PASS").encode())}, False)
        self.assertTrue(p and "only FAIL / NEEDS_USER" in p[0])

    def test_not_append_only_refused(self):
        old = (rec("PASS") + rec("FAIL")).encode()
        p = cw.ledger_problems({FUNC: (old, rec("PASS").encode())}, True)
        self.assertTrue(p and "append-only" in p[0])

    def test_empty_commit_refused(self):
        self.assertTrue(cw.ledger_problems({}, False))
        self.assertEqual(cw.ledger_problems({}, True), [])


class CopyProblems(unittest.TestCase):
    BLOB = b'{"decision": "FAIL"}'
    SHA = hashlib.sha1(BLOB).hexdigest()
    PATH = f"memory/grind/{FUNC}/layer2_verdicts/{SHA}.json"

    def changes(self, vf):
        line = json.dumps({"func": FUNC, "verdict": "FAIL", "body_hash": H1, "verdict_file": vf}) + "\n"
        return {FUNC: (b"", line.encode())}

    def test_new_referenced_copy_ok(self):
        self.assertEqual(cw.copy_problems({self.PATH: ("A", self.BLOB)}, self.changes(self.PATH)), [])

    def test_modified_or_deleted_copy_refused(self):
        for st in ("M", "D"):
            p = cw.copy_problems({self.PATH: (st, self.BLOB)}, self.changes(self.PATH))
            self.assertTrue(p and "write-once" in p[0], p)

    def test_wrong_content_refused(self):
        p = cw.copy_problems({self.PATH: ("A", b"forged")}, self.changes(self.PATH))
        self.assertTrue(p and "SHA1" in p[0], p)

    def test_unreferenced_copy_refused(self):
        p = cw.copy_problems({self.PATH: ("A", self.BLOB)}, self.changes("elsewhere.json"))
        self.assertTrue(p and "no record" in p[0], p)


class PassAfterVeto(unittest.TestCase):
    FAIL_AT = {"verdict": "FAIL", "date": "2026-10-22T12:00:00Z", "reviewer": "r2"}

    def refused(self, records, reviews, sha="b" * 40, mine=None):
        mine = [len(reviews) - 1] if mine is None else mine
        with redirect_stderr(io.StringIO()) as err:
            try:
                cw.pass_after_last_veto(records, reviews, mine, sha)
            except SystemExit as e:
                return e.code, err.getvalue()
        return 0, ""

    def test_review_older_than_fail_refused(self):
        code, err = self.refused([{"verdict": "PASS"}, self.FAIL_AT], [{"ts": "2026-10-22T06:59:00-05:00"}])
        self.assertEqual(code, 2)
        self.assertIn("fresh cheat-reviewer", err)

    def test_review_newer_than_fail_ok(self):
        self.assertEqual(self.refused([self.FAIL_AT], [{"ts": "2026-10-22T07:00:01-05:00"}])[0], 0)

    def test_same_verdict_json_refused(self):
        code, err = self.refused([{"verdict": "PASS", "verdict_sha1": "a" * 40}], [], sha="a" * 40, mine=[])
        self.assertEqual(code, 2)
        self.assertIn("already recorded", err)

    def test_veto_recorded_here_is_ordered_by_position_not_clock(self):
        veto = dict(self.FAIL_AT, verdict_sha1="c" * 40)
        here = {"verdict": "FAIL", "verdict_sha1": "c" * 40, "ts": "2026-10-22T07:00:00-05:00"}
        future = {"verdict": "PASS", "ts": "2030-01-01T00:00:00+00:00"}
        past = {"verdict": "PASS", "ts": "2000-01-01T00:00:00+00:00"}
        self.assertEqual(self.refused([veto], [future, here], mine=[0])[0], 2)   # before it, clock says after
        self.assertEqual(self.refused([veto], [here, past], mine=[1])[0], 0)     # after it, clock says before


class LoadVerdict(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="cw-test-"))

    def tearDown(self):
        shutil.rmtree(self.tmp, onerror=_rm_readonly)  # the test's own temp dir

    def vf(self, obj, prose=False):
        f = self.tmp / "v.json"
        text = json.dumps(obj)
        f.write_text(f"Verdict follows.\n{text}\nend" if prose else text, encoding="utf-8")
        return f

    def test_every_decision_accepted(self):
        for d in cw.VERDICTS:
            v = cw.load_verdict(self.vf({"decision": d, "function": FUNC, "body_hash": H1}, prose=True), FUNC)
            self.assertEqual(v["decision"], d)

    def test_refusals(self):
        for bad in ({"decision": "MAYBE", "function": FUNC, "body_hash": H1},
                    {"decision": "FAIL", "function": "other", "body_hash": H1},
                    {"decision": "FAIL", "function": FUNC, "body_hash": "xyz"}):
            with self.assertRaises(SystemExit) as e, redirect_stderr(io.StringIO()):
                cw.load_verdict(self.vf(bad), FUNC)
            self.assertEqual(e.exception.code, 2)


class Repo(unittest.TestCase):
    """main == trusted == one throwaway repo: branch `main` plus the item branch checked out."""
    ITEM = "fake-test"

    def g(self, *args):
        return subprocess.run(["git", *args], cwd=self.repo, check=True, capture_output=True, text=True,
                              env=cw.clean_env()).stdout.strip()

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="cw-test-"))
        self.repo = self.tmp / "repo"
        self.repo.mkdir()
        self.hooks = self.tmp / "hooks"
        self.hooks.mkdir()
        self.g("init", "-q", "-b", "main")
        for k, v in (("user.name", "t"), ("user.email", "t@example.invalid"), ("commit.gpgsign", "false"),
                     ("core.hooksPath", str(self.hooks)), ("core.autocrlf", "false")):
            self.g("config", k, v)
        (self.repo / ".gitignore").write_text("tmp/\n", encoding="utf-8", newline="\n")
        (self.repo / "src").mkdir()
        (self.repo / "src" / "a.c").write_text(f"int f(void) {{ return 0; }}\nvoid {FUNC}(void) {{ g(0); }}\n",
                                               encoding="utf-8", newline="\n")
        (self.repo / "engine").mkdir()
        (self.repo / "engine" / "queue.json").write_text(json.dumps({"items": [{"func": FUNC}]}),
                                                         encoding="utf-8", newline="\n")
        (self.repo / "src" / "b.c").write_text('INCLUDE_ASM("asm/funcs", asm_func);\n', encoding="utf-8",
                                               newline="\n")
        (self.repo / "asm" / "funcs").mkdir(parents=True)
        (self.repo / "asm" / "funcs" / "asm_func.s").write_text("glabel asm_func\n nop\n", encoding="utf-8",
                                                                newline="\n")
        (self.repo / "memory" / "grind" / "asm_func").mkdir(parents=True)
        (self.repo / "memory" / "grind" / "asm_func" / "layer2.jsonl").write_text(rec("PASS"), encoding="utf-8",
                                                                                  newline="\n")
        led = self.repo / "memory" / "grind" / FUNC
        led.mkdir(parents=True)
        (led / "layer2.jsonl").write_text(rec("PASS"), encoding="utf-8", newline="\n")
        self.g("add", "-A")
        self.g("commit", "-q", "-m", "base")
        self.branch = f"codex/{self.ITEM}"
        self.g("checkout", "-q", "-b", self.branch)
        self.reviews = self.tmp / "reviews.jsonl"
        self.engine_calls = []
        patches = [
            mock.patch.object(cw, "layout", lambda item: (self.repo, self.tmp / "scratch", self.repo, self.branch)),
            mock.patch.object(cw, "verify_trusted", lambda *a, **k: None),
            mock.patch.object(cw, "codex_idle", lambda *a, **k: None),
            mock.patch.object(cw, "scratch_committed", lambda *a, **k: None),
            mock.patch.object(cw, "recreate_scratch", lambda *a, **k: None),
            mock.patch.object(cw, "pin_file", lambda main: self.tmp / "no-pin.json"),
            mock.patch.object(cw, "reviews_file", lambda main, item: self.reviews),
            mock.patch.object(cw, "engine_layer2_record", self.fake_record),
            mock.patch.object(cw, "layer2_module", lambda main: REAL_L2MOD(REPO_ROOT)),
        ]
        for p in patches:
            p.start()
            self.addCleanup(p.stop)

    def tearDown(self):
        shutil.rmtree(self.tmp, onerror=_rm_readonly)  # the test's own temp repo

    def fake_record(self, main, trusted, func, rel, verdict, reviewer, scope, notes):
        """What `engine.cli layer2 record --verdict-file` writes: one ledger line + the verdict copy."""
        self.engine_calls.append((func, verdict["decision"], verdict["body_hash"], reviewer, scope, notes))
        raw = (trusted / rel).read_bytes()
        sha = hashlib.sha1(raw).hexdigest()
        d = trusted / "memory" / "grind" / func
        (d / "layer2_verdicts").mkdir(parents=True, exist_ok=True)
        (d / "layer2_verdicts" / f"{sha}.json").write_bytes(raw)
        line = json.loads(rec(verdict["decision"], verdict["body_hash"], reviewer))
        line.update(date=dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"), verdict_sha1=sha,
                    verdict_file=f"memory/grind/{func}/layer2_verdicts/{sha}.json")
        with open(d / "layer2.jsonl", "a", encoding="utf-8", newline="\n") as fh:
            fh.write(json.dumps(line) + "\n")

    def verdict(self, decision, h=H1, name="v.json"):
        f = self.tmp / name
        f.write_text(json.dumps({"decision": decision, "function": FUNC, "body_hash": h,
                                 "summary": name}), encoding="utf-8")
        return f

    def layer2(self, decision, reviewer="r2", msg=None, h=H1, notes="", name="v.json"):
        a = SimpleNamespace(item=self.ITEM, func=FUNC, verdict_file=str(self.verdict(decision, h, name)),
                            reviewer=reviewer, scope="cheat-cleanup", notes=notes,
                            message_file=str(msg) if msg else "")
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()) as err:
            try:
                cw.cmd_layer2(a)
            except SystemExit as e:
                return e.code, err.getvalue()
        return 0, err.getvalue()

    def code_commit(self, body=True):
        """Change f and, with `body`, FUNC's body (which has a ledger)."""
        (self.repo / "src" / "a.c").write_text(
            f"int f(void) {{ return 1; }}\nvoid {FUNC}(void) {{ g({int(body)}); }}\n", encoding="utf-8",
            newline="\n")
        self.g("commit", "-q", "-am", "code")

    def review_pass(self, agent, later=0):
        key = cw.diff_key(self.repo, self.g("rev-parse", self.branch))
        ts = (dt.datetime.now().astimezone() + dt.timedelta(seconds=later)).isoformat(timespec="seconds")
        with self.reviews.open("a", encoding="utf-8") as fh:
            fh.write(json.dumps({"ts": ts, "kind": "cheat-reviewer", "verdict": "PASS", "key": key,
                                 "agent": agent}) + "\n")

    def msg(self):
        m = self.tmp / "msg.txt"
        m.write_text(f"cheat-cleanup: record {FUNC}'s layer-2 FAIL\n", encoding="utf-8")
        return m


class Layer2Record(Repo):
    def test_fail_without_item_commit_makes_ledger_only_commit(self):
        code, err = self.layer2("FAIL", msg=self.msg())
        self.assertEqual(code, 0, err)
        self.assertEqual(self.engine_calls, [(FUNC, "FAIL", H1, "r2", "cheat-cleanup", "")])
        tip = self.g("rev-parse", "HEAD")
        files = self.g("diff", "--name-only", f"{tip}^", tip).splitlines()
        self.assertIn(f"memory/grind/{FUNC}/layer2.jsonl", files)
        self.assertTrue(all(cw.LEDGER.match(f) for f in files), files)
        self.assertEqual(self.g("rev-list", "--count", "main..HEAD"), "1")
        self.assertEqual(cw.required_kinds(self.repo, tip)[0], [])
        changes = cw.ledger_changes(self.repo, tip)
        self.assertEqual(json.loads(changes[FUNC][1].splitlines()[-1])["verdict"], "FAIL")
        self.assertEqual(cw.ledger_problems(changes, False), [])

    def test_needs_user_without_item_commit_ok(self):
        self.assertEqual(self.layer2("NEEDS_USER", msg=self.msg())[0], 0)

    def test_fail_without_item_commit_needs_message(self):
        code, err = self.layer2("FAIL")
        self.assertEqual(code, 2)
        self.assertIn("--message-file", err)
        self.assertEqual(self.engine_calls, [])

    def test_pass_without_item_commit_refused(self):
        code, err = self.layer2("PASS", msg=self.msg())
        self.assertEqual(code, 2)
        self.assertEqual(self.engine_calls, [])

    def test_pass_needs_review_record(self):
        self.code_commit()
        code, err = self.layer2("PASS", reviewer="r1")
        self.assertEqual(code, 2)
        self.assertIn("no cheat-reviewer PASS by r1", err)

    def test_unsafe_notes_refused(self):
        code, err = self.layer2("FAIL", msg=self.msg(), notes="x; rm $HOME")
        self.assertEqual(code, 2)
        self.assertEqual(self.engine_calls, [])

    def test_later_fail_blocks_code_and_keeps_key(self):
        self.code_commit()
        self.review_pass("r1")
        key = cw.diff_key(self.repo, self.g("rev-parse", "HEAD"))
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertEqual(cw.ledger_problems(cw.ledger_changes(self.repo, tip), True), [])
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertEqual(self.g("rev-list", "--count", "main..HEAD"), "1")     # amended, not stacked
        self.assertEqual(cw.diff_key(self.repo, tip), key)
        self.assertEqual(cw.changed_layer2_bodies(self.repo, tip), {FUNC})
        p = cw.ledger_problems(cw.ledger_changes(self.repo, tip), True, {FUNC})
        self.assertTrue(p and "latest layer-2 record is FAIL" in p[0], p)

    def test_fail_on_code_commit_blocks_its_key(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        p = cw.review_gate(self.repo, self.ITEM, tip)[2]
        self.assertTrue(any("FAIL recorded on this exact diff" in x and "r2" in x for x in p), p)

    def test_old_pass_cannot_be_rerecorded_over_a_fail(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        code, err = self.layer2("PASS", reviewer="r1", h=H2)            # the same verdict JSON
        self.assertEqual(code, 2)
        self.assertIn("already recorded", err)
        code, err = self.layer2("PASS", reviewer="r1", h=H2, name="v3.json")   # a new JSON, old review
        self.assertEqual(code, 2)
        self.assertIn("fresh cheat-reviewer", err)

    def test_fail_folded_on_unchanged_function_refused(self):
        self.code_commit(body=False)
        self.review_pass("r1")
        code, err = self.layer2("FAIL", reviewer="r2")
        self.assertEqual(code, 2)
        self.assertIn("does not change", err)
        self.assertEqual(self.engine_calls, [])

    def test_fail_uncompleting_a_finished_function_refused(self):
        (self.repo / "engine" / "queue.json").write_text('{"items": []}', encoding="utf-8", newline="\n")
        self.g("commit", "-q", "-am", "queue empty")
        self.g("branch", "-f", "main", "HEAD")
        code, err = self.layer2("FAIL", msg=self.msg())                         # the PASS'd body H1
        self.assertEqual(code, 2)
        self.assertIn("queue reopen", err)
        self.assertEqual(self.layer2("FAIL", msg=self.msg(), h=H2)[0], 0)      # another body: no completion

    def test_engine_command(self):
        calls = []
        with mock.patch.object(cw, "run_checked", lambda cmd, what, **k: calls.append(cmd)):
            REAL_RECORD(Path("M"), Path("T"), FUNC, "tmp/codex/v.json",
                        {"decision": "FAIL", "body_hash": H1}, "r2", "cheat-cleanup", "two words")
        cmd = calls[0]
        i = cmd.index("record")
        self.assertEqual(cmd[i - 1:], ["layer2", "record", FUNC, "--verdict-file", "tmp/codex/v.json",
                                       "--verdict", "FAIL", "--expect-hash", H1, "--reviewer", "r2",
                                       "--scope", "cheat-cleanup", "--notes", "two words"])


class LandGate(Repo):
    """cmd_land up to its build: the review + ledger gates (exit 9) and the engine `layer2 check`."""

    def land(self):
        checks = []
        a = SimpleNamespace(item=self.ITEM, dry_run=True, no_push=True)
        with mock.patch.dict(os.environ, {"CLAUDE_CODE_SESSION_ID": "test"}), \
                mock.patch.object(cw, "verify_tip", lambda *a, **k: None), \
                mock.patch.object(cw, "format_tree", lambda main, item, tree, paths: tree), \
                mock.patch.object(cw, "engine_layer2_check", lambda main, trusted, func: checks.append(func)), \
                redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()):
            try:
                cw.cmd_land(a)
            except SystemExit as e:
                return e.code, checks, out.getvalue()
        return 0, checks, out.getvalue()

    def test_ledger_only_fail_lands(self):
        self.assertEqual(self.layer2("FAIL", msg=self.msg())[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertIn("DRY RUN", out)
        self.assertEqual(checks, [])

    def test_code_with_pass_lands_after_engine_check(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [FUNC])

    def test_body_change_without_layer2_refused(self):
        self.code_commit()
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn(f"{FUNC}: this commit changes its body", out)
        self.assertEqual(checks, [])

    def test_change_to_unreviewed_function_lands(self):
        self.code_commit(body=False)               # only f, which has no ledger
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [])

    def test_asm_only_body_change_refused(self):
        (self.repo / "asm" / "funcs" / "asm_func.s").write_text("glabel asm_func\n jr $ra\n", encoding="utf-8",
                                                                newline="\n")
        self.g("commit", "-q", "-am", "asm")
        self.assertEqual(cw.changed_layer2_bodies(self.repo, self.g("rev-parse", "HEAD")), {"asm_func"})
        self.review_pass("r1")
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn("asm_func: this commit changes its body", out)

    def test_needs_user_then_owner_ruling_and_fresh_pass_lands(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("NEEDS_USER", reviewer="r2", h=H2)[0], 0)
        tip = self.g("rev-parse", "HEAD")
        self.assertTrue(cw.review_gate(self.repo, self.ITEM, tip)[2])          # blocked until a ruling
        self.review_pass("r3", later=-3600)                                     # clock skew: position wins
        self.assertEqual(cw.review_gate(self.repo, self.ITEM, tip)[2], [])
        self.assertEqual(self.layer2("PASS", reviewer="r3", h=H2, name="v3.json")[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 0, out)
        self.assertEqual(checks, [FUNC])

    def test_code_with_later_fail_refused(self):
        self.code_commit()
        self.review_pass("r1")
        self.assertEqual(self.layer2("PASS", reviewer="r1", h=H2)[0], 0)
        self.assertEqual(self.layer2("FAIL", reviewer="r2", h=H2, name="v2.json")[0], 0)
        code, checks, out = self.land()
        self.assertEqual(code, 9, out)
        self.assertIn("latest layer-2 record is FAIL", out)
        self.assertEqual(checks, [])


if __name__ == "__main__":
    unittest.main()
